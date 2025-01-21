#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <memory>
#include <utility>
#include <vector>

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

    // MIDI message storage
    struct TimestampedMidiMessage {
        juce::MidiMessage message;
        juce::Time timestamp;

        TimestampedMidiMessage(juce::MidiMessage  msg, const juce::Time& time)
            : message(std::move(msg)), timestamp(time) {}
};

    std::vector<TimestampedMidiMessage> midiMessages;
    static constexpr size_t maxMessages = 1000; // Maximum number of messages to store

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};

} // namespace MidiPortalfsffs