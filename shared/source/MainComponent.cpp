//
// Created by Jamie Benchia on 1/3/25.
//

#include "MainComponent.h"
#include <juce_audio_basics/juce_audio_basics.h> // Added for MidiMessage
#include <fstream> // Include for file operations
#include <iostream> // Include for console output
#include "MidiLogger.h"
#include "SettingsComponent.h"

namespace MidiPortal {

// Define the MidiInputCallback class
class MainComponent::MidiInputCallback : public juce::MidiInputCallback {
public:
  explicit MidiInputCallback(MainComponent& parentOwner) : owner(parentOwner) {}
  void handleIncomingMidiMessage(juce::MidiInput* source,
                                 const juce::MidiMessage& message) override {
      juce::MessageManager::callAsync([this, message, sourceName = source->getName()]() {
          owner.midiLogger->setDeviceName(sourceName);
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

  // Set up menu bar
  juce::MenuBarModel::setMacMainMenu(this);
  
  // Initialize settings component
  settingsComponent = std::make_unique<SettingsComponent>();

  // Set up MIDI Input
  auto availableMidiDevices = juce::MidiInput::getAvailableDevices();
    for (auto& device : availableMidiDevices) {
      juce::Logger::writeToLog("Initialized MIDI device: " + device.name);

      auto input = juce::MidiInput::openDevice(device.identifier, midiInputCallback.get());
      if (input != nullptr) {
          midiLogger->setDeviceName(device.name);
          midiInputs.add(std::move(input));
          midiInputs.getLast()->start();
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

  // Set up Mac main menu
  juce::PopupMenu applicationMenu;
  applicationMenu.addItem(1, "Preferences...", true, false);
  juce::MenuBarModel::setMacMainMenu(this, &applicationMenu);
#endif
}

MainComponent::~MainComponent() {
  // Clean up menu bar
  juce::MenuBarModel::setMacMainMenu(nullptr);
  
  // Clean up MIDI inputs
  midiInputs.clear();
  
  // The rest will be cleaned up automatically by unique_ptr destructors:
  // - midiInputCallback
  // - midiLogger
  // - settingsComponent
  // - settingsWindow
}

void MainComponent::addMidiMessage(const juce::MidiMessage& message) {
    juce::MessageManager::callAsync([this, message]() {
        midiMessages.emplace_back(message, juce::Time::getCurrentTime());
        if (midiMessages.size() > maxMessages) {
            midiMessages.erase(midiMessages.begin());
        }
        
        juce::Logger::writeToLog("Logging MIDI message: " + message.getDescription());
        midiLogger->logMessage(message);

        repaint();
    });
}

void MainComponent::paint(juce::Graphics& g) {
  g.fillAll(juce::Colours::black);

  g.setColour(juce::Colours::white);
  g.setFont(20.0f);
  g.drawText("MidiPortal", getLocalBounds(), juce::Justification::centred, true);
}

void MainComponent::resized() {
    // Resize child components if needed
}

}  // namespace MidiPortal