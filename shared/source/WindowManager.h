#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "DisplaySettingsManager.h"
#include "LogDisplayWindow.h"
#include <map>
#include <set>

namespace MidiPortal {

class WindowManager
{
public:
    explicit WindowManager(DisplaySettingsManager& settingsManager);
    
    void createWindow(const juce::String& windowName);
    void closeWindow(const juce::String& windowName);
    bool hasWindow(const juce::String& windowName) const;
    juce::StringArray getWindowNames() const;
    
    void routeDeviceToWindow(const juce::String& deviceName, const juce::String& windowName);
    void unrouteDeviceFromWindow(const juce::String& deviceName, const juce::String& windowName);
    bool isDeviceRoutedToWindow(const juce::String& deviceName, const juce::String& windowName) const;
    
    juce::StringArray getWindowsForDevice(const juce::String& deviceName) const;
    juce::StringArray getDevicesForWindow(const juce::String& windowName) const;
    
    void routeMidiMessage(const juce::MidiMessage& message, const juce::String& deviceName);
    void registerWindow(juce::Component* window);
    void unregisterWindow(juce::Component* window);
    DisplaySettingsManager& getSettingsManager();

private:
    DisplaySettingsManager& displaySettingsManager;
    std::map<juce::String, std::unique_ptr<LogDisplayWindow>> windows;
    std::map<juce::String, std::set<juce::String>> deviceToWindows;
    std::map<juce::String, std::set<juce::String>> windowToDevices;
    juce::Array<juce::Component*> registeredWindows;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WindowManager)
};

} // namespace MidiPortal 