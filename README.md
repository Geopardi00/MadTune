# MadTune

An Auto-Tune style pitch-correction plugin (VST3, Windows), built with JUCE and CMake.

## Status

Milestone 2: the plugin detects the pitch of the incoming voice and shows the
note, how many cents it is off, and which note of the chosen key and scale it
would be corrected to. Audio still passes through unchanged. Milestones:

1. Skeleton, parameters, scale snapping logic, Windows CI build
2. Pitch detection (YIN) with a pitch readout (this)
3. Pitch shifting (PSOLA) by a fixed amount
4. Snapping to the scale with retune speed and humanize
5. Custom UI with a pitch graph; optional MIDI target notes and formant control

## Getting the plugin

Every push builds on GitHub Actions. Open the latest **Build** run under the
Actions tab, download the `MadTune-VST3-Windows` artifact, unzip it and copy the
`MadTune.vst3` folder to `C:\Program Files\Common Files\VST3\`. In Reaper, run
Options > Preferences > Plug-ins > VST > Re-scan.

## Building locally (Windows)

Needs Visual Studio 2022 or newer (Desktop C++ workload) and CMake 3.22+. JUCE is
downloaded automatically.

```
cmake -S . -B build -A x64
cmake --build build --config Release
ctest --test-dir build -C Release
```

The plugin ends up in `build/MadTune_artefacts/Release/VST3/MadTune.vst3`.

## Layout

- `Source/PluginProcessor.*`: the JUCE plugin and its parameters
- `Source/dsp/`: plain C++ DSP code with no JUCE dependency, unit tested
- `tests/`: unit tests (`-DMADTUNE_BUILD_PLUGIN=OFF` builds only these)
