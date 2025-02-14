#pragma once
#include <juce_gui_extra/juce_gui_extra.h>
#include "SettingsComponent.h"

class SettingsWindow : public juce::DialogWindow
{
public:
    SettingsWindow(const juce::String& name)
        : DialogWindow(name, juce::Colours::lightgrey, true, true)
    {
        auto* content = new SettingsComponent();
        setContentOwned(content, true);
        
        // Set minimum size
        setResizeLimits(400, 300, 1200, 1000);  // min width, min height, max width, max height
        
        // Center on screen with content's preferred size
        centreWithSize(content->getWidth(), content->getHeight());
        
        setVisible(true);
        setResizable(true, false);  // Resizable but no resize border
    }

    void closeButtonPressed() override
    {
        setVisible(false);
    }
}; 