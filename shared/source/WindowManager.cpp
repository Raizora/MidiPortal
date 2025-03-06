#include "WindowManager.h"

namespace MidiPortal {

WindowManager::WindowManager(DisplaySettingsManager& settingsManager)
    : displaySettingsManager(settingsManager)
{
}

void WindowManager::createWindow(const juce::String& windowName)
{
    if (!hasWindow(windowName))
    {
        auto window = std::make_unique<LogDisplayWindow>(windowName, displaySettingsManager);
        window->onCloseCallback = [this, windowName]() {
            closeWindow(windowName);
        };
        windows[windowName] = std::move(window);
    }
}

void WindowManager::closeWindow(const juce::String& windowName)
{
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
    return windows.find(windowName) != windows.end();
}

juce::StringArray WindowManager::getWindowNames() const
{
    juce::StringArray names;
    for (const auto& [name, window] : windows)
        names.add(name);
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