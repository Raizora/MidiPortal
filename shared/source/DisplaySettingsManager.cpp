// DisplaySettingsManager is like a database that stores and manages the settings
/**
 * @file DisplaySettingsManager.cpp
 * @brief Implementation of the DisplaySettingsManager class.
 * 
 * This file contains the implementation of the DisplaySettingsManager class methods,
 * which manage display settings for MIDI message visualization across the application.
 */

#include "DisplaySettingsManager.h"
#include "MidiLogDisplay.h"

namespace MidiPortal {

/**
 * @brief Constructor that initializes the default settings.
 * 
 * Sets up the default color scheme and font size that will be used when
 * no device-specific settings are available.
 */
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

/**
 * @brief Add new settings for a device or window.
 * @param deviceName The name of the device or window to add settings for.
 * @param settings The settings to add.
 * 
 * Adds or updates settings for the specified device/window and notifies
 * all registered listeners about the change.
 */
void DisplaySettingsManager::addSettings(const juce::String& deviceName, const DisplaySettings& settings)
{
    deviceSettings[deviceName] = settings;
    sendChangeMessage();
}

/**
 * @brief Get settings for a specific device or window.
 * @param deviceName The name of the device or window to get settings for.
 * @return A reference to the settings for the specified device/window.
 * 
 * If no settings exist for the specified device, returns the default settings.
 * This method is const to allow read-only access to settings.
 */
const DisplaySettingsManager::DisplaySettings& DisplaySettingsManager::getSettings(const juce::String& deviceName) const
{
    auto it = deviceSettings.find(deviceName);
    if (it != deviceSettings.end())
        return it->second;
    return defaultSettings;
}

/**
 * @brief Update settings for a specific device or window.
 * @param settings The new settings to apply.
 * @param deviceName The name of the device or window to update settings for.
 * 
 * Updates the settings for the specified device/window and notifies
 * all registered listeners about the change.
 */
void DisplaySettingsManager::setSettings(const DisplaySettings& settings, const juce::String& deviceName)
{
    deviceSettings[deviceName] = settings;
    sendChangeMessage();
}

/**
 * @brief Register a display component to receive settings updates.
 * @param display Pointer to the MidiLogDisplay to register.
 * 
 * Adds the display to the list of components that should be notified
 * when settings change. This is separate from the ChangeBroadcaster
 * mechanism and allows direct method calls on the display.
 */
void DisplaySettingsManager::registerDisplay(MidiLogDisplay* display)
{
    if (display != nullptr)
        registeredDisplays.push_back(display);
}

/**
 * @brief Unregister a display component from receiving settings updates.
 * @param display Pointer to the MidiLogDisplay to unregister.
 * 
 * Removes the display from the list of components that should be notified
 * when settings change. This prevents memory leaks when displays are destroyed.
 */
void DisplaySettingsManager::unregisterDisplay(MidiLogDisplay* display)
{
    auto it = std::find(registeredDisplays.begin(), registeredDisplays.end(), display);
    if (it != registeredDisplays.end())
        registeredDisplays.erase(it);
}

/**
 * @brief Get a list of all devices/windows that have custom settings.
 * @return A StringArray containing the names of all devices/windows with custom settings.
 * 
 * Returns a list of all device/window names that have custom settings defined.
 * The "Default" settings are excluded from this list.
 */
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