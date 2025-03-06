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

MainComponent::MainComponent()
    : settingsManager(), // Initialize settings manager
      windowManager(settingsManager) // Initialize window manager with settings manager
{
    // Initialize device manager with no default devices
    deviceManager.initialiseWithDefaultDevices(0, 0);  // No audio inputs/outputs
    
    // Initialize other components
    rustEngine = create_midi_engine();
    setSize(800, 800);
    
    // Initialize MIDI components
    midiInputCallback = std::make_unique<MidiInputCallback>(*this);
    midiLogger = std::make_unique<MidiPortal::MidiLogger>("MidiTraffic.log");
    
    // X- Initialize MidiLogDisplay with settings manager
    midiLogDisplay = std::make_unique<MidiLogDisplay>(settingsManager);
    addAndMakeVisible(midiLogDisplay.get());

    // Set up MIDI callback for the AudioDeviceManager
    deviceManager.addMidiInputDeviceCallback({}, midiInputCallback.get());

    // Set up menu bar
    #if JUCE_MAC
        // Set up application menu (first menu)
        // This is automatically created by macOS with the app name
        // We just need to add our custom items to it
        applicationMenu.addItem(1, "Settings...", true, false);
        
        // Set up View menu
        viewMenu.addItem(100, "List View", true, currentViewMode == ViewMode::List);
        viewMenu.addItem(101, "Grid View", true, false);
        viewMenu.addItem(102, "Timeline View", true, false);
        
        // Set the Mac main menu
        juce::MenuBarModel::setMacMainMenu(this, &applicationMenu);
        
        juce::Process::setDockIconVisible(true);
        juce::Process::makeForegroundProcess();
    #else
        // For non-Mac platforms, the menus will be created through the MenuBarModel methods
        viewMenu.addItem(100, "List View", true, currentViewMode == ViewMode::List);
        viewMenu.addItem(101, "Grid View", true, false);
        viewMenu.addItem(102, "Timeline View", true, false);
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
    
    // X- Initialize the view
    updateCurrentView();
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
  // - midiLogDisplay
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
            
            // Route the message to appropriate displays
            if (midiLogger) {
                const auto& deviceName = midiLogger->getDeviceName();
                midiLogger->logMessage(message);
                
                // Route to main display
                if (midiLogDisplay)
                    midiLogDisplay->addMessage(message, deviceName);
                    
                // Route to device windows
                windowManager.routeMidiMessage(message, deviceName);
            }

            repaint();
        }
        catch (const std::exception& e) {
            DBG("Exception in addMidiMessage: " + juce::String(e.what()));
        }
    });
}

void MainComponent::paint(juce::Graphics& g) {
  // X- Only fill background if we're not showing a view
  if (getNumChildComponents() == 0) {
      g.fillAll(juce::Colours::black);
      g.setColour(juce::Colours::white);
      g.setFont(20.0f);
      g.drawText("MidiPortal", getLocalBounds(), juce::Justification::centred, true);
  }
}

void MainComponent::resized()
{
    // Make the midiLogDisplay fill the entire component
    if (midiLogDisplay != nullptr)
    {
        midiLogDisplay->setBounds(getLocalBounds());
    }
}

// Add a method to trigger activity indicators
void MainComponent::triggerMidiActivity(const juce::String& deviceName)
{
    if (settingsComponent != nullptr)
    {
        settingsComponent->triggerActivityForDevice(deviceName);
    }
}

// Update the getMenuForIndex method to include device windows
juce::PopupMenu MainComponent::getMenuForIndex(int /*index*/, const juce::String& name)
{
    juce::PopupMenu menu;
    
    #if JUCE_MAC
    if (name == "View")
    {
        menu.addItem(kViewModeListId, "List View", true, currentViewMode == ViewMode::List);
        menu.addItem(kViewModeGridId, "Grid View", true, currentViewMode == ViewMode::Grid);
        menu.addItem(kViewModeTimelineId, "Timeline View", true, currentViewMode == ViewMode::Timeline);
        menu.addSeparator();
        menu.addItem(kWindowRoutingMenuItemId, "Window Routing...", true, false);
    }
    else if (name == "File")
    {
        menu.addItem(kLogDisplaySettingsMenuItemId, "Log Display Settings...", true, false);
    }
    #else
    if (name == "MidiPortal")
    {
        menu.addItem(kSettingsMenuItemId, "Settings...", true, false);
        menu.addItem(kLogDisplaySettingsMenuItemId, "Log Display Settings...", true, false);
    }
    else if (name == "View")
    {
        menu.addItem(kViewModeListId, "List View", true, currentViewMode == ViewMode::List);
        menu.addItem(kViewModeGridId, "Grid View", true, currentViewMode == ViewMode::Grid);
        menu.addItem(kViewModeTimelineId, "Timeline View", true, currentViewMode == ViewMode::Timeline);
        menu.addSeparator();
        menu.addItem(kWindowRoutingMenuItemId, "Window Routing...", true, false);
    }
    #endif
    
    return menu;
}

// Update menuItemSelected to handle device window toggles
void MainComponent::menuItemSelected(int menuItemID, int /*topLevelMenuIndex*/)
{
    if (menuItemID == kSettingsMenuItemId)  // Settings
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
    else if (menuItemID == kLogDisplaySettingsMenuItemId) // Log Display Settings
    {
        if (logDisplaySettingsWindow == nullptr && midiLogDisplay != nullptr) {
            logDisplaySettingsWindow.reset(new LogDisplaySettingsWindow("Log Display Settings", *midiLogDisplay));
            logDisplaySettingsWindow->onCloseCallback = [this]() {
                logDisplaySettingsWindow.reset();
            };
            
            // X- Set background color to match system theme
            logDisplaySettingsWindow->setBackgroundColour(juce::LookAndFeel::getDefaultLookAndFeel()
                .findColour(juce::ResizableWindow::backgroundColourId));
        }
        if (logDisplaySettingsWindow != nullptr) {
            logDisplaySettingsWindow->toFront(true);
        }
    }
    else if (menuItemID == kWindowRoutingMenuItemId) // Window Routing
    {
        if (windowRoutingWindow == nullptr) {
            windowRoutingWindow.reset(new WindowRoutingWindow("Window Routing", windowManager));
            windowRoutingWindow->onCloseCallback = [this]() {
                windowRoutingWindow.reset();
            };
            
            // X- Set background color to match system theme
            windowRoutingWindow->setBackgroundColour(juce::LookAndFeel::getDefaultLookAndFeel()
                .findColour(juce::ResizableWindow::backgroundColourId));
        }
        windowRoutingWindow->toFront(true);
    }
    else if (menuItemID >= kViewModeListId && menuItemID <= kViewModeTimelineId)  // View modes
    {
        setViewMode(static_cast<ViewMode>(menuItemID - kViewModeListId));
    }
}

juce::StringArray MainComponent::getMenuBarNames()
{
    #if JUCE_MAC
    return { "File", "View" };
    #else
    return { "MidiPortal", "View" };
    #endif
}

int MainComponent::getNumMenuBarItems()
{
    return getMenuBarNames().size();
}

void MainComponent::setViewMode(ViewMode newMode)
{
    if (currentViewMode != newMode)
    {
        currentViewMode = newMode;
        updateCurrentView();
        updateViewMenu();
    }
}

void MainComponent::updateCurrentView()
{
    // For now, we only have the list view
    // Future implementations will handle grid and timeline views
    if (midiLogDisplay != nullptr)
    {
        midiLogDisplay->setVisible(currentViewMode == ViewMode::List);
    }
}

void MainComponent::updateViewMenu()
{
    #if JUCE_MAC
        viewMenu.clear();
        viewMenu.addItem(kViewModeListId, "List View", true, currentViewMode == ViewMode::List);
        viewMenu.addItem(kViewModeGridId, "Grid View", true, currentViewMode == ViewMode::Grid);
        viewMenu.addItem(kViewModeTimelineId, "Timeline View", true, currentViewMode == ViewMode::Timeline);
    #endif
    
    menuItemsChanged();
}

bool MainComponent::shouldProcessMidiMessage(const juce::MidiMessage& message, const juce::String& deviceName)
{
    // Find the device state
    auto it = std::find_if(deviceChannelStates.begin(), deviceChannelStates.end(),
                          [&](const auto& state) { return state.deviceName == deviceName; });
    
    // If device not found, create a new state with all channels enabled
    if (it == deviceChannelStates.end())
    {
        deviceChannelStates.emplace_back(deviceName);
        return true;  // Process message by default
    }
    
    // Check if the message's channel is enabled
    if (message.getChannel() > 0)  // MIDI channels are 1-16
    {
        int channelIndex = message.getChannel() - 1;
        if (channelIndex >= 0 && channelIndex < static_cast<int>(it->enabledChannels.size()))
            return it->enabledChannels[static_cast<size_t>(channelIndex)];
    }
        
    return true;  // Process non-channel messages by default
}

}  // namespace MidiPortal