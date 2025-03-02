#pragma once
#include <juce_gui_extra/juce_gui_extra.h>
#include "LogDisplaySettingsComponent.h"
#include <functional>

namespace MidiPortal {

class LogDisplaySettingsWindow : public juce::DialogWindow
{
public:
    std::function<void()> onCloseCallback;

    LogDisplaySettingsWindow(const juce::String& name, MidiLogDisplay& logDisplay)
        : DialogWindow(name, juce::Colours::lightgrey, true, true)
    {
        auto* content = new LogDisplaySettingsComponent(logDisplay);
        
        setContentOwned(content, true);
        setResizeLimits(400, 300, 800, 800);
        centreWithSize(600, 800);
        
        setResizable(true, true);  // Allow both horizontal and vertical resizing
        setVisible(true);
        setAlwaysOnTop(true);
    }

    void closeButtonPressed() override
    {
        setVisible(false);
        if (onCloseCallback) onCloseCallback();
    }
};

} // namespace MidiPortal 