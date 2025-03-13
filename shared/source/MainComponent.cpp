/**
 * @file MainComponent.cpp
 * @brief Implementation of the MainComponent class.
 * 
 * This file contains the implementation of the MainComponent class, which is the central
 * component of the MidiPortal application. It handles MIDI input, manages the user interface,
 * and coordinates the various features of the application, such as MIDI logging, display settings,
 * and window routing.
 */

//
// Created by Jamie Benchia on 1/3/25.
//

#include "MainComponent.h"
#include <juce_audio_basics/juce_audio_basics.h> // Added for MidiMessage
#include <fstream> // Include for file operations
#include <iostream> // Include for console output
#include "MidiLogger.h"
#include "SettingsComponent.h"
#include "MidiAIManager.h"
#include "AIInsightComponent.h"
#include "WindowManager.h"

namespace MidiPortal {

/**
 * @class MainComponent::MidiInputCallback
 * @brief Handles incoming MIDI messages from connected devices.
 * 
 * This nested class implements the JUCE MidiInputCallback interface to receive
 * MIDI messages from connected devices. It forwards these messages to the
 * MainComponent for processing and display.
 */
class MainComponent::MidiInputCallback : public juce::MidiInputCallback {
public:
  /**
   * @brief Constructor that takes a reference to the parent MainComponent.
   * @param parentOwner Reference to the parent MainComponent.
   * 
   * Initializes the callback with a reference to the MainComponent that will
   * process the incoming MIDI messages.
   */
  explicit MidiInputCallback(MainComponent& parentOwner) : owner(parentOwner) {}
  
  /**
   * @brief Handles incoming MIDI messages from a MIDI input device.
   * @param source Pointer to the MIDI input device that sent the message.
   * @param message The MIDI message that was received.
   * 
   * This method is called by JUCE when a MIDI message is received from a
   * connected device. It asynchronously forwards the message to the MainComponent
   * for processing if the message's channel is enabled.
   */
  void handleIncomingMidiMessage(juce::MidiInput* source,
                                 const juce::MidiMessage& message) override {
      juce::MessageManager::callAsync([this, message, sourceName = source->getName()]() {
          // Only process if the channel is enabled
          if (owner.shouldProcessMidiMessage(message, sourceName)) {
              // Process the message
              owner.addMidiMessage(message, sourceName);
              
              // Trigger activity indicator
              owner.triggerMidiActivity(sourceName);
          }
      });
  }

private:
    MainComponent& owner;
};

/**
 * @class AIInsightTimer
 * @brief Timer for checking AI insights.
 * 
 * This class is a timer that periodically checks for AI insights.
 */
class AIInsightTimer : public juce::Timer
{
public:
    /**
     * @brief Constructor.
     * @param aiManager The AI manager to get insights from.
     * @param aiInsightComponent The component to display insights in.
     * 
     * Creates a new AIInsightTimer.
     */
    AIInsightTimer(MidiAIManager& aiManager, AIInsightComponent& aiInsightComponent)
        : aiManager(aiManager),
          aiInsightComponent(aiInsightComponent)
    {
    }
    
    /**
     * @brief Timer callback.
     * 
     * Called periodically to check for AI insights.
     */
    void timerCallback() override
    {
        // Get insights from the AI manager
        auto insights = aiManager.getInsights();
        
        // Add them to the insight component
        if (!insights.empty())
        {
            aiInsightComponent.addInsights(insights);
        }
    }
    
private:
    /**
     * @brief The AI manager to get insights from.
     * 
     * The AI manager that generates insights.
     */
    MidiAIManager& aiManager;
    
    /**
     * @brief The component to display insights in.
     * 
     * The component that displays insights.
     */
    AIInsightComponent& aiInsightComponent;
};

class MainComponent::Impl
{
public:
    Impl(MainComponent& owner, SettingsManager& settingsManager, WindowManager& windowManager)
        : owner(owner),
          aiManager(std::make_unique<MidiAIManager>()),
          aiInsightComponent(std::make_unique<AIInsightComponent>()),
          aiInsightTimer(std::make_unique<AIInsightTimer>(*aiManager, *aiInsightComponent))
    {
        // X- Added AI manager and insight component
        
        // Add the AI insight component
        owner.addAndMakeVisible(*aiInsightComponent);
        
        // Start the timer to check for insights every 500ms
        aiInsightTimer->startTimer(500);
    }
    
    ~Impl()
    {
        // Stop the AI insight timer
        aiInsightTimer->stopTimer();
    }
    
    void resized()
    {
        // Position the AI insight component at the bottom of the window
        const int insightHeight = 150;
        aiInsightComponent->setBounds(0, owner.getHeight() - insightHeight, owner.getWidth(), insightHeight);
    }
    
    void processMidiMessage(const juce::MidiMessage& message, const juce::String& deviceName)
    {
        // Process the message with the AI manager
        if (aiManager != nullptr) {
            aiManager->processMidiMessage(message, deviceName);
        }
    }
    
private:
    MainComponent& owner;
    std::unique_ptr<MidiAIManager> aiManager;
    std::unique_ptr<AIInsightComponent> aiInsightComponent;
    std::unique_ptr<AIInsightTimer> aiInsightTimer;
};

/**
 * @brief Constructor that initializes the MainComponent.
 * 
 * Sets up the MIDI input devices, initializes the Rust engine for MIDI processing,
 * creates the user interface components, and configures the menu bar.
 */
MainComponent::MainComponent()
    : settingsManager(), // Initialize settings manager
      windowManager(settingsManager.getDisplaySettingsManager()), // Initialize window manager with display settings manager
      impl(std::make_unique<Impl>(*this, settingsManager, windowManager))
{
    // Initialize device manager with no default devices
    settingsManager.getAudioDeviceManager().initialiseWithDefaultDevices(0, 0);  // No audio inputs/outputs
    
    // Initialize other components
    rustEngine = create_midi_engine();
    setSize(800, 800);
    
    // Initialize MIDI components
    midiInputCallback = std::make_unique<MidiInputCallback>(*this);
    midiLogger = std::make_unique<MidiPortal::MidiLogger>("MidiTraffic.log");
    
    // X- Initialize MidiLogDisplay with settings manager
    midiLogDisplay = std::make_unique<MidiLogDisplay>(settingsManager.getDisplaySettingsManager());
    addAndMakeVisible(midiLogDisplay.get());

    // Set up MIDI callback for the AudioDeviceManager
    settingsManager.getAudioDeviceManager().addMidiInputDeviceCallback({}, midiInputCallback.get());

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
    settingsComponent = std::make_unique<SettingsComponent>(settingsManager.getAudioDeviceManager());
    
    // X- Initialize the view
    updateCurrentView();
}

/**
 * @brief Destructor that cleans up resources.
 * 
 * Closes any open windows, cleans up the menu bar, stops MIDI input,
 * and destroys the Rust engine. The rest of the resources are cleaned up
 * automatically by the unique_ptr destructors.
 */
MainComponent::~MainComponent() {
  // Close any open windows first to prevent accessing destroyed components
  if (logDisplaySettingsWindow != nullptr) {
    logDisplaySettingsWindow->setVisible(false);
    logDisplaySettingsWindow.reset();
  }
  
  // Close any device windows
  deviceWindows.clear();
  
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

/**
 * @brief Adds a MIDI message to the display.
 * @param message The MIDI message to add.
 * @param deviceName The name of the device that sent the message.
 * 
 * This method is called when a MIDI message is received from a device.
 * It processes the message using the Rust engine, stores it for display/history,
 * logs it to the MIDI logger, and routes it to the appropriate displays.
 */
void MainComponent::addMidiMessage(const juce::MidiMessage& message, const juce::String& deviceName) {
    juce::MessageManager::callAsync([this, message, deviceName]() {
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
            
            // Log the message
            if (midiLogger) {
                midiLogger->logMessage(message);
            }
            
            // Route the message to all appropriate displays
            routeMidiMessage(message, deviceName);

            repaint();
        }
        catch (const std::exception& e) {
            DBG("Exception in addMidiMessage: " + juce::String(e.what()));
        }
    });
}

/**
 * @brief Paints the component.
 * @param g The Graphics context to paint into.
 * 
 * Fills the background with black and draws the application name
 * if no child components are visible.
 */
void MainComponent::paint(juce::Graphics& g) {
  // X- Only fill background if we're not showing a view
  if (getNumChildComponents() == 0) {
      g.fillAll(juce::Colours::black);
      g.setColour(juce::Colours::white);
      g.setFont(20.0f);
      g.drawText("MidiPortal", getLocalBounds(), juce::Justification::centred, true);
  }
}

/**
 * @brief Handles component resizing.
 * 
 * Resizes the MidiLogDisplay to fill the entire component.
 */
void MainComponent::resized()
{
    impl->resized();
}

/**
 * @brief Triggers activity indicators for a specific device.
 * @param deviceName The name of the device to trigger activity for.
 * 
 * This method is called when a MIDI message is received from a device,
 * and triggers the activity indicator for that device in the settings component.
 */
void MainComponent::triggerMidiActivity(const juce::String& deviceName)
{
    if (settingsComponent != nullptr)
    {
        settingsComponent->triggerActivityForDevice(deviceName);
    }
}

/**
 * @brief Gets the menu for a specific menu bar index.
 * @param index The index of the menu in the menu bar.
 * @param name The name of the menu.
 * @return A PopupMenu containing the menu items for the specified menu.
 * 
 * This method is called by JUCE to get the menu for a specific menu bar index.
 * It returns different menus based on the platform (Mac or other) and the menu name.
 */
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

/**
 * @brief Handles menu item selection.
 * @param menuItemID The ID of the selected menu item.
 * @param topLevelMenuIndex The index of the top-level menu containing the selected item.
 * 
 * This method is called when a menu item is selected. It handles different
 * actions based on the selected menu item, such as opening settings windows
 * or changing the view mode.
 */
void MainComponent::menuItemSelected(int menuItemID, int topLevelMenuIndex)
{
    if (menuItemID == kSettingsMenuItemId)
    {
        if (settingsWindow == nullptr) {
            settingsWindow.reset(new SettingsWindow("MidiPortal Settings", settingsManager.getAudioDeviceManager()));
            settingsWindow->onCloseCallback = [this]() {
                settingsWindow.reset();
            };
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
    else if (menuItemID == kWindowRoutingMenuItemId)
    {
        if (windowRoutingWindow == nullptr) {
            windowRoutingWindow.reset(new WindowRoutingWindow("Window Routing", windowManager));
            windowRoutingWindow->onCloseCallback = [this]() {
                windowRoutingWindow.reset();
            };
            
            // Set the background color to match the application
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

/**
 * @brief Gets the names of the menu bar items.
 * @return A StringArray containing the names of the menu bar items.
 * 
 * This method is called by JUCE to get the names of the menu bar items.
 * It returns different names based on the platform (Mac or other).
 */
juce::StringArray MainComponent::getMenuBarNames()
{
    #if JUCE_MAC
    return { "File", "View" };
    #else
    return { "MidiPortal", "View" };
    #endif
}

/**
 * @brief Gets the number of menu bar items.
 * @return The number of menu bar items.
 * 
 * This method is called by JUCE to get the number of menu bar items.
 */
int MainComponent::getNumMenuBarItems()
{
    return getMenuBarNames().size();
}

/**
 * @brief Sets the current view mode.
 * @param newMode The new view mode to set.
 * 
 * Changes the current view mode and updates the UI to reflect the change.
 */
void MainComponent::setViewMode(ViewMode newMode)
{
    if (currentViewMode != newMode)
    {
        currentViewMode = newMode;
        updateCurrentView();
        updateViewMenu();
    }
}

/**
 * @brief Updates the current view based on the view mode.
 * 
 * Shows or hides components based on the current view mode.
 * Currently only handles the list view, but will be expanded
 * to handle grid and timeline views in the future.
 */
void MainComponent::updateCurrentView()
{
    // For now, we only have the list view
    // Future implementations will handle grid and timeline views
    if (midiLogDisplay != nullptr)
    {
        midiLogDisplay->setVisible(currentViewMode == ViewMode::List);
    }
}

/**
 * @brief Updates the view menu to reflect the current view mode.
 * 
 * Updates the view menu items to show the current view mode as selected.
 */
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

/**
 * @brief Determines if a MIDI message should be processed.
 * @param message The MIDI message to check.
 * @param deviceName The name of the device that sent the message.
 * @return true if the message should be processed, false otherwise.
 * 
 * Checks if the MIDI message's channel is enabled for the specified device.
 * If the device is not found in the device channel states, a new state is created
 * with all channels enabled.
 */
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

/**
 * @brief Routes a MIDI message to the appropriate windows.
 * @param message The MIDI message to route.
 * @param deviceName The name of the device that sent the message.
 * 
 * Routes a MIDI message to the appropriate windows based on the device
 * and window routing configuration.
 */
void MainComponent::routeMidiMessage(const juce::MidiMessage& message, const juce::String& deviceName)
{
    // Always add to main display if it exists
    if (midiLogDisplay != nullptr) {
        midiLogDisplay->addMessage(message, deviceName);
    }
    
    // Route to device-specific windows through the window manager
    windowManager.routeMidiMessage(message, deviceName);
    
    // Also send to AI manager for processing
    if (impl != nullptr) {
        impl->processMidiMessage(message, deviceName);
    }
}

}  // namespace MidiPortal