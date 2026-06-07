#include "minilab_midi_handler.hpp"

namespace minilab {

void MiniLabMidiHandler::queueSysEx(const std::vector<uint8_t>& packet) {
  pending_packets_.push_back(packet);
}

bool MiniLabMidiHandler::hasPendingPackets() const {
  return !pending_packets_.empty();
}

std::vector<uint8_t> MiniLabMidiHandler::popPacket() {
  if (!hasPendingPackets()) return {};
  std::vector<uint8_t> packet = pending_packets_.front();
  pending_packets_.erase(pending_packets_.begin());
  return packet;
}

void MiniLabMidiHandler::setAutoArm(bool enabled) {
  auto_arm_ = enabled;
}

void MiniLabMidiHandler::handleControlChange(int cc, int value) {
  (void)cc;
  (void)value;
}

void MiniLabMidiHandler::handleNoteOn(int note, int velocity) {
  (void)note;
  (void)velocity;
}

std::string MiniLabMidiHandler::describeControl(int cc) const {
  switch (cc) {
    case 1: return "Track volume";
    case 2: return "Pan";
    case 3: return "Send 1";
    case 4: return "Send 2";
    default: return "Assignable parameter";
  }
}

}  // namespace minilab
