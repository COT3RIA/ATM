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
    void requestReconvert();                        // re-run with current settings on the analysed audio
    bool hasAudio() const;
    bool isBusy() const { return busy.load(); }

    juce::File getMidiFolder() const;
    void setMidiFolder (const juce::File& folder);
    juce::File getLastMidiFile() const;
    juce::String getStatusText() const;

    // --- Notes for the piano roll ---
    std::vector<a2m::Note> getNotesSnapshot() const;
    int getNotesVersion() const { return notesVersion.load(); }
    double getTotalSeconds() const { return totalSeconds.load(); }
    double getPlayPositionSeconds() const { return playPosUi.load(); }

    // --- Preview playback ---
    void togglePlay();
    bool isPlaying() const { return playing.load(); }

    // --- UI state, stored with the plugin state (message thread only) ---
    bool isDarkTheme() const;

    // update notice: once per session the plugin reads a tiny version file (see ATM_UPDATE_URL in CMakeLists.txt);
    // nothing is downloaded or installed, the user is only told that a newer version exists
    bool getCheckUpdates() const;
    void setCheckUpdates (bool);
    void startUpdateCheck();
    bool isUpdateAvailable() const noexcept     { return updateAvailable.load(); }
    juce::String getLatestVersion() const;
    void setDarkTheme (bool dark);
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
    void setStatus (const juce::String& text);

    // analysed audio (kept so that settings changes only need the fast second stage)
    mutable juce::CriticalSection dataLock;
    std::shared_ptr<const a2m::Spectrogram> spectrogram;
    juce::File audioFile;

    mutable juce::CriticalSection infoLock;
    juce::String statusText { "Drop an audio file (MP3 / WAV / FLAC) onto the window" };
    juce::File lastMidiFile;
    juce::String midiFolderPath;

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
