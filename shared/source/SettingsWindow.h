#pragma once
#include <juce_gui_extra/juce_gui_extra.h>
#include "SettingsComponent.h"
#include <functional>

namespace MidiPortal {

class SettingsWindow : public juce::DialogWindow
{
public:
    std::function<void()> onCloseCallback;

    SettingsWindow(const juce::String& name, juce::AudioDeviceManager& deviceManager)
        : DialogWindow(name, juce::Colours::lightgrey, true, true)
    {
        auto* content = new SettingsComponent(deviceManager);
        
        // X- Create viewport for scrolling
        auto* viewport = new juce::Viewport();
        viewport->setViewedComponent(content, true);
        viewport->setScrollBarsShown(true, true);
        
        // X- Make viewport fill the window and resize with it
        viewport->setSize(550, 400);
        
        setContentOwned(viewport, true);
        setResizeLimits(500, 300, 1200, 1200);
        centreWithSize(550, 400);
        
        // X- Make sure content gets viewport's width
        content->setSize(viewport->getWidth(), content->getHeight());
        
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