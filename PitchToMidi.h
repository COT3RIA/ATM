#pragma once
// Audio -> MIDI engine (polyphonic). Pure C++17, no JUCE dependency.
//
// Based on Spotify's open-source "Basic Pitch" (Apache License 2.0, see THIRD_PARTY_NOTICES.md):
// a small neural network predicts, for every 11.6 ms frame, how strongly each of the 88 piano keys is active
// and where notes start. The note decoding follows the reference implementation.
//
// Two stages so that the heavy part runs once per file and the settings can be tweaked live:
//   1. analyse():    resample + run the network over the whole file (slow, cached by the plugin)
//   2. transcribe(): turn the network output into notes using Settings (fast)

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <functional>
#include <memory>
#include <queue>
#include <thread>
#include <vector>

#include "BasicPitchModel.h"
#include "BasicPitchNet.h"

namespace a2m
{
struct Settings
{
    float noteSensitivity       = 0.70f;   // 0..1  higher = quieter / less certain notes are kept
    float splitSensitivity      = 0.40f;   // 0..1  higher = notes are split (re-struck) more eagerly
    float minNoteLengthMs       = 130.0f;  // notes shorter than this are dropped
    float minFrequencyHz        = 0.0f;    // 0 = Off
    float maxFrequencyHz        = 3000.0f;
    int   noteOffHangoverFrames = 11;      // frames of silence tolerated before a note is closed
    bool  inferOnsets           = true;    // also detect note starts from sudden level rises
    bool  smoothing             = true;    // smooth the note activations over time
};

struct Note
{
    double startSec = 0.0;
    double endSec   = 0.0;
    int    pitch    = 60;
    int    velocity = 100;
};

struct Spectrogram
{
    static constexpr int numPitches  = bp::kPitches;     // 88 piano keys
    static constexpr int lowestPitch = 21;               // MIDI number of A0

    int    numFrames = 0;
    double hopSec    = 256.0 / 22050.0;
    double lengthSec = 0.0;
    std::vector<float> note;       // numFrames * numPitches, note (frame) activation 0..1
    std::vector<float> onset;      // numFrames * numPitches, note onset probability 0..1
    std::vector<double> frameTime; // time in seconds of every frame (numFrames + 1 entries)
};

namespace detail
{
constexpr double kPi = 3.14159265358979323846;
constexpr int    kSampleRate    = 22050;
constexpr int    kFft           = 256;
constexpr int    kOverlapFrames = 30;
constexpr int    kOverlap       = kOverlapFrames * kFft;                    // 7680 samples
constexpr int    kHop           = bp::kWindowSamples - kOverlap;            // 36164 samples
constexpr double kFps           = (double) kSampleRate / kFft;              // 86.13 frames per second

inline double bessel0 (double x)
{
    double sum = 1.0, term = 1.0;
    for (int k = 1; k < 40; ++k)
    {
        term *= (x / (2.0 * k)) * (x / (2.0 * k));
        sum += term;
    }
    return sum;
}

// Band-limited resampling (windowed sinc, polyphase table with linear interpolation)
inline std::vector<float> resampleTo22050 (const std::vector<float>& in, double sampleRate)
{
    if (std::abs (sampleRate - kSampleRate) < 0.5)
        return in;

    const double ratio  = (double) kSampleRate / sampleRate;                // output samples per input sample
    const double cutoff = 0.5 * std::min (1.0, ratio) * 0.96;               // cycles per input sample
    const int half = (int) std::ceil (14.0 / (2.0 * cutoff));               // input samples on each side
    constexpr int sub = 512;
    constexpr double beta = 8.0;

    std::vector<float> table ((size_t) (2 * half) * (sub + 1));
    const double i0beta = bessel0 (beta);
    for (int s = 0; s <= sub; ++s)
    {
        const double frac = (double) s / sub;
        for (int k = 0; k < 2 * half; ++k)
        {
            const double t = (double) (k - half + 1) - frac;                // distance from the output position
            const double x = 2.0 * cutoff * t;
            const double sinc = std::abs (x) < 1.0e-9 ? 1.0 : std::sin (kPi * x) / (kPi * x);
            const double r = t / half;
            const double win = std::abs (r) >= 1.0 ? 0.0 : bessel0 (beta * std::sqrt (1.0 - r * r)) / i0beta;
            table[(size_t) s * (size_t) (2 * half) + (size_t) k] = (float) (2.0 * cutoff * sinc * win);
        }
    }

    const size_t outLen = (size_t) ((double) in.size() * ratio);
    std::vector<float> out (outLen);
    const long n = (long) in.size();

    for (size_t o = 0; o < outLen; ++o)
    {
        const double pos = (double) o / ratio;
        const long i = (long) std::floor (pos);
        const double fpos = (pos - (double) i) * sub;
        const int s0 = (int) fpos;
        const float w1 = (float) (fpos - s0);
        const float* t0 = table.data() + (size_t) s0 * (size_t) (2 * half);
        const float* t1 = t0 + (size_t) (2 * half);

        float acc = 0.0f;
        const long base = i - half + 1;
        for (int k = 0; k < 2 * half; ++k)
        {
            const long idx = base + k;
            if (idx < 0 || idx >= n)
                continue;
            acc += in[(size_t) idx] * (t0[k] + w1 * (t1[k] - t0[k]));
        }
        out[o] = acc;
    }
    return out;
}
}

//==============================================================================
// Stage 1: network inference over the whole file
inline std::shared_ptr<Spectrogram> analyse (const std::vector<float>& mono,
                                             double sampleRate,
                                             const std::function<bool()>& shouldCancel = {})
{
    auto spec = std::make_shared<Spectrogram>();
    if (mono.empty() || sampleRate < 4000.0)
        return spec;

    static const bp::Net* net = [] { auto* n = new bp::Net(); n->load (bp::kModelBlob, bp::kModelBlobSize); return n; }();
    if (! net->isLoaded())
        return spec;

    const auto audio = detail::resampleTo22050 (mono, sampleRate);
    const long origLen = (long) audio.size();
    if (origLen < 2048)
        return spec;

    // zero padding of half an overlap in front, then fixed windows with an overlap of 30 frames
    std::vector<float> padded ((size_t) (origLen + detail::kOverlap / 2), 0.0f);
    std::copy (audio.begin(), audio.end(), padded.begin() + detail::kOverlap / 2);

    const int numWin = (int) ((padded.size() + (size_t) detail::kHop - 1) / (size_t) detail::kHop);
    constexpr int fpw = bp::kFramesPerWindow, P = bp::kPitches, keep = fpw - detail::kOverlapFrames;
    constexpr int trim = detail::kOverlapFrames / 2;

    std::vector<std::vector<float>> noteWin ((size_t) numWin), onsetWin ((size_t) numWin);
    std::atomic<int> nextWin { 0 };
    std::atomic<bool> cancelled { false };

    auto worker = [&]
    {
        std::vector<float> window ((size_t) bp::kWindowSamples), note ((size_t) fpw * P), onset ((size_t) fpw * P);
        for (;;)
        {
            const int w = nextWin.fetch_add (1);
            if (w >= numWin || cancelled.load())
                return;
            if (shouldCancel && shouldCancel())
            {
                cancelled = true;
                return;
            }

            std::fill (window.begin(), window.end(), 0.0f);
            const size_t start = (size_t) w * (size_t) detail::kHop;
            const size_t n = std::min<size_t> ((size_t) bp::kWindowSamples, padded.size() - start);
            std::copy (padded.begin() + (long) start, padded.begin() + (long) (start + n), window.begin());

            net->run (window.data(), note.data(), onset.data());

            noteWin[(size_t) w].assign (note.begin() + (long) trim * P, note.begin() + (long) (trim + keep) * P);
            onsetWin[(size_t) w].assign (onset.begin() + (long) trim * P, onset.begin() + (long) (trim + keep) * P);
        }
    };

    const unsigned hw = std::max (1u, std::thread::hardware_concurrency());
    const int nThreads = (int) std::min<unsigned> (std::min<unsigned> (hw, 8u), (unsigned) numWin);
    std::vector<std::thread> threads;
    for (int i = 1; i < nThreads; ++i)
        threads.emplace_back (worker);
    worker();
    for (auto& t : threads)
        t.join();

    if (cancelled.load())
        return std::make_shared<Spectrogram>();

    const long totalFrames = std::min<long> ((long) numWin * keep, (long) ((double) origLen / detail::kHop * keep));
    spec->numFrames = (int) totalFrames;
    spec->lengthSec = (double) origLen / detail::kSampleRate;
    spec->note.resize ((size_t) totalFrames * P);
    spec->onset.resize ((size_t) totalFrames * P);
    spec->frameTime.resize ((size_t) totalFrames + 1);

    for (long f = 0; f < totalFrames; ++f)
    {
        const long w = f / keep, k = f % keep;
        std::copy (noteWin[(size_t) w].begin() + k * P, noteWin[(size_t) w].begin() + (k + 1) * P, spec->note.begin() + f * P);
        std::copy (onsetWin[(size_t) w].begin() + k * P, onsetWin[(size_t) w].begin() + (k + 1) * P, spec->onset.begin() + f * P);
    }
    for (long f = 0; f <= totalFrames; ++f)
    {
        const long w = f / keep, k = f % keep;
        spec->frameTime[(size_t) f] = ((double) w * detail::kHop + (double) k * detail::kFft) / detail::kSampleRate;
    }

    return spec;
}

//==============================================================================
// Stage 2: note decoding (port of the reference "output_to_notes_polyphonic")
inline std::vector<Note> transcribe (const Spectrogram& sp, const Settings& s)
{
    std::vector<Note> out;
    const int F = sp.numFrames, P = Spectrogram::numPitches;
    if (F < 4)
        return out;

    // settings -> thresholds (tuned on two reference tracks: the defaults give frame 0.273, onset 0.52)
    const float frameThresh = std::clamp (0.5233f - 0.3333f * s.noteSensitivity, 0.05f, 0.9f);
    const float onsetThresh = std::clamp (0.72f - 0.4f * s.splitSensitivity, 0.05f, 0.95f);
    const int minLen = std::max (1, (int) std::lround (s.minNoteLengthMs / 1000.0 * detail::kFps));
    const int energyTol = std::max (1, s.noteOffHangoverFrames);

    std::vector<float> frames = sp.note, onsets = sp.onset;
    auto at = [P] (std::vector<float>& v, int f, int p) -> float& { return v[(size_t) f * P + (size_t) p]; };

    // optional smoothing of the note activations over time (3 frames)
    if (s.smoothing)
    {
        const auto src = frames;
        for (int f = 1; f < F - 1; ++f)
            for (int p = 0; p < P; ++p)
                at (frames, f, p) = 0.25f * src[(size_t) (f - 1) * P + (size_t) p] + 0.5f * src[(size_t) f * P + (size_t) p]
                                  + 0.25f * src[(size_t) (f + 1) * P + (size_t) p];
    }

    // frequency limits
    int lo = 0, hi = P;
    if (s.minFrequencyHz > 1.0f)
        lo = std::clamp ((int) std::lround (69.0 + 12.0 * std::log2 (s.minFrequencyHz / 440.0)) - Spectrogram::lowestPitch, 0, P);
    if (s.maxFrequencyHz > 1.0f)
        hi = std::clamp ((int) std::lround (69.0 + 12.0 * std::log2 (s.maxFrequencyHz / 440.0)) - Spectrogram::lowestPitch, 0, P);
    for (int f = 0; f < F; ++f)
        for (int p = 0; p < P; ++p)
            if (p < lo || p >= hi)
            {
                at (onsets, f, p) = 0.0f;
                at (frames, f, p) = 0.0f;
            }

    // onsets inferred from sudden rises of the frame activation
    if (s.inferOnsets)
    {
        float maxOnset = 0.0f, maxDiff = 0.0f;
        for (float v : onsets) maxOnset = std::max (maxOnset, v);

        std::vector<float> diff ((size_t) F * P, 0.0f);
        for (int f = 2; f < F; ++f)
            for (int p = 0; p < P; ++p)
            {
                const float d1 = at (frames, f, p) - at (frames, f - 1, p);
                const float d2 = at (frames, f, p) - at (frames, f - 2, p);
                const float d = std::max (0.0f, std::min (d1, d2));
                diff[(size_t) f * P + (size_t) p] = d;
                maxDiff = std::max (maxDiff, d);
            }
        if (maxDiff > 0.0f)
            for (size_t i = 0; i < diff.size(); ++i)
                onsets[i] = std::max (onsets[i], maxOnset * diff[i] / maxDiff);
    }

    // onset peaks (strict local maxima in time), processed from the end of the file backwards
    struct Onset { int f, p; };
    std::vector<Onset> peaks;
    for (int p = 0; p < P; ++p)
        for (int f = 1; f < F - 1; ++f)
        {
            const float v = at (onsets, f, p);
            if (v > at (onsets, f - 1, p) && v > at (onsets, f + 1, p) && v >= onsetThresh)
                peaks.push_back ({ f, p });
        }
    std::sort (peaks.begin(), peaks.end(), [] (const Onset& a, const Onset& b) { return a.f != b.f ? a.f > b.f : a.p > b.p; });

    struct Ev { int start, end, pitch; float amp; };
    std::vector<Ev> events;
    std::vector<float> rem = frames;
    auto clearAround = [&] (int f0, int f1, int p)
    {
        for (int f = f0; f < f1; ++f)
        {
            at (rem, f, p) = 0.0f;
            if (p < P - 1) at (rem, f, p + 1) = 0.0f;
            if (p > 0)     at (rem, f, p - 1) = 0.0f;
        }
    };
    auto meanAmp = [&] (int f0, int f1, int p)
    {
        double a = 0.0;
        for (int f = f0; f < f1; ++f) a += at (frames, f, p);
        return (float) (a / std::max (1, f1 - f0));
    };

    for (const auto& o : peaks)
    {
        if (o.f >= F - 1)
            continue;

        int i = o.f + 1, k = 0;
        while (i < F - 1 && k < energyTol)
        {
            k = at (rem, i, o.p) < frameThresh ? k + 1 : 0;
            ++i;
        }
        i -= k;

        if (i - o.f <= minLen)
            continue;

        clearAround (o.f, i, o.p);
        events.push_back ({ o.f, i, o.p, meanAmp (o.f, i, o.p) });
    }

    // "melodia trick": strong activations that have no onset are followed forwards and backwards
    {
        struct Cell { float v; size_t idx; };
        auto cmp = [] (const Cell& a, const Cell& b) { return a.v != b.v ? a.v < b.v : a.idx > b.idx; };
        std::priority_queue<Cell, std::vector<Cell>, decltype (cmp)> heap (cmp);
        for (size_t i = 0; i < rem.size(); ++i)
            if (rem[i] > frameThresh)
                heap.push ({ rem[i], i });

        while (! heap.empty())
        {
            const auto c = heap.top();
            heap.pop();
            if (rem[c.idx] != c.v || c.v <= frameThresh)
                continue;                                           // already consumed

            const int im = (int) (c.idx / (size_t) P), p = (int) (c.idx % (size_t) P);
            rem[c.idx] = 0.0f;

            int i = im + 1, k = 0;
            while (i < F - 1 && k < energyTol)
            {
                k = at (rem, i, p) < frameThresh ? k + 1 : 0;
                clearAround (i, i + 1, p);
                ++i;
            }
            const int iEnd = i - 1 - k;

            i = im - 1;
            k = 0;
            while (i > 0 && k < energyTol)
            {
                k = at (rem, i, p) < frameThresh ? k + 1 : 0;
                clearAround (i, i + 1, p);
                --i;
            }
            const int iStart = i + 1 + k;

            if (iEnd - iStart <= minLen)
                continue;

            events.push_back ({ iStart, iEnd, p, meanAmp (iStart, iEnd, p) });
        }
    }

    for (const auto& e : events)
    {
        Note n;
        n.startSec = sp.frameTime[(size_t) std::clamp (e.start, 0, F)];
        n.endSec   = sp.frameTime[(size_t) std::clamp (e.end, 0, F)];
        n.pitch    = e.pitch + Spectrogram::lowestPitch;
        n.velocity = std::clamp ((int) std::lround (127.0f * e.amp), 1, 127);
        if (n.endSec > n.startSec)
            out.push_back (n);
    }

    std::sort (out.begin(), out.end(), [] (const Note& x, const Note& y)
    {
        return x.startSec != y.startSec ? x.startSec < y.startSec : x.pitch < y.pitch;
    });

    return out;
}

// Tempo estimate (BPM) from the note-onset activity of the network output; 0 = not sure.
// Used for the MIDI file's tempo when the DAW does not tell us the project tempo.
inline double estimateBpm (const Spectrogram& sp)
{
    const int F = sp.numFrames, P = Spectrogram::numPitches;
    if (F < 600)                                                    // shorter than ~7 s: not enough to tell
        return 0.0;

    // onset strength per frame
    std::vector<double> env ((size_t) F, 0.0);
    for (int f = 1; f < F; ++f)
    {
        double s = 0.0, flux = 0.0;
        for (int p = 0; p < P; ++p)
        {
            s += sp.onset[(size_t) f * P + (size_t) p];
            flux += std::max (0.0f, sp.note[(size_t) f * P + (size_t) p] - sp.note[(size_t) (f - 1) * P + (size_t) p]);
        }
        env[(size_t) f] = s + 0.5 * flux;
    }

    // remove the slowly varying level (about 0.5 s), keep only the peaks
    {
        const int w = 43;
        std::vector<double> csum ((size_t) F + 1, 0.0), out ((size_t) F, 0.0);
        for (int f = 0; f < F; ++f) csum[(size_t) f + 1] = csum[(size_t) f] + env[(size_t) f];
        for (int f = 0; f < F; ++f)
        {
            const int a = std::max (0, f - w), b = std::min (F, f + w + 1);
            out[(size_t) f] = std::max (0.0, env[(size_t) f] - (csum[(size_t) b] - csum[(size_t) a]) / (double) (b - a));
        }
        env.swap (out);
    }

    // autocorrelation up to 4 s
    const int maxLag = (int) (4.0 * detail::kFps);
    std::vector<double> ac ((size_t) maxLag + 2, 0.0);
    for (int lag = 1; lag <= maxLag + 1; ++lag)
    {
        double s = 0.0;
        for (int f = lag; f < F; ++f)
            s += env[(size_t) f] * env[(size_t) (f - lag)];
        ac[(size_t) lag] = s / (double) (F - lag);
    }
    auto acAt = [&] (double lag)                                    // linear interpolation
    {
        if (lag < 1.0 || lag >= (double) maxLag) return 0.0;
        const int i = (int) lag;
        const double fr = lag - i;
        return ac[(size_t) i] * (1.0 - fr) + ac[(size_t) i + 1] * fr;
    };

    // comb over the beat period and its multiples, with a gentle preference for 70..160 BPM
    double best = 0.0, bestBpm = 0.0, total = 0.0;
    int count = 0;
    for (double bpm = 55.0; bpm <= 190.0; bpm += 0.25)
    {
        const double lag = 60.0 / bpm * detail::kFps;
        double score = acAt (lag) + 0.7 * acAt (2.0 * lag) + 0.5 * acAt (4.0 * lag) + 0.4 * acAt (lag * 0.5);
        const double oct = std::log2 (bpm / 112.0);
        score *= std::exp (-0.5 * oct * oct / (0.55 * 0.55));
        total += score;
        ++count;
        if (score > best) { best = score; bestBpm = bpm; }
    }

    if (bestBpm <= 0.0 || best < 2.5 * total / (double) std::max (1, count))      // no clear peak
        return 0.0;

    const double snapped = std::round (bestBpm);                   // songs are mostly at whole-number tempos
    return std::abs (bestBpm - snapped) < 0.7 ? snapped : std::round (bestBpm * 10.0) / 10.0;
}

// Convenience: both stages in one call
inline std::vector<Note> convert (const std::vector<float>& mono, double sampleRate, const Settings& s,
                                  const std::function<bool()>& shouldCancel = {})
{
    const auto spec = analyse (mono, sampleRate, shouldCancel);
    return transcribe (*spec, s);
}

// ---------------------------------------------------------------------------------------------
// Musical key from the transcribed notes: pitch-class profile (weighted by duration and loudness)
// compared with the Krumhansl-Kessler major / minor profiles (Pearson correlation).
struct KeyResult
{
    bool valid = false;
    int  tonic = 0;           // 0 = C ... 11 = B
    bool minor = false;
    int  altTonic = 0;        // the second best key (often the relative major / minor)
    bool altMinor = false;
    int  level = 0;           // confidence: 1 = low, 2 = medium, 3 = high
    float score = 0.0f;       // correlation of the best key
    float gap = 0.0f;         // distance to the second best
};

inline KeyResult estimateKey (const std::vector<Note>& notes)
{
    KeyResult out;
    if (notes.size() < 4)
        return out;

    double chroma[12] = {};
    double total = 0.0;
    for (const auto& n : notes)
    {
        const double d = std::min (2.0, std::max (0.05, n.endSec - n.startSec));
        const double w = std::sqrt (d) * (0.4 + 0.6 * n.velocity / 127.0);
        chroma[((n.pitch % 12) + 12) % 12] += w;
        total += w;
    }
    if (total <= 0.0)
        return out;

    static const double major[12] = { 6.35, 2.23, 3.48, 2.33, 4.38, 4.09, 2.52, 5.19, 2.39, 3.66, 2.29, 2.88 };
    static const double minor[12] = { 6.33, 2.68, 3.52, 5.38, 2.60, 3.53, 2.54, 4.75, 3.98, 2.69, 3.34, 3.17 };

    auto corr = [&] (const double* prof, int tonic)
    {
        double mx = 0, my = 0;
        for (int i = 0; i < 12; ++i) { mx += chroma[(i + tonic) % 12]; my += prof[i]; }
        mx /= 12; my /= 12;
        double sxy = 0, sxx = 0, syy = 0;
        for (int i = 0; i < 12; ++i)
        {
            const double a = chroma[(i + tonic) % 12] - mx, b = prof[i] - my;
            sxy += a * b; sxx += a * a; syy += b * b;
        }
        return sxx > 0 && syy > 0 ? sxy / std::sqrt (sxx * syy) : 0.0;
    };

    struct C { double r; int t; bool m; };
    C best { -2, 0, false }, second { -2, 0, false };
    for (int t = 0; t < 12; ++t)
        for (int m = 0; m < 2; ++m)
        {
            const C c { corr (m ? minor : major, t), t, m == 1 };
            if (c.r > best.r)        { second = best; best = c; }
            else if (c.r > second.r) { second = c; }
        }

    out.valid = true;
    out.tonic = best.t;   out.minor = best.m;
    out.altTonic = second.t;  out.altMinor = second.m;
    out.score = (float) best.r;
    out.gap = (float) (best.r - second.r);

    if (best.r >= 0.78 && out.gap >= 0.05 && notes.size() >= 16)       out.level = 3;
    else if (best.r >= 0.62 && out.gap >= 0.02 && notes.size() >= 8)   out.level = 2;
    else                                                                out.level = 1;
    return out;
}

inline const char* keyName (int tonic)
{
    static const char* n[12] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    return n[((tonic % 12) + 12) % 12];
}

} // namespace a2m
