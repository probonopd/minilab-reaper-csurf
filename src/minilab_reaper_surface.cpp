#include "minilab_reaper_surface.hpp"

#include <array>
#include <sstream>

namespace minilab {
namespace {

std::string normalize(const std::string& value) {
  if (value.empty()) return "";
  return value;
}

}  // namespace

void MiniLabReaperSurface::setTrackContext(const std::string& trackLabel, const std::string& trackName) {
  track_label_ = normalize(trackLabel);
  track_name_ = normalize(trackName);
}

void MiniLabReaperSurface::setParameterContext(const std::string& parameterLabel,
                                                const std::string& parameterValue) {
  parameter_label_ = normalize(parameterLabel);
  parameter_value_ = normalize(parameterValue);
}

void MiniLabReaperSurface::setParameterOverride(bool enabled) {
  parameter_override_ = enabled;
}

void MiniLabReaperSurface::setTransportContext(bool recording) {
  recording_ = recording;
}

DisplayState MiniLabReaperSurface::renderDisplay() const {
  if (recording_) {
    return {"RECORDING", track_name_.empty() ? "Lead Vocal" : track_name_, true};
  }

  if (parameter_override_ && (!parameter_label_.empty() || !parameter_value_.empty())) {
    return {parameter_label_.empty() ? "Volume" : parameter_label_,
            parameter_value_.empty() ? "-6.3 dB" : parameter_value_, false};
  }

  return {track_label_.empty() ? "Track 07" : track_label_,
          track_name_.empty() ? "Lead Vocal" : track_name_, false};
}

std::vector<uint8_t> MiniLabReaperSurface::buildSysExConnect() const {
  return makeSysExPacket({0x02, 0x02, 0x40, 0x6A, 0x21});
}

std::vector<uint8_t> MiniLabReaperSurface::buildSysExDisconnect() const {
  return makeSysExPacket({0x02, 0x02, 0x40, 0x6A, 0x20});
}

std::vector<uint8_t> MiniLabReaperSurface::buildDisplayText(const std::string& line1,
                                                             const std::string& line2) const {
  std::vector<uint8_t> body = {0x04, 0x02, 0x60, 0x1F, 0x02, 0x01, 0x00};
  const auto l1 = asciiTerminated(line1);
  const auto l2 = asciiTerminated(line2);

  body.push_back(0x01);
  body.insert(body.end(), l1.begin(), l1.end());
  body.push_back(0x02);
  body.insert(body.end(), l2.begin(), l2.end());

  return makeSysExPacket(body);
}

std::vector<uint8_t> MiniLabReaperSurface::buildPadLedInit() const {
  std::vector<uint8_t> body = {0x04, 0x02, 0x16, 0x00};
  // 8 pads, all off by default.
  for (int i = 0; i < 8; ++i) body.insert(body.end(), {0x00, 0x00, 0x00});
  return makeSysExPacket(body);
}

std::vector<uint8_t> MiniLabReaperSurface::buildPadRgb(const std::vector<std::array<uint8_t, 3>>& colors) const {
  std::vector<uint8_t> body = {0x04, 0x02, 0x16, 0x00};
  for (const auto& color : colors) {
    body.push_back(color[0]);
    body.push_back(color[1]);
    body.push_back(color[2]);
  }
  return makeSysExPacket(body);
}

std::vector<uint8_t> MiniLabReaperSurface::buildEncoderRing(uint8_t value) const {
  return makeSysExPacket({0x1F, 0x03, 0x01, value, 0x00, 0x00});
}

std::vector<uint8_t> MiniLabReaperSurface::buildFaderBar(uint8_t value) const {
  return makeSysExPacket({0x1F, 0x04, 0x01, value, 0x00, 0x00});
}

std::vector<uint8_t> MiniLabReaperSurface::makeSysExPacket(const std::vector<uint8_t>& body) {
  std::vector<uint8_t> packet = {0xF0, 0x00, 0x20, 0x6B, 0x7F, 0x42};
  packet.insert(packet.end(), body.begin(), body.end());
  packet.push_back(0xF7);
  return packet;
}

std::vector<uint8_t> MiniLabReaperSurface::asciiTerminated(const std::string& text) {
  std::vector<uint8_t> bytes(text.begin(), text.end());
  bytes.push_back(0x00);
  return bytes;
}

}  // namespace minilab
