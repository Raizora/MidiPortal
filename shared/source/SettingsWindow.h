/**
 * @file SettingsWindow.h
 * @brief Defines a dialog window for configuring application settings.
 * 
 * This file defines the SettingsWindow class, which creates a dialog window
 * containing a SettingsComponent for configuring audio and MIDI device settings
 * in the MidiPortal application. The window provides scrolling capabilities
 * to accommodate settings that don't fit in the visible area.
 */

#pragma once
#include <juce_gui_extra/juce_gui_extra.h>
#include "SettingsComponent.h"
#include <functional>

namespace MidiPortal {

/**
 * @class SettingsWindow
 * @brief A dialog window for configuring application settings.
 * 
 * This class creates a dialog window containing a SettingsComponent for
 * configuring audio and MIDI device settings in the MidiPortal application.
 * The window provides scrolling capabilities to accommodate settings that
 * don't fit in the visible area.
 * 
 * The window is typically opened from the main application menu and
 * allows users to configure audio and MIDI devices.
 */
class SettingsWindow : public juce::DialogWindow
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
     * @brief Constructor that creates a new settings dialog.
     * @param name The name of the window, which will appear in the title bar.
     * @param deviceManager Reference to the application's AudioDeviceManager.
     * 
     * Creates a new dialog window with a SettingsComponent and configures it
     * with the provided AudioDeviceManager reference. The window includes
     * a viewport for scrolling if the settings don't fit in the visible area.
     */
    SettingsWindow(const juce::String& name, juce::AudioDeviceManager& deviceManager)
        : DialogWindow(name, juce::Colours::lightgrey, true, true)
    {
        auto* content = new SettingsComponent(deviceManager);
        
        // X- Create viewport for scrolling
        auto* viewport = new juce::Viewport();
        viewport->setViewedComponent(content, true);
        viewport->setScrollBarsShown(true, true);
        
        // X- Make viewport fill the window and resize with it
        viewport->setSize(550, 400);
        
        setContentOwned(viewport, true);
        
        // X- Use native title bar with all window buttons
        setUsingNativeTitleBar(true);
        setTitleBarButtonsRequired(juce::DocumentWindow::allButtons, false);
        
        // X- Get the current display dimensions for proper resize limits
        auto mainDisplay = juce::Desktop::getInstance().getDisplays().getMainDisplay();
        auto userArea = mainDisplay.userArea;
        
        // X- Set resize limits based on the current display's dimensions
        setResizeLimits(500, 300, userArea.getWidth(), userArea.getHeight());
        centreWithSize(550, 400);
        
        // X- Make sure content gets viewport's width
        content->setSize(viewport->getWidth(), content->getHeight());
        
        setResizable(true, true);  // Allow both horizontal and vertical resizing
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