# MiniLab 3 REAPER control-surface baseline

This workspace now contains a small C++ implementation and TDD test harness for the MiniLab 3 control-surface logic described in the spec:

- SysEx connect/disconnect helpers
- display text packet generation
- track/parameter/transport priority model
- pad RGB and encoder/fader packet builders

## Build

```sh
cmake -S . -B build
cmake --build build
./build/minilab_reaper_surface_tests
```

## Notes

The portable logic in `src/minilab_reaper_surface.cpp` is designed to be the core of a future REAPER native control-surface extension. The full REAPER SDK integration can be placed on top of this base once the official SDK headers are copied into the plugin build environment.

For live verification with the real MiniLab hardware, use the clean launcher in `devel/run_reaper_clean.sh`. It removes REAPER’s cached ALSA MIDI hardware files before launch, which avoids the stale `alsa_rawmidi: ... (-16)` busy-device errors that can block the input path when the cache is out of date.
