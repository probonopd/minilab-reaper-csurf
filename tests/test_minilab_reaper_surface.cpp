#include <algorithm>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

#include "../src/minilab_midi_handler.hpp"
#include "../src/minilab_reaper_extension.hpp"
#include "../src/minilab_reaper_plugin.hpp"
#include "../src/minilab_reaper_surface.hpp"

static bool expect_eq(const std::string& label, const std::vector<uint8_t>& actual,
                      const std::vector<uint8_t>& expected) {
  if (actual == expected) {
    std::cout << "PASS: " << label << "\n";
    return true;
  }

  std::cout << "FAIL: " << label << "\n";
  std::cout << "  expected: ";
  for (unsigned char v : expected) std::cout << static_cast<int>(v) << ' ';
  std::cout << "\n  actual:   ";
  for (unsigned char v : actual) std::cout << static_cast<int>(v) << ' ';
  std::cout << "\n";
  return false;
}

int main() {
  using namespace minilab;

  MiniLabReaperSurface surface;

  const auto connect = surface.buildSysExConnect();
  const std::vector<uint8_t> expected_connect = {
      0xF0, 0x00, 0x20, 0x6B, 0x7F, 0x42,
      0x02, 0x02, 0x40, 0x6A, 0x21, 0xF7
  };

  bool ok = expect_eq("connect packet", connect, expected_connect);

  const auto display = surface.buildDisplayText("Track 07", "Lead Vocal");
  ok &= expect_eq("display packet header", std::vector<uint8_t>(display.begin(), display.begin() + 6),
                  std::vector<uint8_t>{0xF0, 0x00, 0x20, 0x6B, 0x7F, 0x42});

  surface.setTrackContext("Track 07", "Lead Vocal");
  surface.setParameterContext("Volume", "-6.3 dB");
  const auto state = surface.renderDisplay();
  ok &= (state.line1 == "Track 07" && state.line2 == "Lead Vocal");
  std::cout << (ok ? "PASS: priority model" : "FAIL: priority model") << "\n";

  surface.setTransportContext(true);
  const auto recState = surface.renderDisplay();
  ok &= (recState.line1 == "RECORDING" && recState.line2 == "Lead Vocal");
  std::cout << (ok ? "PASS: recording override" : "FAIL: recording override") << "\n";

  minilab::MiniLabReaperExtension extension;
  extension.onTrackSelection("Track 08", "Lead Vocal");
  extension.onVolumeChanged(-6.3);
  ok &= (extension.displayState().line1 == "Volume");
  std::cout << (ok ? "PASS: extension adapter" : "FAIL: extension adapter") << "\n";

  minilab::MiniLabMidiHandler midi;
  midi.queueSysEx({0xF0, 0x00, 0x7F});
  ok &= midi.hasPendingPackets();
  ok &= midi.describeControl(1) == "Track volume";
  std::cout << (ok ? "PASS: midi handler" : "FAIL: midi handler") << "\n";

  ok &= minilab::matchesMidiPortName("MiniLab3 MIDI", {"minilab3 midi"});
  ok &= minilab::matchesMidiPortName("MiniLab3 ALV", {"alv"});
  ok &= minilab::matchesMidiPortName("Arturia MiniLab 3 MIDI", {"arturia", "minilab"});
  std::cout << (ok ? "PASS: port matcher" : "FAIL: port matcher") << "\n";

  minilab::MiniLabReaperControlSurface wrapper;
  wrapper.SetSurfaceVolume(nullptr, -6.3);
  ok &= wrapper.displayState().line1 == "Volume";
  wrapper.SetTrackListChange();
  ok &= wrapper.displayState().line1 == "Track 07" || wrapper.displayState().line1 == "Volume";
  ok &= std::string(wrapper.GetTypeString()) == "MINILAB3";
  ok &= std::string(wrapper.GetDescString()).find("MiniLab 3") != std::string::npos;
  std::cout << (ok ? "PASS: REAPER wrapper" : "FAIL: REAPER wrapper") << "\n";

  return ok ? 0 : 1;
}
