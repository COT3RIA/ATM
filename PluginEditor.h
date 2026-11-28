#pragma once

#include <array>
#include <functional>
#include <vector>
#include "PluginProcessor.h"
#include "Lang.h"

//==============================================================================
// Colours for the dark and the light theme
struct Palette
{
    bool dark = true;
    juce::Colour bg, panel, panelBorder, track, fill, thumb, button, outline, text, textDim,
                 noteFill, rollBg, grid, helmLight, helmMid, helmDark, helmEdge, bodyTop, bodyBot;

    static Palette make (bool dark);
};

class GreenLookAndFeel : public juce::LookAndFeel_V4
{
public:
    Palette pal;

    GreenLookAndFeel() { setDark (true); }
    void setDark (bool dark);

    int getSliderThumbRadius (juce::Slider&) override { return 10; }
    void drawLinearSlider (juce::Graphics&, int x, int y, int w, int h, float sliderPos, float minPos,
                           float maxPos, juce::Slider::SliderStyle, juce::Slider&) override;
    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour&, bool highlighted, bool down) override;
    void drawToggleButton (juce::Graphics&, juce::ToggleButton&, bool highlighted, bool down) override;
    juce::Font getTextButtonFont (juce::TextButton&, int buttonHeight) override;
    juce::Font getPopupMenuFont() override;
    float popupFontSize = 15.0f;        // follows the size of the plugin window
};

// The palette of whatever look-and-feel a component currently uses
const Palette& paletteOf (const juce::Component&);

//==============================================================================
// Small square buttons with a hand-drawn icon
class IconButton : public juce::Button
{
public:
    enum class Kind { Theme, Eyes };

    explicit IconButton (Kind k) : juce::Button ({}), kind (k) {}
    void paintButton (juce::Graphics&, bool highlighted, bool down) override;

    int eyeStyle = 0;           // only used by Kind::Eyes

private:
    Kind kind;
};

//==============================================================================
// Piano roll showing what the audio turned into
class NoteRoll : public juce::Component
{
public:
    explicit NoteRoll (AudioToMidiProcessor&);

    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseUp (const juce::MouseEvent& e) override;

    void refresh();                                 // polled by the editor timer
    void setBusyTick (bool busy, int tick);         // the conversion animation
    float getBeatLevel() const { return beat; }     // 0..1, pulses with the notes while the preview plays
    void setDragHighlight (bool b) { dragHighlight = b; repaint(); }
    void setStatus (const juce::String& s) { if (s != status) { status = s; repaint(); } }
    void setSoftLabels (std::vector<juce::String> left, std::vector<juce::String> right)
    {
        if (left != leftLabels || right != rightLabels)
        {
            leftLabels = std::move (left);
            rightLabels = std::move (right);
            repaint();
        }
    }

    void setHover (bool h) { hover = h; }           // the mouse is over the screen or one of its keys: show the key labels

    std::function<void()> onEmptyClick;

private:
    AudioToMidiProcessor& processor;
    bool hover = false;
    std::vector<a2m::Note> notes;
    int version = -1;
    double total = 0.0, playPos = -1.0;
    bool dragHighlight = false;
    std::vector<juce::String> leftLabels, rightLabels;
    juce::String status;
    bool busyNow = false;
    int busyTickNow = 0;
    float beat = 0.0f;
    float labelAlpha = 0.0f;                        // the key labels fade in once there is a conversion on the screen
};

//==============================================================================
// Drag this out of the plugin window into the DAW's timeline / a MIDI track
class MidiDragHandle : public juce::Component
{
public:
    explicit MidiDragHandle (AudioToMidiProcessor& p) : processor (p) {}

    void paint (juce::Graphics&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseEnter (const juce::MouseEvent&) override { setMouseCursor (juce::MouseCursor::DraggingHandCursor); }
    bool hitTest (int x, int y) override;

private:
    AudioToMidiProcessor& processor;
};

//==============================================================================
// The glow of the ATM sign: pulses with the music while the preview plays
class SignGlow : public juce::Component
{
public:
    SignGlow() { setInterceptsMouseClicks (false, false); }
    void paint (juce::Graphics&) override;
    void setLevel (float l) { if (std::abs (l - level) > 0.01f) { level = l; repaint(); } }

private:
    float level = 0.0f;
};

// The receipt printed by the RECEIPT slot after every new conversion. It can be saved as a picture.
struct ReceiptData
{
    bool valid = false;
    juce::String track, key;
    double bpm = 0.0, seconds = 0.0;
    int notes = 0;
    juce::Time when;
};

class ReceiptPaper : public juce::Component, public juce::SettableTooltipClient
{
public:
    ReceiptPaper() { setInterceptsMouseClicks (false, false); }

    void setData (const ReceiptData& d, bool print);     // print = start the animation
    void clear();
    void tick();                                         // 30 Hz
    bool isPrinting() const noexcept { return progress < 1.0f && data.valid; }
    void paint (juce::Graphics&) override;
    void mouseUp (const juce::MouseEvent&) override;
    std::function<void()> onClick;

    static void paintFull (juce::Graphics&, juce::Rectangle<float> area, const ReceiptData&);   // the shareable picture
    static juce::File makePicture (const ReceiptData&, const juce::File& folder);

private:
    ReceiptData data;
    float progress = 1.0f;                               // 0 = nothing out of the slot, 1 = fully out
    int stepTicks = 0;
};

// Enlarged receipt shown inside the plugin window (never outside it); drag the paper out to share the picture
class ReceiptOverlay : public juce::Component
{
public:
    ReceiptOverlay();
    void show (const ReceiptData&);
    void paint (juce::Graphics&) override;
    void mouseUp (const juce::MouseEvent&) override;
    std::function<juce::File()> folder;
private:
    juce::Rectangle<float> paperArea() const;
    ReceiptData data;
    juce::File saved;
};

// Invisible clickable area (the "ATM" sign): opens the COTERIA Telegram link
class SignLink : public juce::Component, public juce::SettableTooltipClient
{
public:
    SignLink() { setMouseCursor (juce::MouseCursor::PointingHandCursor); }
    void mouseUp (const juce::MouseEvent& e) override { if (contains (e.getPosition()) && onClick) onClick(); }
    std::function<void()> onClick;
};

//==============================================================================
// The ATM picture (background of the whole window)
class AtmBackground : public juce::Component
{
public:
    AtmBackground();
    void paint (juce::Graphics&) override;
    void useFile (const juce::File& f);          // user's own background behind the ATM; an empty / unreadable file gives the built-in one

private:
    juce::Image cabinet, backdrop, builtInBackdrop;
};

// A hardware key drawn in the picture: invisible, lights up under the mouse
class HwButton : public juce::Button
{
public:
    HwButton() : juce::Button ({}) { setMouseCursor (juce::MouseCursor::PointingHandCursor); }
    void paintButton (juce::Graphics&, bool highlighted, bool down) override;
    float radius = 6.0f;
    bool lit = false;                   // a mode that is switched on
};

// Small framed picture icon in the top right corner: opens the menu for the background behind the ATM
class SkinButton : public juce::Button
{
public:
    SkinButton() : juce::Button ({}) { setMouseCursor (juce::MouseCursor::PointingHandCursor); }
    void paintButton (juce::Graphics&, bool highlighted, bool down) override;
};

// The card reader: the audio file is "inserted" here (click, or drop a file on the window)
class CardReader : public juce::Button
{
public:
    CardReader() : juce::Button ({}) { setMouseCursor (juce::MouseCursor::PointingHandCursor); }
    void paintButton (juce::Graphics&, bool highlighted, bool down) override;
    void setState (bool isBusy, bool isDrag, int tick);

private:
    bool busy = false, drag = false;
    float phase = 0.0f;                              // seconds, drives the scan animation
};

// The numeric pad: ENTER = play / stop, CANCEL = stop, CLEAR = close the history
class KeypadComponent : public juce::Component
{
public:
    KeypadComponent();
    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;
    void mouseMove (const juce::MouseEvent&) override;
    void mouseExit (const juce::MouseEvent&) override { hot = -1; repaint(); }

    std::function<void()> onEnter, onCancel, onClear;

private:
    struct Key { juce::Rectangle<int> r; int kind; };      // kind: 0 digit, 1 cancel, 2 clear, 3 enter
    int keyAt (juce::Point<int>) const;

    std::vector<Key> keys;
    int pressed = -1, hot = -1;
};

// The light strip under the sign: pulses while a file is being converted
class StatusLed : public juce::Component
{
public:
    StatusLed() { setInterceptsMouseClicks (false, false); }
    void paint (juce::Graphics&) override;
    void setState (bool isBusy, int tick);

private:
    bool busy = false;
    float level = 0.0f;
};

//==============================================================================
// History: every MIDI file the plugin has written into the MIDI folder (newest first); drag them into the DAW
class HistoryPanel : public juce::Component, private juce::ListBoxModel
{
public:
    explicit HistoryPanel (AudioToMidiProcessor&);

    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseDrag (const juce::MouseEvent&) override;
    void refresh();
    void relocalise();

    std::function<void()> onClose;

private:
    struct Item { juce::File file; juce::String info; int noteCount = 0; };

    int getNumRows() override { return (int) items.size(); }
    void paintListBoxItem (int row, juce::Graphics&, int w, int h, bool selected) override;
    void listBoxItemDoubleClicked (int row, const juce::MouseEvent&) override;

    AudioToMidiProcessor& processor;
    std::vector<Item> items;
    juce::Label title, hint;
    juce::ListBox list { {}, this };
    juce::TextButton folderButton { "Open folder" }, deleteButton { "Delete" }, closeButton { "Close" };
};

//==============================================================================
// BPM / key readout with a confidence bar: 1/3 red, 2/3 amber, full green
class KeyReadout : public juce::Component, public juce::SettableTooltipClient
{
public:
    KeyReadout() { setMouseCursor (juce::MouseCursor::PointingHandCursor);
                   setTooltip ("Detected tempo and key. Click the tempo to type it in by hand, click the key to switch between the two most likely keys"); }
    void update (AudioToMidiProcessor&);
    void relocalise() { setTooltip (lang::tr ("Detected tempo and key. Click the tempo to type it in by hand, click the key to switch between the two most likely keys")); repaint(); }
    void paint (juce::Graphics&) override;
    void mouseUp (const juce::MouseEvent& e) override
    {
        if (! contains (e.getPosition()))
            return;
        if (e.x < bpmHitWidth)                                       // the tempo part: type it in by hand
        {
            if (onBpmClick) onBpmClick();
        }
        else if (key.valid)
        {
            showAlt = ! showAlt;
            repaint();
        }
    }
    std::function<void()> onBpmClick;

private:
    a2m::KeyResult key;
    double bpm = 0.0;
    int version = -1;
    bool showAlt = false;
    int bpmHitWidth = 0;
};

class AdvancedPanel : public juce::Component
{
public:
    explicit AdvancedPanel (AudioToMidiProcessor&);

    void paint (juce::Graphics&) override;
    void resized() override;
    void lookAndFeelChanged() override;
    void refresh();                          // update play button text
    void relocalise();                       // set all texts in the current language
    void chooseFolder();
    void askBpm();                           // type the tempo in by hand
    void resetDefaults();

private:
    struct Row
    {
        juce::Label name, value;
        juce::String key;
        juce::Slider slider;
        std::function<juce::String (double)> format;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    };

    void setupRow (Row&, const juce::String& label, const char* paramId, std::function<juce::String (double)> fmt);
    void setupToggle (juce::ToggleButton&, std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>&,
                      const juce::String& label, const char* paramId);

    AudioToMidiProcessor& processor;

    juce::Label title;
    juce::TextButton folderButton { "Set MIDI folder..." };
    std::array<Row, 6> rows;
    juce::ToggleButton inferToggle, smoothToggle, sfxToggle;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> inferAtt, smoothAtt, sfxAtt;
    juce::TextButton playButton { "Play" }, resetButton { "Reset to defaults" };
    juce::ToggleButton updatesToggle { "Updates" };
    KeyReadout readout;
    std::unique_ptr<juce::FileChooser> chooser;
};

//==============================================================================
class AudioToMidiEditor : public juce::AudioProcessorEditor,
                          public juce::FileDragAndDropTarget,
                          private juce::Timer
{
public:
    explicit AudioToMidiEditor (AudioToMidiProcessor&);
    ~AudioToMidiEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    bool isInterestedInFileDrag (const juce::StringArray& files) override;
    void filesDropped (const juce::StringArray& files, int, int) override;
    void fileDragEnter (const juce::StringArray&, int, int) override;
    void fileDragExit (const juce::StringArray&) override;

private:
    static constexpr int baseW = 1024, baseH = 1536;    // the layout is designed on the picture (1024 x 1536), then scaled

    void timerCallback() override;
    void layoutContent();
    void openChooser();
    void applyTheme (bool dark);
    void toggleHistory();
    void showHistory (bool);
    void toggleSettings();
    bool page2 = false;                                 // second page of the right-hand keys
    void applyKeyPage();
    void relocalise();
    bool settingsOn = true;                             // the Advanced Settings window is shown
    void setHighlight (bool on);
    void updateSoftLabels();
    int lastMode = -1;

    AudioToMidiProcessor& processor;
    GreenLookAndFeel laf;
    juce::TooltipWindow tooltip { this, 500 };

    juce::Component content;                            // everything lives here and is scaled as a whole
    AtmBackground background;
    SkinButton skinButton;
    void chooseBackground();                            // menu: pick a picture / back to the built-in one
    StatusLed led;
    SignGlow signGlow;
    SignLink signLink;
    ReceiptPaper receiptPaper;
    ReceiptOverlay receiptOverlay;
    ReceiptData currentReceipt;
    juce::String printedTrack;
    int printedVersion = -1;
    void updateReceipt();
    void applyPanelVisibility();
    bool hasResult = false;
    NoteRoll roll;                                      // the ATM screen
    HwButton keyOpen, keyPlay, keyHistory, keySettings, keySolo, keyStart, keyTheme, keyReset, keyTelegram, keyNext, keyLang, keyTempo, keySign;   // keys beside the screen (left 1-3, right 1, 4, 5)
    HwButton receipt;                                   // history
    CardReader card;
    KeypadComponent keypad;
    MidiDragHandle dragHandle;
    AdvancedPanel panel;
    HistoryPanel history;

    std::unique_ptr<juce::FileChooser> chooser;
    juce::File lastShownMidi;
    int tickCount = 0;
    bool dragging = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioToMidiEditor)
};
