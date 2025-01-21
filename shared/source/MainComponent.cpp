//
// Created by Jamie Benchia on 1/3/25.
//

#include "MainComponent.h"
#include <juce_audio_basics/juce_audio_basics.h> // Added for MidiMessage
#include <fstream> // Include for file operations
#include <iostream> // Include for console output
#include "MidiLogger.h"

namespace MidiPortal {

// Define the MidiInputCallback class
class MainComponent::MidiInputCallback : public juce::MidiInputCallback {
public:
  explicit MidiInputCallback(MainComponent& parentOwner) : owner(parentOwner) {}
  void handleIncomingMidiMessage(juce::MidiInput* /*source*/,
                                 const juce::MidiMessage& message) override {
      juce::MessageManager::callAsync([this, message]() { // Ensure thread safety for GUI updates
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
  midiLogger = std::make_unique<MidiPortal::MidiLogger>("MidiTraffic.log");


  // Set up MIDI Input
  auto availableMidiDevices = juce::MidiInput::getAvailableDevices();
    for (auto& device : availableMidiDevices) {
      juce::Logger::writeToLog("Initialized MIDI device: " + device.name); // Log the device name

      auto input = juce::MidiInput::openDevice(device.identifier, midiInputCallback.get());
      if (input != nullptr) {
          midiInputs.add(std::move(input)); // Store active MIDI inputs
          midiInputs.getLast()->start(); // Start receiving MIDI messages
      }
  }

  // Ensure the window is displayed and focused
  juce::MessageManager::callAsync([this]() { // Delay to ensure thread safety
      if (isShowing() || isOnDesktop()) { // Verify visibility before setting focus
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
  // Ensure that the following operations are executed on the message thread.
  juce::MessageManager::callAsync([this, message]() {
      // Store the MIDI message along with the current timestamp in the vector.
      midiMessages.emplace_back(message, juce::Time::getCurrentTime()); // Add message to storage

      // If the number of stored messages exceeds the maximum allowed, remove the oldest one.
      if (midiMessages.size() > maxMessages) { // Check if max size is exceeded
          midiMessages.erase(midiMessages.begin()); // Remove the oldest message
      }

      // Log the received MIDI message to a file using the logger.
      juce::Logger::writeToLog("Logging MIDI message: " + message.getDescription()); // Log for debugging

      midiLogger->logMessage(message); // Log the message to the file

      // Trigger a repaint of the GUI to reflect the latest MIDI message.
      repaint(); // Update the display with the new message
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