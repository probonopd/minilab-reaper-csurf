# SDK Integration Notes

This file captures the next implementation milestone for the REAPER native control-surface layer.

## Planned integration steps
1. Add the official REAPER SDK headers to the build environment.
2. Create a control-surface wrapper around the portable `MiniLabReaperSurface` model.
3. Route REAPER callbacks for track selection, volume, pan, solo, mute, and transport into the wrapper.
4. Reuse the portable SysEx builders for display/LED output once the SDK bridge is available.
