#pragma once

#include "juce_gui_basics/juce_gui_basics.h"
#include "juce_audio_devices/juce_audio_devices.h"
#include "juce_audio_basics/juce_audio_basics.h" // Added for MidiMessage
#include <memory>
#include <utility>
#include <vector>
#include "MidiLogger.h"

// Define C-compatible structs
struct ColorWithOpacity {
  float hue;
  float saturation;
  float value;
  float opacity;
};

struct Position {
  float x;
  float y;
};

// Use C-compatible linkage
extern "C" ColorWithOpacity midi_note_to_color_with_opacity(uint8_t note, uint8_t velocity);
extern "C" Position generate_position();

namespace MidiPortal {

class MainComponent : public juce::Component {
public:
  MainComponent();
  ~MainComponent() override;

  void paint(juce::Graphics& g) override;
  void resized() override;
  void addMidiMessage(const juce::MidiMessage& message);

private:
  // Forward declare the MidiInputCallback class
  class MidiInputCallback;
  std::unique_ptr<MidiInputCallback> midiInputCallback;

  // MIDI input management
  juce::OwnedArray<juce::MidiInput> midiInputs;

  // MIDI logger
  std::unique_ptr<MidiPortal::MidiLogger> midiLogger; // To manage logging functionality

  // MIDI message storage
  struct TimestampedMidiMessage {
    juce::MidiMessage message;
    juce::Time timestamp;

    TimestampedMidiMessage(juce::MidiMessage  msg, const juce::Time& time)
      : message(std::move(msg)), timestamp(time) {}
  };

  struct VisualNote {
    juce::Point<float> position;
    juce::Colour color;
    float opacity;
  };

  std::vector<VisualNote> visualNotes; // Stores the visuals for MIDI messages

  std::vector<TimestampedMidiMessage> midiMessages;
  static constexpr size_t maxMessages = 1000; // Maximum number of messages to store

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};

} // namespace MidiPortal