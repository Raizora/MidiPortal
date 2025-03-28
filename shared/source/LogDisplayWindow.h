/**
 * @file LogDisplayWindow.h
 * @brief Defines a window for displaying MIDI message logs.
 * 
 * The LogDisplayWindow class creates a standalone window that contains a MidiLogDisplay
 * component for visualizing MIDI messages. Each window can be customized with its own
 * background color and can receive MIDI messages from specific devices.
 * 
 * These windows are managed by the WindowManager and use the DisplaySettingsManager
 * for visual customization.
 */

#pragma once
#include <juce_gui_extra/juce_gui_extra.h>
#include "MidiLogDisplay.h"
#include "DisplaySettingsManager.h"
#include <functional>

namespace MidiPortal {

/**
 * @class LogDisplayWindow
 * @brief A window that displays a log of MIDI messages.
 * 
 * This class creates a standalone window containing a MidiLogDisplay component
 * for visualizing MIDI messages. It listens for changes to display settings
 * and updates its appearance accordingly.
 * 
 * Each window can be customized with its own background color and can receive
 * MIDI messages from specific devices based on routing configured in the WindowManager.
 */
class LogDisplayWindow : public juce::DocumentWindow,
                        private juce::ChangeListener
{
public:
    /**
     * @brief Callback function type for window close events.
     * 
     * This callback is triggered when the window is closed, allowing
     * the owner to perform cleanup or update UI state.
     */
    std::function<void()> onCloseCallback;

    /**
     * @brief Constructor that creates a new log display window.
     * @param name The name of the window, which will appear in the title bar.
     * @param settingsManager Reference to the DisplaySettingsManager for visual settings.
     * 
     * Creates a new window with a MidiLogDisplay component and configures it
     * with the appropriate settings from the DisplaySettingsManager.
     */
    LogDisplayWindow(const juce::String& name, DisplaySettingsManager& settingsManager)
        : DocumentWindow(name + " - MIDI Log", juce::Colours::darkgrey, 
                         true, // use native title bar
                         juce::DocumentWindow::allButtons), // X- Enable all buttons including maximize
          displaySettingsManager(settingsManager)
    {
        // Create a new MidiLogDisplay that uses the settings manager
        auto* display = new MidiLogDisplay(settingsManager);
        
        // Set the window name on the display so it knows which settings to use
        display->setWindowName(name);
        
        setContentOwned(display, true);
        
        // X- Get the current display dimensions for proper resize limits
        auto mainDisplay = juce::Desktop::getInstance().getDisplays().getMainDisplay();
        auto userArea = mainDisplay.userArea;
        
        // X- Set resize limits based on the current display's dimensions
        setResizeLimits(400, 300, userArea.getWidth(), userArea.getHeight());
        centreWithSize(600, 400);
        
        // X- Enable fullscreen mode and proper window behavior
        setUsingNativeTitleBar(true);
        setResizable(true, true);
        setVisible(true);
        
        // Store the display pointer for message routing
        midiDisplay = display;
        
        // Listen for settings changes
        settingsManager.addChangeListener(this);
        
        // Apply initial background color
        applyBackgroundColor();
    }
    
    /**
     * @brief Destructor that cleans up resources and unregisters from the DisplaySettingsManager.
     * 
     * Stops listening for settings changes to prevent memory leaks and dangling references.
     */
    ~LogDisplayWindow() override
    {
        // Stop listening for settings changes
        displaySettingsManager.removeChangeListener(this);
    }

    /**
     * @brief Handles the window close button being pressed.
     * 
     * Hides the window and calls the onCloseCallback if one is set.
     * This allows the owner to perform cleanup or update UI state.
     */
    void closeButtonPressed() override
    {
        setVisible(false);
        if (onCloseCallback) onCloseCallback();
    }
    
    /**
     * @brief Adds a MIDI message to the display.
     * @param message The MIDI message to add.
     * @param deviceName The name of the device that sent the message.
     * 
     * Forwards the message to the contained MidiLogDisplay component.
     * This method is called by the WindowManager when a message is routed to this window.
     */
    void addMessage(const juce::MidiMessage& message, const juce::String& deviceName)
    {
        if (midiDisplay != nullptr)
            midiDisplay->addMessage(message, deviceName);
    }
    
    /**
     * @brief Applies the background color from settings.
     * 
     * Gets the background color from the DisplaySettingsManager for this window
     * and applies it to the window background. Handles the MAIN window specially
     * by using the Default settings.
     */
    void applyBackgroundColor()
    {
        // Get the window name without the " - MIDI Log" suffix
        juce::String windowName = getName();
        int suffixPos = windowName.indexOf(" - MIDI Log");
        if (suffixPos > 0)
            windowName = windowName.substring(0, suffixPos);
        
        // Use the correct settings based on window name
        if (windowName == "MAIN")
        {
            // For MAIN window, use the Default settings
            auto settings = displaySettingsManager.getSettings("Default");
            setBackgroundColour(settings.backgroundColor);
        }
        else
        {
            // For other windows, use their specific settings
            auto settings = displaySettingsManager.getSettings(windowName);
            setBackgroundColour(settings.backgroundColor);
        }
    }
    
    /**
     * @brief Handles change notifications from the DisplaySettingsManager.
     * @param source The ChangeBroadcaster that triggered the notification.
     * 
     * Called when display settings change, updating the window's background color
     * ONLY if the settings for THIS specific window have changed.
     */
    void changeListenerCallback(juce::ChangeBroadcaster* source) override
    {
        if (source == &displaySettingsManager)
        {
            // Get the window name without the " - MIDI Log" suffix
            juce::String windowName = getName();
            int suffixPos = windowName.indexOf(" - MIDI Log");
            if (suffixPos > 0)
                windowName = windowName.substring(0, suffixPos);
                
            // Only update if this is the MAIN window and Default settings changed,
            // or if this is another window and its specific settings changed
            if ((windowName == "MAIN" && displaySettingsManager.getSettings("Default").backgroundColor != getBackgroundColour()) ||
                (windowName != "MAIN" && displaySettingsManager.getSettings(windowName).backgroundColor != getBackgroundColour()))
            {
                // Settings have changed for this specific window, update the background color
                applyBackgroundColor();
            }
        }
    }

private:
    /**
     * @brief Pointer to the MidiLogDisplay component contained in this window.
     * 
     * This pointer is used to forward MIDI messages to the display component.
     * The memory is owned by the DocumentWindow base class through setContentOwned().
     */
    MidiLogDisplay* midiDisplay = nullptr;
    
    /**
     * @brief Reference to the DisplaySettingsManager for visual settings.
     * 
     * Used to get display settings for this window and to register for
     * change notifications when settings change.
     */
    DisplaySettingsManager& displaySettingsManager;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LogDisplayWindow)
};

} // namespace MidiPortal 