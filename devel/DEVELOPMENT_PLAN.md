# MiniLab 3 REAPER Control Surface — Development Plan

## Goal
Create a REAPER-native control-surface extension for the Arturia MiniLab 3, reusing the portable SysEx/display baseline already present in this repository.

## Current Baseline
- Portable C++ model in `src/minilab_reaper_surface.hpp` and `src/minilab_reaper_surface.cpp`
- SysEx connect/disconnect helpers, display text packets, RGB pad, encoder ring, and fader bar builders
- TDD test harness in `tests/test_minilab_reaper_surface.cpp`
- CMake build path for fast verification

## Implementation Order
1. REAPER SDK integration scaffold
2. MIDI input/output adapter layer
3. Track selection and banking binding
4. Display update pipeline and parameter override handling
5. Pad, encoder, and fader feedback mapping
6. Transport and auto-arm workflow
7. Live hardware verification and release hardening
8. Runtime note: launch REAPER through `devel/run_reaper_clean.sh` to clear stale MIDI hardware cache entries before testing the MiniLab input path

## Scope
- Keep the portable model independent from the REAPER SDK to preserve unit testing
- Build the REAPER bridge on top of the current display/state logic
- Use the existing tests as the red/green verification baseline for every phase

## Verification
1. `cmake -S . -B build`
2. `cmake --build build`
3. `./build/minilab_reaper_surface_tests`
4. Add REAPER SDK integration tests once the SDK headers are available

## Deliverables
- `src/minilab_reaper_extension.hpp` / `.cpp` for the REAPER wrapper
- `src/minilab_midi_handler.hpp` / `.cpp` for MiniLab I/O mapping
- `devel/SDK_INTEGRATION_NOTES.md` and `devel/MIDI_MAPPING.md` for phase documentation
