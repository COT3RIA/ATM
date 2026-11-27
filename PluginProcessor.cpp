#include "PluginProcessor.h"
#include "Lang.h"
#include <BinaryData.h>
#include "PluginEditor.h"

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout AudioToMidiProcessor::createLayout()
{
    using namespace juce;
    AudioProcessorValueTreeState::ParameterLayout l;

    l.add (std::make_unique<AudioParameterFloat> (ParameterID { ids::noteSens, 1 }, "Note sensitivity",
                                                  NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.70f));
    l.add (std::make_unique<AudioParameterFloat> (ParameterID { ids::splitSens, 1 }, "Split sensitivity",
                                                  NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.40f));
    l.add (std::make_unique<AudioParameterFloat> (ParameterID { ids::minNoteMs, 1 }, "Min note length",
                                                  NormalisableRange<float> (10.0f, 1000.0f, 1.0f), 130.0f));
    l.add (std::make_unique<AudioParameterFloat> (ParameterID { ids::minFreq, 1 }, "Min frequency",
                                                  NormalisableRange<float> (0.0f, 2000.0f, 1.0f), 0.0f));
    l.add (std::make_unique<AudioParameterFloat> (ParameterID { ids::maxFreq, 1 }, "Max frequency",
                                                  NormalisableRange<float> (500.0f, 13000.0f, 10.0f), 3000.0f));
    l.add (std::make_unique<AudioParameterInt> (ParameterID { ids::hangover, 1 }, "Note-off hangover", 0, 50, 11));
    l.add (std::make_unique<AudioParameterBool> (ParameterID { ids::inferOn, 1 }, "Infer Onsets", true));
    l.add (std::make_unique<AudioParameterBool> (ParameterID { ids::smoothing, 1 }, "Smoothing", true));
    l.add (std::make_unique<AudioParameterBool> (ParameterID { ids::sfx, 1 }, "ATM SFX", false));
    l.add (std::make_unique<AudioParameterBool> (ParameterID { ids::startZero, 1 }, "Start MIDI at 0", false));
    return l;
}

//==============================================================================
AudioToMidiProcessor::AudioToMidiProcessor()
    : AudioProcessor (BusesProperties()
                        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "STATE", createLayout())
{
    loadSfx();

    for (auto* id : { ids::noteSens, ids::splitSens, ids::minNoteMs, ids::minFreq,
                      ids::maxFreq, ids::hangover, ids::inferOn, ids::smoothing, ids::startZero })
        apvts.addParameterListener (id, this);

    midiFolderPath = juce::File::getSpecialLocation (juce::File::userDocumentsDirectory)
                         .getChildFile ("ATM").getFullPathName();

    startTimerHz (20);
}

void AudioToMidiProcessor::loadSfx()
{
#if ATM_HAS_SFX
    juce::WavAudioFormat wav;
    auto in = std::make_unique<juce::MemoryInputStream> (BinaryData::sfx_money_counter_wav,
                                                         (size_t) BinaryData::sfx_money_counter_wavSize, false);
    if (std::unique_ptr<juce::AudioFormatReader> r { wav.createReaderFor (in.release(), true) })
    {
        sfxBuffer.setSize ((int) r->numChannels, (int) r->lengthInSamples);
        r->read (&sfxBuffer, 0, (int) r->lengthInSamples, 0, true, true);
        sfxBuffer.applyGain (0.5f);                 // the source file is normalised to 0 dB: keep it polite
        sfxRate = r->sampleRate;
    }
#endif
}

AudioToMidiProcessor::~AudioToMidiProcessor()
{
    stopTimer();
    ++generation;                       // cancel running work
    pool.removeAllJobs (true, 5000);

    for (auto* id : { ids::noteSens, ids::splitSens, ids::minNoteMs, ids::minFreq,
                      ids::maxFreq, ids::hangover, ids::inferOn, ids::smoothing })
        apvts.removeParameterListener (id, this);
}

bool AudioToMidiProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto out = layouts.getMainOutputChannelSet();
    return (out == juce::AudioChannelSet::stereo() || out == juce::AudioChannelSet::mono())
           && layouts.getMainInputChannelSet() == out;
}

juce::AudioProcessorEditor* AudioToMidiProcessor::createEditor()
{
    return new AudioToMidiEditor (*this);
}

//==============================================================================
// Settings / status helpers
a2m::Settings AudioToMidiProcessor::currentSettings() const
{
    a2m::Settings s;
    s.noteSensitivity       = apvts.getRawParameterValue (ids::noteSens)->load();
    s.splitSensitivity      = apvts.getRawParameterValue (ids::splitSens)->load();
    s.minNoteLengthMs       = apvts.getRawParameterValue (ids::minNoteMs)->load();
    s.minFrequencyHz        = apvts.getRawParameterValue (ids::minFreq)->load();
    s.maxFrequencyHz        = apvts.getRawParameterValue (ids::maxFreq)->load();
    s.noteOffHangoverFrames = (int) std::lround (apvts.getRawParameterValue (ids::hangover)->load());
    s.inferOnsets           = apvts.getRawParameterValue (ids::inferOn)->load() > 0.5f;
    s.smoothing             = apvts.getRawParameterValue (ids::smoothing)->load() > 0.5f;
    return s;
}

void AudioToMidiProcessor::setStatus (const StatusInfo& s)
{
    const juce::ScopedLock sl (infoLock);
    statusInfo = s;
}

juce::String AudioToMidiProcessor::getStatusText() const
{
    StatusInfo s;
    {
        const juce::ScopedLock sl (infoLock);
        s = statusInfo;
    }
    using lang::tr;
    switch (s.kind)
    {
        case StatusInfo::drop:        return tr ("Drop an audio file (MP3 / WAV / FLAC) onto the window");
        case StatusInfo::loading:     return tr ("Loading ") + s.file + "...";
        case StatusInfo::reanalysing: return tr ("Re-analysing...");
        case StatusInfo::cannotRead:  return tr ("Could not read ") + s.file;
        case StatusInfo::analysing:   return tr ("Analysing ") + s.file + "...";
        case StatusInfo::tooShort:    return tr ("The audio is too short or silent");
        case StatusInfo::noFile:      return lang::notes (s.count) + " - " + tr ("could not write the MIDI file, check the MIDI folder");
        case StatusInfo::done:
            return lang::notes (s.count) + "  ->  " + s.file + "  |  "
                   + (s.bpm > 0.0 ? juce::String (s.bpm, 1).trimCharactersAtEnd ("0").trimCharactersAtEnd (".") + " BPM"
                                  : juce::String ("-- BPM"))
                   + (s.fromZero ? "  |  " + tr ("from 0") : juce::String());
    }
    return {};
}

juce::File AudioToMidiProcessor::getLastMidiFile() const
{
    const juce::ScopedLock sl (infoLock);
    return lastMidiFile;
}

//==============================================================================
// "Solo instrument" preset
static void setParamValue (juce::AudioProcessorValueTreeState& apvts, const char* id, float v)
{
    if (auto* p = apvts.getParameter (id))
        p->setValueNotifyingHost (p->convertTo0to1 (v));
}

int AudioToMidiProcessor::getMode() const
{
    const float n = apvts.getRawParameterValue (ids::noteSens)->load();
    const float s = apvts.getRawParameterValue (ids::splitSens)->load();
    auto is = [&] (float a, float b) { return std::abs (n - a) < 0.005f && std::abs (s - b) < 0.005f; };
    if (is (0.70f, 0.40f)) return modeNormal;
    if (is (0.60f, 0.30f)) return modeClean;
    if (is (0.75f, 0.50f)) return modeMore;
    if (is (0.50f, 0.30f)) return modeStrings;
    return modeCustom;
}

void AudioToMidiProcessor::cycleMode()
{
    // NORMAL -> CLEAN -> MORE -> STRINGS -> NORMAL; from custom values it goes back to NORMAL
    static const float note[]  = { 0.70f, 0.60f, 0.75f, 0.50f };
    static const float split[] = { 0.40f, 0.30f, 0.50f, 0.30f };
    const int m = getMode();
    const int next = m == modeCustom ? (int) modeNormal : (m + 1) % 4;
    setParamValue (apvts, ids::noteSens, note[next]);
    setParamValue (apvts, ids::splitSens, split[next]);
}

//==============================================================================
// Update notice
static juce::File settingsFile()
{
    return juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
               .getChildFile ("COTERIA").getChildFile ("ATM.settings");
}

static bool isNewerVersion (const juce::String& remote, const juce::String& local)
{
    auto a = juce::StringArray::fromTokens (remote.trim(), ".", "");
    auto b = juce::StringArray::fromTokens (local.trim(),  ".", "");
    for (int i = 0; i < juce::jmax (a.size(), b.size()); ++i)
    {
        const int x = i < a.size() ? a[i].getIntValue() : 0;
        const int y = i < b.size() ? b[i].getIntValue() : 0;
        if (x != y)
            return x > y;
    }
    return false;
}

bool AudioToMidiProcessor::getCheckUpdates() const
{
    juce::PropertiesFile::Options o;
    juce::PropertiesFile pf (settingsFile(), o);
    return pf.getBoolValue ("checkUpdates", true);
}

void AudioToMidiProcessor::setCheckUpdates (bool on)
{
    settingsFile().getParentDirectory().createDirectory();
    juce::PropertiesFile pf (settingsFile(), juce::PropertiesFile::Options());
    pf.setValue ("checkUpdates", on);
    pf.saveIfNeeded();
    if (! on)
        updateAvailable = false;
    else
        startUpdateCheck();
}

juce::String AudioToMidiProcessor::getLatestVersion() const
{
    const juce::ScopedLock sl (infoLock);
    return latestVersion;
}

void AudioToMidiProcessor::startUpdateCheck()
{
#ifdef ATM_UPDATE_URL
    static std::atomic<bool> running { false };
    if (! getCheckUpdates() || running.exchange (true))
        return;

    netPool.addJob ([this]
    {
        juce::String body;
        {
            auto opts = juce::URL::InputStreamOptions (juce::URL::ParameterHandling::inAddress)
                            .withConnectionTimeoutMs (4000);
            if (auto s = juce::URL (ATM_UPDATE_URL).createInputStream (opts))
                body = s->readEntireStreamAsString();
        }
        const auto json = juce::JSON::parse (body);
        const auto remote = json.getProperty ("version", {}).toString();
        if (remote.isNotEmpty() && isNewerVersion (remote, JucePlugin_VersionString))
        {
            { const juce::ScopedLock sl (infoLock); latestVersion = remote; }
            updateAvailable = true;
        }
        running = false;
    });
#endif
}

juce::File AudioToMidiProcessor::getBackgroundFile() const
{
    juce::PropertiesFile pf (settingsFile(), juce::PropertiesFile::Options());
    const auto p = pf.getValue ("background");
    return p.isEmpty() ? juce::File() : juce::File (p);
}

void AudioToMidiProcessor::setBackgroundFile (const juce::File& f)
{
    settingsFile().getParentDirectory().createDirectory();
    juce::PropertiesFile pf (settingsFile(), juce::PropertiesFile::Options());
    pf.setValue ("background", f.getFullPathName());
    pf.saveIfNeeded();
}

juce::File AudioToMidiProcessor::getMidiFolder() const
{
    const juce::ScopedLock sl (infoLock);
    return juce::File (midiFolderPath);
}

void AudioToMidiProcessor::setMidiFolder (const juce::File& folder)
{
    {
        const juce::ScopedLock sl (infoLock);
        midiFolderPath = folder.getFullPathName();
    }
    apvts.state.setProperty ("midiFolder", folder.getFullPathName(), nullptr);
}

bool AudioToMidiProcessor::hasAudio() const
{
    const juce::ScopedLock sl (dataLock);
    return spectrogram != nullptr;
}

std::vector<a2m::Note> AudioToMidiProcessor::getNotesSnapshot() const
{
    const juce::SpinLock::ScopedLockType sl (noteLock);
    return notes;
}

//==============================================================================
// UI state (kept in the plugin state so it survives closing the window and reloading the project)
bool AudioToMidiProcessor::isDarkTheme() const
{
    return (bool) apvts.state.getProperty ("darkTheme", true);
}

void AudioToMidiProcessor::setDarkTheme (bool dark)
{
    apvts.state.setProperty ("darkTheme", dark, nullptr);
}

bool AudioToMidiProcessor::getSignPulse() const
{
    return (bool) apvts.state.getProperty ("signPulse", true);
}

void AudioToMidiProcessor::setSignPulse (bool on)
{
    apvts.state.setProperty ("signPulse", on, nullptr);
}

bool AudioToMidiProcessor::isResultStatus() const
{
    const juce::ScopedLock sl (infoLock);
    return statusInfo.kind == StatusInfo::done;
}

int AudioToMidiProcessor::getEyeStyle() const
{
    return juce::jlimit (0, 1, (int) apvts.state.getProperty ("eyeStyle", 0));
}

void AudioToMidiProcessor::setEyeStyle (int style)
{
    apvts.state.setProperty ("eyeStyle", juce::jlimit (0, 1, style), nullptr);
}

juce::Point<int> AudioToMidiProcessor::getEditorSize() const
{
    return { (int) apvts.state.getProperty ("editorW2", 0), (int) apvts.state.getProperty ("editorH2", 0) };
}

void AudioToMidiProcessor::setEditorSize (int w, int h)
{
    apvts.state.setProperty ("editorW2", w, nullptr);
    apvts.state.setProperty ("editorH2", h, nullptr);
}

//==============================================================================
// Conversion
void AudioToMidiProcessor::loadAudioFile (const juce::File& file)
{
    startJob (file, true);
}

void AudioToMidiProcessor::clearAll()
{
    ++generation;                                // cancels a running conversion
    dirtyCountdown = 0;
    playing = false;
    stopFlush = true;

    {
        const juce::ScopedLock sl (dataLock);
        spectrogram.reset();
        audioFile = juce::File();
    }
    {
        const juce::SpinLock::ScopedLockType sl (noteLock);
        notes.clear();
    }
    {
        const juce::ScopedLock sl (infoLock);
        lastMidiFile = juce::File();
        keyInfo = {};
        statusInfo = StatusInfo();
    }
    totalSeconds = 0.0;
    detectedBpm = 0.0;
    tempoMode = 0;
    tempoFromHost = false;
    busy = false;
    ++notesVersion;
}

void AudioToMidiProcessor::cycleTempoMode()
{
    if (detectedBpm.load() < 40.0)
        return;
    tempoMode = (tempoMode.load() + 1) % 3;
    ++notesVersion;                                 // refresh the readout
    if (hasAudio() && ! busy.load())
        requestReconvert();                         // without a DAW tempo the file is written in the track tempo
}

void AudioToMidiProcessor::requestReconvert()
{
    if (hasAudio())
        startJob ({}, false);
}

void AudioToMidiProcessor::startJob (const juce::File& file, bool decode)
{
    const int gen = ++generation;                 // cancels whatever is running
    busy = true;
    {
        StatusInfo si;
        si.kind = decode ? StatusInfo::loading : StatusInfo::reanalysing;
        si.file = file.getFileName();
        setStatus (si);
    }

    pool.addJob ([this, file, decode, gen] { runConversion (file, decode, gen); });
}

void AudioToMidiProcessor::parameterChanged (const juce::String&, float)
{
    dirtyCountdown = 8;     // ~400 ms debounce, handled in timerCallback on the message thread
}

void AudioToMidiProcessor::timerCallback()
{
    int c = dirtyCountdown.load();
    if (c > 0)
    {
        dirtyCountdown = --c;
        if (c == 0)
            requestReconvert();
    }

    // the project tempo became known / changed: rewrite the .mid so it still lines up with the audio
    const double hb = hostBpm.load();
    if (hb >= 20.0 && std::abs (hb - midiBpm.load()) > 0.05 && hasAudio() && ! busy.load() && dirtyCountdown.load() == 0)
        requestReconvert();
}

std::shared_ptr<const AudioToMidiProcessor::AudioData>
AudioToMidiProcessor::decodeFile (const juce::File& file, const std::function<bool()>& cancelled)
{
    juce::AudioFormatManager fm;
    fm.registerBasicFormats();      // WAV, AIFF, FLAC, OGG and MP3 (JUCE_USE_MP3AUDIOFORMAT=1)

    std::unique_ptr<juce::AudioFormatReader> reader (fm.createReaderFor (file));
    if (reader == nullptr || reader->lengthInSamples <= 0 || reader->numChannels == 0)
        return nullptr;

    auto data = std::make_shared<AudioData>();
    data->sampleRate = reader->sampleRate;
    data->mono.resize ((size_t) reader->lengthInSamples);

    constexpr int chunk = 1 << 16;
    const int chans = (int) std::min<unsigned int> (reader->numChannels, 2u);
    juce::AudioBuffer<float> tmp (chans, chunk);

    for (juce::int64 pos = 0; pos < reader->lengthInSamples; pos += chunk)
    {
        if (cancelled())
            return nullptr;

        const int n = (int) std::min<juce::int64> (chunk, reader->lengthInSamples - pos);
        tmp.clear();
        reader->read (&tmp, 0, n, pos, true, chans > 1);

        for (int i = 0; i < n; ++i)
        {
            float v = 0.0f;
            for (int c = 0; c < chans; ++c)
                v += tmp.getSample (c, i);
            data->mono[(size_t) (pos + i)] = v / (float) chans;
        }
    }

    return data;
}

void AudioToMidiProcessor::runConversion (juce::File file, bool decode, int gen)
{
    auto cancelled = [this, gen] { return generation.load() != gen; };
    std::shared_ptr<const a2m::Spectrogram> spec;

    if (decode)
    {
        auto data = decodeFile (file, cancelled);
        if (cancelled())
            return;

        if (data == nullptr)
        {
            { StatusInfo si; si.kind = StatusInfo::cannotRead; si.file = file.getFileName(); setStatus (si); }
            busy = false;
            return;
        }

        const double seconds = (double) data->mono.size() / data->sampleRate;
        { StatusInfo si; si.kind = StatusInfo::analysing; si.file = file.getFileName(); setStatus (si); }

        std::shared_ptr<const a2m::Spectrogram> analysed = a2m::analyse (data->mono, data->sampleRate, cancelled);
        data.reset();                                   // free the raw audio, only the spectrogram is needed
        if (cancelled())
            return;

        if (analysed == nullptr || analysed->numFrames < 4)
        {
            { StatusInfo si; si.kind = StatusInfo::tooShort; setStatus (si); }
            busy = false;
            return;
        }

        spec = analysed;
        detectedBpm = a2m::estimateBpm (*analysed);
        tempoMode = 0;
        totalSeconds = seconds;
        const juce::ScopedLock sl (dataLock);
        spectrogram = spec;
        audioFile = file;
    }
    else
    {
        const juce::ScopedLock sl (dataLock);
        spec = spectrogram;
        file = audioFile;
    }

    if (spec == nullptr)
    {
        busy = false;
        return;
    }

    auto result = a2m::transcribe (*spec, currentSettings());
    if (cancelled())
        return;

    // optional: the first note starts exactly at the zero of the grid (instead of keeping the position it has in the audio)
    const bool startAtZero = apvts.getRawParameterValue (ids::startZero)->load() > 0.5f;
    if (startAtZero && ! result.empty())
    {
        double first = result.front().startSec;
        for (const auto& n : result)
            first = std::min (first, n.startSec);
        for (auto& n : result)
        {
            n.startSec -= first;
            n.endSec   -= first;
        }
    }

    const auto midiFile = writeMidi (result, file);
    const auto count = result.size();

    const auto key = a2m::estimateKey (result);

    {
        const juce::SpinLock::ScopedLockType sl (noteLock);
        notes.swap (result);
    }
    {
        const juce::ScopedLock sl (infoLock);
        keyInfo = key;
    }
    ++notesVersion;

    {
        const juce::ScopedLock sl (infoLock);
        lastMidiFile = midiFile;
    }

    {
        StatusInfo si;
        si.count = (int) count;
        if (midiFile == juce::File())
            si.kind = StatusInfo::noFile;
        else
        {
            si.kind = StatusInfo::done;
            si.file = midiFile.getFileName();
            si.bpm = getTrackBpm();
            si.hostTempo = true;
            si.fromZero = startAtZero;
        }
        setStatus (si);
    }

    if (decode && apvts.getRawParameterValue (ids::sfx)->load() > 0.5f)
        sfxRequested = true;

    busy = false;
}

juce::File AudioToMidiProcessor::writeMidi (const std::vector<a2m::Note>& n, const juce::File& sourceAudio)
{
    const auto folder = getMidiFolder();
    if (! folder.createDirectory().wasOk())
        return {};

    const auto out = folder.getChildFile (sourceAudio.getFileNameWithoutExtension() + ".mid");

    // The file is written in the tempo of the DAW project, so that the notes line up with the audio in time:
    // most DAWs place a MIDI file by its beats and ignore the tempo stored in the file.
    // Without a host tempo (standalone, or the host does not tell) the tempo detected in the audio is used.
    const double hb = hostBpm.load(), db = getTrackBpm();
    const bool fromHost = hb >= 20.0 && hb <= 400.0;
    const double bpm = fromHost ? hb : (db >= 40.0 && db <= 240.0 ? db : 120.0);
    tempoFromHost = fromHost;
    midiBpm = bpm;

    constexpr int ticksPerQuarter = 480;
    const double ticksPerSecond = ticksPerQuarter * bpm / 60.0;

    juce::MidiMessageSequence seq;
    seq.addEvent (juce::MidiMessage::tempoMetaEvent ((int) std::lround (60000000.0 / bpm)), 0.0);
    seq.addEvent (juce::MidiMessage::timeSignatureMetaEvent (4, 4), 0.0);

    for (const auto& note : n)
    {
        const double on  = std::round (note.startSec * ticksPerSecond);
        const double off = std::max (on + 1.0, std::round (note.endSec * ticksPerSecond));
        seq.addEvent (juce::MidiMessage::noteOn (1, note.pitch, (juce::uint8) note.velocity), on);
        seq.addEvent (juce::MidiMessage::noteOff (1, note.pitch), off);
    }
    seq.updateMatchedPairs();

    juce::MidiFile mf;
    mf.setTicksPerQuarterNote (ticksPerQuarter);
    mf.addTrack (seq);

    out.deleteFile();
    juce::FileOutputStream os (out);
    if (! os.openedOk())
        return {};

    mf.writeTo (os);
    os.flush();
    return out;
}

//==============================================================================
// Preview playback: a plain synth + MIDI out, so it also drives other instruments in the DAW.
void AudioToMidiProcessor::togglePlay()
{
    if (playing.load())
    {
        playing = false;
        stopFlush = true;
    }
    else
    {
        startRequested = true;
        playing = true;
    }
}

void AudioToMidiProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    // Effect: the channel's audio passes through untouched, the preview is mixed on top of it
    midi.clear();

    if (auto* ph = getPlayHead())
        if (auto pos = ph->getPosition())
            if (auto bpm = pos->getBpm())
                hostBpm = *bpm;

    const int n = buffer.getNumSamples();
    const double sr = getSampleRate();
    if (sr <= 0.0 || n <= 0)
        return;

    const int numCh = buffer.getNumChannels();

    if (startRequested.exchange (false))
        playPos = 0.0;

    if (stopFlush.exchange (false))
        midi.addEvent (juce::MidiMessage::allNotesOff (1), 0);

    if (playing.load())
    {
        const double t0 = playPos;
        const double t1 = t0 + (double) n / sr;

        const juce::SpinLock::ScopedTryLockType lock (noteLock);
        if (lock.isLocked())
        {
            for (const auto& nt : notes)                 // sorted by start time
            {
                if (nt.startSec >= t1)
                    break;
                if (nt.endSec < t0)
                    continue;

                if (nt.startSec >= t0)
                    midi.addEvent (juce::MidiMessage::noteOn (1, nt.pitch, (juce::uint8) nt.velocity),
                                   juce::jlimit (0, n - 1, (int) ((nt.startSec - t0) * sr)));

                if (nt.endSec >= t0 && nt.endSec < t1)
                    midi.addEvent (juce::MidiMessage::noteOff (1, nt.pitch),
                                   juce::jlimit (0, n - 1, (int) ((nt.endSec - t0) * sr)));

                const int i0 = std::max (0, (int) std::ceil ((nt.startSec - t0) * sr));
                const int i1 = std::min (n, (int) std::ceil ((nt.endSec - t0) * sr));
                if (i1 <= i0)
                    continue;

                const double freq = 440.0 * std::pow (2.0, (nt.pitch - 69) / 12.0);
                const float amp = 0.07f * (float) nt.velocity / 127.0f;     // headroom for many voices

                for (int i = i0; i < i1; ++i)
                {
                    const double t = t0 + (double) i / sr;
                    const float env = (float) std::min (1.0, (t - nt.startSec) * 200.0)
                                    * (float) std::min (1.0, (nt.endSec - t) * 50.0);
                    const double ph = juce::MathConstants<double>::twoPi * freq * t;
                    const float s = amp * env * (float) (std::sin (ph) + 0.25 * std::sin (2.0 * ph));

                    for (int c = 0; c < numCh; ++c)
                        buffer.getWritePointer (c)[i] += s;
                }
            }
        }

        playPos = t1;
        playPosUi = t1;

        if (t1 > totalSeconds.load() + 0.1)
        {
            playing = false;
            midi.addEvent (juce::MidiMessage::allNotesOff (1), n - 1);
        }
    }

    // --- "ATM SFX": the embedded money-counter sound, played once when a conversion finishes ---
    if (sfxRequested.exchange (false) && sfxBuffer.getNumSamples() > 1)
        sfxPos = 0.0;

    if (sfxPos >= 0.0)
    {
        const double step = sfxRate / sr;
        const int len = sfxBuffer.getNumSamples();
        const int srcCh = sfxBuffer.getNumChannels();
        for (int i = 0; i < n && sfxPos >= 0.0; ++i)
        {
            const int i0 = (int) sfxPos;
            const float fr = (float) (sfxPos - i0);
            const int i1 = std::min (i0 + 1, len - 1);
            for (int c = 0; c < numCh; ++c)
            {
                const auto* src = sfxBuffer.getReadPointer (std::min (c, srcCh - 1));
                buffer.getWritePointer (c)[i] += src[i0] + fr * (src[i1] - src[i0]);
            }
            sfxPos += step;
            if (sfxPos >= (double) (len - 1))
                sfxPos = -1.0;
        }
    }
}

//==============================================================================
void AudioToMidiProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    auto state = apvts.copyState();
    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, dest);
}

void AudioToMidiProcessor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size))
    {
        if (xml->hasTagName (apvts.state.getType()))
        {
            apvts.replaceState (juce::ValueTree::fromXml (*xml));

            const auto folder = apvts.state.getProperty ("midiFolder").toString();
            if (folder.isNotEmpty())
            {
                const juce::ScopedLock sl (infoLock);
                midiFolderPath = folder;
            }
        }
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AudioToMidiProcessor();
}
