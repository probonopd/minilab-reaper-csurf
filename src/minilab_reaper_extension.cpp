#include "minilab_reaper_extension.hpp"

namespace minilab {

void MiniLabReaperExtension::setAutoArm(bool enabled) {
  auto_arm_ = enabled;
}

void MiniLabReaperExtension::onTrackSelection(const std::string& label, const std::string& name) {
  surface_.setTrackContext(label, name);
  surface_.setParameterOverride(false);
  if (auto_arm_) {
    // The actual REAPER callback would set rec-arm here.
  }
}

void MiniLabReaperExtension::onTrackListChange() {
  surface_.setParameterOverride(false);
}

void MiniLabReaperExtension::onVolumeChanged(double volume) {
  surface_.setParameterContext("Volume", std::to_string(volume));
  surface_.setParameterOverride(true);
}

void MiniLabReaperExtension::onPanChanged(double pan) {
  surface_.setParameterContext("Pan", std::to_string(pan));
  surface_.setParameterOverride(true);
}

void MiniLabReaperExtension::onMuteChanged(bool mute) {
  surface_.setParameterContext("Mute", mute ? "On" : "Off");
  surface_.setParameterOverride(true);
}

void MiniLabReaperExtension::onSoloChanged(bool solo) {
  surface_.setParameterContext("Solo", solo ? "On" : "Off");
  surface_.setParameterOverride(true);
}

void MiniLabReaperExtension::onTransportChanged(bool recording) {
  surface_.setTransportContext(recording);
}

DisplayState MiniLabReaperExtension::displayState() const {
  return surface_.renderDisplay();
}

bool MiniLabReaperExtension::autoArmEnabled() const {
  return auto_arm_;
}

}  // namespace minilab
