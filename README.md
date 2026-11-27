# ATM by COTERIA (VST3 effect)

**ATM** is an effect plugin (put it in a mixer insert slot, the channel audio passes through untouched) that converts MP3 / WAV / FLAC audio into a MIDI file you can drag straight into your DAW.
The engine is **polyphonic** (neural network, Basic Pitch): chords, bass and melody at once.

## How it works

1. Drop an audio file onto the window (or press the OPEN key / click the card reader).
2. The plugin analyses it in the background and writes `<name>.mid` into the MIDI folder
   (default: `Documents/ATM`, change it with "Set MIDI folder..." in the settings panel).
3. The piano roll under the settings shows what the audio turned into.
4. Grab the green **MIDI handle** at the bottom and drag it onto a track or timeline in your DAW.
5. Turn the sliders: the notes are recomputed ~0.4 s after you stop moving a slider (the slow
   spectrum analysis runs only once per file). "Play" previews the result.

## Interface

The window is an ATM (silver machine with the COTERIA graffiti panel, `assets/atm_full.png`); every part of it works:

* **Screen**: the piano roll with the notes and the status line. The "ATM" sign above it pulses softly in time
  with the music while it plays.
* **Keys beside the screen** (labels are shown on the screen). Left, top to bottom: OPEN, PLAY/STOP, HISTORY,
  SETTINGS (show / hide the Advanced Settings), (free). Right column, page 1: START ("AUDIO" keeps the notes where they
  are in the audio, "0" moves the first note to the start of the grid), MODE (note mode: NORMAL 0.70/0.40 is the default; CLEAN 0.60/0.30 gives fewer extra notes, good for vocals with a beat; MORE 0.75/0.50 gives more notes, good for live instruments and band recordings; STRINGS 0.50/0.30 gives the fewest extra notes, good for violin, orchestra and heavy distorted guitars; the key cycles them and only sets Note / Split sensitivity), SKIN (in the Advanced Settings panel: your own background behind the ATM, PNG or JPG 2:3 ideally 1024 x 1536, or back to the built-in graffiti wall), RESET (empties the screen and the MIDI slot and returns the settings to defaults), NEXT, TELEGRAM (opens
  the COTERIA channels). Page 2 (after NEXT): THEME (dark / light panels), left column key 5: SIGN (the "ATM" sign blinks in time with the MIDI: on / off), LANG (English / Russian interface), TEMPO (as detected / x2 / half: for tracks whose detected tempo is twice or half the real one), BACK, TELEGRAM.
  The MIDI folder is set with "Set MIDI folder..." in the settings panel. The "ATM" sign at the top is an invisible link to the same page.
* **Card reader**: insert the audio file here - click it or drop a file on the window. It blinks amber while converting.
* **Receipt printer**: opens the **History** of all MIDI files the plugin has written (drag them into the DAW).
  After every conversion of a new track the printer slot also pushes out a small paper **receipt** (track, BPM, key, notes, length). Click it to open it enlarged inside the plugin window; click the enlarged paper to save it as a PNG into the `Receipts` folder inside your MIDI folder (it is shown in the file manager), click next to it to close. The picture cannot be dragged out on purpose: DAWs such as FL Studio turn a dropped picture into a clip that is hard to remove. RESET removes it.
* **Numeric pad**: ENTER = play / stop, CANCEL = stop, CLEAR = close the history.
* **MIDI slot**: the finished file sticks out like a ticket - drag it into your DAW.
* **Lower cabinet**: the Advanced Settings. They appear only after the first conversion (and disappear again after RESET). Besides the slider, every value can be typed: click the number on the right, enter a value (e.g. 0.6 or 250), press Enter; values outside the range are clamped.
* **Resizable**: drag the bottom-right corner; the whole machine scales and the size is remembered.

## Tempo

The MIDI file is written in the tempo of your DAW project (read from the host), so the notes line up with the
audio in time; if you change the project tempo, the file is rewritten. When the host gives no tempo (standalone,
some hosts) the tempo is detected from the audio itself (shown in the status line as "BPM auto"; 120 BPM if it
cannot tell, e.g. for files shorter than ~7 s). Note positions in seconds are exact either way; only the bar grid
depends on the tempo.

## Settings

| Setting | What it does |
|---|---|
| Note sensitivity | Higher = quieter notes (relative to the loudest part) are kept |
| Split sensitivity | Higher = repeated notes of the same pitch are split more eagerly |
| Min note length | Notes shorter than this are dropped |
| Min / Max frequency | Pitch range that ends up in the MIDI (Min "Off" = no lower limit) |
| Note-off hangover | Gap (in frames) that is bridged before a note is closed |
| Infer Onsets | Splits repeated notes by their volume attack |
| Smoothing | Smooths the analysis over time, removes flicker |
| ATM SFX | Money-counter sound (assets/sfx_money_counter.wav) played when a conversion finishes |

Defaults: 0.70 / 0.40 / 130 ms / Off / 3000 Hz / 11 frames, Infer Onsets and Smoothing on.

## How the engine works

The note detection is based on Spotify's open-source **Basic Pitch** (Apache 2.0, see `THIRD_PARTY_NOTICES.md`):

1. The audio is resampled to 22.05 kHz and cut into overlapping windows of about 2 s.
2. A small built-in neural network (35k parameters, embedded in `BasicPitchModel.h`, run by `BasicPitchNet.h`,
   no external runtime) gives for every 11.6 ms frame the activation and the onset probability of each of the
   88 piano keys. Windows are processed in parallel on all CPU cores.
3. The notes are decoded from that output with the settings above (onset peaks, following the activation until
   it fades, then "melodia" tracking of notes that have no clear onset).

The slow part (network) runs once per file; moving a slider only repeats the fast decoding.

On the reference loop (trap piano) the result matches the Cymatics MIDI Shredder output with note F1 ~0.90.
Limits: the range is A0-C8, the model is trained on tonal instruments, so drums/noise are ignored and very dense
mixes can still miss notes.

## Installers

The GitHub workflow also produces ready installers (Artifacts: `ATM-Installer-Windows` -> `ATM-Setup-Windows.exe`,
`ATM-Installer-macOS` -> `ATM-Setup-macOS.pkg`). The Windows one copies the plugin to
`C:\Program Files\Common Files\VST3\ATM.vst3`, optionally installs the standalone app and removes the old
"Audio To MIDI.vst3". The installers are not code-signed, so Windows SmartScreen / macOS Gatekeeper may warn.

## Build

Requirements: CMake 3.22+, a C++17 compiler (Visual Studio 2022 / Xcode / GCC), internet access on the first
configure (JUCE is downloaded automatically).

```
cmake -B build
cmake --build build --config Release
```

The VST3 appears under `build/AudioToMidi_artefacts/Release/VST3/`. Or just push the project to GitHub:
the workflow in `.github/workflows/build.yml` builds Windows, macOS and Linux versions for you
(download them from the "Artifacts" section of the finished run).

`PitchToMidi.h` has no JUCE dependency, so the conversion engine can be tested or reused on its own.

## Licensing

JUCE is dual-licensed (AGPLv3 / commercial). Check the terms before distributing the plugin.
Basic Pitch (Spotify) is Apache 2.0; see `THIRD_PARTY_NOTICES.md`.

## Updates

The plugin can tell you that a newer version exists. Once per session it reads `latest.json` from the fixed address
`https://raw.githubusercontent.com/COT3RIA/ATM/main/latest.json` (set in `CMakeLists.txt`, the same for every build); if the version there is higher than the installed one, the
TELEGRAM key turns into a lit **UPDATE!** key that opens the COTERIA channel. Nothing is downloaded or installed
automatically. The check can be turned off with "Check updates" in the settings. That repository must stay public.

To publish a new version: raise `project(... VERSION x.y.z)` in `CMakeLists.txt` (and `AppVersion` in `ATM.iss`),
upload the new installer to your channel, then change the number in `latest.json`.

## Tempo and key readout

In the lower right of the settings panel the plugin shows the tempo of the audio track itself (detected from the sound, not the DAW project tempo; the MIDI file is still written in the project tempo) and the musical key
found in the notes (`BPM 100 · KEY C# min`). The bar under it shows how sure the plugin is: one third red = low,
two thirds yellow = medium, full green = high. Click the key to switch to the second most likely one (usually the
relative major / minor). The key is an estimate from the transcribed notes; short or atonal material gives a low confidence.

## Language

The whole interface is available in English and Russian (key LANG on page 2). The default follows the language of the
system; the choice is remembered in `COTERIA/ATM.settings` in the user's application-data folder. All texts are in `Lang.h`.
