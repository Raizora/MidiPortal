/**
 * @file WindowManager.cpp
 * @brief Implementation of the WindowManager class.
 * 
 * This file contains the implementation of the WindowManager class methods,
 * which manage multiple display windows and MIDI device routing in the MidiPortal application.
 */

#include "WindowManager.h"

namespace MidiPortal {

/**
 * @brief Constructor that takes a reference to the DisplaySettingsManager.
 * @param settingsManager Reference to the DisplaySettingsManager that will be used for window settings.
 * 
 * Initializes the WindowManager with a reference to the DisplaySettingsManager.
 * The "MAIN" window is considered to always exist as it's the main application window.
 */
WindowManager::WindowManager(DisplaySettingsManager& settingsManager)
    : displaySettingsManager(settingsManager)
{
    // MAIN is the main application window that's always present
    // We'll include it in our window list for routing purposes
    // but we don't need to create it as it's already part of the application
}

/**
 * @brief Creates a new display window with the specified name.
 * @param windowName The name for the new window.
 * 
 * Creates a new LogDisplayWindow with the specified name and registers it
 * with the DisplaySettingsManager. If a window with the same name already
 * exists or if the name is "MAIN", this method does nothing.
 * 
 * Each new window is assigned a unique background color based on its name
 * to help visually distinguish between different windows.
 */
void WindowManager::createWindow(const juce::String& windowName)
{
    // Don't allow creating a window named "MAIN" as it's the main application window
    if (windowName == "MAIN")
        return;
        
    if (!hasWindow(windowName))
    {
        // Create a new window
        auto window = std::make_unique<LogDisplayWindow>(windowName, displaySettingsManager);
        window->onCloseCallback = [this, windowName]() {
            closeWindow(windowName);
        };
        
        // Create custom settings for this window
        // Start with default settings but give it a unique background color
        auto settings = displaySettingsManager.getSettings("Default");
        
        // Generate a unique color based on the window name
        // This ensures each window gets its own color
        juce::Random random(windowName.hashCode());
        juce::Colour uniqueColor = juce::Colour::fromHSV(
            random.nextFloat(), // Hue (0.0 - 1.0)
            0.7f,               // Saturation (0.7 for vibrant colors)
            0.3f,               // Brightness (0.3 for darker colors)
            1.0f                // Alpha (fully opaque)
        );
        
        // Set the unique background color
        settings.backgroundColor = uniqueColor;
        
        // Add the settings to the settings manager
        displaySettingsManager.addSettings(windowName, settings);
        
        // Store the window
        windows[windowName] = std::move(window);
    }
}

/**
 * @brief Closes and destroys a window with the specified name.
 * @param windowName The name of the window to close.
 * 
 * Closes the specified window and removes all device routings to it.
 * If the window doesn't exist or if the name is "MAIN", this method does nothing.
 * The "MAIN" window cannot be closed as it's the main application window.
 */
void WindowManager::closeWindow(const juce::String& windowName)
{
    // Don't allow closing the MAIN window as it's the main application window
    if (windowName == "MAIN")
        return;
        
    // Remove all device routings for this window
    auto devicesIt = windowToDevices.find(windowName);
    if (devicesIt != windowToDevices.end())
    {
        for (const auto& deviceName : devicesIt->second)
        {
            auto& windowSet = deviceToWindows[deviceName];
            windowSet.erase(windowName);
            if (windowSet.empty())
                deviceToWindows.erase(deviceName);
        }
        windowToDevices.erase(devicesIt);
    }

    // Close and remove the window
    windows.erase(windowName);
}

/**
 * @brief Checks if a window with the specified name exists.
 * @param windowName The name of the window to check.
 * @return true if the window exists, false otherwise.
 * 
 * Note that "MAIN" is always considered to exist, as it's the main application window,
 * even though it's not stored in the windows map.
 */
bool WindowManager::hasWindow(const juce::String& windowName) const
{
    // MAIN is always considered to exist, even though it's not in our windows map
    if (windowName == "MAIN")
        return true;
        
    return windows.find(windowName) != windows.end();
}

/**
 * @brief Gets a list of all window names.
 * @return A StringArray containing the names of all windows.
 * 
 * The returned list always includes "MAIN" (the main application window)
 * as the first item, followed by any additional windows.
 */
juce::StringArray WindowManager::getWindowNames() const
{
    juce::StringArray names;
    
    // Always add MAIN first - this is the main application window
    names.add("MAIN");
    
    // Add other windows
    for (const auto& [name, window] : windows)
    {
        if (name != "MAIN") // Skip MAIN as we already added it
            names.add(name);
    }
    return names;
}

/**
 * @brief Routes a MIDI device to a specific window.
 * @param deviceName The name of the MIDI device to route.
 * @param windowName The name of the window to route the device to.
 * 
 * After routing, MIDI messages from the specified device will be
 * forwarded to the specified window. If the window doesn't exist,
 * this method does nothing.
 */
void WindowManager::routeDeviceToWindow(const juce::String& deviceName, const juce::String& windowName)
{
    if (hasWindow(windowName))
    {
        deviceToWindows[deviceName].insert(windowName);
        windowToDevices[windowName].insert(deviceName);
    }
}

/**
 * @brief Removes a routing between a MIDI device and a window.
 * @param deviceName The name of the MIDI device to unroute.
 * @param windowName The name of the window to remove the device from.
 * 
 * After unrouting, MIDI messages from the specified device will no
 * longer be forwarded to the specified window. If the device or window
 * doesn't exist, this method does nothing.
 */
void WindowManager::unrouteDeviceFromWindow(const juce::String& deviceName, const juce::String& windowName)
{
    auto deviceIt = deviceToWindows.find(deviceName);
    if (deviceIt != deviceToWindows.end())
    {
        deviceIt->second.erase(windowName);
        if (deviceIt->second.empty())
            deviceToWindows.erase(deviceIt);
    }

    auto windowIt = windowToDevices.find(windowName);
    if (windowIt != windowToDevices.end())
    {
        windowIt->second.erase(deviceName);
        if (windowIt->second.empty())
            windowToDevices.erase(windowIt);
    }
}

/**
 * @brief Checks if a MIDI device is routed to a specific window.
 * @param deviceName The name of the MIDI device to check.
 * @param windowName The name of the window to check.
 * @return true if the device is routed to the window, false otherwise.
 */
bool WindowManager::isDeviceRoutedToWindow(const juce::String& deviceName, const juce::String& windowName) const
{
    auto it = deviceToWindows.find(deviceName);
    return it != deviceToWindows.end() && it->second.find(windowName) != it->second.end();
}

/**
 * @brief Gets a list of all windows that a MIDI device is routed to.
 * @param deviceName The name of the MIDI device to check.
 * @return A StringArray containing the names of all windows the device is routed to.
 * 
 * If the device isn't routed to any windows, an empty StringArray is returned.
 */
juce::StringArray WindowManager::getWindowsForDevice(const juce::String& deviceName) const
{
    juce::StringArray windowNames;
    auto it = deviceToWindows.find(deviceName);
    if (it != deviceToWindows.end())
    {
        for (const auto& windowName : it->second)
            windowNames.add(windowName);
    }
    return windowNames;
}

/**
 * @brief Gets a list of all MIDI devices that are routed to a window.
 * @param windowName The name of the window to check.
 * @return A StringArray containing the names of all devices routed to the window.
 * 
 * If no devices are routed to the window, an empty StringArray is returned.
 */
juce::StringArray WindowManager::getDevicesForWindow(const juce::String& windowName) const
{
    juce::StringArray deviceNames;
    auto it = windowToDevices.find(windowName);
    if (it != windowToDevices.end())
    {
        for (const auto& deviceName : it->second)
            deviceNames.add(deviceName);
    }
    return deviceNames;
}

/**
 * @brief Routes a MIDI message to all windows that the specified device is routed to.
 * @param message The MIDI message to route.
 * @param deviceName The name of the MIDI device that the message came from.
 * 
 * This method is called when a MIDI message is received from a device,
 * and forwards the message to all windows that the device is routed to.
 * If the device isn't routed to any windows, this method does nothing.
 */
void WindowManager::routeMidiMessage(const juce::MidiMessage& message, const juce::String& deviceName)
{
    auto it = deviceToWindows.find(deviceName);
    if (it != deviceToWindows.end())
    {
        for (const auto& windowName : it->second)
        {
            auto windowIt = windows.find(windowName);
            if (windowIt != windows.end() && windowIt->second != nullptr)
            {
                windowIt->second->addMessage(message, deviceName);
            }
        }
    }
}

/**
 * @brief Registers a window component with the WindowManager.
 * @param window Pointer to the window component to register.
 * 
 * This allows the WindowManager to track windows that aren't LogDisplayWindows.
 * If the window is already registered, this method does nothing.
 */
void WindowManager::registerWindow(juce::Component* window)
{
    if (window != nullptr && !registeredWindows.contains(window))
    {
        registeredWindows.add(window);
    }
}

/**
 * @brief Unregisters a window component from the WindowManager.
 * @param window Pointer to the window component to unregister.
 * 
 * This should be called when a window is closed to prevent memory leaks.
 * If the window isn't registered, this method does nothing.
 */
void WindowManager::unregisterWindow(juce::Component* window)
{
    registeredWindows.removeFirstMatchingValue(window);
}

/**
 * @brief Gets a reference to the DisplaySettingsManager.
 * @return A reference to the DisplaySettingsManager.
 * 
 * This allows other components to access the DisplaySettingsManager
 * through the WindowManager.
 */
DisplaySettingsManager& WindowManager::getSettingsManager()
{
    return displaySettingsManager;
}

} // namespace MidiPortal 