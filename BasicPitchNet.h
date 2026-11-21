#pragma once
// Minimal inference engine for the Basic Pitch network (Spotify, Apache License 2.0).
// The network graph was compiled offline into a flat program with static shapes (see BasicPitchModel.h),
// so only a handful of float operators are needed and no external runtime is required.
// Pure C++17, no JUCE dependency. run() is const and thread-safe.

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <memory>
#include <vector>

namespace a2m
{
namespace bp
{
inline constexpr int kWindowSamples = 43844;     // network input: 2 s at 22050 Hz
inline constexpr int kFramesPerWindow = 172;     // network output frames (hop 256 samples)
inline constexpr int kPitches = 88;              // A0 .. C8

class Net
{
public:
    bool load (const unsigned char* data, size_t size)
    {
        Reader r { data, size };
        if (size < 8 || std::memcmp (data, "BPN1", 4) != 0)
            return false;
        r.pos = 4;

        const auto nConst = r.u32();
        consts.resize (nConst);
        for (auto& c : consts)
        {
            const auto rank = r.u32();
            c.shape.resize (rank);
            size_t n = 1;
            for (auto& d : c.shape) { d = r.i32(); n *= (size_t) d; }
            c.data.resize (n);
            r.bytes (c.data.data(), n * 4);
        }

        const auto nT = r.u32();
        tensorShapes.resize (nT);
        for (auto& sh : tensorShapes)
        {
            const auto rank = r.u32();
            sh.resize (rank);
            for (auto& d : sh) d = r.i32();
        }

        const auto nN = r.u32();
        nodes.resize (nN);
        for (auto& n : nodes)
        {
            uint8_t head[4];
            r.bytes (head, 4);
            n.op = head[0];
            n.in.resize (head[1]);
            for (auto& v : n.in) v = r.i32();
            n.out = r.i32();
            n.ints.resize (r.u32());
            for (auto& v : n.ints) v = r.i32();
            n.floats.resize (r.u32());
            for (auto& v : n.floats) r.bytes (&v, 4);
        }

        const auto nO = r.u32();
        outputs.resize (nO);
        for (auto& v : outputs) v = r.i32();

        if (r.failed || outputs.size() != 2)
            return false;

        // last use of every tensor, so intermediate results can be released early
        lastUse.assign (tensorShapes.size(), -1);
        for (size_t i = 0; i < nodes.size(); ++i)
            for (int in : nodes[i].in)
                if (in >= 0)
                    lastUse[(size_t) in] = (int) i;
        for (int o : outputs)
            lastUse[(size_t) o] = (int) nodes.size();       // keep

        loaded = true;
        return true;
    }

    bool isLoaded() const { return loaded; }

    // in: kWindowSamples audio samples. note / onset: kFramesPerWindow * kPitches values each.
    void run (const float* in, float* note, float* onset) const
    {
        std::vector<Tensor> t (tensorShapes.size());
        t[0].shape = tensorShapes[0];
        t[0].data.assign (in, in + kWindowSamples);

        for (size_t i = 0; i < nodes.size(); ++i)
        {
            const auto& n = nodes[i];
            std::vector<const Tensor*> src;
            for (int in_ : n.in)
                src.push_back (in_ >= 0 ? &t[(size_t) in_] : &consts[(size_t) (-in_ - 1)]);

            Tensor out;
            out.shape = tensorShapes[(size_t) n.out];
            exec (n, src, out);
            t[(size_t) n.out] = std::move (out);

            for (int in_ : n.in)
                if (in_ >= 0 && lastUse[(size_t) in_] == (int) i)
                    std::vector<float>().swap (t[(size_t) in_].data);
        }

        std::memcpy (note,  t[(size_t) outputs[0]].data.data(), sizeof (float) * kFramesPerWindow * kPitches);
        std::memcpy (onset, t[(size_t) outputs[1]].data.data(), sizeof (float) * kFramesPerWindow * kPitches);
    }

private:
    struct Tensor
    {
        std::vector<int> shape;
        std::vector<float> data;
    };

    struct Node
    {
        int op = 0, out = 0;
        std::vector<int> in, ints;
        std::vector<float> floats;
    };

    struct Reader
    {
        const unsigned char* d;
        size_t size, pos = 0;
        bool failed = false;
        void bytes (void* dst, size_t n)
        {
            if (pos + n > size) { failed = true; std::memset (dst, 0, n); return; }
            std::memcpy (dst, d + pos, n);
            pos += n;
        }
        uint32_t u32() { uint32_t v; bytes (&v, 4); return v; }
        int32_t  i32() { int32_t v;  bytes (&v, 4); return v; }
    };

    enum Op { Reshape = 0, Slice, Pad, Conv, Neg, Transpose, Concat, Mul, Add, Sub, Div,
              ReduceSum, ReduceMin, ReduceMax, Sqrt, Log, Relu, Sigmoid, Equal, Where };

    static size_t count (const std::vector<int>& s)
    {
        size_t n = 1;
        for (int d : s) n *= (size_t) d;
        return n;
    }

    static std::vector<size_t> stridesOf (const std::vector<int>& s)
    {
        std::vector<size_t> st (s.size(), 1);
        for (int i = (int) s.size() - 2; i >= 0; --i)
            st[(size_t) i] = st[(size_t) i + 1] * (size_t) s[(size_t) i + 1];
        return st;
    }

    // For every element of an output of shape outShape, the matching element of a broadcast input
    static std::vector<uint32_t> broadcastMap (const std::vector<int>& outShape, const std::vector<int>& inShape)
    {
        const size_t rank = outShape.size(), off = rank - inShape.size();
        std::vector<size_t> inSt (rank, 0);
        const auto st = stridesOf (inShape);
        for (size_t d = 0; d < inShape.size(); ++d)
            inSt[off + d] = inShape[d] == 1 ? 0 : st[d];

        const size_t total = count (outShape);
        std::vector<uint32_t> map (total);
        std::vector<int> idx (rank, 0);
        size_t cur = 0;
        for (size_t i = 0; i < total; ++i)
        {
            map[i] = (uint32_t) cur;
            for (int d = (int) rank - 1; d >= 0; --d)
            {
                cur += inSt[(size_t) d];
                if (++idx[(size_t) d] < outShape[(size_t) d])
                    break;
                cur -= inSt[(size_t) d] * (size_t) outShape[(size_t) d];
                idx[(size_t) d] = 0;
            }
        }
        return map;
    }

    template <typename F>
    static void binary (const Tensor& a, const Tensor& b, Tensor& out, F f)
    {
        const size_t total = count (out.shape);
        out.data.resize (total);
        if (a.shape == out.shape && b.shape == out.shape)
        {
            for (size_t i = 0; i < total; ++i) out.data[i] = f (a.data[i], b.data[i]);
        }
        else if (b.data.size() == 1 && a.shape == out.shape)
        {
            const float v = b.data[0];
            for (size_t i = 0; i < total; ++i) out.data[i] = f (a.data[i], v);
        }
        else if (a.data.size() == 1 && b.shape == out.shape)
        {
            const float v = a.data[0];
            for (size_t i = 0; i < total; ++i) out.data[i] = f (v, b.data[i]);
        }
        else
        {
            const auto ma = broadcastMap (out.shape, a.shape), mb = broadcastMap (out.shape, b.shape);
            for (size_t i = 0; i < total; ++i) out.data[i] = f (a.data[ma[i]], b.data[mb[i]]);
        }
    }

    template <typename F>
    static void unary (const Tensor& a, Tensor& out, F f)
    {
        out.data.resize (a.data.size());
        for (size_t i = 0; i < a.data.size(); ++i) out.data[i] = f (a.data[i]);
    }

    void exec (const Node& n, const std::vector<const Tensor*>& in, Tensor& out) const
    {
        switch (n.op)
        {
            case Reshape:   out.data = in[0]->data; break;
            case Neg:       unary (*in[0], out, [] (float x) { return -x; }); break;
            case Sqrt:      unary (*in[0], out, [] (float x) { return std::sqrt (x); }); break;
            case Log:       unary (*in[0], out, [] (float x) { return std::log (x); }); break;
            case Relu:      unary (*in[0], out, [] (float x) { return x > 0.0f ? x : 0.0f; }); break;
            case Sigmoid:   unary (*in[0], out, [] (float x) { return 1.0f / (1.0f + std::exp (-x)); }); break;
            case Mul:       binary (*in[0], *in[1], out, [] (float a, float b) { return a * b; }); break;
            case Add:       binary (*in[0], *in[1], out, [] (float a, float b) { return a + b; }); break;
            case Sub:       binary (*in[0], *in[1], out, [] (float a, float b) { return a - b; }); break;
            case Div:       binary (*in[0], *in[1], out, [] (float a, float b) { return a / b; }); break;
            case Equal:     binary (*in[0], *in[1], out, [] (float a, float b) { return a == b ? 1.0f : 0.0f; }); break;
            case Where:     where (in, out); break;
            case Transpose: transpose (n, *in[0], out); break;
            case Concat:    concat (n, in, out); break;
            case Slice:     slice (n, *in[0], out); break;
            case Pad:       pad (n, *in[0], out); break;
            case ReduceSum: case ReduceMin: case ReduceMax: reduce (n, *in[0], out); break;
            case Conv:      conv (n, *in[0], *in[1], in.size() > 2 ? in[2] : nullptr, out); break;
            default: break;
        }
    }

    static void where (const std::vector<const Tensor*>& in, Tensor& out)
    {
        const size_t total = count (out.shape);
        out.data.resize (total);
        const auto mc = broadcastMap (out.shape, in[0]->shape);
        const auto mx = broadcastMap (out.shape, in[1]->shape);
        const auto my = broadcastMap (out.shape, in[2]->shape);
        for (size_t i = 0; i < total; ++i)
            out.data[i] = in[0]->data[mc[i]] != 0.0f ? in[1]->data[mx[i]] : in[2]->data[my[i]];
    }

    static void transpose (const Node& n, const Tensor& a, Tensor& out)
    {
        const size_t rank = a.shape.size();
        const auto st = stridesOf (a.shape);
        std::vector<size_t> pst (rank);
        for (size_t d = 0; d < rank; ++d) pst[d] = st[(size_t) n.ints[d]];

        const size_t total = count (out.shape);
        out.data.resize (total);
        std::vector<int> idx (rank, 0);
        size_t cur = 0;
        for (size_t i = 0; i < total; ++i)
        {
            out.data[i] = a.data[cur];
            for (int d = (int) rank - 1; d >= 0; --d)
            {
                cur += pst[(size_t) d];
                if (++idx[(size_t) d] < out.shape[(size_t) d])
                    break;
                cur -= pst[(size_t) d] * (size_t) out.shape[(size_t) d];
                idx[(size_t) d] = 0;
            }
        }
    }

    static void concat (const Node& n, const std::vector<const Tensor*>& in, Tensor& out)
    {
        const int axis = n.ints[0];
        size_t outer = 1, inner = 1;
        for (int d = 0; d < axis; ++d) outer *= (size_t) out.shape[(size_t) d];
        for (size_t d = (size_t) axis + 1; d < out.shape.size(); ++d) inner *= (size_t) out.shape[d];

        out.data.resize (count (out.shape));
        size_t offset = 0;
        const size_t outAxis = (size_t) out.shape[(size_t) axis];
        for (const auto* t : in)
        {
            const size_t len = (size_t) t->shape[(size_t) axis];
            for (size_t o = 0; o < outer; ++o)
                std::memcpy (out.data.data() + (o * outAxis + offset) * inner,
                             t->data.data() + o * len * inner, len * inner * sizeof (float));
            offset += len;
        }
    }

    static void slice (const Node& n, const Tensor& a, Tensor& out)
    {
        const size_t rank = a.shape.size();
        std::vector<int> start (rank, 0), step (rank, 1);
        const int na = n.ints[0];
        for (int k = 0; k < na; ++k)
        {
            const size_t ax = (size_t) n.ints[(size_t) (1 + 4 * k)];
            start[ax] = n.ints[(size_t) (2 + 4 * k)];
            step[ax]  = n.ints[(size_t) (4 + 4 * k)];
        }

        const auto st = stridesOf (a.shape);
        const size_t total = count (out.shape);
        out.data.resize (total);
        std::vector<int> idx (rank, 0);
        for (size_t i = 0; i < total; ++i)
        {
            size_t src = 0;
            for (size_t d = 0; d < rank; ++d)
                src += (size_t) (start[d] + idx[d] * step[d]) * st[d];
            out.data[i] = a.data[src];

            for (int d = (int) rank - 1; d >= 0; --d)
                if (++idx[(size_t) d] < out.shape[(size_t) d]) break; else idx[(size_t) d] = 0;
        }
    }

    static void pad (const Node& n, const Tensor& a, Tensor& out)
    {
        const int mode = n.ints[0];
        const size_t rank = a.shape.size();
        const auto st = stridesOf (a.shape);
        const size_t total = count (out.shape);
        out.data.resize (total);
        std::vector<int> idx (rank, 0);
        for (size_t i = 0; i < total; ++i)
        {
            size_t src = 0;
            bool inside = true;
            for (size_t d = 0; d < rank && inside; ++d)
            {
                int s = idx[d] - n.ints[1 + d];
                const int len = a.shape[d];
                if (s < 0 || s >= len)
                {
                    if (mode == 1)      { s = s < 0 ? -s : 2 * (len - 1) - s; }
                    else if (mode == 2) { s = s < 0 ? 0 : len - 1; }
                    else                { inside = false; break; }
                }
                src += (size_t) s * st[d];
            }
            out.data[i] = inside ? a.data[src] : n.floats[0];

            for (int d = (int) rank - 1; d >= 0; --d)
                if (++idx[(size_t) d] < out.shape[(size_t) d]) break; else idx[(size_t) d] = 0;
        }
    }

    static void reduce (const Node& n, const Tensor& a, Tensor& out)
    {
        const size_t rank = a.shape.size();
        std::vector<char> red (rank, 0);
        for (int k = 0; k < n.ints[1]; ++k)
            red[(size_t) n.ints[(size_t) (2 + k)]] = 1;

        const auto st = stridesOf (a.shape);
        std::vector<int> keepShape;
        std::vector<size_t> keepStride;
        for (size_t d = 0; d < rank; ++d)
            if (! red[d]) { keepShape.push_back (a.shape[d]); keepStride.push_back (st[d]); }

        size_t outCount = 1;
        for (int d : keepShape) outCount *= (size_t) d;

        const float init = n.op == ReduceSum ? 0.0f : (n.op == ReduceMin ? 3.4e38f : -3.4e38f);
        out.data.assign (outCount, init);
        std::vector<size_t> outSt (keepShape.size(), 1);
        for (int i = (int) keepShape.size() - 2; i >= 0; --i)
            outSt[(size_t) i] = outSt[(size_t) i + 1] * (size_t) keepShape[(size_t) i + 1];

        std::vector<int> idx (rank, 0);
        for (size_t i = 0; i < a.data.size(); ++i)
        {
            size_t o = 0, k = 0;
            for (size_t d = 0; d < rank; ++d)
                if (! red[d]) o += (size_t) idx[d] * outSt[k++];

            float& dst = out.data[o];
            const float v = a.data[i];
            if (n.op == ReduceSum) dst += v;
            else if (n.op == ReduceMin) dst = std::min (dst, v);
            else dst = std::max (dst, v);

            for (int d = (int) rank - 1; d >= 0; --d)
                if (++idx[(size_t) d] < a.shape[(size_t) d]) break; else idx[(size_t) d] = 0;
        }
    }

    // NCHW convolution (batch 1). ints: group, strideH, strideW, dilH, dilW, padTop, padLeft, padBottom, padRight
    static void conv (const Node& n, const Tensor& x, const Tensor& w, const Tensor* bias, Tensor& out)
    {
        const int group = n.ints[0], sh = n.ints[1], sw = n.ints[2];
        const int pt = n.ints[5], pl = n.ints[6];
        const int C = x.shape[1], H = x.shape[2], W = x.shape[3];
        const int M = w.shape[0], Cg = w.shape[1], kh = w.shape[2], kw = w.shape[3];
        const int oh = out.shape[2], ow = out.shape[3];
        const int mg = M / group;

        out.data.assign ((size_t) M * oh * ow, 0.0f);
        (void) C;

        for (int m = 0; m < M; ++m)
        {
            float* o = out.data.data() + (size_t) m * oh * ow;
            const float b = bias != nullptr ? bias->data[(size_t) m] : 0.0f;
            const int g = m / mg;

            if (kh == 1 && pt == 0 && sh == 1)
            {
                // 1-D convolution along time: a dot product per output sample
                for (int y = 0; y < oh; ++y)
                    for (int xo = 0; xo < ow; ++xo)
                    {
                        float acc = b;
                        for (int c = 0; c < Cg; ++c)
                        {
                            const float* wk = w.data.data() + (((size_t) m * Cg + (size_t) c) * kh) * kw;
                            const float* in = x.data.data() + ((size_t) (g * Cg + c) * H + (size_t) y) * W;
                            const int x0 = xo * sw - pl;
                            const int k0 = std::max (0, -x0), k1 = std::min (kw, W - x0);
                            float s = 0.0f;
                            for (int k = k0; k < k1; ++k)
                                s += wk[k] * in[x0 + k];
                            acc += s;
                        }
                        o[(size_t) y * ow + (size_t) xo] = acc;
                    }
                continue;
            }

            for (int i = 0; i < oh * ow; ++i) o[i] = b;

            for (int c = 0; c < Cg; ++c)
            {
                const float* plane = x.data.data() + (size_t) (g * Cg + c) * H * W;
                for (int ky = 0; ky < kh; ++ky)
                    for (int kx = 0; kx < kw; ++kx)
                    {
                        const float wv = w.data[(((size_t) m * Cg + (size_t) c) * kh + (size_t) ky) * kw + (size_t) kx];
                        if (wv == 0.0f)
                            continue;

                        // output columns whose input column is inside the image
                        int xLo = 0, xHi = ow;
                        while (xLo < ow && xLo * sw + kx - pl < 0) ++xLo;
                        while (xHi > xLo && (xHi - 1) * sw + kx - pl >= W) --xHi;

                        for (int y = 0; y < oh; ++y)
                        {
                            const int iy = y * sh - pt + ky;
                            if (iy < 0 || iy >= H)
                                continue;
                            const float* irow = plane + (size_t) iy * W + (kx - pl);
                            float* orow = o + (size_t) y * ow;
                            if (sw == 1)
                                for (int xo = xLo; xo < xHi; ++xo)
                                    orow[xo] += wv * irow[xo];
                            else
                                for (int xo = xLo; xo < xHi; ++xo)
                                    orow[xo] += wv * irow[xo * sw];
                        }
                    }
            }
        }
    }

    std::vector<Tensor> consts;
    std::vector<std::vector<int>> tensorShapes;
    std::vector<Node> nodes;
    std::vector<int> outputs, lastUse;
    bool loaded = false;
};
} // namespace bp
} // namespace a2m
