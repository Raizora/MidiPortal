// DisplaySettingsManager is like a database that stores and manages the settings
/**
 * @file DisplaySettingsManager.h
 * @brief Manages display settings for MIDI message visualization across the application.
 * 
 * The DisplaySettingsManager acts as a centralized repository for all display-related settings
 * in the MidiPortal application. It stores color schemes, font sizes, and other visual properties
 * for different MIDI devices and windows. This class follows the Observer pattern by extending
 * juce::ChangeBroadcaster, allowing other components to be notified when settings change.
 * 
 * Key responsibilities:
 * - Store and retrieve display settings for different devices and windows
 * - Notify registered components when settings change
 * - Provide default settings when device-specific settings aren't available
 */

#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <map>
#include <vector>
#include <functional>

namespace MidiPortal {

// Forward declare MidiLogDisplay to avoid circular dependency
class MidiLogDisplay;

/**
 * @class DisplaySettingsManager
 * @brief Central manager for all display settings in the application.
 * 
 * This class manages color schemes and visual properties for MIDI message displays.
 * It allows different settings to be applied to different devices or windows,
 * and broadcasts changes to registered listeners when settings are modified.
 */
class DisplaySettingsManager : public juce::ChangeBroadcaster
{
public:
    /**
     * @struct DisplaySettings
     * @brief Container for all visual settings related to MIDI message display.
     * 
     * Defines colors for different types of MIDI messages, font size, and background color.
     * Each device or window can have its own instance of these settings.
     */
    struct DisplaySettings {
        float fontSize = 12.0f;                                  ///< Font size for text display
        juce::Colour backgroundColor = juce::Colours::black;     ///< Background color for the display
        juce::Colour noteOnColor = juce::Colours::green;         ///< Color for Note On messages
        juce::Colour noteOffColor = juce::Colours::red;          ///< Color for Note Off messages
        juce::Colour controllerColor = juce::Colours::yellow;    ///< Color for Controller Change messages
        juce::Colour pitchBendColor = juce::Colours::orange;     ///< Color for Pitch Bend messages
        juce::Colour pressureColor = juce::Colours::purple;      ///< Color for Pressure/Aftertouch messages
        juce::Colour programChangeColor = juce::Colours::blue;   ///< Color for Program Change messages
        juce::Colour clockColor = juce::Colours::grey;           ///< Color for MIDI Clock messages
        juce::Colour sysExColor = juce::Colours::white;          ///< Color for System Exclusive messages
        juce::Colour defaultColor = juce::Colours::lightgrey;    ///< Default color for other message types
        
        // X- Added mute flags for each MIDI message type
        bool muteNoteOn = false;           ///< Whether to mute Note On messages
        bool muteNoteOff = false;          ///< Whether to mute Note Off messages
        bool muteController = false;       ///< Whether to mute Controller Change messages
        bool mutePitchBend = false;        ///< Whether to mute Pitch Bend messages
        bool mutePressure = false;         ///< Whether to mute Pressure/Aftertouch messages
        bool muteProgramChange = false;    ///< Whether to mute Program Change messages
        bool muteClock = false;            ///< Whether to mute MIDI Clock messages
        bool muteSysEx = false;            ///< Whether to mute System Exclusive messages
        bool muteDefault = false;          ///< Whether to mute other message types
    };

    /**
     * @brief Constructor that initializes the default settings.
     */
    DisplaySettingsManager();
    
    /**
     * @brief Default destructor.
     */
    ~DisplaySettingsManager() = default;

    /**
     * @brief Get settings for a specific device or window.
     * @param deviceName The name of the device or window to get settings for.
     * @return A reference to the settings for the specified device/window.
     * 
     * If no settings exist for the specified device, returns the default settings.
     * The "Default" option represents global settings that are used when no
     * device-specific settings exist. These settings can be edited directly by
     * selecting "Default" in the device selector.
     */
    const DisplaySettings& getSettings(const juce::String& deviceName = "Default") const;
    
    /**
     * @brief Update settings for a specific device or window.
     * @param settings The new settings to apply.
     * @param deviceName The name of the device or window to update settings for.
     * 
     * Triggers a change notification to all registered listeners.
     */
    void setSettings(const DisplaySettings& settings, const juce::String& deviceName);
    
    /**
     * @brief Add new settings for a device or window.
     * @param deviceName The name of the device or window to add settings for.
     * @param settings The settings to add.
     * 
     * Triggers a change notification to all registered listeners.
     */
    void addSettings(const juce::String& deviceName, const DisplaySettings& settings);

    /**
     * @brief Get a list of all devices/windows that have custom settings.
     * @return A StringArray containing the names of all devices/windows with custom settings.
     */
    juce::StringArray getDevicesWithCustomSettings() const;

    /**
     * @brief Register a display component to receive settings updates.
     * @param display Pointer to the MidiLogDisplay to register.
     */
    void registerDisplay(MidiLogDisplay* display);
    
    /**
     * @brief Unregister a display component from receiving settings updates.
     * @param display Pointer to the MidiLogDisplay to unregister.
     */
    void unregisterDisplay(MidiLogDisplay* display);
    
    /**
     * @brief Add a listener to be notified when settings change.
     * @param listener The listener to add.
     */
    void addChangeListener(juce::ChangeListener* listener)
    {
        juce::ChangeBroadcaster::addChangeListener(listener);
    }
    
    /**
     * @brief Remove a listener from being notified when settings change.
     * @param listener The listener to remove.
     */
    void removeChangeListener(juce::ChangeListener* listener)
    {
        juce::ChangeBroadcaster::removeChangeListener(listener);
    }

private:
    /**
     * @brief Map of device/window names to their specific display settings.
     * 
     * This map stores custom settings for each device or window. If a device
     * doesn't have an entry in this map, the default settings are used.
     */
    std::map<juce::String, DisplaySettings> deviceSettings;
    
    /**
     * @brief Default settings used when no device-specific settings exist.
     * 
     * These settings are returned when getSettings() is called with a device
     * name that doesn't have custom settings.
     */
    DisplaySettings defaultSettings;
    
    /**
     * @brief List of registered display components that should be notified of settings changes.
     * 
     * These displays are directly updated when settings change, in addition to
     * any registered ChangeListeners.
     */
    std::vector<MidiLogDisplay*> registeredDisplays;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DisplaySettingsManager)
};

} // namespace MidiPortal 