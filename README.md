# MiniLab 3 REAPER control-surface baseline


Work-in-progress MiniLab 3 control-surface logic described in the spec:


## Showstopper

When the MiniLab 3 is **also** selected as a MIDI input in REAPER's preferences (so that notes are recorded to tracks), `CreateMIDIInput` on the same device index either fails or returns a handle that delivers no events. The result is that the plugin cannot see incoming CC messages (encoder turns, button presses) or SysEx from the hardware while REAPER owns the port.

## Build

```sh
cmake -S . -B build
cmake --build build
./build/minilab_reaper_surface_tests
sudo cp build/reaper_csurf_minilab.so /opt/REAPER/Plugins/reaper_csurf_minilab.so
```