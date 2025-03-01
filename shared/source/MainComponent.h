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
#include "MidiLogDisplay.h"
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
    #if JUCE_MAC
    // On macOS, the application menu is automatically created with the app name
    // We just need to return the other menu names
    return { "File", "View" };
    #else
    // On other platforms, we need to include the application menu
    return { "MidiPortal", "File", "View" };
    #endif
  }
  
  juce::PopupMenu getMenuForIndex(int /*index*/, const juce::String& name) override
  {
    #if JUCE_MAC
    // On macOS, we don't need to handle the application menu here
    // as it's automatically created by the OS
    if (name == "File")
    {
        juce::PopupMenu menu;
        // File menu can be empty or have other items
        return menu;
    }
    else if (name == "View")
    {
        return viewMenu;
    }
    #else
    // On other platforms, we need to handle all menus
    if (name == "MidiPortal")
    {
        juce::PopupMenu menu;
        menu.addItem(1, "Settings...", true, false);
        return menu;
    }
    else if (name == "File")
    {
        juce::PopupMenu menu;
        return menu;
    }
    else if (name == "View")
    {
        return viewMenu;
    }
    #endif
    return {};
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
    else if (menuItemID >= 100 && menuItemID <= 102)  // View modes
    {
        setViewMode(static_cast<ViewMode>(menuItemID - 100));
    }
  }

private:
  // Forward declare the MidiInputCallback class
  class MidiInputCallback;
  std::unique_ptr<MidiInputCallback> midiInputCallback;

  // Add view mode enum and management
  enum class ViewMode {
    List = 0,
    Grid,
    Timeline
  };
  
  ViewMode currentViewMode = ViewMode::List;
  
  void setViewMode(ViewMode newMode)
  {
    if (currentViewMode != newMode)
    {
        currentViewMode = newMode;
        updateViewMenu();
        updateCurrentView();  // X- Add this to update the visible component
        repaint();
    }
  }

  // X- Method to update the current view based on the selected mode
  void updateCurrentView()
  {
    // X- Remove all child components first
    for (int i = getNumChildComponents() - 1; i >= 0; --i)
    {
        if (getChildComponent(i) != nullptr && 
            getChildComponent(i) != midiLogDisplay.get())
        {
            removeChildComponent(getChildComponent(i));
        }
    }
    
    // X- Show the appropriate view based on the current mode
    switch (currentViewMode)
    {
        case ViewMode::List:
            // X- Show the log display
            if (midiLogDisplay != nullptr)
            {
                addAndMakeVisible(midiLogDisplay.get());
                midiLogDisplay->setBounds(getLocalBounds().reduced(10));
            }
            break;
            
        case ViewMode::Grid:
            // X- Future: Show grid view
            break;
            
        case ViewMode::Timeline:
            // X- Future: Show timeline view
            break;
    }
    
    resized();
  }

  void updateViewMenu()
  {
    #if JUCE_MAC
        // Update just the view menu
        viewMenu.clear();
        viewMenu.addItem(100, "List View", true, currentViewMode == ViewMode::List);
        viewMenu.addItem(101, "Grid View", true, currentViewMode == ViewMode::Grid);
        viewMenu.addItem(102, "Timeline View", true, currentViewMode == ViewMode::Timeline);
        
        // Update the menu bar
        juce::MenuBarModel::setMacMainMenu(this, &applicationMenu);
    #else
        // Update view menu for non-Mac platforms
        viewMenu.clear();
        viewMenu.addItem(100, "List View", true, currentViewMode == ViewMode::List);
        viewMenu.addItem(101, "Grid View", true, currentViewMode == ViewMode::Grid);
        viewMenu.addItem(102, "Timeline View", true, currentViewMode == ViewMode::Timeline);
        
        // For non-Mac platforms, we need to update the menu bar differently
        menuItemsChanged();
    #endif
  }

  // Add menu as member variable
  juce::PopupMenu applicationMenu;
  juce::PopupMenu viewMenu;  // Add view menu as member

  // MIDI input management
  juce::OwnedArray<juce::MidiInput> midiInputs;

  // MIDI logger
  std::unique_ptr<MidiPortal::MidiLogger> midiLogger; // To manage logging functionality

  // X- Add MidiLogDisplay
  std::unique_ptr<MidiLogDisplay> midiLogDisplay;

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