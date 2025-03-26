/**
 * @file LogDisplaySettingsWindow.h
 * @brief Defines a dialog window for configuring MIDI log display settings.
 * 
 * This file defines the LogDisplaySettingsWindow class, which creates a dialog window
 * containing a LogDisplaySettingsComponent for configuring the visual appearance
 * of MIDI message logs in the MidiPortal application.
 */

#pragma once
#include <juce_gui_extra/juce_gui_extra.h>
#include "LogDisplaySettingsComponent.h"
#include <functional>

namespace MidiPortal {

/**
 * @class LogDisplaySettingsWindow
 * @brief A dialog window for configuring MIDI log display settings.
 * 
 * This class creates a dialog window containing a LogDisplaySettingsComponent
 * for configuring the visual appearance of MIDI message logs, including colors
 * for different message types, font size, and other display properties.
 * 
 * The window is typically opened from the main application menu and
 * allows users to customize how MIDI messages are displayed in the log.
 */
class LogDisplaySettingsWindow : public juce::DialogWindow
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
     * @brief Constructor that creates a new log display settings dialog.
     * @param name The name of the window, which will appear in the title bar.
     * @param logDisplay Reference to the MidiLogDisplay to configure.
     * 
     * Creates a new dialog window with a LogDisplaySettingsComponent and configures it
     * with a reference to the MidiLogDisplay to be configured.
     */
    LogDisplaySettingsWindow(const juce::String& name, MidiLogDisplay& logDisplay)
        : DialogWindow(name, juce::Colours::lightgrey, true, true)
    {
        auto* content = new LogDisplaySettingsComponent(logDisplay);
        
        setContentOwned(content, true);
        
        // X- Use native title bar with all window buttons
        setUsingNativeTitleBar(true);
        setTitleBarButtonsRequired(juce::DocumentWindow::allButtons, false);
        
        // X- Get the current display dimensions for proper resize limits
        auto mainDisplay = juce::Desktop::getInstance().getDisplays().getMainDisplay();
        auto userArea = mainDisplay.userArea;
        
        // X- Set resize limits based on the current display's dimensions
        setResizable(true, true);  
        setResizeLimits(600, 600, userArea.getWidth(), userArea.getHeight());
        centreWithSize(600, 600);
        
        setVisible(true);
        setAlwaysOnTop(true);
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
};

} // namespace MidiPortal 