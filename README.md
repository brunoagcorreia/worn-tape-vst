# TapeWorn

An audio effect plugin that makes clean audio sound like it was recorded on an old
cassette or VHS tape. Wow and flutter, tape saturation, bandwidth loss, random
dropouts, pops and hiss, all controllable in real time.

- Formats: AU and VST3 (macOS)
- Built with [JUCE](https://juce.com) and CMake
- The DSP lives in a plain C++ class (`TapeDSP.h`) with no JUCE dependency, so it
  can be reused in other plugin formats or platforms

## Build

    cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
    cmake --build build

The plugin is copied to `~/Library/Audio/Plug-Ins/` after a successful build.

## Status

Early work in progress. Planned: stutter/repetition effect, signal-dependent
behavior, parameter smoothing.
