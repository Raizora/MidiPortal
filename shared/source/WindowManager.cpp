#include "WindowManager.h"

namespace MidiPortal {

WindowManager::WindowManager(DisplaySettingsManager& settingsManager)
    : displaySettingsManager(settingsManager)
{
    // MAIN is the main application window that's always present
    // We'll include it in our window list for routing purposes
    // but we don't need to create it as it's already part of the application
}

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

bool WindowManager::hasWindow(const juce::String& windowName) const
{
    // MAIN is always considered to exist, even though it's not in our windows map
    if (windowName == "MAIN")
        return true;
        
    return windows.find(windowName) != windows.end();
}

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

void WindowManager::routeDeviceToWindow(const juce::String& deviceName, const juce::String& windowName)
{
    if (hasWindow(windowName))
    {
        deviceToWindows[deviceName].insert(windowName);
        windowToDevices[windowName].insert(deviceName);
    }
}

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

bool WindowManager::isDeviceRoutedToWindow(const juce::String& deviceName, const juce::String& windowName) const
{
    auto it = deviceToWindows.find(deviceName);
    return it != deviceToWindows.end() && it->second.find(windowName) != it->second.end();
}

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

void WindowManager::registerWindow(juce::Component* window)
{
    if (window != nullptr && !registeredWindows.contains(window))
    {
        registeredWindows.add(window);
    }
}

void WindowManager::unregisterWindow(juce::Component* window)
{
    registeredWindows.removeFirstMatchingValue(window);
}

DisplaySettingsManager& WindowManager::getSettingsManager()
{
    return displaySettingsManager;
}

} // namespace MidiPortal 