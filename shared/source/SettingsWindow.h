#pragma once
#include <juce_gui_extra/juce_gui_extra.h>
#include "SettingsComponent.h"

class SettingsWindow : public juce::DialogWindow
{
public:
    SettingsWindow(const juce::String& name, std::unique_ptr<SettingsComponent>& settings)
        : DialogWindow(name, juce::Colours::lightgrey, true, true)
    {
        setContentOwned(settings.get(), true);
        centreWithSize(400, 300);
        setVisible(true);
        setResizable(true, true);
    }

    void closeButtonPressed() override
    {
        setVisible(false);
    }
}; 