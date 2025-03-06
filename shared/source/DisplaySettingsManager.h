// DisplaySettingsManager is like a database that stores and manages the settings

#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <map>
#include <vector>
#include <functional>

namespace MidiPortal {

// Forward declare MidiLogDisplay to avoid circular dependency
class MidiLogDisplay;

class DisplaySettingsManager : public juce::ChangeBroadcaster
{
public:
    // X- Settings struct for display customization
    struct DisplaySettings {
        float fontSize = 12.0f;
        juce::Colour backgroundColor = juce::Colours::black;
        juce::Colour noteOnColor = juce::Colours::green;
        juce::Colour noteOffColor = juce::Colours::red;
        juce::Colour controllerColor = juce::Colours::yellow;
        juce::Colour pitchBendColor = juce::Colours::orange;
        juce::Colour pressureColor = juce::Colours::purple;
        juce::Colour programChangeColor = juce::Colours::blue;
        juce::Colour clockColor = juce::Colours::grey;
        juce::Colour sysExColor = juce::Colours::white;
        juce::Colour defaultColor = juce::Colours::lightgrey;
    };

    DisplaySettingsManager();
    ~DisplaySettingsManager() = default;

    // X- Get/Set settings for a specific device
    const DisplaySettings& getSettings(const juce::String& deviceName = "Default") const;
    void setSettings(const DisplaySettings& settings, const juce::String& deviceName);
    void addSettings(const juce::String& deviceName, const DisplaySettings& settings);

    // X- Get list of devices that have custom settings
    juce::StringArray getDevicesWithCustomSettings() const;

    // X- Register/unregister displays to receive settings updates
    void registerDisplay(MidiLogDisplay* display);
    void unregisterDisplay(MidiLogDisplay* display);

private:
    // X- Store settings per device
    std::map<juce::String, DisplaySettings> deviceSettings;
    
    // X- Default settings used when no device-specific settings exist
    DisplaySettings defaultSettings;
    
    // X- Store registered displays
    std::vector<MidiLogDisplay*> registeredDisplays;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DisplaySettingsManager)
};

} // namespace MidiPortal 