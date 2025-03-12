/**
 * @file WindowRoutingWindow.h
 * @brief Defines a dialog window for configuring MIDI device routing to display windows.
 * 
 * The WindowRoutingWindow class creates a dialog window that contains a WindowRoutingComponent,
 * which allows users to configure which MIDI devices are routed to which display windows.
 * This window also provides controls for creating new windows and customizing their appearance.
 */

#pragma once
#include <juce_gui_extra/juce_gui_extra.h>
#include "WindowRoutingComponent.h"
#include <functional>

namespace MidiPortal {

/**
 * @class WindowRoutingWindow
 * @brief A dialog window for configuring MIDI device routing to display windows.
 * 
 * This class creates a dialog window containing a WindowRoutingComponent,
 * which provides a user interface for:
 * - Creating new display windows
 * - Routing MIDI devices to specific windows
 * - Customizing window background colors
 * 
 * The window is typically opened from the main application menu and
 * allows users to configure how MIDI messages are distributed across
 * multiple display windows.
 */
class WindowRoutingWindow : public juce::DialogWindow
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
     * @brief Constructor that creates a new window routing dialog.
     * @param name The name of the window, which will appear in the title bar.
     * @param windowManager Reference to the WindowManager for creating and routing windows.
     * 
     * Creates a new dialog window with a WindowRoutingComponent and configures it
     * with the provided WindowManager reference.
     */
    WindowRoutingWindow(const juce::String& name, WindowManager& windowManager)
        : DialogWindow(name, juce::Colours::lightgrey, true, true)
    {
        auto* content = new WindowRoutingComponent(windowManager);
        
        setContentOwned(content, true);
        setResizeLimits(400, 300, 1200, 1200);
        centreWithSize(600, 400);
        
        setResizable(true, true);
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