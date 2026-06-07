#pragma once

#include <string>
#include <vector>

#include "minilab_reaper_extension.hpp"
#include "reaper_plugin.h"

#undef min
#undef max

namespace minilab {

bool matchesMidiPortName(const char* name, const std::vector<std::string>& hints);
bool findMidiPortByHints(const std::vector<std::string>& hints, int* index_out, bool want_input);

template <typename TrackState>
void applyExclusiveTrackState(std::vector<TrackState>& tracks, MediaTrack* target, bool arm_target) {
  for (TrackState& track : tracks) {
    const bool is_target = (track.track == target);
    track.selected = is_target;
    track.armed = is_target && arm_target;
  }
}

class MiniLabReaperControlSurface : public IReaperControlSurface {
 public:
  MiniLabReaperControlSurface() = default;

  const char* GetTypeString() override;
  const char* GetDescString() override;
  const char* GetConfigString() override;

  void SetTrackListChange() override;
  void SetSurfaceVolume(MediaTrack* trackid, double volume) override;
  void SetSurfacePan(MediaTrack* trackid, double pan) override;
  void SetSurfaceMute(MediaTrack* trackid, bool mute) override;
  void SetSurfaceSelected(MediaTrack* trackid, bool selected) override;
  void SetSurfaceSolo(MediaTrack* trackid, bool solo) override;
  void SetSurfaceRecArm(MediaTrack* trackid, bool recarm) override;
  void SetPlayState(bool play, bool pause, bool rec) override;
  void SetRepeatState(bool rep) override;
  void SetTrackTitle(MediaTrack* trackid, const char* title) override;
  void OnTrackSelection(MediaTrack* trackid) override;
  void Run() override;

  DisplayState displayState() const;

 private:
  MiniLabReaperExtension extension_;
  MiniLabReaperSurface surface_;
};

}  // namespace minilab
