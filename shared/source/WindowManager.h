/**
 * @file WindowManager.h
 * @brief Manages multiple display windows and MIDI device routing.
 * 
 * The WindowManager is responsible for creating, managing, and routing MIDI data
 * to multiple display windows in the MidiPortal application. It maintains the
 * relationships between MIDI devices and display windows, allowing MIDI messages
 * from specific devices to be routed to specific windows.
 * 
 * This class works closely with DisplaySettingsManager to ensure each window
 * has appropriate visual settings.
 */

#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "DisplaySettingsManager.h"
#include "LogDisplayWindow.h"
#include <map>
#include <set>

namespace MidiPortal {

/**
 * @class WindowManager
 * @brief Manages the creation, destruction, and routing of MIDI display windows.
 * 
 * This class is responsible for:
 * - Creating and destroying display windows
 * - Managing the routing of MIDI devices to specific windows
 * - Forwarding MIDI messages to the appropriate windows based on routing
 * - Tracking which devices are routed to which windows
 */
class WindowManager
{
public:
    /**
     * @brief Constructor that takes a reference to the DisplaySettingsManager.
     * @param settingsManager Reference to the DisplaySettingsManager that will be used for window settings.
     * 
     * The WindowManager uses the DisplaySettingsManager to ensure each window
     * has appropriate visual settings.
     */
    explicit WindowManager(DisplaySettingsManager& settingsManager);
    
    /**
     * @brief Creates a new display window with the specified name.
     * @param windowName The name for the new window.
     * 
     * Creates a new LogDisplayWindow with the specified name and registers it
     * with the DisplaySettingsManager. If a window with the same name already
     * exists, this method does nothing.
     */
    void createWindow(const juce::String& windowName);
    
    /**
     * @brief Closes and destroys a window with the specified name.
     * @param windowName The name of the window to close.
     * 
     * Closes the specified window and removes all device routings to it.
     * If the window doesn't exist, this method does nothing.
     */
    void closeWindow(const juce::String& windowName);
    
    /**
     * @brief Checks if a window with the specified name exists.
     * @param windowName The name of the window to check.
     * @return true if the window exists, false otherwise.
     * 
     * Note that "MAIN" is always considered to exist, as it's the main application window.
     */
    bool hasWindow(const juce::String& windowName) const;
    
    /**
     * @brief Gets a list of all window names.
     * @return A StringArray containing the names of all windows.
     * 
     * The returned list always includes "MAIN" (the main application window)
     * as the first item, followed by any additional windows.
     */
    juce::StringArray getWindowNames() const;
    
    /**
     * @brief Routes a MIDI device to a specific window.
     * @param deviceName The name of the MIDI device to route.
     * @param windowName The name of the window to route the device to.
     * 
     * After routing, MIDI messages from the specified device will be
     * forwarded to the specified window.
     */
    void routeDeviceToWindow(const juce::String& deviceName, const juce::String& windowName);
    
    /**
     * @brief Removes a routing between a MIDI device and a window.
     * @param deviceName The name of the MIDI device to unroute.
     * @param windowName The name of the window to remove the device from.
     * 
     * After unrouting, MIDI messages from the specified device will no
     * longer be forwarded to the specified window.
     */
    void unrouteDeviceFromWindow(const juce::String& deviceName, const juce::String& windowName);
    
    /**
     * @brief Checks if a MIDI device is routed to a specific window.
     * @param deviceName The name of the MIDI device to check.
     * @param windowName The name of the window to check.
     * @return true if the device is routed to the window, false otherwise.
     */
    bool isDeviceRoutedToWindow(const juce::String& deviceName, const juce::String& windowName) const;
    
    /**
     * @brief Gets a list of all windows that a MIDI device is routed to.
     * @param deviceName The name of the MIDI device to check.
     * @return A StringArray containing the names of all windows the device is routed to.
     */
    juce::StringArray getWindowsForDevice(const juce::String& deviceName) const;
    
    /**
     * @brief Gets a list of all MIDI devices that are routed to a window.
     * @param windowName The name of the window to check.
     * @return A StringArray containing the names of all devices routed to the window.
     */
    juce::StringArray getDevicesForWindow(const juce::String& windowName) const;
    
    /**
     * @brief Routes a MIDI message to all windows that the specified device is routed to.
     * @param message The MIDI message to route.
     * @param deviceName The name of the MIDI device that the message came from.
     * 
     * This method is called when a MIDI message is received from a device,
     * and forwards the message to all windows that the device is routed to.
     */
    void routeMidiMessage(const juce::MidiMessage& message, const juce::String& deviceName);
    
    /**
     * @brief Registers a window component with the WindowManager.
     * @param window Pointer to the window component to register.
     * 
     * This allows the WindowManager to track windows that aren't LogDisplayWindows.
     */
    void registerWindow(juce::Component* window);
    
    /**
     * @brief Unregisters a window component from the WindowManager.
     * @param window Pointer to the window component to unregister.
     * 
     * This should be called when a window is closed to prevent memory leaks.
     */
    void unregisterWindow(juce::Component* window);
    
    /**
     * @brief Gets a reference to the DisplaySettingsManager.
     * @return A reference to the DisplaySettingsManager.
     */
    DisplaySettingsManager& getSettingsManager();

private:
    /**
     * @brief Reference to the DisplaySettingsManager used for window settings.
     * 
     * This is used to ensure each window has appropriate visual settings.
     */
    DisplaySettingsManager& displaySettingsManager;
    
    /**
     * @brief Map of window names to LogDisplayWindow instances.
     * 
     * This stores all the LogDisplayWindows that have been created.
     */
    std::map<juce::String, std::unique_ptr<LogDisplayWindow>> windows;
    
    /**
     * @brief Map of device names to the set of windows they are routed to.
     * 
     * This allows quick lookup of which windows a device is routed to.
     */
    std::map<juce::String, std::set<juce::String>> deviceToWindows;
    
    /**
     * @brief Map of window names to the set of devices routed to them.
     * 
     * This allows quick lookup of which devices are routed to a window.
     */
    std::map<juce::String, std::set<juce::String>> windowToDevices;
    
    /**
     * @brief Array of registered window components.
     * 
     * This tracks windows that aren't LogDisplayWindows.
     */
    juce::Array<juce::Component*> registeredWindows;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WindowManager)
};

} // namespace MidiPortal 