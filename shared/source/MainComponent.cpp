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
    // Add Rust engine handle as member
    rustEngine = create_midi_engine();  // Create the Rust engine
    
    // Initialize UI components here
    setSize(800, 800);
    
    // Initialize MIDI components first
    midiInputCallback = std::make_unique<MidiInputCallback>(*this);
    midiLogger = std::make_unique<MidiPortal::MidiLogger>("MidiTraffic.log");

    // Set up MIDI Input
    auto availableMidiDevices = juce::MidiInput::getAvailableDevices();
    for (auto& device : availableMidiDevices) {
        DBG("Found MIDI device: " + device.name);
        
        if (auto input = juce::MidiInput::openDevice(device.identifier, midiInputCallback.get())) {
            midiInputs.add(std::move(input));
            midiInputs.getLast()->start();
        }
    }

    // Set up menu bar
    #if JUCE_MAC
        applicationMenu.addItem(1, "Preferences...", true, false);
        juce::MenuBarModel::setMacMainMenu(this, &applicationMenu);
        
        juce::Process::setDockIconVisible(true);
        juce::Process::makeForegroundProcess();
    #endif

    // Initialize settings component
    settingsComponent = std::make_unique<SettingsComponent>();

    // Automatically open settings window
    juce::MessageManager::callAsync([this]() {
        if (settingsWindow == nullptr) {
            settingsWindow.reset(new SettingsWindow("MidiPortal Preferences"));
            settingsWindow->toFront(true);
            settingsWindow->grabKeyboardFocus();
        }
    });

    // Ensure the window is displayed and focused
    juce::MessageManager::callAsync([this]() {
        if (isShowing() || isOnDesktop()) {
            toFront(true);
            grabKeyboardFocus();
        }
    });
}

MainComponent::~MainComponent() {
  // Clean up menu bar
  juce::MenuBarModel::setMacMainMenu(nullptr);
  
  // Clean up MIDI inputs
  midiInputs.clear();
  
  // Clean up Rust engine
  if (rustEngine) {
      destroy_midi_engine(rustEngine);
  }
  
  // The rest will be cleaned up automatically by unique_ptr destructors:
  // - midiInputCallback
  // - midiLogger
  // - settingsComponent
  // - settingsWindow
}

void MainComponent::addMidiMessage(const juce::MidiMessage& message) {
    juce::MessageManager::callAsync([this, message]() {
        try {
            DBG("Received MIDI message, size: " + juce::String(message.getRawDataSize()));
            
            const uint8_t* data = message.getRawData();
            if (!data) {
                DBG("Error: Null MIDI data pointer");
                return;
            }
            
            const auto len = static_cast<size_t>(message.getRawDataSize());
            double timestamp = juce::Time::getMillisecondCounterHiRes() / 1000.0;

            // Pass the engine handle to process_midi_message
            if (!process_midi_message(rustEngine, data, len, timestamp)) {
                DBG("Error: Failed to process MIDI message in Rust");
                return;
            }

            // Store message for display/history
            midiMessages.emplace_back(message, juce::Time::getCurrentTime());
            if (midiMessages.size() > maxMessages) {
                midiMessages.erase(midiMessages.begin());
            }
            
            // Log the raw message
            if (midiLogger) {
                midiLogger->logMessage(message);
            }

            repaint();
        }
        catch (const std::exception& e) {
            DBG("Exception in addMidiMessage: " + juce::String(e.what()));
        }
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