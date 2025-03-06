// DisplaySettingsManager is like a database that stores and manages the settings

#include "DisplaySettingsManager.h"
#include "MidiLogDisplay.h"

namespace MidiPortal {

DisplaySettingsManager::DisplaySettingsManager()
{
    // Initialize with default settings
    defaultSettings.fontSize = 12.0f;
    defaultSettings.backgroundColor = juce::Colours::black;
    defaultSettings.noteOnColor = juce::Colours::green;
    defaultSettings.noteOffColor = juce::Colours::red;
    defaultSettings.controllerColor = juce::Colours::yellow;
    defaultSettings.pitchBendColor = juce::Colours::orange;
    defaultSettings.pressureColor = juce::Colours::purple;
    defaultSettings.programChangeColor = juce::Colours::blue;
    defaultSettings.clockColor = juce::Colours::grey;
    defaultSettings.sysExColor = juce::Colours::white;
    defaultSettings.defaultColor = juce::Colours::lightgrey;
}

void DisplaySettingsManager::addSettings(const juce::String& deviceName, const DisplaySettings& settings)
{
    deviceSettings[deviceName] = settings;
    sendChangeMessage();
}

const DisplaySettingsManager::DisplaySettings& DisplaySettingsManager::getSettings(const juce::String& deviceName) const
{
    auto it = deviceSettings.find(deviceName);
    if (it != deviceSettings.end())
        return it->second;
    return defaultSettings;
}

void DisplaySettingsManager::setSettings(const DisplaySettings& settings, const juce::String& deviceName)
{
    deviceSettings[deviceName] = settings;
    sendChangeMessage();
}

void DisplaySettingsManager::registerDisplay(MidiLogDisplay* display)
{
    if (display != nullptr)
        registeredDisplays.push_back(display);
}

void DisplaySettingsManager::unregisterDisplay(MidiLogDisplay* display)
{
    auto it = std::find(registeredDisplays.begin(), registeredDisplays.end(), display);
    if (it != registeredDisplays.end())
        registeredDisplays.erase(it);
}

juce::StringArray DisplaySettingsManager::getDevicesWithCustomSettings() const
{
    juce::StringArray devices;
    for (const auto& [device, settings] : deviceSettings)
    {
        if (device != "Default")
        {
            devices.add(device);
        }
    }
    return devices;
}

} // namespace MidiPortal 