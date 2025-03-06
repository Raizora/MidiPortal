// LogDisplaySettingsComponent is like a form that lets users view and edit DisplaySettingsManager settings

#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include "MidiLogDisplay.h"
#include "DisplaySettingsManager.h"
#include "SettingsSection.h"

namespace MidiPortal {

// X- Define a proper ColorChangeListener class for each color type
class ColorChangeListener : public juce::ChangeListener {
public:
    enum class ColorType {
        Background,
        NoteOn,
        NoteOff,
        Controller,
        PitchBend,
        Pressure,
        ProgramChange,
        Clock,
        SysEx,
        Default
    };

    ColorChangeListener(DisplaySettingsManager::DisplaySettings* settingsPtr, 
                       juce::ColourSelector* selector,
                       ColorType type)
        : settings(settingsPtr), colourSelector(selector), colorType(type) {}

    void changeListenerCallback(juce::ChangeBroadcaster*) override {
        if (settings != nullptr && colourSelector != nullptr) {
            auto newColor = colourSelector->getCurrentColour();
            switch (colorType) {
                case ColorType::Background:     settings->backgroundColor = newColor; break;
                case ColorType::NoteOn:         settings->noteOnColor = newColor; break;
                case ColorType::NoteOff:        settings->noteOffColor = newColor; break;
                case ColorType::Controller:      settings->controllerColor = newColor; break;
                case ColorType::PitchBend:      settings->pitchBendColor = newColor; break;
                case ColorType::Pressure:        settings->pressureColor = newColor; break;
                case ColorType::ProgramChange:   settings->programChangeColor = newColor; break;
                case ColorType::Clock:          settings->clockColor = newColor; break;
                case ColorType::SysEx:          settings->sysExColor = newColor; break;
                case ColorType::Default:        settings->defaultColor = newColor; break;
            }
        }
    }

private:
    DisplaySettingsManager::DisplaySettings* settings;
    juce::ColourSelector* colourSelector;
    ColorType colorType;
};

// X- Component for customizing MidiLogDisplay settings
class LogDisplaySettingsComponent : public juce::Component,
                                  private juce::ComboBox::Listener
{
public:
    LogDisplaySettingsComponent(MidiLogDisplay& logDisplayToControl);
    ~LogDisplaySettingsComponent() override;
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
private:
    // Reference to the log display we're controlling
    MidiLogDisplay& logDisplay;
    
    // Flag to track if we're being destroyed
    bool isBeingDestroyed = false;
    
    // Settings sections
    std::unique_ptr<SettingsSection> deviceSection;
    std::unique_ptr<SettingsSection> appearanceSection;
    std::unique_ptr<SettingsSection> colorSection;
    
    // Current and cached settings
    DisplaySettingsManager::DisplaySettings currentSettings;
    DisplaySettingsManager::DisplaySettings previousSettings;  // For reset functionality
    DisplaySettingsManager::DisplaySettings defaultSettings;   // Original defaults
    juce::String currentDevice;
    bool hasAppliedOnce = false;
    
    // UI Components
    juce::Label deviceLabel;
    juce::ComboBox deviceSelector;
    
    juce::Label fontSizeLabel;
    juce::Slider fontSizeSlider;

    // X- Added for scrollable color section area
    std::unique_ptr<juce::Component> colorContainer;
    juce::Viewport colorViewport;
    
    // Color selectors for different MIDI message types
    struct ColorSection {
        juce::Label label;
        std::unique_ptr<juce::ColourSelector> selector;
        std::unique_ptr<ColorChangeListener> listener;
    };

    ColorSection backgroundColorSection;
    ColorSection noteOnColorSection;
    ColorSection noteOffColorSection;
    ColorSection controllerColorSection;
    ColorSection pitchBendColorSection;
    ColorSection pressureColorSection;
    ColorSection programChangeColorSection;
    ColorSection clockColorSection;
    ColorSection sysExColorSection;
    ColorSection defaultColorSection;
    
    juce::TextButton applyButton;
    juce::TextButton resetButton;
    
    // Helper methods
    void setupColorSection(ColorSection& section, const juce::String& name, const juce::Colour& initialColor);
    void deviceSelectorChanged();
    void fontSizeChanged();
    void updateControls();
    void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) override;
    void handleApplyButton();
    void handleResetButton();
    void cacheCurrentSettings();
    void applySettings(const DisplaySettingsManager::DisplaySettings& settings);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LogDisplaySettingsComponent)
};

} // namespace MidiPortal 