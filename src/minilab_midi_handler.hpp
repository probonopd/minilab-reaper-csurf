#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace minilab {

class MiniLabMidiHandler {
 public:
  void queueSysEx(const std::vector<uint8_t>& packet);
  bool hasPendingPackets() const;
  std::vector<uint8_t> popPacket();

  void setAutoArm(bool enabled);
  void handleControlChange(int cc, int value);
  void handleNoteOn(int note, int velocity);
  std::string describeControl(int cc) const;

 private:
  bool auto_arm_ = true;
  std::vector<std::vector<uint8_t>> pending_packets_;
};

}  // namespace minilab
