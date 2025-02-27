#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <memory>
#include <utility>
#include <vector>
#include "MidiLogger.h"
#include "../include/RustBindings.h"
#include "SettingsComponent.h"
#include "SettingsWindow.h"
#include <juce_gui_extra/juce_gui_extra.h>  // For DialogWindow

namespace MidiPortal {

class MainComponent : public juce::Component,
                     public juce::MenuBarModel  // Add this
{
public:
  MainComponent();
  ~MainComponent() override;

  void paint(juce::Graphics& g) override;
  void resized() override;
  void addMidiMessage(const juce::MidiMessage& message);
  
  // X- Add method to trigger activity indicators for a device
  void triggerMidiActivity(const juce::String& deviceName);

  // Add these required MenuBarModel methods
  juce::StringArray getMenuBarNames() override
  {
    return { "File" };
  }
  
  juce::PopupMenu getMenuForIndex(int /*index*/, const juce::String& /*name*/) override
  {
    return {};  // Empty menu for now until we have file operations
  }
  
  void menuItemSelected(int menuItemID, int /*topLevelMenuIndex*/) override
  {
    if (menuItemID == 1)  // Settings
    {
      if (settingsWindow == nullptr) {
        settingsWindow.reset(new SettingsWindow("MidiPortal Settings", deviceManager));
        settingsWindow->onCloseCallback = [this]() {
          settingsWindow.reset();
        };
            
            // X- Set background color to match system theme
            settingsWindow->setBackgroundColour(juce::LookAndFeel::getDefaultLookAndFeel()
                .findColour(juce::ResizableWindow::backgroundColourId));
        }
      settingsWindow->toFront(true);
    }
  }

private:
  // Forward declare the MidiInputCallback class
  class MidiInputCallback;
  std::unique_ptr<MidiInputCallback> midiInputCallback;

  // Add menu as member variable
  juce::PopupMenu applicationMenu;

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

  std::unique_ptr<SettingsComponent> settingsComponent;
  std::unique_ptr<SettingsWindow> settingsWindow;

  // Add Rust engine handle
  void* rustEngine = nullptr;

  // X- Add AudioDeviceManager as member
  juce::AudioDeviceManager deviceManager;

  struct MidiDeviceChannelState {
    juce::String deviceName;
    std::array<bool, 16> enabledChannels;
    
    MidiDeviceChannelState(const juce::String& name) : deviceName(name) 
    {
        // Default to all channels enabled
        enabledChannels.fill(true);
    }
  };

  std::vector<MidiDeviceChannelState> deviceChannelStates;

  // Add method to check if a message should be processed
  bool shouldProcessMidiMessage(const juce::MidiMessage& message, const juce::String& deviceName)
  {
    // Find the device state
    for (const auto& state : deviceChannelStates)
    {
        if (state.deviceName == deviceName)
        {
            // Check if the channel is enabled (MIDI channels are 1-16, array is 0-15)
            int channel = message.getChannel() - 1;
            return channel >= 0 && channel < 16 && 
                   (channel >= 0 ? state.enabledChannels[static_cast<size_t>(channel)] : false);
        }
    }
    
    // If device not found, default to enabled
    return true;
  }

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};

} // namespace MidiPortal