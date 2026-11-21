#include "PluginEditor.h"
#include <BinaryData.h>

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
        const float cy[] = { 64.0f, 95.5f, 127.0f, 160.0f, 193.0f };   // the keys' centres, relative to the screen
        g.setFont (juce::Font (juce::FontOptions (15.0f, juce::Font::bold)));
        auto pill = [&] (const juce::String& text, bool left, float y)
        {
            juce::GlyphArrangement ga;
            ga.addLineOfText (g.getCurrentFont(), text, 0.0f, 0.0f);
            const float tw = ga.getBoundingBox (0, -1, true).getWidth() + 14.0f;
            const float x = left ? 3.0f : bounds.getRight() - 3.0f - tw;
            const auto rr = juce::Rectangle<float> (x, y - 10.0f, tw, 20.0f);
            g.setColour (juce::Colour (0xff06225e).withAlpha (0.6f));
            g.fillRoundedRectangle (rr, 5.0f);
            g.setColour (ink.withAlpha (0.95f));
            g.drawText (text, rr, juce::Justification::centred);
        };
        for (size_t i = 0; i < leftLabels.size() && i < 5; ++i)
            if (leftLabels[i].isNotEmpty()) pill (leftLabels[i], true, cy[i]);
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
        g.setColour (ink.withAlpha (0.92f));
        g.strokePath (arrow, juce::PathStrokeType (5.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        g.setColour (ink);
        g.setFont (juce::Font (juce::FontOptions (26.0f, juce::Font::bold)));
        g.drawText ("Drop audio here", getLocalBounds().withTrimmedTop (getHeight() - 80).withHeight (34), juce::Justification::centred);
        g.setColour (ink.withAlpha (0.85f));
        g.setFont (juce::Font (juce::FontOptions (19.0f)));
        g.drawText ("MP3 / WAV / FLAC", getLocalBounds().withTrimmedTop (getHeight() - 46).withHeight (26), juce::Justification::centred);
    }

    if (busyNow)
    {
        // conversion: a bar of "bits" running across the screen
        const float cy = bounds.getCentreY() - 14.0f;
        g.setColour (ink.withAlpha (0.95f));
        g.setFont (juce::Font (juce::FontOptions (28.0f, juce::Font::bold)));
        g.drawText ("Converting...", getLocalBounds().withY ((int) cy - 52).withHeight (40), juce::Justification::centred);

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

    // the music-note badge beside "MIDI" (the one in the picture was blurred and crooked, it was removed from the picture)
    {
        const auto white = juce::Colour (0xfff4f4f4);
        g.setColour (juce::Colours::black.withAlpha (0.45f));
        g.drawRoundedRectangle (juce::Rectangle<float> (472.0f, 948.0f, 24.0f, 24.0f), 4.5f, 3.2f);
        g.setColour (white);
        g.drawRoundedRectangle (juce::Rectangle<float> (471.0f, 947.0f, 24.0f, 24.0f), 4.5f, 2.2f);

        juce::Path note;
        note.startNewSubPath (480.0f, 962.0f);      // left stem
        note.lineTo (480.0f, 953.0f);
        note.lineTo (489.5f, 951.0f);               // beam
        note.lineTo (489.5f, 960.0f);               // right stem
        g.strokePath (note, juce::PathStrokeType (1.9f, juce::PathStrokeType::mitered, juce::PathStrokeType::butt));
        g.fillRoundedRectangle (juce::Rectangle<float> (480.0f, 952.5f, 9.5f, 3.0f), 0.8f);
        g.fillEllipse (475.8f, 960.3f, 5.6f, 4.4f);
        g.fillEllipse (485.3f, 958.3f, 5.6f, 4.4f);
    }
}

//==============================================================================
// Sign glow
void SignGlow::paint (juce::Graphics& g)
{
    if (level <= 0.0f)
        return;

    // the sign's blue panel is (301, 49)-(722, 185) in the picture; this component starts at (288, 36)
    const auto sign = juce::Rectangle<float> (13.0f, 13.0f, 421.0f, 136.0f);
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
    const bool b = (tick / 8) % 2 == 0;
    if (isBusy != busy || isDrag != drag || (isBusy && b != blink))
    {
        busy = isBusy;
        drag = isDrag;
        blink = b;
        repaint();
    }
}

void CardReader::paintButton (juce::Graphics& g, bool highlighted, bool)
{
    // positions of the three green lights of the picture, relative to this component (it starts at 615, 646)
    const juce::Rectangle<float> lights[] = { { 28.0f, 43.0f, 98.0f, 10.0f }, { 50.0f, 62.0f, 58.0f, 14.0f }, { 53.0f, 114.0f, 52.0f, 16.0f } };

    if (busy)
    {
        const auto amber = blink ? juce::Colour (0xffffb43a) : juce::Colour (0xff8a5a10);
        for (auto l : lights)
        {
            g.setColour (amber.withAlpha (0.35f));
            g.fillRoundedRectangle (l.expanded (4.0f), 6.0f);
            g.setColour (amber.withAlpha (0.92f));
            g.fillRoundedRectangle (l, 4.0f);
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
    // component origin in the picture: (400, 740)
    const int cols[] = { 440, 480, 520 }, rows[] = { 763, 788, 816, 845 };
    for (int r = 0; r < 4; ++r)
        for (int cI = 0; cI < 3; ++cI)
            keys.push_back ({ juce::Rectangle<int> (cols[cI] - 18 - 400, rows[r] - 12 - 740, 36, 24), 0 });

    keys.push_back ({ juce::Rectangle<int> (545 - 400, 752 - 740, 52, 23), 1 });     // cancel
    keys.push_back ({ juce::Rectangle<int> (545 - 400, 779 - 740, 52, 22), 2 });     // clear
    keys.push_back ({ juce::Rectangle<int> (547 - 400, 804 - 740, 55, 53), 3 });     // enter
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
static juce::Rectangle<float> ticketRect() { return { 100.0f, 84.0f, 230.0f, 78.0f }; }

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
    g.fillRoundedRectangle (juce::Rectangle<float> (60.0f, 64.0f, 310.0f, 9.0f), 4.0f);
    g.setColour (juce::Colour (0xffb8ffc4));
    g.fillRoundedRectangle (juce::Rectangle<float> (64.0f, 67.0f, 302.0f, 3.0f), 1.5f);

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
    g.drawText ("drag into your DAW", t.withTrimmedLeft (34.0f).withTrimmedTop (t.getHeight() * 0.62f).toNearestInt(),
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
// History panel
HistoryPanel::HistoryPanel (AudioToMidiProcessor& p) : processor (p)
{
    title.setText ("History", juce::dontSendNotification);
    title.setFont (juce::Font (juce::FontOptions (25.0f, juce::Font::bold)));
    addAndMakeVisible (title);

    hint.setText ("Drag a file into your DAW. Double-click shows it in the folder.", juce::dontSendNotification);
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

        items.push_back ({ f, f.getLastModificationTime().formatted ("%d.%m.%Y  %H:%M")
                              + "   -   " + juce::String (noteCount) + " notes" });
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
    g.drawText (items[(size_t) row].info, 14, 31, w - 28, 20, juce::Justification::centredLeft, true);
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
        g.drawText ("No MIDI files yet", getLocalBounds().withTrimmedTop (60).withTrimmedBottom (60), juce::Justification::centred);
    }
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
    const double b = p.getMidiBpm();
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
        keyText = juce::String (a2m::keyName (t)) + (m ? " min" : " maj");
    }
    const juce::String bpmText = key.valid ? juce::String (juce::roundToInt (bpm)) : juce::String ("--");

    const juce::String line = "BPM " + bpmText + "  " + juce::String::charToString ((juce::juce_wchar) 0xb7) + "  KEY " + keyText;
    const juce::Font font (juce::FontOptions (17.0f, juce::Font::bold));
    juce::GlyphArrangement ga;
    ga.addLineOfText (font, line, 0.0f, 0.0f);
    const float tw = std::ceil (ga.getBoundingBox (0, -1, true).getWidth());

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
    title.setText ("Advanced Settings", juce::dontSendNotification);
    title.setFont (juce::Font (juce::FontOptions (25.0f, juce::Font::bold)));
    addAndMakeVisible (title);

    addAndMakeVisible (folderButton);
    folderButton.onClick = [this] { chooseFolder(); };

    setupRow (rows[0], "Note sensitivity",  ids::noteSens,  [] (double v) { return juce::String (v, 2); });
    setupRow (rows[1], "Split sensitivity", ids::splitSens, [] (double v) { return juce::String (v, 2); });
    setupRow (rows[2], "Min note length",   ids::minNoteMs, [] (double v) { return juce::String ((int) std::lround (v)) + " ms"; });
    setupRow (rows[3], "Min frequency",     ids::minFreq,   [] (double v) { return v < 1.0 ? juce::String ("Off")
                                                                                          : juce::String ((int) std::lround (v)) + " Hz"; });
    setupRow (rows[4], "Max frequency",     ids::maxFreq,   [] (double v) { return juce::String ((int) std::lround (v)) + " Hz"; });
    setupRow (rows[5], "Note-off hangover", ids::hangover,  [] (double v) { return juce::String ((int) std::lround (v)) + " frames"; });

    setupToggle (inferToggle,  inferAtt,  "Infer Onsets", ids::inferOn);
    setupToggle (smoothToggle, smoothAtt, "Smoothing",    ids::smoothing);
    setupToggle (sfxToggle,    sfxAtt,    "ATM SFX", ids::sfx);

    addAndMakeVisible (playButton);
    addAndMakeVisible (resetButton);

    addAndMakeVisible (readout);
    addAndMakeVisible (updatesToggle);
    updatesToggle.setToggleState (processor.getCheckUpdates(), juce::dontSendNotification);
    updatesToggle.setTooltip ("Tell me when a new version of ATM is available");
    updatesToggle.onClick = [this] { processor.setCheckUpdates (updatesToggle.getToggleState()); };

    playButton.onClick = [this] { processor.togglePlay(); refresh(); };
    resetButton.onClick = [this] { resetDefaults(); };
}

void AdvancedPanel::chooseFolder()
{
    chooser = std::make_unique<juce::FileChooser> ("Choose the folder for generated MIDI files", processor.getMidiFolder());
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

    row.name.setText (label, juce::dontSendNotification);
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
    row.value.setTooltip ("Click to type a value");
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
        else if (txt.equalsIgnoreCase ("off"))
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
    playButton.setButtonText (processor.isPlaying() ? juce::String ("Stop") : juce::String ("Play"));
}

void AdvancedPanel::paint (juce::Graphics& g)
{
    const auto& pal = paletteOf (*this);
    auto r = getLocalBounds().toFloat().reduced (1.0f);
    g.setColour (pal.panel.withAlpha (0.78f));
    g.fillRoundedRectangle (r, 14.0f);
    g.setColour (pal.panelBorder);
    g.drawRoundedRectangle (r, 14.0f, 1.8f);
}

void AdvancedPanel::resized()
{
    const int W = getWidth();

    title.setBounds (18, 8, 280, 36);
    folderButton.setBounds (W - 16 - 190, 8, 190, 36);

    const int centres[] = { 74, 112, 150, 188, 226, 264 };
    for (size_t i = 0; i < rows.size(); ++i)
    {
        rows[i].name.setBounds (18, centres[i] - 16, 184, 32);
        rows[i].slider.setBounds (206, centres[i] - 16, W - 206 - 128, 32);
        rows[i].value.setBounds (W - 18 - 108, centres[i] - 16, 108, 32);
    }

    inferToggle.setBounds (10, 286, 142, 30);
    smoothToggle.setBounds (152, 286, 130, 30);
    sfxToggle.setBounds (282, 286, 120, 30);
    updatesToggle.setBounds (402, 286, 116, 30);

    playButton.setBounds (16, 320, 130, 36);
    resetButton.setBounds (156, 320, 176, 36);
    readout.setBounds (342, 322, 176, 34);
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
    for (auto* k : { &keyOpen, &keyPlay, &keyHistory, &keySettings, &keyStart, &keyTheme, &keyReset, &keyTelegram })
        content.addAndMakeVisible (*k);
    content.addAndMakeVisible (receipt);
    content.addAndMakeVisible (card);
    content.addAndMakeVisible (keypad);
    content.addAndMakeVisible (dragHandle);
    content.addAndMakeVisible (panel);
    content.addAndMakeVisible (history);
    history.setVisible (false);
    history.onClose = [this] { showHistory (false); };

    keyOpen.setTooltip ("Open an audio file");
    keyPlay.setTooltip ("Play / stop the preview");
    keyHistory.setTooltip ("History of your MIDI files");
    keyTheme.setTooltip ("Dark / light theme of the panels");
    keySettings.setTooltip ("Show / hide the Advanced Settings");
    keyTelegram.setTooltip ("COTERIA on Telegram");
    signLink.setTooltip ("COTERIA on Telegram");
    keyReset.setTooltip ("Reset the settings to defaults");
    keyStart.setTooltip ("MIDI position: keep the position the notes have in the audio / start the first note at 0 of the grid");
    receipt.setTooltip ("History of your MIDI files");
    card.setTooltip ("Insert an audio file (MP3 / WAV / FLAC), or drop it on the window");

    keyOpen.onClick    = [this] { openChooser(); };
    keyPlay.onClick    = [this] { processor.togglePlay(); };
    keyHistory.onClick = [this] { toggleHistory(); };
    receipt.onClick    = [this] { toggleHistory(); };
    keyTheme.onClick   = [this] { applyTheme (! processor.isDarkTheme()); };
    keyTelegram.onClick = [] { juce::URL ("https://t.me/addlist/K6LKlIp6UWI4Y2Uy").launchInDefaultBrowser(); };
    signLink.onClick    = [] { juce::URL ("https://t.me/addlist/K6LKlIp6UWI4Y2Uy").launchInDefaultBrowser(); };
    keySettings.onClick = [this] { toggleSettings(); };
    keyReset.onClick   = [this] { panel.resetDefaults(); };
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
    led.setBounds (395, 243, 237, 18);
    roll.setBounds (348, 364, 331, 221);

    signGlow.setBounds (288, 36, 448, 160);
    signLink.setBounds (301, 49, 421, 136);         // the blue panel of the sign

    // the keys beside the screen stand on an arc: their real positions in the picture (x, y of each 39 x 26 key)
    const juce::Point<int> leftKeys[]  = { { 292, 416 }, { 290, 448 }, { 288, 479 }, { 287, 512 }, { 285, 544 } };
    const juce::Point<int> rightKeys[] = { { 696, 416 }, { 697, 448 }, { 699, 479 }, { 700, 512 }, { 702, 544 } };
    keyOpen.setBounds    (juce::Rectangle<int> (39, 26).withPosition (leftKeys[0]));     // 1
    keyPlay.setBounds    (juce::Rectangle<int> (39, 26).withPosition (leftKeys[1]));     // 2
    keyHistory.setBounds (juce::Rectangle<int> (39, 26).withPosition (leftKeys[2]));     // 3
    keySettings.setBounds (juce::Rectangle<int> (39, 26).withPosition (leftKeys[3]));    // 4
    keyStart.setBounds   (juce::Rectangle<int> (39, 26).withPosition (rightKeys[0]));    // 6
    keyTheme.setBounds   (juce::Rectangle<int> (39, 26).withPosition (rightKeys[1]));    // 7
    keyReset.setBounds   (juce::Rectangle<int> (39, 26).withPosition (rightKeys[2]));    // 8
    keyTelegram.setBounds (juce::Rectangle<int> (39, 26).withPosition (rightKeys[3]));   // 9

    receipt.setBounds (265, 650, 137, 98);
    card.setBounds (615, 646, 150, 148);
    keypad.setBounds (400, 740, 210, 125);
    dragHandle.setBounds (300, 930, 430, 170);

    panel.setBounds (250, 1128, 524, 362);          // centred in the cabinet (its middle is x = 512)
    history.setBounds (250, 1128, 524, 362);
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
    panel.setVisible (settingsOn && ! show);
    if (show)
        history.toFront (false);
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
        panel.setVisible (settingsOn);
    }
    updateSoftLabels();
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
    const bool update = processor.isUpdateAvailable();
    if (keyTelegram.lit != update)
    {
        keyTelegram.lit = update;
        keyTelegram.repaint();
        keyTelegram.setTooltip (update ? "New version " + processor.getLatestVersion() + " is available - open the COTERIA channel"
                                       : juce::String ("COTERIA on Telegram"));
    }
    roll.setSoftLabels ({ "OPEN", processor.isPlaying() ? "STOP" : "PLAY", "HISTORY", "SETTINGS", {} },
                        { startZero ? "START: 0" : "START: AUDIO", "THEME", "RESET", update ? "UPDATE!" : "TELEGRAM", {} });
}

void AudioToMidiEditor::timerCallback()
{
    roll.setStatus (processor.getStatusText());
    updateSoftLabels();

    panel.refresh();
    roll.refresh();

    ++tickCount;
    card.setState (processor.isBusy(), dragging, tickCount);
    roll.setBusyTick (processor.isBusy(), tickCount);
    signGlow.setLevel (roll.getBeatLevel());
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
    chooser = std::make_unique<juce::FileChooser> ("Choose an audio file",
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
