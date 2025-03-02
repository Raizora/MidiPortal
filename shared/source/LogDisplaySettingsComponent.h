#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include "MidiLogDisplay.h"

namespace MidiPortal {

// X- Define a ChangeListener class
class BackgroundColorChangeListener : public juce::ChangeListener {
public:
    BackgroundColorChangeListener(MidiLogDisplay::DisplaySettings& settings, juce::ColourSelector& selector)
        : settings(settings), selector(selector) {}

    void changeListenerCallback(juce::ChangeBroadcaster* source) override {
        if (source == &selector) {
            settings.backgroundColor = selector.getCurrentColour();
        }
    }

private:
    MidiLogDisplay::DisplaySettings& settings;
    juce::ColourSelector& selector;
};

// X- Component for customizing MidiLogDisplay settings
class LogDisplaySettingsComponent : public juce::Component
{
public:
    LogDisplaySettingsComponent(MidiLogDisplay& logDisplayToControl);
    ~LogDisplaySettingsComponent() override = default;
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
private:
    // X- Reference to the log display we're controlling
    MidiLogDisplay& logDisplay;
    
    // X- Current settings (working copy)
    MidiLogDisplay::DisplaySettings currentSettings;
    
    // X- UI Components
    juce::Label titleLabel;
    
    // X- Color selectors
    struct ColorSelector {
        juce::Label nameLabel;
        juce::ColourSelector selector;
        juce::TextButton resetButton;
        std::unique_ptr<juce::ChangeListener> changeListener;
    };
    
    // X- Font size slider
    juce::Slider fontSizeSlider;
    juce::Label fontSizeLabel;
    
    // X- Background color selector
    juce::Label backgroundLabel;
    juce::ColourSelector backgroundSelector;
    juce::TextButton resetBackgroundButton;
    
    // X- Apply and Reset buttons
    juce::TextButton applyButton;
    juce::TextButton resetAllButton;
    
    // X- Helper method to create a color selector
    std::unique_ptr<ColorSelector> createColorSelector(const juce::String& name, juce::Colour& targetColor);
    
    // X- Color selectors for each message type
    std::unique_ptr<ColorSelector> noteOnSelector;
    std::unique_ptr<ColorSelector> noteOffSelector;
    std::unique_ptr<ColorSelector> pitchBendSelector;
    std::unique_ptr<ColorSelector> controllerSelector;
    std::unique_ptr<ColorSelector> pressureSelector;
    std::unique_ptr<ColorSelector> programChangeSelector;
    std::unique_ptr<ColorSelector> clockSelector;
    std::unique_ptr<ColorSelector> sysExSelector;
    std::unique_ptr<ColorSelector> defaultSelector;
    
    // X- Change listener for background color
    std::unique_ptr<BackgroundColorChangeListener> backgroundChangeListener;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LogDisplaySettingsComponent)
};

} // namespace MidiPortal 