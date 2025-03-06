#pragma once
#include <juce_gui_extra/juce_gui_extra.h>
#include "WindowRoutingComponent.h"
#include <functional>

namespace MidiPortal {

class WindowRoutingWindow : public juce::DialogWindow
{
public:
    std::function<void()> onCloseCallback;

    WindowRoutingWindow(const juce::String& name, WindowManager& windowManager)
        : DialogWindow(name, juce::Colours::lightgrey, true, true)
    {
        auto* content = new WindowRoutingComponent(windowManager);
        
        setContentOwned(content, true);
        setResizeLimits(400, 300, 1200, 1200);
        centreWithSize(600, 400);
        
        setResizable(true, true);
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