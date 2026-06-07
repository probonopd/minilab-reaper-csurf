# MIDI Mapping Notes

This file records the target MiniLab 3 mapping plan for the next integration pass.

## Current mapping targets
- Main encoder: track banking / selection
- Encoder click: auto-arm / monitor toggle
- Faders: volume, pan, sends
- Pads: mute, solo, arm, monitor, assignable actions

## Output behavior
- Update SysEx display text whenever the selected track or parameter changes.
- Refresh pad RGB state when mute/solo/arm status changes.
- Use a small queue/debounce layer to keep output stable and non-blocking.
