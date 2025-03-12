/**
 * @file MainComponent.h
 * @brief Defines the main component of the MidiPortal application.
 * 
 * This file defines the MainComponent class, which is the central component
 * of the MidiPortal application. It handles MIDI input, manages the user interface,
 * and coordinates the various features of the application, such as MIDI logging,
 * display settings, and window routing.
 */

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

/**
 * @class MainComponent
 * @brief The main component of the MidiPortal application.
 * 
 * This class is the central component of the MidiPortal application. It handles
 * MIDI input, manages the user interface, and coordinates the various features
 * of the application, such as MIDI logging, display settings, and window routing.
 * 
 * The component implements a menu bar for accessing application features and
 * provides different view modes for visualizing MIDI data.
 */
class MainComponent : public juce::Component,
                     public juce::MenuBarModel
{
public:
  /**
   * @brief Menu item IDs for the application menu.
   * 
   * These constants define the IDs for menu items in the application menu.
   * They are used to identify which menu item was selected in the menuItemSelected method.
   */
  static constexpr int kSettingsMenuItemId = 1;
  static constexpr int kLogDisplaySettingsMenuItemId = 2;
  static constexpr int kWindowRoutingMenuItemId = 3;
  static constexpr int kViewModeListId = 100;
  static constexpr int kViewModeGridId = 101;
  static constexpr int kViewModeTimelineId = 102;
  static constexpr int kDeviceWindowBaseId = 1000; // Base ID for device window menu items

  /**
   * @brief Enum for different view modes.
   * 
   * Defines the different ways MIDI data can be visualized in the application.
   * - List: Shows MIDI messages as a scrolling list of text.
   * - Grid: Shows MIDI notes as a grid of colored cells.
   * - Timeline: Shows MIDI messages on a timeline.
   */
  enum class ViewMode {
    List = 0,
    Grid,
    Timeline
  };

  /**
   * @brief Default constructor.
   * 
   * Initializes the MainComponent, sets up MIDI input, creates the menu bar,
   * and initializes the various managers and displays.
   */
  MainComponent();
  
  /**
   * @brief Destructor that cleans up resources.
   * 
   * Stops MIDI input, closes windows, and releases resources.
   */
  ~MainComponent() override;

  /**
   * @brief Paints the component.
   * @param g The Graphics context to paint into.
   * 
   * Draws the background and any visual elements of the component.
   */
  void paint(juce::Graphics& g) override;
  
  /**
   * @brief Handles component resizing.
   * 
   * Positions child components based on the new size of the component.
   */
  void resized() override;
  
  /**
   * @brief Adds a MIDI message to the display.
   * @param message The MIDI message to add.
   * 
   * Processes a MIDI message, logs it, and updates the display.
   */
  void addMidiMessage(const juce::MidiMessage& message);
  
  /**
   * @brief Triggers the activity indicator for a specific device.
   * @param deviceName The name of the device to trigger activity for.
   * 
   * Causes the activity indicator for the specified device to flash,
   * indicating that MIDI activity has been detected on that device.
   */
  void triggerMidiActivity(const juce::String& deviceName);

  /**
   * @brief Gets the names of the menu bar items.
   * @return A StringArray containing the names of the menu bar items.
   * 
   * Required by the MenuBarModel interface. Returns the names of the
   * top-level menu items in the menu bar.
   */
  juce::StringArray getMenuBarNames() override;
  
  /**
   * @brief Gets the popup menu for a specific menu bar item.
   * @param index The index of the menu bar item.
   * @param name The name of the menu bar item.
   * @return A PopupMenu containing the menu items for the specified menu bar item.
   * 
   * Required by the MenuBarModel interface. Returns the popup menu for
   * a specific top-level menu item in the menu bar.
   */
  juce::PopupMenu getMenuForIndex(int index, const juce::String& name) override;
  
  /**
   * @brief Handles menu item selection.
   * @param menuItemID The ID of the selected menu item.
   * @param topLevelMenuIndex The index of the top-level menu containing the selected item.
   * 
   * Required by the MenuBarModel interface. Called when a menu item is selected.
   * Performs the appropriate action based on the selected menu item.
   */
  void menuItemSelected(int menuItemID, int topLevelMenuIndex) override;
  
  /**
   * @brief Gets the number of menu bar items.
   * @return The number of top-level menu items in the menu bar.
   * 
   * Returns the number of top-level menu items in the menu bar.
   */
  int getNumMenuBarItems();  // Remove override since it's not virtual

  /**
   * @brief Sets the current view mode.
   * @param newMode The new view mode to set.
   * 
   * Changes the way MIDI data is visualized in the application.
   */
  void setViewMode(ViewMode newMode);
  
  /**
   * @brief Updates the current view based on the view mode.
   * 
   * Updates the display to reflect the current view mode.
   */
  void updateCurrentView();
  
  /**
   * @brief Updates the view menu.
   * 
   * Updates the view menu to reflect the current view mode.
   */
  void updateViewMenu();
  
  /**
   * @brief Determines if a MIDI message should be processed.
   * @param message The MIDI message to check.
   * @param deviceName The name of the device that sent the message.
   * @return true if the message should be processed, false otherwise.
   * 
   * Checks if a MIDI message should be processed based on the device
   * and channel settings.
   */
  bool shouldProcessMidiMessage(const juce::MidiMessage& message, const juce::String& deviceName);

private:
  /**
   * @brief Forward declaration of the MidiInputCallback class.
   * 
   * This class handles MIDI input callbacks from the JUCE MIDI system.
   */
  class MidiInputCallback;
  
  /**
   * @brief Pointer to the MIDI input callback handler.
   * 
   * Handles MIDI input callbacks from the JUCE MIDI system.
   */
  std::unique_ptr<MidiInputCallback> midiInputCallback;

  /**
   * @brief The current view mode.
   * 
   * Determines how MIDI data is visualized in the application.
   */
  ViewMode currentViewMode = ViewMode::List;
  
  /**
   * @brief Array of MIDI input devices.
   * 
   * Stores the MIDI input devices that are currently open.
   */
  juce::OwnedArray<juce::MidiInput> midiInputs;

  /**
   * @brief Pointer to the MIDI logger.
   * 
   * Logs MIDI messages to a file and analyzes timing information.
   */
  std::unique_ptr<MidiPortal::MidiLogger> midiLogger; // To manage logging functionality

  /**
   * @brief Pointer to the MIDI log display.
   * 
   * Displays MIDI messages in the main window.
   */
  std::unique_ptr<MidiLogDisplay> midiLogDisplay;

  /**
   * @brief Manager for display settings.
   * 
   * Manages display settings for MIDI message visualization.
   */
  DisplaySettingsManager settingsManager;
  
  /**
   * @brief Manager for windows and routing.
   * 
   * Manages display windows and MIDI device routing.
   */
  WindowManager windowManager;

  /**
   * @brief Pointer to the window routing window.
   * 
   * Dialog window for configuring MIDI device routing to display windows.
   */
  std::unique_ptr<WindowRoutingWindow> windowRoutingWindow;

  /**
   * @struct TimestampedMidiMessage
   * @brief A MIDI message with a timestamp.
   * 
   * Stores a MIDI message and the time when it was received.
   */
  struct TimestampedMidiMessage {
    juce::MidiMessage message;
    juce::Time timestamp;

    TimestampedMidiMessage(juce::MidiMessage  msg, const juce::Time& time)
      : message(std::move(msg)), timestamp(time) {}
  };

  /**
   * @struct VisualNote
   * @brief Visual representation of a MIDI note.
   * 
   * Stores the position, color, and opacity of a visual representation
   * of a MIDI note for the Grid view mode.
   */
  struct VisualNote {
    juce::Point<float> position;
    juce::Colour color;
    float opacity;
  };

  /**
   * @brief Array of visual notes.
   * 
   * Stores the visual representations of MIDI notes for the Grid view mode.
   */
  std::vector<VisualNote> visualNotes; // Stores the visuals for MIDI messages

  /**
   * @brief Array of MIDI messages.
   * 
   * Stores the most recent MIDI messages for display and analysis.
   */
  std::vector<TimestampedMidiMessage> midiMessages;
  
  /**
   * @brief Maximum number of messages to store.
   * 
   * Limits the number of MIDI messages stored in memory to prevent
   * excessive memory usage.
   */
  static constexpr size_t maxMessages = 1000; // Maximum number of messages to store

  /**
   * @brief Pointer to the settings component.
   * 
   * Component for configuring audio and MIDI device settings.
   */
  std::unique_ptr<SettingsComponent> settingsComponent;
  
  /**
   * @brief Pointer to the settings window.
   * 
   * Dialog window for configuring audio and MIDI device settings.
   */
  std::unique_ptr<SettingsWindow> settingsWindow;
  
  /**
   * @brief Pointer to the log display settings window.
   * 
   * Dialog window for configuring MIDI log display settings.
   */
  std::unique_ptr<LogDisplaySettingsWindow> logDisplaySettingsWindow;

  /**
   * @brief Handle to the Rust engine instance.
   * 
   * Used by the Rust FFI functions to identify the specific
   * instance of the Rust engine to operate on.
   */
  void* rustEngine = nullptr;

  /**
   * @brief Audio device manager.
   * 
   * Manages audio and MIDI devices for the application.
   */
  juce::AudioDeviceManager deviceManager;

  /**
   * @struct MidiDeviceChannelState
   * @brief State of MIDI channels for a device.
   * 
   * Stores which MIDI channels are enabled for a specific device.
   */
  struct MidiDeviceChannelState {
    juce::String deviceName;
    std::array<bool, 16> enabledChannels;
    
    MidiDeviceChannelState(const juce::String& name) : deviceName(name) 
    {
        // Default to all channels enabled
        enabledChannels.fill(true);
    }
  };

  /**
   * @brief Array of MIDI device channel states.
   * 
   * Stores which MIDI channels are enabled for each device.
   */
  std::vector<MidiDeviceChannelState> deviceChannelStates;

  /**
   * @brief Map of device names to log display windows.
   * 
   * Stores the log display windows for each device.
   */
  std::map<juce::String, std::unique_ptr<LogDisplayWindow>> deviceWindows;
  
  /**
   * @brief Opens a device window.
   * @param deviceName The name of the device to open a window for.
   * 
   * Creates and shows a log display window for the specified device.
   */
  void openDeviceWindow(const juce::String& deviceName);
  
  /**
   * @brief Closes a device window.
   * @param deviceName The name of the device to close the window for.
   * 
   * Closes and destroys the log display window for the specified device.
   */
  void closeDeviceWindow(const juce::String& deviceName);
  
  /**
   * @brief Updates the device window menu.
   * @param menu The menu to update.
   * 
   * Updates the device window menu to reflect the current state of device windows.
   */
  void updateDeviceWindowMenu(juce::PopupMenu& menu);
  
  /**
   * @brief Routes a MIDI message to the appropriate windows.
   * @param message The MIDI message to route.
   * @param deviceName The name of the device that sent the message.
   * 
   * Routes a MIDI message to the appropriate windows based on the device
   * and window routing configuration.
   */
  void routeMidiMessage(const juce::MidiMessage& message, const juce::String& deviceName);

  /**
   * @brief Application menu.
   * 
   * Popup menu for the Application menu item in the menu bar.
   */
  juce::PopupMenu applicationMenu;
  
  /**
   * @brief View menu.
   * 
   * Popup menu for the View menu item in the menu bar.
   */
  juce::PopupMenu viewMenu;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};

} // namespace MidiPortal