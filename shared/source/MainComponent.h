#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_gui_extra/juce_gui_extra.h>  // For DialogWindow
#include <memory>
#include <utility>
#include <vector>
#include <deque>
#include <map>
#include <array>
#include "MidiLogger.h"
#include "../include/RustBindings.h"
#include "SettingsComponent.h"
#include "SettingsWindow.h"
#include "MidiLogDisplay.h"
#include "LogDisplaySettingsWindow.h"
#include "LogDisplayWindow.h"
#include "DisplaySettingsManager.h"
#include "WindowManager.h"
#include "WindowRoutingWindow.h"

namespace MidiPortal {

class MainComponent : public juce::Component,
                     public juce::MenuBarModel
{
public:
  // Menu item IDs
  static constexpr int kSettingsMenuItemId = 1;
  static constexpr int kLogDisplaySettingsMenuItemId = 2;
  static constexpr int kWindowRoutingMenuItemId = 3;
  static constexpr int kViewModeListId = 100;
  static constexpr int kViewModeGridId = 101;
  static constexpr int kViewModeTimelineId = 102;
  static constexpr int kDeviceWindowBaseId = 1000; // Base ID for device window menu items

  // Add view mode enum
  enum class ViewMode {
    List = 0,
    Grid,
    Timeline
  };

  MainComponent();
  ~MainComponent() override;

  void paint(juce::Graphics& g) override;
  void resized() override;
  void addMidiMessage(const juce::MidiMessage& message);
  
  // X- Add method to trigger activity indicators for a device
  void triggerMidiActivity(const juce::String& deviceName);

  // Add these required MenuBarModel methods
  juce::StringArray getMenuBarNames() override;
  juce::PopupMenu getMenuForIndex(int index, const juce::String& name) override;
  void menuItemSelected(int menuItemID, int topLevelMenuIndex) override;
  int getNumMenuBarItems();  // Remove override since it's not virtual

  void setViewMode(ViewMode newMode);
  
  void updateCurrentView();
  
  void updateViewMenu();
  
  bool shouldProcessMidiMessage(const juce::MidiMessage& message, const juce::String& deviceName);

private:
  // Forward declare the MidiInputCallback class
  class MidiInputCallback;
  std::unique_ptr<MidiInputCallback> midiInputCallback;

  ViewMode currentViewMode = ViewMode::List;
  
  // MIDI input management
  juce::OwnedArray<juce::MidiInput> midiInputs;

  // MIDI logger
  std::unique_ptr<MidiPortal::MidiLogger> midiLogger; // To manage logging functionality

  // X- Add MidiLogDisplay
  std::unique_ptr<MidiLogDisplay> midiLogDisplay;

  // X- Add managers
  DisplaySettingsManager settingsManager;
  WindowManager windowManager;

  // X- Add window pointers
  std::unique_ptr<WindowRoutingWindow> windowRoutingWindow;

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
  std::unique_ptr<LogDisplaySettingsWindow> logDisplaySettingsWindow;

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

  // Add these to the private section before JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR
  std::map<juce::String, std::unique_ptr<LogDisplayWindow>> deviceWindows;
  
  // Helper methods for device windows
  void openDeviceWindow(const juce::String& deviceName);
  void closeDeviceWindow(const juce::String& deviceName);
  void updateDeviceWindowMenu(juce::PopupMenu& menu);
  void routeMidiMessage(const juce::MidiMessage& message, const juce::String& deviceName);

  // Add menu-related member variables
  juce::PopupMenu applicationMenu;
  juce::PopupMenu viewMenu;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};

} // namespace MidiPortal