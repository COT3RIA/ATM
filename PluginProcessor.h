#pragma once

#include <juce_audio_utils/juce_audio_utils.h>
#include <atomic>
#include <memory>
#include <vector>
#include "PitchToMidi.h"

namespace ids
{
inline constexpr const char* noteSens   = "noteSens";
inline constexpr const char* splitSens  = "splitSens";
inline constexpr const char* minNoteMs  = "minNoteMs";
inline constexpr const char* minFreq    = "minFreq";
inline constexpr const char* maxFreq    = "maxFreq";
inline constexpr const char* hangover   = "hangover";
inline constexpr const char* inferOn    = "inferOnsets";
inline constexpr const char* smoothing  = "smoothing";
inline constexpr const char* sfx        = "shredderSfx";
inline constexpr const char* startZero  = "startAtZero";
}

class AudioToMidiProcessor : public juce::AudioProcessor,
                             private juce::AudioProcessorValueTreeState::Listener,
                             private juce::Timer
{
public:
    struct AudioData
    {
        std::vector<float> mono;
        double sampleRate = 44100.0;
    };

    AudioToMidiProcessor();
    ~AudioToMidiProcessor() override;

    // --- AudioProcessor ---
    void prepareToPlay (double, int) override {}
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout&) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "ATM"; }
    bool acceptsMidi() const override  { return false; }
    bool producesMidi() const override { return true; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    // --- Conversion API (message thread) ---
    void loadAudioFile (const juce::File& file);   // decode + analyse + convert
    void clearAll();                                // forget the loaded audio, the notes and the MIDI file (the RESET key)
    void requestReconvert();                        // re-run with current settings on the analysed audio
    bool hasAudio() const;
    bool isBusy() const { return busy.load(); }

    juce::File getMidiFolder() const;
    juce::File getBackgroundFile() const;           // the user's own ATM picture (empty = built-in)
    void setBackgroundFile (const juce::File&);
    void setMidiFolder (const juce::File& folder);
    juce::File getLastMidiFile() const;
    juce::String getStatusText() const;           // formatted in the current interface language

    struct StatusInfo
    {
        enum Kind { drop, loading, reanalysing, cannotRead, analysing, tooShort, noFile, done } kind = drop;
        juce::String file;
        int count = 0;
        double bpm = 0.0;
        bool hostTempo = false, fromZero = false;
    };

    // --- Notes for the piano roll ---
    std::vector<a2m::Note> getNotesSnapshot() const;
    int getNotesVersion() const { return notesVersion.load(); }
    a2m::KeyResult getKeyInfo() const        { const juce::ScopedLock sl (infoLock); return keyInfo; }
    double getMidiBpm() const noexcept       { return midiBpm.load(); }
    // Tempo of the audio itself (what the track really runs at); 0 when it cannot be found (never the DAW project tempo)
    double getTrackBpm() const noexcept
    {
        const double d = detectedBpm.load();
        if (! (d >= 40.0 && d <= 240.0))
            return 0.0;
        const int m = tempoMode.load();
        const double t = m == 1 ? d * 2.0 : (m == 2 ? d * 0.5 : d);        // x2 / half-tempo correction chosen by the user
        return (t >= 40.0 && t <= 240.0) ? t : d;
    }
    int  getTempoMode() const noexcept       { return tempoMode.load(); }     // 0 = as detected, 1 = x2, 2 = /2
    void cycleTempoMode();
    double getTotalSeconds() const { return totalSeconds.load(); }
    double getPlayPositionSeconds() const { return playPosUi.load(); }

    // --- Preview playback ---
    void togglePlay();
    bool isPlaying() const { return playing.load(); }

    // --- UI state, stored with the plugin state (message thread only) ---
    bool isDarkTheme() const;

    // update notice: once per session the plugin reads a tiny version file (see ATM_UPDATE_URL in CMakeLists.txt);
    // nothing is downloaded or installed, the user is only told that a newer version exists
    // Note modes (one key cycles them): they only set Note / Split sensitivity.
    //   NORMAL 0.70 / 0.40 - the default, best on average
    //   CLEAN  0.60 / 0.30 - fewer extra notes: vocals with a beat, electronic music with stray notes
    //   MORE   0.75 / 0.50 - more notes: live instruments, band recordings, clean synths
    // The mode is read from the two sliders (CUSTOM when they sit on none of the presets); no extra state is stored.
    enum Mode { modeNormal = 0, modeClean, modeMore, modeStrings, modeCustom };
    int  getMode() const;
    void cycleMode();

    bool getCheckUpdates() const;
    void setCheckUpdates (bool);
    void startUpdateCheck();
    bool isUpdateAvailable() const noexcept     { return updateAvailable.load(); }
    juce::String getLatestVersion() const;
    void setDarkTheme (bool dark);
    bool getSignPulse() const;                      // the "ATM" sign blinks in time with the MIDI
    void setSignPulse (bool on);
    bool isResultStatus() const;                    // the status is the "N notes -> file" summary (not shown on the screen)
    int  getEyeStyle() const;                       // 0 = red orbs, 1 = follows the theme
    void setEyeStyle (int style);
    juce::Point<int> getEditorSize() const;         // {0,0} = not stored yet
    void setEditorSize (int w, int h);

    juce::AudioProcessorValueTreeState apvts;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

    void parameterChanged (const juce::String&, float) override;
    void timerCallback() override;

    a2m::Settings currentSettings() const;
    void startJob (const juce::File& file, bool decode);
    void runConversion (juce::File file, bool decode, int gen);
    std::shared_ptr<const AudioData> decodeFile (const juce::File& file, const std::function<bool()>& cancelled);
    juce::File writeMidi (const std::vector<a2m::Note>& notes, const juce::File& sourceAudio);
    void setStatus (const StatusInfo& s);

    // analysed audio (kept so that settings changes only need the fast second stage)
    mutable juce::CriticalSection dataLock;
    std::shared_ptr<const a2m::Spectrogram> spectrogram;
    juce::File audioFile;

    mutable juce::CriticalSection infoLock;
    StatusInfo statusInfo;
    juce::File lastMidiFile;
    juce::String midiFolderPath;
    a2m::KeyResult keyInfo;                    // key of the last transcription (guarded by infoLock)

    mutable juce::SpinLock noteLock;
    std::vector<a2m::Note> notes;
    std::atomic<int> notesVersion { 0 };
    std::atomic<double> totalSeconds { 0.0 };

    std::atomic<int>  generation { 0 };
    std::atomic<bool> busy { false };
    std::atomic<int>  dirtyCountdown { 0 };
    std::atomic<double> hostBpm { 0.0 };        // tempo of the DAW project (0 = unknown), set in processBlock
    std::atomic<double> detectedBpm { 0.0 };    // tempo found in the audio (0 = unknown), used when the DAW gives none
    std::atomic<bool>   tempoFromHost { false };
    std::atomic<int>    tempoMode { 0 };
    std::atomic<double> midiBpm { 120.0 };                     // tempo the current .mid was written with

    // preview playback (audio thread)
    std::atomic<bool> playing { false }, startRequested { false }, stopFlush { false };
    std::atomic<double> playPosUi { 0.0 };
    double playPos = 0.0;

    // "ATM SFX"
    std::atomic<bool> sfxRequested { false };
    juce::AudioBuffer<float> sfxBuffer;        // decoded sound (embedded WAV)
    double sfxRate = 44100.0;
    double sfxPos = -1.0;                      // position in sfxBuffer samples, < 0 = idle
    void loadSfx();

    std::atomic<bool> updateAvailable { false };
    juce::String latestVersion;
    juce::ThreadPool netPool { 1 };

    juce::ThreadPool pool { 1 };   // must stay the last member

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioToMidiProcessor)
};
