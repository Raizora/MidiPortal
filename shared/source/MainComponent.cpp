//
// Created by Jamie Benchia on 1/3/25.
//

#include "MainComponent.h"

namespace MidiPortal {

// Define the MidiInputCallback class
class MainComponent::MidiInputCallback : public juce::MidiInputCallback {
public:
  explicit MidiInputCallback(MainComponent& parentOwner) : owner(parentOwner) {}
  void handleIncomingMidiMessage(juce::MidiInput* /*source*/,
                                 const juce::MidiMessage& message) override {
      juce::MessageManager::callAsync([this, message]() { // X - Ensure thread safety for GUI updates
          owner.addMidiMessage(message);
      });
  }

private:
    MainComponent& owner;
};

MainComponent::MainComponent() {
  // Initialize UI components here
  setSize(800, 800);
  midiInputCallback = std::make_unique<MidiInputCallback>(*this);

  // Set up MIDI Input
  auto availableMidiDevices = juce::MidiInput::getAvailableDevices();
    for (auto& device : availableMidiDevices) {
      auto input = juce::MidiInput::openDevice(device.identifier, midiInputCallback.get());
      if (input != nullptr) {
          midiInputs.add(std::move(input)); // Store active MIDI inputs
          midiInputs.getLast()->start();
      }
  }

  // Ensure the window is displayed and focused
  juce::MessageManager::callAsync([this]() { // X - Delay to ensure thread safety
      if (isShowing() || isOnDesktop()) { // X - Verify visibility before setting focus
          toFront(true);
          grabKeyboardFocus();
      }
  });

#if JUCE_MAC
  juce::Process::setDockIconVisible(true); // Ensures the app is visible in the macOS dock
  juce::Process::makeForegroundProcess(); // Activates the app explicitly
#endif


}

MainComponent::~MainComponent() = default;

void MainComponent::addMidiMessage(const juce::MidiMessage& message) {
  juce::MessageManager::callAsync([this, message]() { // X - Ensure thread safety
      // Add the MIDI message to the vector with its timestamp
      midiMessages.emplace_back(message, juce::Time::getCurrentTime());

      // Limit the size of the vector to the maximum allowed messages
      if (midiMessages.size() > maxMessages) {
          midiMessages.erase(midiMessages.begin());
      }

      // Trigger a repaint to update the GUI
      repaint(); // X - Safe repaint on the message thread
  });
}

void MainComponent::paint(juce::Graphics& g) {
  g.fillAll(juce::Colours::black);
  g.setColour(juce::Colours::white);
    g.drawText("MidiPortal", getLocalBounds(), juce::Justification::centred, true);
  g.setFont(20.0f);
}

void MainComponent::resized() {
    // Resize child components if needed
}

}  // namespace MidiPortal