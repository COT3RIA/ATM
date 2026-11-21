# Third-party notices

## Basic Pitch (Spotify)
The note-detection neural network embedded in this plugin (`BasicPitchModel.h`) and the note decoding in
`PitchToMidi.h` are derived from Spotify's Basic Pitch.

* Source: https://github.com/spotify/basic-pitch
* Copyright 2022 Spotify AB
* Licensed under the Apache License, Version 2.0 (http://www.apache.org/licenses/LICENSE-2.0)

The network weights were converted offline into a compact C++ array and are evaluated by a small built-in
runtime (`BasicPitchNet.h`); no external inference library is needed. Basic Pitch is described in
Bittner et al., "A Lightweight Instrument-Agnostic Model for Polyphonic Note Transcription and Multipitch
Estimation", ICASSP 2022.

## JUCE
https://juce.com - dual licensed (AGPLv3 / commercial).
