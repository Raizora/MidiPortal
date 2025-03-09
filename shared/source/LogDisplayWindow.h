#pragma once
#include <juce_gui_extra/juce_gui_extra.h>
#include "MidiLogDisplay.h"
#include "DisplaySettingsManager.h"
#include <functional>

namespace MidiPortal {

class LogDisplayWindow : public juce::DocumentWindow,
                        private juce::ChangeListener
{
public:
    std::function<void()> onCloseCallback;

    LogDisplayWindow(const juce::String& name, DisplaySettingsManager& settingsManager)
        : DocumentWindow(name + " - MIDI Log", juce::Colours::darkgrey, true),
          displaySettingsManager(settingsManager)
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
        
        // Listen for settings changes
        settingsManager.addChangeListener(this);
        
        // Apply initial background color
        applyBackgroundColor();
    }
    
    ~LogDisplayWindow() override
    {
        // Stop listening for settings changes
        displaySettingsManager.removeChangeListener(this);
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
    
    // Apply the background color from settings
    void applyBackgroundColor()
    {
        auto settings = displaySettingsManager.getSettings(getName());
        setBackgroundColour(settings.backgroundColor);
    }
    
    // Change listener callback for settings changes
    void changeListenerCallback(juce::ChangeBroadcaster* source) override
    {
        if (source == &displaySettingsManager)
        {
            // Settings have changed, update the background color
            applyBackgroundColor();
        }
    }

private:
    MidiLogDisplay* midiDisplay = nullptr;
    DisplaySettingsManager& displaySettingsManager;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LogDisplayWindow)
};

} // namespace MidiPortal 