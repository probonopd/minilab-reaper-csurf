#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace minilab {

struct DisplayState {
  std::string line1;
  std::string line2;
  bool recording = false;
};

class MiniLabReaperSurface {
 public:
  MiniLabReaperSurface() = default;

  void setTrackContext(const std::string& trackLabel, const std::string& trackName);
  void setParameterContext(const std::string& parameterLabel, const std::string& parameterValue);
  void setParameterOverride(bool enabled);
  void setTransportContext(bool recording);

  DisplayState renderDisplay() const;

  std::vector<uint8_t> buildSysExConnect() const;
  std::vector<uint8_t> buildSysExDisconnect() const;
  std::vector<uint8_t> buildDisplayText(const std::string& line1, const std::string& line2) const;
  std::vector<uint8_t> buildPadLedInit() const;
  std::vector<uint8_t> buildPadRgb(const std::vector<std::array<uint8_t, 3>>& colors) const;
  std::vector<uint8_t> buildEncoderRing(uint8_t value) const;
  std::vector<uint8_t> buildFaderBar(uint8_t value) const;

 private:
  static std::vector<uint8_t> makeSysExPacket(const std::vector<uint8_t>& body);
  static std::vector<uint8_t> asciiTerminated(const std::string& text);

  std::string track_label_;
  std::string track_name_;
  std::string parameter_label_;
  std::string parameter_value_;
  bool parameter_override_ = false;
  bool recording_ = false;
};

}  // namespace minilab
