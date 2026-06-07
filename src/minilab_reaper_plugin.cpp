#include "minilab_reaper_plugin.hpp"

#include <cctype>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#define REAPERAPI_IMPLEMENT
#include "reaper_plugin_functions.h"

namespace minilab {

bool (*g_getTrackName)(MediaTrack* track, char* bufOut, int bufOut_sz) = nullptr;
midi_Input* g_midi_input = nullptr;
midi_Output* g_midi_output = nullptr;
midi_Output* g_midi_display_output = nullptr;
int g_midi_input_device = -1;
int g_midi_output_device = -1;
int g_midi_display_output_device = -1;

namespace {

std::string toLowerCopy(std::string value) {
  for (char& ch : value) {
    ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
  }
  return value;
}

bool containsIgnoreCase(const std::string& haystack, const std::string& needle) {
  if (needle.empty()) {
    return true;
  }

  const std::string hay = toLowerCopy(haystack);
  const std::string need = toLowerCopy(needle);
  return hay.find(need) != std::string::npos;
}

}  // namespace

bool matchesMidiPortName(const char* name, const std::vector<std::string>& hints) {
  if (!name) {
    return false;
  }

  const std::string candidate(name);
  for (const std::string& hint : hints) {
    if (containsIgnoreCase(candidate, hint)) {
      return true;
    }
  }
  return false;
}

std::string safeTrackName(MediaTrack* track) {
  if (!track || !g_getTrackName) {
    return "Lead Vocal";
  }

  char buffer[128] = {};
  if (g_getTrackName(track, buffer, sizeof(buffer))) {
    return buffer;
  }
  return "Lead Vocal";
}

void logMidiEvent(const char* direction, const uint8_t* data, int size) {
  if (!data || size <= 0) {
    return;
  }

  const auto emit = [&](FILE* stream) {
    std::fprintf(stream, "[minilab-midi] %s", direction);
    for (int i = 0; i < size; ++i) {
      std::fprintf(stream, " %02X", static_cast<unsigned int>(data[i]));
    }
    std::fprintf(stream, "\n");
    std::fflush(stream);
  };

  emit(stdout);
  emit(stderr);
}

void sendSysEx(const std::vector<uint8_t>& packet) {
  if (packet.empty()) {
    return;
  }

  midi_Output* target = g_midi_display_output ? g_midi_display_output : g_midi_output;
  if (!target) {
    return;
  }

  logMidiEvent("OUT", packet.data(), static_cast<int>(packet.size()));

  MIDI_event_t evt{};
  evt.frame_offset = -1;
  const int send_size = packet.size() < sizeof(evt.midi_message) ? static_cast<int>(packet.size()) : static_cast<int>(sizeof(evt.midi_message));
  evt.size = send_size;
  memcpy(evt.midi_message, packet.data(), static_cast<size_t>(evt.size));
  target->SendMsg(&evt, -1);
}

void refreshDisplay(MediaTrack* trackid) {
  if (!g_midi_output) {
    return;
  }

  MiniLabReaperSurface surface;
  const std::string name = safeTrackName(trackid);
  surface.setTrackContext("Track", name);
  surface.setParameterOverride(false);
  const auto state = surface.renderDisplay();
  sendSysEx(surface.buildDisplayText(state.line1, state.line2));
}

bool findMidiPort(const char* token, int* index_out, bool want_input) {
  if (!index_out) {
    return false;
  }

  const std::vector<std::string> hints = token ? std::vector<std::string>{token} : std::vector<std::string>{};
  return findMidiPortByHints(hints, index_out, want_input);
}

bool collectMidiPortCandidates(const std::vector<std::string>& hints, std::vector<int>* out_indices, bool want_input) {
  if (!out_indices) {
    return false;
  }

  out_indices->clear();
  const int count = want_input ? GetNumMIDIInputs() : GetNumMIDIOutputs();
  for (int i = 0; i < count; ++i) {
    char buf[256] = {};
    bool ok = false;

    if (want_input) {
      if (GetMIDIInputNameNoAlias && GetMIDIInputNameNoAlias(i, buf, sizeof(buf))) {
        ok = true;
      } else if (GetMIDIInputName) {
        ok = GetMIDIInputName(i, buf, sizeof(buf));
      }
    } else {
      if (GetMIDIOutputNameNoAlias && GetMIDIOutputNameNoAlias(i, buf, sizeof(buf))) {
        ok = true;
      } else if (GetMIDIOutputName) {
        ok = GetMIDIOutputName(i, buf, sizeof(buf));
      }
    }

    if (ok && matchesMidiPortName(buf, hints)) {
      out_indices->push_back(i);
    }
  }
  return !out_indices->empty();
}

bool findMidiPortByHints(const std::vector<std::string>& hints, int* index_out, bool want_input) {
  if (!index_out) {
    return false;
  }

  const int count = want_input ? GetNumMIDIInputs() : GetNumMIDIOutputs();
  const auto log = [&](const char* fmt, auto... args) {
    std::fprintf(stdout, fmt, args...);
    std::fflush(stdout);
    std::fprintf(stderr, fmt, args...);
    std::fflush(stderr);
  };

  log("[minilab-midi] scanning %s ports (count=%d)\n", want_input ? "input" : "output", count);

  for (int i = 0; i < count; ++i) {
    char buf[256] = {};
    bool ok = false;

    if (want_input) {
      if (GetMIDIInputNameNoAlias && GetMIDIInputNameNoAlias(i, buf, sizeof(buf))) {
        ok = true;
      } else if (GetMIDIInputName) {
        ok = GetMIDIInputName(i, buf, sizeof(buf));
      }
    } else {
      if (GetMIDIOutputNameNoAlias && GetMIDIOutputNameNoAlias(i, buf, sizeof(buf))) {
        ok = true;
      } else if (GetMIDIOutputName) {
        ok = GetMIDIOutputName(i, buf, sizeof(buf));
      }
    }

    if (ok) {
      log("[minilab-midi]   port %d -> '%s'\n", i, buf);
    }
    if (ok && matchesMidiPortName(buf, hints)) {
      *index_out = i;
      log("[minilab-midi]   matched port %d -> '%s'\n", i, buf);
      return true;
    }
  }
  return false;
}

void selectTrackRelative(int delta) {
  const int total = CountTracks(0);
  if (total <= 0) {
    return;
  }

  MediaTrack* current = GetSelectedTrack(0, 0);
  int current_index = -1;
  for (int i = 0; i < total; ++i) {
    MediaTrack* tr = GetTrack(0, i);
    if (tr == current) {
      current_index = i;
      break;
    }
  }

  const int next = current_index < 0 ? 0 : (current_index + delta + total) % total;
  MediaTrack* tr = GetTrack(0, next);
  if (tr) {
    SetTrackSelected(tr, true);
    refreshDisplay(tr);
  }
}

void ensureMidiOpen() {
  if (g_midi_input && g_midi_output) {
    return;
  }

  const std::vector<std::string> input_hints = {"minilab3", "arturia", "midi"};
  const std::vector<std::string> output_hints = {"minilab3", "arturia", "midi", "alv"};

  std::vector<int> input_candidates;
  if (collectMidiPortCandidates(input_hints, &input_candidates, true)) {
    for (int candidate : input_candidates) {
      if (g_midi_input) {
        break;
      }
      if (candidate < 0 || !CreateMIDIInput) {
        continue;
      }
      std::fprintf(stdout, "[minilab-midi] opening input device index=%d on demand\n", candidate);
      std::fflush(stdout);
      midi_Input* opened = CreateMIDIInput(candidate);
      if (opened) {
        opened->start();
        g_midi_input = opened;
        g_midi_input_device = candidate;
        std::fprintf(stdout, "[minilab-midi] started input device index=%d\n", candidate);
        std::fflush(stdout);
      } else {
        std::fprintf(stderr, "[minilab-midi] failed to open input device index=%d\n", candidate);
        std::fflush(stderr);
      }
    }
  }

  std::vector<int> output_candidates;
  if (collectMidiPortCandidates(output_hints, &output_candidates, false)) {
    for (int candidate : output_candidates) {
      if (g_midi_output) {
        break;
      }
      if (candidate < 0 || !CreateMIDIOutput) {
        continue;
      }
      std::fprintf(stdout, "[minilab-midi] opening control output device index=%d on demand\n", candidate);
      std::fflush(stdout);
      midi_Output* opened = CreateMIDIOutput(candidate, false, nullptr);
      if (opened) {
        g_midi_output = opened;
        g_midi_output_device = candidate;
        std::fprintf(stdout, "[minilab-midi] opened control output device index=%d\n", candidate);
        std::fflush(stdout);
      } else {
        std::fprintf(stderr, "[minilab-midi] failed to open control output device index=%d\n", candidate);
        std::fflush(stderr);
      }
    }
  }

  std::vector<int> display_candidates;
  if (collectMidiPortCandidates({"0,3", "minilab3", "alv", "arturia"}, &display_candidates, false)) {
    for (int candidate : display_candidates) {
      if (g_midi_display_output) {
        break;
      }
      if (candidate < 0 || !CreateMIDIOutput) {
        continue;
      }
      if (candidate == g_midi_output_device) {
        continue;
      }
      std::fprintf(stdout, "[minilab-midi] opening display output device index=%d on demand\n", candidate);
      std::fflush(stdout);
      midi_Output* opened = CreateMIDIOutput(candidate, false, nullptr);
      if (opened) {
        g_midi_display_output = opened;
        g_midi_display_output_device = candidate;
        std::fprintf(stdout, "[minilab-midi] opened display output device index=%d\n", candidate);
        std::fflush(stdout);
      } else {
        std::fprintf(stderr, "[minilab-midi] failed to open display output device index=%d\n", candidate);
        std::fflush(stderr);
      }
    }
  }
}

void armSelectedTrack() {
  MediaTrack* tr = GetSelectedTrack(0, 0);
  if (!tr) {
    return;
  }

  if (CSurf_OnRecArmChange) {
    CSurf_OnRecArmChange(tr, -1);
  }
  if (CSurf_SetSurfaceRecArm) {
    CSurf_SetSurfaceRecArm(tr, true, nullptr);
  }
  refreshDisplay(tr);
}

}  // namespace minilab

namespace minilab {

const char* MiniLabReaperControlSurface::GetTypeString() {
  return "MINILAB3";
}

const char* MiniLabReaperControlSurface::GetDescString() {
  return "MiniLab 3 REAPER control surface";
}

const char* MiniLabReaperControlSurface::GetConfigString() {
  return "minilab3";
}

void MiniLabReaperControlSurface::SetTrackListChange() {
  extension_.onTrackListChange();
}

void MiniLabReaperControlSurface::SetSurfaceVolume(MediaTrack* trackid, double volume) {
  (void)trackid;
  extension_.onVolumeChanged(volume);
}

void MiniLabReaperControlSurface::SetSurfacePan(MediaTrack* trackid, double pan) {
  (void)trackid;
  extension_.onPanChanged(pan);
}

void MiniLabReaperControlSurface::SetSurfaceMute(MediaTrack* trackid, bool mute) {
  (void)trackid;
  extension_.onMuteChanged(mute);
}

void MiniLabReaperControlSurface::SetSurfaceSelected(MediaTrack* trackid, bool selected) {
  (void)selected;
  extension_.onTrackSelection("Track 01", safeTrackName(trackid));
  refreshDisplay(trackid);
}

void MiniLabReaperControlSurface::SetSurfaceSolo(MediaTrack* trackid, bool solo) {
  (void)trackid;
  extension_.onSoloChanged(solo);
}

void MiniLabReaperControlSurface::SetSurfaceRecArm(MediaTrack* trackid, bool recarm) {
  if (!trackid) return;
  if (CSurf_SetSurfaceRecArm) {
    CSurf_SetSurfaceRecArm(trackid, recarm, this);
  }
  extension_.setAutoArm(recarm);
  refreshDisplay(trackid);
}

void MiniLabReaperControlSurface::SetPlayState(bool play, bool pause, bool rec) {
  (void)play;
  (void)pause;
  extension_.onTransportChanged(rec);
}

void MiniLabReaperControlSurface::SetRepeatState(bool rep) {
  (void)rep;
}

void MiniLabReaperControlSurface::SetTrackTitle(MediaTrack* trackid, const char* title) {
  extension_.onTrackSelection("Track 01", title ? title : safeTrackName(trackid));
  refreshDisplay(trackid);
}

void MiniLabReaperControlSurface::OnTrackSelection(MediaTrack* trackid) {
  extension_.onTrackSelection("Track 01", safeTrackName(trackid));
  refreshDisplay(trackid);
}

void MiniLabReaperControlSurface::Run() {
  ensureMidiOpen();

  if (!g_midi_input) {
    return;
  }

  g_midi_input->SwapBufs(0);
  MIDI_eventlist* list = g_midi_input->GetReadBuf();
  if (!list) {
    return;
  }

  int pos = 0;
  MIDI_event_t* evt = nullptr;
  while ((evt = list->EnumItems(&pos))) {
    if (evt->size > 0) {
      logMidiEvent("IN ", evt->midi_message, evt->size);
    }

    if (evt->size >= 2 && (evt->midi_message[0] & 0xF0) == 0xB0) {
      const int cc = evt->midi_message[1];
      const int val = evt->midi_message[2];

      if (cc == 0x1C || cc == 0x1D || cc == 0x76 || cc == 0x77) {
        if (val > 64) {
          selectTrackRelative(1);
        } else if (val < 64) {
          selectTrackRelative(-1);
        } else {
          armSelectedTrack();
        }
      }
    }

    if (evt->size >= 2 && ((evt->midi_message[0] & 0xF0) == 0x90) && evt->midi_message[2] > 0) {
      armSelectedTrack();
    }
  }
}

DisplayState MiniLabReaperControlSurface::displayState() const {
  return extension_.displayState();
}

}  // namespace minilab

namespace {

IReaperControlSurface* createSurface(const char* type_string, const char* config_string, int* err_stats) {
  (void)config_string;
  if (err_stats) {
    *err_stats = 0;
  }

  if (type_string && std::strcmp(type_string, "MINILAB3") == 0) {
    return new minilab::MiniLabReaperControlSurface();
  }

  return nullptr;
}

HWND showConfig(const char* type_string, HWND parent, const char* init_config_string) {
  (void)type_string;
  (void)parent;
  (void)init_config_string;
  return nullptr;
}

reaper_csurf_reg_t g_surface_reg = {
    "MINILAB3",
    "MiniLab 3 REAPER control surface",
    createSurface,
    showConfig,
};

}  // namespace

extern "C" {

REAPER_PLUGIN_DLL_EXPORT int REAPER_PLUGIN_ENTRYPOINT(REAPER_PLUGIN_HINSTANCE hInstance,
                                                     reaper_plugin_info_t* rec) {
  (void)hInstance;
  if (!rec || !rec->Register || !rec->GetFunc) {
    return 0;
  }

  const int api_load_rc = REAPERAPI_LoadAPI(rec->GetFunc);
  if (api_load_rc != 0) {
    std::fprintf(stderr,
                 "[minilab-midi] REAPERAPI_LoadAPI returned %d; continuing with partial API\n",
                 api_load_rc);
    std::fflush(stderr);
  }

  minilab::g_getTrackName = GetTrackName;

  const auto log = [&](const char* fmt, auto... args) {
    std::fprintf(stdout, fmt, args...);
    std::fflush(stdout);
    std::fprintf(stderr, fmt, args...);
    std::fflush(stderr);
  };

  log("[minilab-midi] plugin entry: opening MIDI ports\n");

  const bool found_input = minilab::findMidiPortByHints(
      {"minilab3", "arturia", "midi"},
      &minilab::g_midi_input_device,
      true);
  if (found_input) {
    std::fprintf(stdout,
                 "[minilab-midi] matched input device index=%d; deferred open via REAPER MIDI API\n",
                 minilab::g_midi_input_device);
    std::fflush(stdout);
  } else {
    std::fprintf(stdout, "[minilab-midi] no input device matched Minilab/MiniLab/Arturia\n");
    std::fflush(stdout);
  }

  const bool found_output = minilab::findMidiPortByHints(
      {"minilab3", "arturia", "midi", "alv"},
      &minilab::g_midi_output_device,
      false);
  const bool found_display_output = minilab::findMidiPortByHints(
      {"0,3", "minilab3", "alv", "arturia"},
      &minilab::g_midi_display_output_device,
      false);

  if (minilab::g_midi_output_device == minilab::g_midi_display_output_device) {
    std::fprintf(stdout,
                 "[minilab-midi] display output device index matches control output; ignoring duplicate ALV match\n");
    std::fflush(stdout);
    minilab::g_midi_display_output_device = -1;
  }
  if (found_output) {
    std::fprintf(stdout,
                 "[minilab-midi] matched control output device index=%d; deferred open via REAPER MIDI API\n",
                 minilab::g_midi_output_device);
    std::fflush(stdout);
  } else {
    std::fprintf(stdout, "[minilab-midi] no control output device matched Minilab3 MIDI\n");
    std::fflush(stdout);
  }

  if (found_display_output) {
    std::fprintf(stdout,
                 "[minilab-midi] matched display output device index=%d; deferred open via REAPER MIDI API\n",
                 minilab::g_midi_display_output_device);
    std::fflush(stdout);
  } else {
    std::fprintf(stdout, "[minilab-midi] no display output device matched Minilab3 ALV\n");
    std::fflush(stdout);
  }

  return rec->Register("csurf", &g_surface_reg) ? 1 : 0;
}

}  // extern "C"
