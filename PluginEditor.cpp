#include "PluginEditor.h"
#include <BinaryData.h>
#include "Lang.h"
using lang::tr;

//==============================================================================
// Palette / look and feel
Palette Palette::make (bool dark)
{
    Palette p;
    p.dark = dark;

    if (dark)
    {
        p.bg          = juce::Colour (0xff17181a);
        p.panel       = juce::Colour (0xff212326);
        p.panelBorder = juce::Colour (0xff5f8f63);
        p.track       = juce::Colour (0xff2d2f31);
        p.fill        = juce::Colour (0xff5f8f63);
        p.thumb       = juce::Colour (0xffa5d6ac);
        p.button      = juce::Colour (0xff343637);
        p.outline     = juce::Colour (0xff6b6d6e);
        p.text        = juce::Colour (0xffe8e8e8);
        p.textDim     = juce::Colour (0xff9aa09c);
        p.noteFill    = juce::Colour (0xffa5d6ac);
        p.rollBg      = juce::Colour (0xff1c1e20);
        p.grid        = juce::Colour (0x14ffffff);
        p.helmLight   = juce::Colour (0xff7a828b);
        p.helmMid     = juce::Colour (0xff474d54);
        p.helmDark    = juce::Colour (0xff23272b);
        p.helmEdge    = juce::Colour (0xff0b0c0e);
        p.bodyTop     = juce::Colour (0xff484e55);
        p.bodyBot     = juce::Colour (0xff25292d);
    }
    else
    {
        p.bg          = juce::Colour (0xffe6e8ea);
        p.panel       = juce::Colour (0xfff5f6f7);
        p.panelBorder = juce::Colour (0xff5f8f63);
        p.track       = juce::Colour (0xffd0d4d7);
        p.fill        = juce::Colour (0xff6b9f6f);
        p.thumb       = juce::Colour (0xff3d7a46);
        p.button      = juce::Colour (0xffe4e7ea);
        p.outline     = juce::Colour (0xff98a0a6);
        p.text        = juce::Colour (0xff1c1f23);
        p.textDim     = juce::Colour (0xff5d646a);
        p.noteFill    = juce::Colour (0xff3d7a46);
        p.rollBg      = juce::Colour (0xffdde1e4);
        p.grid        = juce::Colour (0x18000000);
        p.helmLight   = juce::Colour (0xfff4f6f8);
        p.helmMid     = juce::Colour (0xffc3c9cf);
        p.helmDark    = juce::Colour (0xff8e969e);
        p.helmEdge    = juce::Colour (0xff343a40);
        p.bodyTop     = juce::Colour (0xffe3e6e9);
        p.bodyBot     = juce::Colour (0xff9aa2a9);
    }
    return p;
}

void GreenLookAndFeel::setDark (bool dark)
{
    pal = Palette::make (dark);

    setColour (juce::ResizableWindow::backgroundColourId, pal.bg);
    setColour (juce::TextButton::buttonColourId, pal.button);
    setColour (juce::TextButton::textColourOffId, pal.text);
    setColour (juce::TextButton::textColourOnId, pal.text);
    setColour (juce::Label::textColourId, pal.text);
    setColour (juce::ToggleButton::textColourId, pal.text);
    setColour (juce::TooltipWindow::backgroundColourId, pal.panel);
    setColour (juce::TooltipWindow::textColourId, pal.text);
    setColour (juce::TooltipWindow::outlineColourId, pal.outline);
}

const Palette& paletteOf (const juce::Component& c)
{
    if (auto* l = dynamic_cast<GreenLookAndFeel*> (&c.getLookAndFeel()))
        return l->pal;

    static const Palette fallback = Palette::make (true);
    return fallback;
}

void GreenLookAndFeel::drawLinearSlider (juce::Graphics& g, int x, int y, int w, int h, float sliderPos,
                                         float, float, juce::Slider::SliderStyle, juce::Slider&)
{
    const float cy = (float) y + (float) h * 0.5f;
    const float left = (float) x, right = (float) (x + w);

    g.setColour (pal.track);
    g.fillRoundedRectangle (left, cy - 3.0f, right - left, 6.0f, 3.0f);

    g.setColour (pal.fill);
    g.fillRoundedRectangle (left, cy - 3.0f, std::max (6.0f, sliderPos - left), 6.0f, 3.0f);

    g.setColour (pal.thumb);
    g.fillEllipse (sliderPos - 10.0f, cy - 10.0f, 20.0f, 20.0f);
}

void GreenLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& b, const juce::Colour& colour,
                                             bool highlighted, bool down)
{
    auto r = b.getLocalBounds().toFloat().reduced (1.0f);
    auto c = colour;
    if (down)             c = c.brighter (0.15f);
    else if (highlighted) c = c.brighter (0.08f);

    g.setColour (c);
    g.fillRoundedRectangle (r, 9.0f);
    g.setColour (pal.outline);
    g.drawRoundedRectangle (r, 9.0f, 1.5f);
}

void GreenLookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& b, bool highlighted, bool)
{
    const auto box = juce::Rectangle<float> (2.0f, (float) b.getHeight() * 0.5f - 10.0f, 20.0f, 20.0f);

    g.setColour (highlighted ? pal.text : pal.text.withAlpha (0.75f));
    g.drawRoundedRectangle (box, 5.0f, 1.6f);

    if (b.getToggleState())
    {
        juce::Path tick;
        tick.startNewSubPath (box.getX() + 5.0f, box.getCentreY());
        tick.lineTo (box.getX() + 9.0f, box.getBottom() - 6.0f);
        tick.lineTo (box.getRight() - 4.5f, box.getY() + 5.5f);
        g.setColour (pal.thumb);
        g.strokePath (tick, juce::PathStrokeType (2.4f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    g.setColour (pal.text);
    g.setFont (juce::Font (juce::FontOptions (19.0f)));
    g.drawText (b.getButtonText(), 34, 0, b.getWidth() - 34, b.getHeight(), juce::Justification::centredLeft);
}

juce::Font GreenLookAndFeel::getTextButtonFont (juce::TextButton&, int)
{
    return juce::Font (juce::FontOptions (19.0f));
}

//==============================================================================
// Icon buttons
void IconButton::paintButton (juce::Graphics& g, bool highlighted, bool down)
{
    const auto& pal = paletteOf (*this);
    auto r = getLocalBounds().toFloat().reduced (1.0f);

    auto fillCol = pal.button;
    if (down)             fillCol = fillCol.brighter (0.15f);
    else if (highlighted) fillCol = fillCol.brighter (0.08f);

    g.setColour (fillCol);
    g.fillRoundedRectangle (r, 9.0f);
    g.setColour (pal.outline);
    g.drawRoundedRectangle (r, 9.0f, 1.5f);

    const auto c = r.getCentre();

    if (kind == Kind::Theme)
    {
        g.setColour (pal.text);
        if (pal.dark)       // moon
        {
            g.fillEllipse (c.x - 7.0f, c.y - 7.0f, 14.0f, 14.0f);
            g.setColour (fillCol);
            g.fillEllipse (c.x - 2.5f, c.y - 9.0f, 13.0f, 13.0f);
        }
        else                // sun
        {
            g.fillEllipse (c.x - 4.5f, c.y - 4.5f, 9.0f, 9.0f);
            for (int i = 0; i < 8; ++i)
            {
                const float a = juce::MathConstants<float>::twoPi * (float) i / 8.0f;
                g.drawLine (c.x + std::cos (a) * 7.0f, c.y + std::sin (a) * 7.0f,
                            c.x + std::cos (a) * 10.0f, c.y + std::sin (a) * 10.0f, 1.8f);
            }
        }
    }
    else
    {
        const bool lightSlit = (eyeStyle == 1 && ! pal.dark);
        g.setColour (lightSlit ? juce::Colour (0xffb3b9bf) : juce::Colour (0xff0b0c0e));
        g.fillRoundedRectangle (c.x - 13.0f, c.y - 6.0f, 26.0f, 12.0f, 6.0f);

        juce::Colour core = eyeStyle == 0 ? juce::Colour (0xffff4040)
                                          : (pal.dark ? juce::Colour (0xfff4faff) : juce::Colour (0xff000000));
        for (float dx : { -5.5f, 5.5f })
        {
            if (! lightSlit)
            {
                juce::ColourGradient gg (core.withAlpha (0.55f), c.x + dx, c.y, core.withAlpha (0.0f), c.x + dx + 6.0f, c.y, true);
                g.setGradientFill (gg);
                g.fillEllipse (c.x + dx - 6.0f, c.y - 6.0f, 12.0f, 12.0f);
            }
            g.setColour (core);
            g.fillEllipse (c.x + dx - 2.4f, c.y - 2.4f, 4.8f, 4.8f);
        }
    }
}

//==============================================================================
// Piano roll
NoteRoll::NoteRoll (AudioToMidiProcessor& p) : processor (p)
{
}

void NoteRoll::resized()
{
}

void NoteRoll::mouseUp (const juce::MouseEvent& e)
{
    if (e.mouseWasClicked() && notes.empty() && onEmptyClick)
        onEmptyClick();
}

void NoteRoll::refresh()
{
    const int v = processor.getNotesVersion();
    if (v != version)
    {
        version = v;
        notes = processor.getNotesSnapshot();
        repaint();
    }

    total = processor.getTotalSeconds();

    {
        const float want = (! notes.empty() || (hover && ! processor.isBusy())) ? 1.0f : 0.0f;
        if (labelAlpha != want)
        {
            // after a conversion the labels fade in slowly (~0.7 s); on hover they come up faster, and they leave quickly
            labelAlpha += juce::jlimit (-0.12f, notes.empty() ? 0.10f : 0.045f, want - labelAlpha);
            if (std::abs (labelAlpha - want) < 0.01f)
                labelAlpha = want;
            repaint();
        }
    }

    const double pos = processor.isPlaying() ? processor.getPlayPositionSeconds() : -1.0;
    if (pos != playPos)
    {
        playPos = pos;
        repaint();
    }

    // pulse: the notes that started during the last 90 ms (the notes are sorted by their start)
    float target = 0.0f;
    if (pos >= 0.0 && ! notes.empty())
    {
        auto it = std::lower_bound (notes.begin(), notes.end(), pos - 0.09,
                                    [] (const a2m::Note& n, double t) { return n.startSec < t; });
        float sum = 0.0f;
        for (; it != notes.end() && it->startSec <= pos; ++it)
            sum += 0.4f + 0.6f * (float) it->velocity / 127.0f;
        target = std::min (1.0f, sum / 3.0f);
    }
    beat = std::max (target, beat * 0.80f);
    if (beat < 0.02f)
        beat = 0.0f;
}

void NoteRoll::setBusyTick (bool busy, int tick)
{
    if (busy || busyNow)
    {
        busyNow = busy;
        busyTickNow = tick;
        repaint();
    }
}

void NoteRoll::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    const auto ink = juce::Colours::white;

    // the blue ATM screen
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff1a6cf0), 0.0f, 0.0f, juce::Colour (0xff0a3aa6), 0.0f, bounds.getBottom(), false));
    g.fillRoundedRectangle (bounds, 6.0f);
    g.setGradientFill (juce::ColourGradient (juce::Colour (0x40ffffff), bounds.getCentreX(), bounds.getY() - 20.0f,
                                             juce::Colour (0x00ffffff), bounds.getCentreX(), bounds.getBottom() * 0.7f, true));
    g.fillRoundedRectangle (bounds, 6.0f);

    if (! notes.empty() && total > 0.0)
    {
        auto area = bounds.reduced (8.0f, 10.0f).withTrimmedBottom (22.0f);

        int lo = 127, hi = 0;
        for (const auto& n : notes)
        {
            lo = std::min (lo, n.pitch);
            hi = std::max (hi, n.pitch);
        }
        lo = std::max (0, lo - 2);
        hi = std::min (127, hi + 2);
        while (hi - lo < 23)
        {
            if (lo > 0) --lo;
            if (hi - lo < 23 && hi < 127) ++hi;
        }

        const float rows = (float) (hi - lo + 1);
        const float rowH = area.getHeight() / rows;

        g.setColour (ink.withAlpha (0.16f));
        for (int p = lo; p <= hi; ++p)
            if (p % 12 == 0)
            {
                const float y = area.getY() + (float) (hi - p) * rowH + rowH;
                g.drawHorizontalLine ((int) y, area.getX(), area.getRight());
            }

        for (const auto& n : notes)
        {
            const float x = area.getX() + (float) (n.startSec / total) * area.getWidth();
            const float w = std::max (1.5f, (float) ((n.endSec - n.startSec) / total) * area.getWidth());
            const float y = area.getY() + (float) (hi - n.pitch) * rowH;
            const float h = std::max (1.5f, rowH - 0.8f);

            g.setColour (juce::Colour (0xffd6ecff).withAlpha (0.55f + 0.45f * (float) n.velocity / 127.0f));
            g.fillRect (x, y, w, h);
        }

        if (playPos >= 0.0)
        {
            const float x = area.getX() + (float) (playPos / total) * area.getWidth();
            g.setColour (ink.withAlpha (0.9f));
            g.drawVerticalLine ((int) x, area.getY(), area.getBottom());
        }
    }

    // the keys beside the screen: what each one does
    {
        const float cy[] = { 61.0f, 96.0f, 130.0f, 164.0f, 199.0f };   // the keys' centres, relative to the screen
        g.setFont (juce::Font (juce::FontOptions (15.0f, juce::Font::bold)));
        auto pill = [&] (const juce::String& text, bool left, float y)
        {
            juce::GlyphArrangement ga;
            ga.addLineOfText (g.getCurrentFont(), text, 0.0f, 0.0f);
            const float tw = ga.getBoundingBox (0, -1, true).getWidth() + 14.0f;
            const float x = left ? 3.0f : bounds.getRight() - 3.0f - tw;
            const auto rr = juce::Rectangle<float> (x, y - 10.0f, tw, 20.0f);
            g.setColour (juce::Colour (0xff06225e).withAlpha (0.6f * labelAlpha));
            g.fillRoundedRectangle (rr, 5.0f);
            g.setColour (ink.withAlpha (0.95f * labelAlpha));
            g.drawText (text, rr, juce::Justification::centred);
        };
        if (labelAlpha > 0.005f)
        for (size_t i = 0; i < leftLabels.size() && i < 5; ++i)
            if (leftLabels[i].isNotEmpty()) pill (leftLabels[i], true, cy[i]);
        if (labelAlpha > 0.005f)
        for (size_t i = 0; i < rightLabels.size() && i < 5; ++i)
            if (rightLabels[i].isNotEmpty()) pill (rightLabels[i], false, cy[i]);
    }

    if (status.isNotEmpty() && (! notes.empty() || processor.isBusy()))
    {
        const auto rr = juce::Rectangle<float> (6.0f, bounds.getBottom() - 21.0f, bounds.getWidth() - 12.0f, 18.0f);
        g.setColour (juce::Colour (0xff06225e).withAlpha (0.72f));
        g.fillRoundedRectangle (rr, 5.0f);
        g.setColour (ink);
        g.setFont (juce::Font (juce::FontOptions (15.0f)));
        g.drawText (status, rr.reduced (8.0f, 0.0f), juce::Justification::centredLeft, true);
    }

    if (notes.empty() && ! processor.isBusy())
    {
        // "drop audio here": a tray with an arrow
        const float dropA = 1.0f - labelAlpha;          // the hint fades out while the key labels show
        if (dropA > 0.005f)
        {
        const float cx = bounds.getCentreX(), cy = bounds.getCentreY() - 22.0f;
        juce::Path arrow;
        arrow.startNewSubPath (cx, cy - 34.0f);
        arrow.lineTo (cx, cy + 10.0f);
        arrow.startNewSubPath (cx - 17.0f, cy - 8.0f);
        arrow.lineTo (cx, cy + 10.0f);
        arrow.lineTo (cx + 17.0f, cy - 8.0f);
        arrow.startNewSubPath (cx - 30.0f, cy + 12.0f);
        arrow.lineTo (cx - 30.0f, cy + 30.0f);
        arrow.lineTo (cx + 30.0f, cy + 30.0f);
        arrow.lineTo (cx + 30.0f, cy + 12.0f);
        g.setColour (ink.withAlpha (0.92f * dropA));
        g.strokePath (arrow, juce::PathStrokeType (5.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        g.setColour (ink.withAlpha (dropA));
        g.setFont (juce::Font (juce::FontOptions (26.0f, juce::Font::bold)));
        g.drawText (tr ("Drop audio here"), getLocalBounds().withTrimmedTop (getHeight() - 80).withHeight (34), juce::Justification::centred);
        g.setColour (ink.withAlpha (0.85f * dropA));
        g.setFont (juce::Font (juce::FontOptions (19.0f)));
        g.drawText ("MP3 / WAV / FLAC", getLocalBounds().withTrimmedTop (getHeight() - 46).withHeight (26), juce::Justification::centred);
        }
    }

    if (busyNow)
    {
        // conversion: a bar of "bits" running across the screen
        const float cy = bounds.getCentreY() - 14.0f;
        g.setColour (ink.withAlpha (0.95f));
        g.setFont (juce::Font (juce::FontOptions (28.0f, juce::Font::bold)));
        g.drawText (tr ("Converting..."), getLocalBounds().withY ((int) cy - 52).withHeight (40), juce::Justification::centred);

        const int n = 22;
        const float w = bounds.getWidth() - 70.0f, step = w / (float) n;
        for (int i = 0; i < n; ++i)
        {
            const float ph = std::fmod ((float) busyTickNow * 0.12f - (float) i * 0.18f, 3.0f);
            const float hgt = 8.0f + 26.0f * std::max (0.0f, std::sin (juce::MathConstants<float>::pi * std::min (1.0f, ph / 1.5f)));
            g.setColour (ink.withAlpha (0.45f + 0.5f * (hgt - 8.0f) / 26.0f));
            g.fillRoundedRectangle (35.0f + (float) i * step, cy + 16.0f - hgt * 0.5f, step * 0.6f, hgt, 2.0f);
        }
    }

    // border (brighter while a file is being dragged over the window)
    g.setColour (dragHighlight ? ink : ink.withAlpha (0.0f));
    if (dragHighlight)
        g.drawRoundedRectangle (bounds.reduced (2.0f), 6.0f, 3.0f);
}

//==============================================================================
// The ATM picture
AtmBackground::AtmBackground()
{
    setInterceptsMouseClicks (false, false);
    image = juce::ImageCache::getFromMemory (BinaryData::atm_full_png, BinaryData::atm_full_pngSize);
}

void AtmBackground::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);
    if (image.isValid())
    {
        g.setImageResamplingQuality (juce::Graphics::highResamplingQuality);
        g.drawImage (image, getLocalBounds().toFloat());
    }
}

//==============================================================================
// Sign glow
void SignGlow::paint (juce::Graphics& g)
{
    if (level <= 0.0f)
        return;

    // the sign's blue panel is (301, 49)-(722, 185) in the picture; this component starts at (288, 36)
    const auto sign = juce::Rectangle<float> (12.0f, 20.0f, 424.0f, 136.0f);
    for (int i = 4; i >= 1; --i)                                // soft halo
    {
        g.setColour (juce::Colour (0xff8fc4ff).withAlpha (0.045f * level * (float) (5 - i)));
        g.fillRoundedRectangle (sign.expanded ((float) i * 3.0f), 18.0f + (float) i * 2.0f);
    }
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xffeaf4ff).withAlpha (0.42f * level), sign.getCentreX(), sign.getCentreY(),
                                             juce::Colour (0xff6aa8ff).withAlpha (0.10f * level), sign.getX(), sign.getY(), true));
    g.fillRoundedRectangle (sign, 16.0f);
}

//==============================================================================
// Hardware keys
void HwButton::paintButton (juce::Graphics& g, bool highlighted, bool down)
{
    auto r = getLocalBounds().toFloat().reduced (1.0f);
    if (lit)
    {
        g.setColour (juce::Colour (0xff7ee08a).withAlpha (0.28f));
        g.fillRoundedRectangle (r, radius);
        g.setColour (juce::Colour (0xffb8ffc4));
        g.drawRoundedRectangle (r, radius, 2.0f);
    }
    if (down)
    {
        g.setColour (juce::Colours::black.withAlpha (0.28f));
        g.fillRoundedRectangle (r, radius);
    }
    else if (highlighted)
    {
        g.setColour (juce::Colours::white.withAlpha (0.20f));
        g.fillRoundedRectangle (r, radius);
        g.setColour (juce::Colour (0xffbfe3ff).withAlpha (0.8f));
        g.drawRoundedRectangle (r, radius, 1.6f);
    }
}

//==============================================================================
// Card reader
void CardReader::setState (bool isBusy, bool isDrag, int tick)
{
    if (isBusy != busy || isDrag != drag || isBusy)          // while busy the scan animation repaints on every tick
    {
        busy = isBusy;
        drag = isDrag;
        phase = (float) tick / 30.0f;
        repaint();
    }
}

void CardReader::paintButton (juce::Graphics& g, bool highlighted, bool)
{
    // The three green lights of the picture, relative to this component (it starts at 615, 646).
    // While converting we do not repaint them in another colour (that muddied the green); we only add light on top of them:
    // a bright spot runs along the top strip and a scan line sweeps the slot, like a card being read.
    if (busy)
    {
        const juce::Colour mint (0xffdcffe6), green (0xff35ff6b);
        auto tri = [] (float x) { x -= std::floor (x); return x < 0.5f ? x * 2.0f : 2.0f - x * 2.0f; };   // 0..1..0

        // top strip: a soft highlight travelling back and forth
        {
            const juce::Rectangle<float> strip (33.0f, 44.0f, 99.0f, 10.0f);
            const float cx = strip.getX() + strip.getWidth() * tri (phase / 1.6f);
            g.saveState();
            g.reduceClipRegion (strip.toNearestInt());
            juce::ColourGradient grad (mint.withAlpha (0.0f), cx - 9.0f, 0.0f, mint.withAlpha (0.0f), cx + 9.0f, 0.0f, false);
            grad.addColour (0.5, mint.withAlpha (0.95f));
            g.setGradientFill (grad);
            g.fillRoundedRectangle (strip, 4.0f);
            g.restoreState();
        }

        // the two lights of the slot take turns to brighten
        const float p = 0.5f + 0.5f * std::sin (phase * juce::MathConstants<float>::twoPi / 1.1f);
        g.setColour (mint.withAlpha (0.55f * p));
        g.fillRoundedRectangle (juce::Rectangle<float> (65.0f, 73.0f, 42.0f, 7.0f), 3.0f);
        g.setColour (mint.withAlpha (0.55f * (1.0f - p)));
        g.fillRoundedRectangle (juce::Rectangle<float> (66.0f, 125.0f, 40.0f, 6.0f), 3.0f);

        // scan line inside the slot
        {
            const juce::Rectangle<float> slot (62.0f, 84.0f, 46.0f, 38.0f);
            const float y = slot.getY() + slot.getHeight() * tri (phase / 1.2f);
            g.saveState();
            g.reduceClipRegion (slot.toNearestInt());
            juce::ColourGradient glow (green.withAlpha (0.0f), 0.0f, y - 4.0f, green.withAlpha (0.0f), 0.0f, y + 4.0f, false);
            glow.addColour (0.5, green.withAlpha (0.45f));
            g.setGradientFill (glow);
            g.fillRect (slot.getX(), y - 4.0f, slot.getWidth(), 8.0f);
            g.setColour (mint.withAlpha (0.95f));
            g.fillRoundedRectangle (slot.getX() + 1.0f, y - 0.6f, slot.getWidth() - 2.0f, 1.2f, 0.6f);
            g.restoreState();
        }
    }

    if (drag || highlighted)
    {
        auto r = getLocalBounds().toFloat().reduced (3.0f);
        g.setColour (juce::Colour (0xffb8ffc4).withAlpha (drag ? 0.95f : 0.6f));
        g.drawRoundedRectangle (r, 8.0f, drag ? 3.0f : 1.8f);
    }
}

//==============================================================================
// Numeric pad
KeypadComponent::KeypadComponent()
{
    // component origin in the picture: (396, 736); the keys stand on a slight perspective, so each row has its own columns
    const int ox = 396, oy = 736;
    const int colsRow[4][3] = { { 439, 479, 520 }, { 436, 479, 522 }, { 433, 477, 521 }, { 430, 477, 521 } };
    const int rows[] = { 764, 795, 827, 859 };
    for (int r = 0; r < 4; ++r)
        for (int cI = 0; cI < 3; ++cI)
            keys.push_back ({ juce::Rectangle<int> (colsRow[r][cI] - 18 - ox, rows[r] - 12 - oy, 36, 25), 0 });

    keys.push_back ({ juce::Rectangle<int> (544 - ox, 751 - oy, 57, 27), 1 });       // cancel
    keys.push_back ({ juce::Rectangle<int> (546 - ox, 782 - oy, 58, 27), 2 });       // clear
    keys.push_back ({ juce::Rectangle<int> (549 - ox, 814 - oy, 65, 60), 3 });       // enter
    setMouseCursor (juce::MouseCursor::PointingHandCursor);
}

int KeypadComponent::keyAt (juce::Point<int> p) const
{
    for (size_t i = 0; i < keys.size(); ++i)
        if (keys[i].r.contains (p))
            return (int) i;
    return -1;
}

void KeypadComponent::paint (juce::Graphics& g)
{
    auto draw = [&] (int i, float alpha, bool isPressed)
    {
        if (i < 0) return;
        auto r = keys[(size_t) i].r.toFloat();
        g.setColour (isPressed ? juce::Colours::black.withAlpha (0.32f) : juce::Colours::white.withAlpha (alpha));
        g.fillRoundedRectangle (r, 5.0f);
    };
    if (hot != pressed) draw (hot, 0.22f, false);
    draw (pressed, 0.0f, true);
}

void KeypadComponent::mouseMove (const juce::MouseEvent& e)
{
    const int k = keyAt (e.getPosition());
    if (k != hot) { hot = k; repaint(); }
}

void KeypadComponent::mouseDown (const juce::MouseEvent& e)
{
    pressed = keyAt (e.getPosition());
    repaint();
}

void KeypadComponent::mouseUp (const juce::MouseEvent& e)
{
    const int k = keyAt (e.getPosition());
    const int was = pressed;
    pressed = -1;
    repaint();

    if (k < 0 || k != was)
        return;

    switch (keys[(size_t) k].kind)
    {
        case 1: if (onCancel) onCancel(); break;
        case 2: if (onClear)  onClear();  break;
        case 3: if (onEnter)  onEnter();  break;
        default: break;
    }
}

//==============================================================================
// Light strip
void StatusLed::setState (bool isBusy, int tick)
{
    const float target = isBusy ? 0.5f + 0.5f * std::sin ((float) tick * 0.35f) : 0.0f;
    if (isBusy != busy || std::abs (target - level) > 0.02f)
    {
        busy = isBusy;
        level = target;
        repaint();
    }
}

void StatusLed::paint (juce::Graphics& g)
{
    if (level <= 0.0f)
        return;
    auto r = getLocalBounds().toFloat().reduced (0.0f, 3.0f);
    g.setColour (juce::Colour (0xffffb43a).withAlpha (0.25f * level));
    g.fillRoundedRectangle (getLocalBounds().toFloat(), 8.0f);
    g.setColour (juce::Colour (0xffffc860).withAlpha (0.35f + 0.6f * level));
    g.fillRoundedRectangle (r, 5.0f);
}

//==============================================================================
// MIDI output slot: the finished MIDI file sticks out of it like a printed ticket; drag it into the DAW.
// This component starts at (300, 930) in the picture.
static juce::Rectangle<float> ticketRect() { return { 100.0f, 92.0f, 230.0f, 78.0f }; }

bool MidiDragHandle::hitTest (int x, int y)
{
    return processor.getLastMidiFile().existsAsFile() && ticketRect().expanded (4.0f).contains ((float) x, (float) y);
}

void MidiDragHandle::paint (juce::Graphics& g)
{
    const auto file = processor.getLastMidiFile();
    if (! file.existsAsFile())
        return;

    // the slot's light turns green
    g.setColour (juce::Colour (0xff7ee08a).withAlpha (0.35f));
    g.fillRoundedRectangle (juce::Rectangle<float> (44.0f, 72.0f, 338.0f, 12.0f), 5.0f);
    g.setColour (juce::Colour (0xffb8ffc4));
    g.fillRoundedRectangle (juce::Rectangle<float> (50.0f, 75.0f, 326.0f, 5.0f), 2.5f);

    // the ticket
    const auto t = ticketRect();
    g.setColour (juce::Colours::black.withAlpha (0.35f));
    g.fillRoundedRectangle (t.translated (2.0f, 4.0f), 6.0f);
    g.setColour (juce::Colour (0xfff4f1e8));
    g.fillRoundedRectangle (t, 6.0f);
    g.setColour (juce::Colour (0xff9a968a));
    g.drawRoundedRectangle (t, 6.0f, 1.2f);

    g.setColour (juce::Colour (0xff6f6c63));
    for (int row = 0; row < 3; ++row)
        for (int col = 0; col < 2; ++col)
            g.fillEllipse (t.getX() + 10.0f + (float) col * 8.0f, t.getCentreY() - 11.0f + (float) row * 10.0f, 4.0f, 4.0f);

    g.setColour (juce::Colour (0xff1d1c18));
    g.setFont (juce::Font (juce::FontOptions (19.0f, juce::Font::bold)));
    g.drawText (file.getFileName(), t.withTrimmedLeft (34.0f).withTrimmedRight (8.0f).withHeight (t.getHeight() * 0.58f).toNearestInt(),
                juce::Justification::bottomLeft, true);
    g.setColour (juce::Colour (0xff6f6c63));
    g.setFont (juce::Font (juce::FontOptions (14.0f)));
    g.drawText (tr ("drag into your DAW"), t.withTrimmedLeft (34.0f).withTrimmedTop (t.getHeight() * 0.62f).toNearestInt(),
                juce::Justification::topLeft);
}

void MidiDragHandle::mouseDrag (const juce::MouseEvent& e)
{
    if (e.getDistanceFromDragStart() < 6)
        return;

    const auto file = processor.getLastMidiFile();
    if (! file.existsAsFile())
        return;

    juce::DragAndDropContainer::performExternalDragDropOfFiles ({ file.getFullPathName() }, false, this);
}

//==============================================================================
// Receipt
static juce::Font monoFont (float h, bool bold = false)
{
    return juce::Font (juce::FontOptions (juce::Font::getDefaultMonospacedFontName(), h, bold ? juce::Font::bold : juce::Font::plain));
}

static juce::String fitText (juce::String s, int maxChars)
{
    return s.length() > maxChars ? s.substring (0, maxChars - 1) + juce::String::charToString ((juce::juce_wchar) 0x2026) : s;
}

void ReceiptPaper::setData (const ReceiptData& d, bool print)
{
    data = d;
    if (print)
    {
        progress = 0.0f;
        stepTicks = 0;
    }
    setTooltip (tr ("Click to enlarge the receipt"));
    setInterceptsMouseClicks (d.valid, false);
    repaint();
}

void ReceiptPaper::clear()
{
    data = ReceiptData();
    progress = 1.0f;
    setInterceptsMouseClicks (false, false);
    repaint();
}

void ReceiptPaper::tick()
{
    if (! isPrinting())
        return;

    // the printer pushes the paper out in small jerks
    if (++stepTicks % 2 == 0)
    {
        progress = std::min (1.0f, progress + 0.055f);
        repaint();
    }
}

void ReceiptPaper::paint (juce::Graphics& g)
{
    if (! data.valid || progress <= 0.0f)
        return;

    const float slotY = 9.0f, paperX = 6.0f, paperW = 100.0f, stripH = 124.0f;
    const float bottom = slotY + progress * stripH;
    const float top = bottom - stripH;

    g.reduceClipRegion (0, (int) slotY, getWidth(), getHeight());          // everything above the slot is "inside the machine"

    juce::Path paper;
    paper.startNewSubPath (paperX, top);
    paper.lineTo (paperX + paperW, top);
    paper.lineTo (paperX + paperW, bottom - 4.0f);
    const int teeth = 14;
    for (int i = 0; i < teeth; ++i)
    {
        const float x1 = paperX + paperW - (i + 0.5f) * paperW / teeth, x2 = paperX + paperW - (i + 1.0f) * paperW / teeth;
        paper.lineTo (x1, bottom);
        paper.lineTo (x2, bottom - 4.0f);
    }
    paper.closeSubPath();

    g.setColour (juce::Colours::black.withAlpha (0.35f));
    g.fillPath (paper, juce::AffineTransform::translation (2.0f, 3.0f));
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xfffbf8ef), paperX, 0.0f, juce::Colour (0xffe3dfd2), paperX + paperW, 0.0f, false));
    g.fillPath (paper);

    g.saveState();
    g.reduceClipRegion (paper);

    auto line = [&] (const juce::String& l, const juce::String& r, float y, float fh, bool bold)
    {
        g.setFont (monoFont (fh, bold));
        g.setColour (juce::Colour (0xff2a2a2a));
        g.drawText (l, juce::Rectangle<float> (paperX + 4.0f, top + y, paperW - 8.0f, fh + 2.0f), juce::Justification::centredLeft, false);
        if (r.isNotEmpty())
            g.drawText (r, juce::Rectangle<float> (paperX + 4.0f, top + y, paperW - 8.0f, fh + 2.0f), juce::Justification::centredRight, false);
    };
    auto dashes = [&] (float y)
    {
        g.setColour (juce::Colour (0xff6a6a6a));
        for (float x = paperX + 4.0f; x < paperX + paperW - 4.0f; x += 4.0f)
            g.fillRect (x, top + y, 2.2f, 0.8f);
    };

    g.setFont (monoFont (14.0f, true));
    g.setColour (juce::Colour (0xff1a1a1a));
    g.drawText ("COTERIA", juce::Rectangle<float> (paperX, top + 4.0f, paperW, 15.0f), juce::Justification::centred, false);
    line ("ATM " + tr ("RECEIPT"), {}, 20.0f, 8.0f, false);
    dashes (31.0f);
    g.setFont (monoFont (9.5f, false));
    g.setColour (juce::Colour (0xff2a2a2a));
    g.drawText (fitText (data.track, 15), juce::Rectangle<float> (paperX + 4.0f, top + 35.0f, paperW - 8.0f, 12.0f), juce::Justification::centred, false);
    dashes (49.0f);
    line ("BPM", data.bpm > 0.0 ? juce::String (juce::roundToInt (data.bpm)) : juce::String ("--"), 53.0f, 9.5f, true);
    line (tr ("KEY"), data.key, 65.0f, 9.5f, true);
    line (tr ("NOTES"), juce::String (data.notes), 77.0f, 9.5f, true);
    line (tr ("LENGTH"), juce::String ((int) data.seconds / 60) + ":" + juce::String ((int) data.seconds % 60).paddedLeft ('0', 2), 89.0f, 9.5f, true);
    dashes (103.0f);

    // barcode from the track name
    uint32_t hsh = (uint32_t) data.track.hashCode();
    g.setColour (juce::Colour (0xff1a1a1a));
    float x = paperX + 8.0f;
    while (x < paperX + paperW - 9.0f)
    {
        hsh = hsh * 1664525u + 1013904223u;
        const float w = 0.8f + (float) ((hsh >> 24) % 3) * 0.7f;
        g.fillRect (x, top + 107.0f, w, 12.0f);
        x += w + 0.9f + (float) ((hsh >> 20) % 2) * 0.7f;
    }
    g.restoreState();
}

void ReceiptPaper::paintFull (juce::Graphics& g, juce::Rectangle<float> area, const ReceiptData& d)
{
    const float s = area.getWidth() / 300.0f;
    g.addTransform (juce::AffineTransform::scale (s).translated (area.getX(), area.getY()));

    const float W = 300.0f, H = area.getHeight() / s;
    // paper with serrated top and bottom
    juce::Path p;
    const int teeth = 25;
    p.startNewSubPath (0.0f, 8.0f);
    for (int i = 0; i < teeth; ++i) { p.lineTo ((i + 0.5f) * W / teeth, 0.0f); p.lineTo ((i + 1.0f) * W / teeth, 8.0f); }
    p.lineTo (W, H - 8.0f);
    for (int i = 0; i < teeth; ++i) { p.lineTo (W - (i + 0.5f) * W / teeth, H); p.lineTo (W - (i + 1.0f) * W / teeth, H - 8.0f); }
    p.closeSubPath();
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xfffbf8ef), 0.0f, 0.0f, juce::Colour (0xffe9e5d8), W, 0.0f, false));
    g.fillPath (p);
    g.reduceClipRegion (p);

    const auto ink = juce::Colour (0xff1c1c1c);
    float y = 26.0f;
    g.setColour (ink);
    g.setFont (monoFont (34.0f, true));
    g.drawText ("COTERIA", juce::Rectangle<float> (0.0f, y, W, 40.0f), juce::Justification::centred, false);
    y += 40.0f;
    g.setFont (monoFont (13.0f));
    g.drawText ("ATM  -  AUDIO TO MIDI", juce::Rectangle<float> (0.0f, y, W, 16.0f), juce::Justification::centred, false);
    y += 26.0f;

    auto dashes = [&]
    {
        g.setColour (juce::Colour (0xff7a7a7a));
        for (float x = 18.0f; x < W - 18.0f; x += 8.0f) g.fillRect (x, y, 4.5f, 1.4f);
        y += 16.0f;
    };
    auto row = [&] (const juce::String& l, const juce::String& r, bool bold = true)
    {
        g.setColour (ink);
        g.setFont (monoFont (16.0f, bold));
        g.drawText (l, juce::Rectangle<float> (18.0f, y, W - 36.0f, 20.0f), juce::Justification::centredLeft, false);
        g.drawText (r, juce::Rectangle<float> (18.0f, y, W - 36.0f, 20.0f), juce::Justification::centredRight, false);
        y += 26.0f;
    };

    dashes();
    row (tr ("TRACK"), {});
    g.setFont (monoFont (14.0f));
    g.setColour (ink);
    g.drawFittedText (d.track, juce::Rectangle<int> (18, (int) y - 8, (int) W - 36, 38), juce::Justification::topLeft, 2, 0.9f);
    y += 40.0f;
    row (tr ("DATE"), d.when.formatted ("%d.%m.%Y  %H:%M"), false);
    dashes();
    row ("BPM", d.bpm > 0.0 ? juce::String (juce::roundToInt (d.bpm)) : juce::String ("--"));
    row (tr ("KEY"), d.key);
    row (tr ("NOTES"), juce::String (d.notes));
    row (tr ("LENGTH"), juce::String ((int) d.seconds / 60) + ":" + juce::String ((int) d.seconds % 60).paddedLeft ('0', 2));
    dashes();

    g.setColour (ink);
    g.setFont (monoFont (20.0f, true));
    g.drawText (tr ("THANK YOU!"), juce::Rectangle<float> (0.0f, y, W, 26.0f), juce::Justification::centred, false);
    y += 40.0f;

    uint32_t hsh = (uint32_t) d.track.hashCode();
    float x = 30.0f;
    while (x < W - 32.0f)
    {
        hsh = hsh * 1664525u + 1013904223u;
        const float w = 1.6f + (float) ((hsh >> 24) % 3) * 1.4f;
        g.fillRect (x, y, w, 44.0f);
        x += w + 2.0f + (float) ((hsh >> 20) % 2) * 1.4f;
    }
    y += 52.0f;
    g.setFont (monoFont (11.0f));
    g.drawText ("COTERIA", juce::Rectangle<float> (0.0f, y, W, 14.0f), juce::Justification::centred, false);
}

juce::File ReceiptPaper::makePicture (const ReceiptData& d, const juce::File& folder)
{
    const int pxW = 720, pxH = 1100;
    juce::Image img (juce::Image::ARGB, pxW, pxH, true);
    {
        juce::Graphics g (img);
        paintFull (g, juce::Rectangle<float> (0.0f, 0.0f, (float) pxW, (float) pxH), d);
    }
    // crop to the paper: the layout is 300 x (1100 / 2.4) units
    const auto dir = folder;
    dir.createDirectory();
    auto name = juce::File::createLegalFileName (d.track).trim();
    if (name.isEmpty()) name = "track";
    const auto f = dir.getChildFile ("ATM receipt - " + name + ".png");
    f.deleteFile();
    juce::FileOutputStream out (f);
    if (out.openedOk())
    {
        out.setPosition (0);
        out.truncate();
        juce::PNGImageFormat().writeImageToStream (img, out);
    }
    return f;
}

void ReceiptPaper::mouseUp (const juce::MouseEvent& e)
{
    if (data.valid && e.mouseWasClicked() && onClick)
        onClick();
}

//==============================================================================
ReceiptOverlay::ReceiptOverlay()
{
    setInterceptsMouseClicks (true, true);
}

void ReceiptOverlay::show (const ReceiptData& d)
{
    data = d;
    saved = juce::File();
    setVisible (true);
    toFront (false);
    repaint();
}

juce::Rectangle<float> ReceiptOverlay::paperArea() const
{
    const float w = std::min (440.0f, (float) getWidth() - 60.0f);
    const float h = w * 1100.0f / 720.0f;
    return { ((float) getWidth() - w) * 0.5f, ((float) getHeight() - h) * 0.5f - 20.0f, w, h };
}

void ReceiptOverlay::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black.withAlpha (0.72f));
    const auto r = paperArea();
    g.setColour (juce::Colours::black.withAlpha (0.45f));
    g.fillRect (r.translated (4.0f, 6.0f));
    {
        juce::Graphics::ScopedSaveState ss (g);
        ReceiptPaper::paintFull (g, r, data);
    }
    g.setColour (juce::Colours::white.withAlpha (0.85f));
    g.setFont (juce::FontOptions (22.0f));
    g.drawText (saved.existsAsFile() ? tr ("Saved: ") + saved.getFileName() : tr ("Click the receipt to save it as a picture. Click outside to close."),
                juce::Rectangle<float> (0.0f, r.getBottom() + 14.0f, (float) getWidth(), 30.0f), juce::Justification::centred, false);
}

void ReceiptOverlay::mouseUp (const juce::MouseEvent& e)
{
    if (! e.mouseWasClicked())
        return;

    if (paperArea().contains (e.getPosition().toFloat()) && folder)
    {
        // saving instead of dragging: a dragged picture dropped onto a DAW grid gets stuck there as a clip
        const auto f = ReceiptPaper::makePicture (data, folder());
        if (f.existsAsFile())
        {
            saved = f;
            f.revealToUser();
            repaint();
        }
        return;
    }
    saved = juce::File();
    setVisible (false);
}

//==============================================================================
// History panel
HistoryPanel::HistoryPanel (AudioToMidiProcessor& p) : processor (p)
{
    title.setFont (juce::Font (juce::FontOptions (25.0f, juce::Font::bold)));
    addAndMakeVisible (title);

    hint.setFont (juce::Font (juce::FontOptions (16.0f)));
    addAndMakeVisible (hint);

    list.setRowHeight (54);
    list.setOutlineThickness (0);
    list.setColour (juce::ListBox::backgroundColourId, juce::Colours::transparentBlack);
    list.addMouseListener (this, true);
    addAndMakeVisible (list);

    addAndMakeVisible (folderButton);
    addAndMakeVisible (deleteButton);
    addAndMakeVisible (closeButton);

    folderButton.onClick = [this]
    {
        const int r = list.getSelectedRow();
        if (juce::isPositiveAndBelow (r, (int) items.size()))
            items[(size_t) r].file.revealToUser();
        else
            processor.getMidiFolder().revealToUser();
    };

    deleteButton.onClick = [this]
    {
        const int r = list.getSelectedRow();
        if (juce::isPositiveAndBelow (r, (int) items.size()))
        {
            items[(size_t) r].file.moveToTrash();
            refresh();
        }
    };

    closeButton.onClick = [this] { if (onClose) onClose(); };
    relocalise();
}

void HistoryPanel::refresh()
{
    items.clear();

    auto files = processor.getMidiFolder().findChildFiles (juce::File::findFiles, false, "*.mid");
    std::sort (files.begin(), files.end(), [] (const juce::File& a, const juce::File& b)
               { return a.getLastModificationTime() > b.getLastModificationTime(); });

    if (files.size() > 300)
        files.removeRange (300, files.size() - 300);

    for (auto& f : files)
    {
        int noteCount = 0;
        juce::FileInputStream in (f);
        juce::MidiFile mf;
        if (in.openedOk() && mf.readFrom (in))
            for (int t = 0; t < mf.getNumTracks(); ++t)
                if (auto* seq = mf.getTrack (t))
                    for (auto* ev : *seq)
                        if (ev->message.isNoteOn())
                            ++noteCount;

        items.push_back ({ f, f.getLastModificationTime().formatted ("%d.%m.%Y  %H:%M"), noteCount });
    }

    list.updateContent();
    list.repaint();
}

void HistoryPanel::paintListBoxItem (int row, juce::Graphics& g, int w, int h, bool selected)
{
    if (! juce::isPositiveAndBelow (row, (int) items.size()))
        return;

    const auto& pal = paletteOf (*this);
    auto r = juce::Rectangle<float> (0.0f, 1.0f, (float) w, (float) h - 2.0f).reduced (2.0f, 0.0f);

    if (selected)
    {
        g.setColour (pal.fill.withAlpha (0.35f));
        g.fillRoundedRectangle (r, 8.0f);
        g.setColour (pal.thumb);
        g.drawRoundedRectangle (r, 8.0f, 1.2f);
    }

    g.setColour (pal.text);
    g.setFont (juce::Font (juce::FontOptions (20.0f, juce::Font::bold)));
    g.drawText (items[(size_t) row].file.getFileName(), 14, 5, w - 28, 26, juce::Justification::centredLeft, true);

    g.setColour (pal.textDim);
    g.setFont (juce::Font (juce::FontOptions (15.0f)));
    g.drawText (items[(size_t) row].info + "   -   " + lang::notes (items[(size_t) row].noteCount), 14, 31, w - 28, 20, juce::Justification::centredLeft, true);
}

void HistoryPanel::listBoxItemDoubleClicked (int row, const juce::MouseEvent&)
{
    if (juce::isPositiveAndBelow (row, (int) items.size()))
        items[(size_t) row].file.revealToUser();
}

void HistoryPanel::mouseDrag (const juce::MouseEvent& e)
{
    if (e.getDistanceFromDragStart() < 8)
        return;

    const int r = list.getSelectedRow();
    if (juce::isPositiveAndBelow (r, (int) items.size()) && items[(size_t) r].file.existsAsFile())
        juce::DragAndDropContainer::performExternalDragDropOfFiles ({ items[(size_t) r].file.getFullPathName() }, false, this);
}

void HistoryPanel::paint (juce::Graphics& g)
{
    const auto& pal = paletteOf (*this);
    auto r = getLocalBounds().toFloat().reduced (1.0f);
    g.setColour (pal.panel.withAlpha (0.96f));
    g.fillRoundedRectangle (r, 14.0f);
    g.setColour (pal.panelBorder);
    g.drawRoundedRectangle (r, 14.0f, 1.8f);

    if (items.empty())
    {
        g.setColour (pal.textDim);
        g.setFont (juce::Font (juce::FontOptions (19.0f)));
        g.drawText (tr ("No MIDI files yet"), getLocalBounds().withTrimmedTop (60).withTrimmedBottom (60), juce::Justification::centred);
    }
}

void HistoryPanel::relocalise()
{
    title.setText (tr ("History"), juce::dontSendNotification);
    hint.setText (tr ("Drag a file into your DAW. Double-click shows it in the folder."), juce::dontSendNotification);
    folderButton.setButtonText (tr ("Open folder"));
    deleteButton.setButtonText (tr ("Delete"));
    closeButton.setButtonText (tr ("Close"));
    list.repaint();
    repaint();
}

void HistoryPanel::resized()
{
    const int W = getWidth(), H = getHeight();
    title.setBounds (18, 8, 240, 36);
    hint.setBounds (18, 42, W - 36, 22);
    list.setBounds (10, 68, W - 20, H - 68 - 54);

    const int y = H - 48;
    folderButton.setBounds (14, y, 150, 38);
    deleteButton.setBounds (172, y, 110, 38);
    closeButton.setBounds (W - 14 - 110, y, 110, 38);
}

//==============================================================================
// BPM / key readout
void KeyReadout::update (AudioToMidiProcessor& p)
{
    const int v = p.getNotesVersion();
    const double b = p.getTrackBpm();
    if (v == version && std::abs (b - bpm) < 0.01)
        return;
    if (v != version)
        showAlt = false;
    version = v;
    bpm = b;
    key = p.getKeyInfo();
    repaint();
}

void KeyReadout::paint (juce::Graphics& g)
{
    const auto& pal = paletteOf (*this);
    auto r = getLocalBounds().toFloat();

    juce::String keyText = "--";
    if (key.valid)
    {
        const int t = showAlt ? key.altTonic : key.tonic;
        const bool m = showAlt ? key.altMinor : key.minor;
        keyText = juce::String (a2m::keyName (t)) + tr (m ? " min" : " maj");
    }
    const juce::String bpmText = bpm > 0.0 ? juce::String (juce::roundToInt (bpm)) : juce::String ("--");

    const juce::String line = "BPM " + bpmText + "  " + juce::String::charToString ((juce::juce_wchar) 0xb7) + "  " + tr ("KEY") + " " + keyText;
    // shrink the font until the whole line fits the component (longest case: Russian "ТОН D# мин")
    float fh = 17.0f, tw = 0.0f;
    juce::Font font (juce::FontOptions (fh, juce::Font::bold));
    for (;; fh -= 0.5f)
    {
        font = juce::Font (juce::FontOptions (fh, juce::Font::bold));
        juce::GlyphArrangement ga;
        ga.addLineOfText (font, line, 0.0f, 0.0f);
        tw = std::ceil (ga.getBoundingBox (0, -1, true).getWidth());
        if (tw + 6.0f <= r.getWidth() || fh <= 9.0f)
            break;
    }

    auto top = r.removeFromTop (22.0f);
    g.setColour (pal.text);
    g.setFont (font);
    g.drawText (line, top.withWidth (tw + 4.0f), juce::Justification::centredLeft, false);

    // confidence bar: exactly as long as the text, right under it
    const juce::Rectangle<float> bar (0.0f, 22.0f, tw, 7.0f);
    g.setColour (pal.track);
    g.fillRoundedRectangle (bar, 3.5f);
    if (key.valid)
    {
        const auto col = key.level >= 3 ? juce::Colour (0xff4cc35a)
                       : key.level == 2 ? juce::Colour (0xffe8b923)
                                        : juce::Colour (0xffd9423b);
        g.setColour (col);
        g.fillRoundedRectangle (bar.withWidth (bar.getWidth() * (float) key.level / 3.0f), 3.5f);
    }
    g.setColour (pal.panelBorder);
    g.drawRoundedRectangle (bar, 3.5f, 1.0f);
}

//==============================================================================
// Advanced settings panel
AdvancedPanel::AdvancedPanel (AudioToMidiProcessor& p) : processor (p)
{
    title.setFont (juce::Font (juce::FontOptions (25.0f, juce::Font::bold)));
    addAndMakeVisible (title);

    addAndMakeVisible (folderButton);
    folderButton.onClick = [this] { chooseFolder(); };

    setupRow (rows[0], "Note sensitivity",  ids::noteSens,  [] (double v) { return juce::String (v, 2); });
    setupRow (rows[1], "Split sensitivity", ids::splitSens, [] (double v) { return juce::String (v, 2); });
    setupRow (rows[2], "Min note length",   ids::minNoteMs, [] (double v) { return juce::String ((int) std::lround (v)) + tr (" ms"); });
    setupRow (rows[3], "Min frequency",     ids::minFreq,   [] (double v) { return v < 1.0 ? tr ("Off")
                                                                                          : juce::String ((int) std::lround (v)) + tr (" Hz"); });
    setupRow (rows[4], "Max frequency",     ids::maxFreq,   [] (double v) { return juce::String ((int) std::lround (v)) + tr (" Hz"); });
    setupRow (rows[5], "Note-off hangover", ids::hangover,  [] (double v) { return juce::String ((int) std::lround (v)) + tr (" frames"); });

    setupToggle (inferToggle,  inferAtt,  "Infer Onsets", ids::inferOn);
    setupToggle (smoothToggle, smoothAtt, "Smoothing",    ids::smoothing);
    setupToggle (sfxToggle,    sfxAtt,    "ATM SFX", ids::sfx);

    addAndMakeVisible (playButton);
    addAndMakeVisible (resetButton);

    addAndMakeVisible (readout);
    addAndMakeVisible (updatesToggle);
    updatesToggle.setToggleState (processor.getCheckUpdates(), juce::dontSendNotification);
    updatesToggle.onClick = [this] { processor.setCheckUpdates (updatesToggle.getToggleState()); };

    playButton.onClick = [this] { processor.togglePlay(); refresh(); };
    resetButton.onClick = [this] { resetDefaults(); };
    relocalise();
}

void AdvancedPanel::relocalise()
{
    title.setText (tr ("Advanced Settings"), juce::dontSendNotification);
    folderButton.setButtonText (tr ("Set MIDI folder..."));
    for (auto& row : rows)
    {
        row.name.setText (tr (row.key), juce::dontSendNotification);
        row.value.setTooltip (tr ("Click to type a value"));
        if (row.slider.onValueChange)
            row.slider.onValueChange();
    }
    inferToggle.setButtonText (tr ("Infer Onsets"));
    smoothToggle.setButtonText (tr ("Smoothing"));
    sfxToggle.setButtonText ("ATM SFX");
    updatesToggle.setButtonText (tr ("Updates"));
    updatesToggle.setTooltip (tr ("Tell me when a new version of ATM is available"));
    resetButton.setButtonText (tr ("Reset to defaults"));
    readout.relocalise();
    refresh();
    repaint();
}

void AdvancedPanel::chooseFolder()
{
    chooser = std::make_unique<juce::FileChooser> (tr ("Choose the folder for generated MIDI files"), processor.getMidiFolder());
    chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectDirectories,
                          [this] (const juce::FileChooser& fc)
                          {
                              const auto f = fc.getResult();
                              if (f.isDirectory())
                              {
                                  processor.setMidiFolder (f);
                                  processor.requestReconvert();    // rewrite the .mid into the new folder
                              }
                          });
}

void AdvancedPanel::resetDefaults()
{
    for (auto* id : { ids::noteSens, ids::splitSens, ids::minNoteMs, ids::minFreq, ids::maxFreq,
                      ids::hangover, ids::inferOn, ids::smoothing, ids::sfx })
        if (auto* prm = processor.apvts.getParameter (id))
        {
            prm->beginChangeGesture();
            prm->setValueNotifyingHost (prm->getDefaultValue());
            prm->endChangeGesture();
        }
}

void AdvancedPanel::lookAndFeelChanged()
{
    const auto& pal = paletteOf (*this);
    for (auto& row : rows)
    {
        row.value.setColour (juce::Label::textColourId, pal.thumb);
        row.value.setColour (juce::Label::textWhenEditingColourId, pal.text);
        row.value.setColour (juce::Label::backgroundWhenEditingColourId, pal.panel);
        row.value.setColour (juce::Label::outlineWhenEditingColourId, pal.thumb);
    }
    repaint();
}

void AdvancedPanel::setupRow (Row& row, const juce::String& label, const char* paramId,
                              std::function<juce::String (double)> fmt)
{
    row.format = std::move (fmt);

    row.key = label;
    row.name.setText (tr (label), juce::dontSendNotification);
    row.name.setFont (juce::Font (juce::FontOptions (19.0f)));
    addAndMakeVisible (row.name);

    row.value.setFont (juce::Font (juce::FontOptions (18.0f, juce::Font::bold)));
    row.value.setJustificationType (juce::Justification::centredRight);
    addAndMakeVisible (row.value);

    row.slider.setSliderStyle (juce::Slider::LinearHorizontal);
    row.slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    row.slider.setPopupMenuEnabled (false);
    addAndMakeVisible (row.slider);

    row.attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processor.apvts, paramId, row.slider);

    auto* r = &row;

    // the number can also be typed: click the value, enter a number, press Enter
    row.value.setEditable (true, true, false);
    row.value.setMouseCursor (juce::MouseCursor::IBeamCursor);
    row.value.onEditorShow = [r]
    {
        if (auto* ed = r->value.getCurrentTextEditor())
        {
            auto t = r->value.getText().retainCharacters ("0123456789.");
            ed->setText (t, false);           // only the number, without the unit
            ed->selectAll();
        }
    };
    row.value.onTextChange = [r]
    {
        const auto txt = r->value.getText().trim();
        auto num = txt.replaceCharacter (',', '.').retainCharacters ("0123456789.-");
        if (num.containsAnyOf ("0123456789"))
            r->slider.setValue (juce::jlimit (r->slider.getMinimum(), r->slider.getMaximum(), num.getDoubleValue()),
                                juce::sendNotificationSync);
        else if (txt.equalsIgnoreCase ("off") || txt == tr ("Off"))
            r->slider.setValue (r->slider.getMinimum(), juce::sendNotificationSync);
        r->slider.onValueChange();            // always restore the formatted text
    };

    row.slider.onValueChange = [r] { r->value.setText (r->format (r->slider.getValue()), juce::dontSendNotification); };
    row.slider.onValueChange();
    row.slider.setDoubleClickReturnValue (true, row.slider.getValue());
}

void AdvancedPanel::setupToggle (juce::ToggleButton& t,
                                 std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>& att,
                                 const juce::String& label, const char* paramId)
{
    t.setButtonText (label);
    addAndMakeVisible (t);
    att = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (processor.apvts, paramId, t);
}

void AdvancedPanel::refresh()
{
    readout.update (processor);
    playButton.setButtonText (processor.isPlaying() ? tr ("Stop") : tr ("Play"));
}

void AdvancedPanel::paint (juce::Graphics& g)
{
    const auto& pal = paletteOf (*this);
    auto r = getLocalBounds().toFloat().reduced (1.0f);
    g.setColour (pal.panel.withAlpha (0.92f));
    g.fillRoundedRectangle (r, 14.0f);
    g.setColour (pal.panelBorder);
    g.drawRoundedRectangle (r, 14.0f, 1.8f);
}

void AdvancedPanel::resized()
{
    const int W = getWidth();

    title.setBounds (18, 6, 280, 36);
    folderButton.setBounds (W - 16 - 190, 6, 190, 36);

    const int centres[] = { 62, 97, 132, 167, 202, 237 };
    for (size_t i = 0; i < rows.size(); ++i)
    {
        rows[i].name.setBounds (18, centres[i] - 16, 184, 32);
        rows[i].slider.setBounds (206, centres[i] - 16, W - 206 - 128, 32);
        rows[i].value.setBounds (W - 18 - 108, centres[i] - 16, 108, 32);
    }

    inferToggle.setBounds (10, 259, 142, 30);
    smoothToggle.setBounds (152, 259, 130, 30);
    sfxToggle.setBounds (282, 259, 120, 30);
    updatesToggle.setBounds (402, 259, 116, 30);

    playButton.setBounds (16, 295, 130, 36);
    resetButton.setBounds (156, 295, 176, 36);
    readout.setBounds (342, 297, 176, 34);
}

//==============================================================================
// Main editor
AudioToMidiEditor::AudioToMidiEditor (AudioToMidiProcessor& p)
    : AudioProcessorEditor (&p), processor (p), roll (p), dragHandle (p), panel (p), history (p)
{
    laf.setDark (processor.isDarkTheme());
    setLookAndFeel (&laf);

    addAndMakeVisible (content);

    content.addAndMakeVisible (background);
    content.addAndMakeVisible (led);
    content.addAndMakeVisible (roll);
    content.addAndMakeVisible (signGlow);
    content.addAndMakeVisible (signLink);
    content.addAndMakeVisible (receiptPaper);
    content.addChildComponent (receiptOverlay);
    receiptOverlay.folder = [this] { return processor.getMidiFolder().getChildFile ("Receipts"); };
    receiptPaper.onClick = [this] { if (currentReceipt.valid) receiptOverlay.show (currentReceipt); };
    for (auto* k : { &keyOpen, &keyPlay, &keyHistory, &keySettings, &keySolo, &keyStart, &keyTheme, &keyReset, &keyTelegram, &keyNext, &keyLang, &keyTempo, &keySign })
        content.addAndMakeVisible (*k);
    content.addAndMakeVisible (receipt);
    content.addAndMakeVisible (card);
    content.addAndMakeVisible (keypad);
    content.addAndMakeVisible (dragHandle);
    content.addAndMakeVisible (panel);
    content.addAndMakeVisible (history);
    history.setVisible (false);
    applyPanelVisibility();
    history.onClose = [this] { showHistory (false); };

    keyLang.onClick = [this]
    {
        lang::set (! lang::isRu());
        relocalise();
    };

    keyOpen.onClick    = [this] { openChooser(); };
    keyPlay.onClick    = [this] { processor.togglePlay(); };
    keyHistory.onClick = [this] { toggleHistory(); };
    receipt.onClick    = [this] { toggleHistory(); };
    keyTheme.onClick   = [this] { applyTheme (! processor.isDarkTheme()); };
    keyTelegram.onClick = [] { juce::URL ("https://t.me/addlist/K6LKlIp6UWI4Y2Uy").launchInDefaultBrowser(); };
    signLink.onClick    = [] { juce::URL ("https://t.me/addlist/K6LKlIp6UWI4Y2Uy").launchInDefaultBrowser(); };
    keyNext.onClick = [this] { page2 = ! page2; applyKeyPage(); updateSoftLabels(); };
    keySolo.onClick = [this] { processor.cycleMode(); updateSoftLabels(); };
    keySettings.onClick = [this] { toggleSettings(); };
    keySign.onClick = [this] { processor.setSignPulse (! processor.getSignPulse()); updateSoftLabels(); };
    keyTempo.onClick = [this] { processor.cycleTempoMode(); updateSoftLabels(); };
    keyReset.onClick   = [this]
    {
        processor.clearAll();                    // empty the screen and the MIDI slot first ...
        if (history.isVisible())
            showHistory (false);
        panel.resetDefaults();                  // ... then the settings go back to their defaults
        updateSoftLabels();
    };
    keyStart.onClick   = [this]
    {
        if (auto* prm = processor.apvts.getParameter (ids::startZero))
        {
            prm->beginChangeGesture();
            prm->setValueNotifyingHost (prm->getValue() > 0.5f ? 0.0f : 1.0f);
            prm->endChangeGesture();
        }
    };
    card.onClick       = [this] { openChooser(); };

    keypad.onEnter  = [this] { processor.togglePlay(); };
    keypad.onCancel = [this] { if (processor.isPlaying()) processor.togglePlay(); };
    keypad.onClear  = [this] { showHistory (false); };

    roll.onEmptyClick = [this] { openChooser(); };
    updateSoftLabels();

    // freely resizable, the whole layout scales with the window
    const double aspect = (double) baseW / (double) baseH;
    setResizable (true, true);
    setResizeLimits (400, (int) (400 / aspect), 1400, (int) (1400 / aspect));
    getConstrainer()->setFixedAspectRatio (aspect);

    const auto stored = processor.getEditorSize();
    const int w = stored.x >= 400 ? juce::jmin (stored.x, 1400) : 640;
    setSize (w, (int) std::lround ((double) w / aspect));

    relocalise();
    processor.startUpdateCheck();

    startTimerHz (30);
    timerCallback();
}

AudioToMidiEditor::~AudioToMidiEditor()
{
    setLookAndFeel (nullptr);
}

void AudioToMidiEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);
}

void AudioToMidiEditor::resized()
{
    const float scale = (float) getWidth() / (float) baseW;
    content.setTransform (juce::AffineTransform::scale (scale));
    content.setBounds (0, 0, baseW, baseH);
    layoutContent();

    processor.setEditorSize (getWidth(), getHeight());
}

void AudioToMidiEditor::layoutContent()
{
    // everything is placed on the picture (1024 x 1536)
    background.setBounds (0, 0, baseW, baseH);
    led.setBounds (380, 238, 264, 20);
    roll.setBounds (350, 360, 328, 229);

    signGlow.setBounds (288, 36, 448, 160);
    receiptPaper.setBounds (274, 703, 112, 150);
    receiptOverlay.setBounds (0, 0, 1024, 1536);
    signLink.setBounds (300, 56, 424, 138);         // the blue panel of the sign

    // the keys beside the screen stand on an arc: their real positions in the picture (x, y of each 39 x 26 key)
    const juce::Point<int> leftKeys[]  = { { 286, 409 }, { 284, 444 }, { 280, 478 }, { 279, 512 }, { 277, 547 } };
    const juce::Point<int> rightKeys[] = { { 705, 409 }, { 706, 444 }, { 708, 478 }, { 709, 513 }, { 711, 547 } };
    keyOpen.setBounds    (juce::Rectangle<int> (39, 26).withPosition (leftKeys[0]));     // 1
    keyPlay.setBounds    (juce::Rectangle<int> (39, 26).withPosition (leftKeys[1]));     // 2
    keyHistory.setBounds (juce::Rectangle<int> (39, 26).withPosition (leftKeys[2]));     // 3
    keySettings.setBounds (juce::Rectangle<int> (39, 26).withPosition (leftKeys[3]));    // 4
    keySign.setBounds     (juce::Rectangle<int> (39, 26).withPosition (leftKeys[4]));    // 5 (page 2)
    // right column, page 1: START, SOLO, RESET, NEXT, TELEGRAM    page 2: THEME, -, -, BACK, TELEGRAM
    keyStart.setBounds    (juce::Rectangle<int> (39, 26).withPosition (rightKeys[0]));   // 6
    keyTheme.setBounds    (juce::Rectangle<int> (39, 26).withPosition (rightKeys[0]));   // 6 (page 2)
    keyLang.setBounds     (juce::Rectangle<int> (39, 26).withPosition (rightKeys[1]));   // 7 (page 2)
    keySolo.setBounds     (juce::Rectangle<int> (39, 26).withPosition (rightKeys[1]));   // 7
    keyReset.setBounds    (juce::Rectangle<int> (39, 26).withPosition (rightKeys[2]));   // 8
    keyTempo.setBounds    (juce::Rectangle<int> (39, 26).withPosition (rightKeys[2]));   // 8 (page 2)
    keyNext.setBounds     (juce::Rectangle<int> (39, 26).withPosition (rightKeys[3]));   // 9
    keyTelegram.setBounds (juce::Rectangle<int> (39, 26).withPosition (rightKeys[4]));   // 10
    applyKeyPage();

    receipt.setBounds (258, 648, 144, 100);
    card.setBounds (615, 646, 160, 170);
    keypad.setBounds (396, 736, 228, 148);
    dragHandle.setBounds (300, 930, 430, 170);

    panel.setBounds (250, 1130, 524, 340);          // centred in the cabinet (its middle is x = 512)
    history.setBounds (250, 1130, 524, 340);
}

void AudioToMidiEditor::applyTheme (bool dark)
{
    laf.setDark (dark);
    processor.setDarkTheme (dark);
    sendLookAndFeelChange();
    repaint();
}

void AudioToMidiEditor::toggleHistory()
{
    showHistory (! history.isVisible());
}

void AudioToMidiEditor::showHistory (bool show)
{
    if (show)
        history.refresh();
    history.setVisible (show);
    applyPanelVisibility();
    if (show)
        history.toFront (false);
}

// The settings are shown only once there is a result to tune (and the person has not hidden them)
void AudioToMidiEditor::applyPanelVisibility()
{
    panel.setVisible (settingsOn && hasResult && ! history.isVisible());
}

void AudioToMidiEditor::relocalise()
{
    keyOpen.setTooltip (tr ("Open an audio file"));
    keyPlay.setTooltip (tr ("Play / stop the preview"));
    keyHistory.setTooltip (tr ("History of your MIDI files"));
    keyTheme.setTooltip (tr ("Dark / light theme of the panels"));
    keySettings.setTooltip (tr ("Show / hide the Advanced Settings"));
    keyTelegram.setTooltip (tr ("COTERIA on Telegram"));
    signLink.setTooltip (tr ("COTERIA on Telegram"));
    keySolo.setTooltip (tr ("Note mode: Normal (default), Clean (fewer extra notes: vocals with a beat, stray notes) or More (more notes: live instruments, band recordings, clean synths). It only sets Note and Split sensitivity"));
    keySign.setTooltip (tr ("The ATM sign blinks in time with the MIDI: on / off"));
    keyTempo.setTooltip (tr ("Tempo of the track: press to switch between as detected, x2 and half. Use it when the detected tempo is twice or half the real one"));
    keyNext.setTooltip (tr ("Next / previous page of keys"));
    keyReset.setTooltip (tr ("Reset the settings to defaults"));
    keyStart.setTooltip (tr ("MIDI position: keep the position the notes have in the audio / start the first note at 0 of the grid"));
    keyLang.setTooltip (tr ("Interface language: English / Russian"));
    receipt.setTooltip (tr ("History of your MIDI files"));
    card.setTooltip (tr ("Insert an audio file (MP3 / WAV / FLAC), or drop it on the window"));
    panel.relocalise();
    history.relocalise();
    updateSoftLabels();
    roll.repaint();
    repaint();
}

void AudioToMidiEditor::applyKeyPage()
{
    keyStart.setVisible (! page2);
    keySolo.setVisible  (! page2);
    keyReset.setVisible (! page2);
    keyTheme.setVisible (page2);
    keyLang.setVisible (page2);
    keyTempo.setVisible (page2);
    keySign.setVisible (page2);
}

void AudioToMidiEditor::toggleSettings()
{
    if (history.isVisible())
    {
        settingsOn = true;              // from the history the key brings the settings back
        showHistory (false);
    }
    else
    {
        settingsOn = ! settingsOn;
        applyPanelVisibility();
    }
    updateSoftLabels();
}

static juce::String modeLabelFor (int mode)
{
    switch (mode)
    {
        case AudioToMidiProcessor::modeClean:  return tr ("MODE: CLEAN");
        case AudioToMidiProcessor::modeMore:   return tr ("MODE: MORE");
        case AudioToMidiProcessor::modeCustom: return tr ("MODE: CUSTOM");
        default:                               return tr ("MODE: NORMAL");
    }
}

void AudioToMidiEditor::updateSoftLabels()
{
    const bool startZero = processor.apvts.getRawParameterValue (ids::startZero)->load() > 0.5f;
    if (keyStart.lit != startZero)
    {
        keyStart.lit = startZero;
        keyStart.repaint();
    }
    const bool settingsLit = settingsOn && ! history.isVisible();
    if (keySettings.lit != settingsLit)
    {
        keySettings.lit = settingsLit;
        keySettings.repaint();
    }
    const bool pulse = processor.getSignPulse();
    if (keySign.lit != pulse)
    {
        keySign.lit = pulse;
        keySign.repaint();
    }
    const int modeNow = processor.getMode();
    if (modeNow != lastMode)
    {
        lastMode = modeNow;
        updateSoftLabels();                         // the sliders may also be moved by hand
    }
    const bool solo = modeNow != AudioToMidiProcessor::modeNormal;
    if (keySolo.lit != solo)
    {
        keySolo.lit = solo;
        keySolo.repaint();
    }
    const bool update = processor.isUpdateAvailable();
    if (keyTelegram.lit != update)
    {
        keyTelegram.lit = update;
        keyTelegram.repaint();
        keyTelegram.setTooltip (update ? tr ("New version ") + processor.getLatestVersion() + tr (" is available - open the COTERIA channel")
                                       : tr ("COTERIA on Telegram"));
    }
    const juce::String tg = update ? tr ("UPDATE!") : juce::String ("TELEGRAM");
    roll.setSoftLabels ({ tr ("OPEN"), tr (processor.isPlaying() ? "STOP" : "PLAY"), tr ("HISTORY"), tr ("SETTINGS"), page2 ? tr (processor.getSignPulse() ? "SIGN: ON" : "SIGN: OFF") : juce::String() },
                        page2 ? std::vector<juce::String> { tr ("THEME"), lang::isRu() ? juce::String::fromUTF8 ("ЯЗЫК: RU") : juce::String ("LANG: EN"), juce::String (processor.getTempoMode() == 1 ? "TEMPO x2" : processor.getTempoMode() == 2 ? "TEMPO /2" : "TEMPO x1"), tr ("BACK"), tg }
                              : std::vector<juce::String> { tr (startZero ? "START: 0" : "START: AUDIO"), tr ("CLEAN"), tr ("RESET"), tr ("NEXT"), tg });
}

void AudioToMidiEditor::updateReceipt()
{
    const int v = processor.getNotesVersion();
    if (v == printedVersion || processor.isBusy())
        return;
    printedVersion = v;

    const auto notes = processor.getNotesSnapshot();
    const bool nowHas = ! notes.empty();
    if (nowHas != hasResult)
    {
        hasResult = nowHas;
        applyPanelVisibility();
    }
    if (notes.empty())
    {
        printedTrack = {};
        currentReceipt = ReceiptData();
        receiptOverlay.setVisible (false);
        receiptPaper.clear();
        return;
    }

    ReceiptData d;
    d.valid = true;
    d.track = processor.getLastMidiFile().getFileNameWithoutExtension();
    d.bpm = processor.getTrackBpm();
    d.notes = (int) notes.size();
    d.seconds = processor.getTotalSeconds();
    d.when = juce::Time::getCurrentTime();
    const auto key = processor.getKeyInfo();
    d.key = key.valid ? juce::String (a2m::keyName (key.tonic)) + tr (key.minor ? " min" : " maj") : juce::String ("--");

    const bool print = d.track != printedTrack;           // a new track prints a new receipt; a settings change only updates it
    printedTrack = d.track;
    currentReceipt = d;
    receiptPaper.setData (d, print);
}

void AudioToMidiEditor::timerCallback()
{
    updateReceipt();
    receiptPaper.tick();
    roll.setStatus (processor.isResultStatus() ? juce::String() : processor.getStatusText());
    updateSoftLabels();

    panel.refresh();
    roll.refresh();

    ++tickCount;
    card.setState (processor.isBusy(), dragging, tickCount);
    roll.setBusyTick (processor.isBusy(), tickCount);
    {
        bool hover = roll.isMouseOver (true);
        for (auto* k : { &keyOpen, &keyPlay, &keyHistory, &keySettings, &keySolo, &keyStart, &keyTheme, &keyReset, &keyTelegram, &keyNext, &keyLang, &keyTempo, &keySign })
            hover = hover || (k->isVisible() && k->isMouseOver (true));
        roll.setHover (hover);
    }
    signGlow.setLevel (processor.getSignPulse() ? roll.getBeatLevel() : 0.0f);
    led.setState (processor.isBusy(), tickCount);

    const auto midi = processor.getLastMidiFile();
    if (midi != lastShownMidi)
    {
        lastShownMidi = midi;
        dragHandle.repaint();
        if (history.isVisible())
            history.refresh();
    }
}

//==============================================================================
// Opening / dropping audio
void AudioToMidiEditor::openChooser()
{
    chooser = std::make_unique<juce::FileChooser> (tr ("Choose an audio file"),
                                                   juce::File::getSpecialLocation (juce::File::userMusicDirectory),
                                                   "*.mp3;*.wav;*.flac;*.aif;*.aiff;*.ogg");

    chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                          [this] (const juce::FileChooser& fc)
                          {
                              const auto f = fc.getResult();
                              if (f.existsAsFile())
                                  processor.loadAudioFile (f);
                          });
}

static bool isAudioFile (const juce::File& f)
{
    return f.hasFileExtension ("mp3;wav;flac;aif;aiff;ogg");
}

bool AudioToMidiEditor::isInterestedInFileDrag (const juce::StringArray& files)
{
    for (auto& f : files)
        if (isAudioFile (juce::File (f)))
            return true;
    return false;
}

void AudioToMidiEditor::setHighlight (bool on)
{
    dragging = on;
    roll.setDragHighlight (on);
}

void AudioToMidiEditor::fileDragEnter (const juce::StringArray&, int, int) { setHighlight (true); }
void AudioToMidiEditor::fileDragExit (const juce::StringArray&)           { setHighlight (false); }

void AudioToMidiEditor::filesDropped (const juce::StringArray& files, int, int)
{
    setHighlight (false);

    for (auto& f : files)
        if (isAudioFile (juce::File (f)))
        {
            processor.loadAudioFile (juce::File (f));
            break;
        }
}
