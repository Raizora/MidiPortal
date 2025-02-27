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
          // Only process if the channel is enabled
          if (owner.shouldProcessMidiMessage(message, sourceName)) {
              owner.midiLogger->setDeviceName(sourceName);
              owner.addMidiMessage(message);
          }
      });
  }

private:
    MainComponent& owner;
};

MainComponent::MainComponent() {
    // Initialize device manager with no default devices
    deviceManager.initialiseWithDefaultDevices(0, 0);  // No audio inputs/outputs
    
    // Initialize other components
    rustEngine = create_midi_engine();
    setSize(800, 800);
    
    // Initialize MIDI components
    midiInputCallback = std::make_unique<MidiInputCallback>(*this);
    midiLogger = std::make_unique<MidiPortal::MidiLogger>("MidiTraffic.log");

    // Set up MIDI callback for the AudioDeviceManager
    deviceManager.addMidiInputDeviceCallback({}, midiInputCallback.get());

    // Set up menu bar
    #if JUCE_MAC
        applicationMenu.addItem(1, "Settings...", true, false);
        juce::MenuBarModel::setMacMainMenu(this, &applicationMenu);
        
        juce::Process::setDockIconVisible(true);
        juce::Process::makeForegroundProcess();
    #endif

    // Create settings component with device manager
    settingsComponent = std::make_unique<SettingsComponent>(deviceManager);

    // Update settings window creation
    juce::MessageManager::callAsync([this]() {
        if (settingsWindow == nullptr) {
            settingsWindow.reset(new SettingsWindow("MidiPortal Settings", deviceManager));
            settingsWindow->onCloseCallback = [this]() {
                settingsWindow.reset();
            };
            settingsWindow->setBackgroundColour(juce::LookAndFeel::getDefaultLookAndFeel()
                .findColour(juce::ResizableWindow::backgroundColourId));
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

            // X- Use the device name from the logger instead of trying to get it from the message
            if (midiLogger) {
                triggerMidiActivity(midiLogger->getDeviceName());
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

// Add a method to trigger activity indicators
void MainComponent::triggerMidiActivity(const juce::String& deviceName)
{
    if (settingsComponent != nullptr)
    {
        settingsComponent->triggerActivityForDevice(deviceName);
    }
}

}  // namespace MidiPortal