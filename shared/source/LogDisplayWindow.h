#pragma once
#include <juce_gui_extra/juce_gui_extra.h>
#include "MidiLogDisplay.h"
#include "DisplaySettingsManager.h"
#include <functional>

namespace MidiPortal {

class LogDisplayWindow : public juce::DocumentWindow
{
public:
    std::function<void()> onCloseCallback;

    LogDisplayWindow(const juce::String& name, DisplaySettingsManager& settingsManager)
        : DocumentWindow(name + " - MIDI Log", juce::Colours::darkgrey, true)
    {
        // Create a new MidiLogDisplay that uses the settings manager
        auto* display = new MidiLogDisplay(settingsManager);
        
        setContentOwned(display, true);
        setResizeLimits(400, 300, 1200, 1200);
        centreWithSize(600, 400);
        
        setResizable(true, true);
        setVisible(true);
        
        // Store the display pointer for message routing
        midiDisplay = display;
    }

    void closeButtonPressed() override
    {
        setVisible(false);
        if (onCloseCallback) onCloseCallback();
    }
    
    // Method to add messages to this window's display
    void addMessage(const juce::MidiMessage& message, const juce::String& deviceName)
    {
        if (midiDisplay != nullptr)
            midiDisplay->addMessage(message, deviceName);
    }

private:
    MidiLogDisplay* midiDisplay = nullptr;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LogDisplayWindow)
};

} // namespace MidiPortal 