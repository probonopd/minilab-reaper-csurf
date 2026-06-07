#pragma once

#include <string>

#include "minilab_reaper_surface.hpp"

namespace minilab {

class MiniLabReaperExtension {
 public:
  MiniLabReaperExtension() = default;

  void setAutoArm(bool enabled);
  void onTrackSelection(const std::string& label, const std::string& name);
  void onTrackListChange();
  void onVolumeChanged(double volume);
  void onPanChanged(double pan);
  void onMuteChanged(bool mute);
  void onSoloChanged(bool solo);
  void onTransportChanged(bool recording);

  DisplayState displayState() const;
  bool autoArmEnabled() const;

 private:
  MiniLabReaperSurface surface_;
  bool auto_arm_ = true;
};

}  // namespace minilab
