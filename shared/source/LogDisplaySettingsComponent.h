/**
 * @file LogDisplaySettingsComponent.h
 * @brief Component for configuring MIDI log display settings.
 * 
 * This file defines the LogDisplaySettingsComponent class, which provides a user interface
 * for configuring the visual appearance of MIDI message logs, including colors for different
 * message types, font size, and other display properties.
 */

// LogDisplaySettingsComponent is like a form that lets users view and edit DisplaySettingsManager settings

#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include "MidiLogDisplay.h"
#include "DisplaySettingsManager.h"
#include "SettingsSection.h"

namespace MidiPortal {

/**
 * @class ColorChangeListener
 * @brief Listener for color selector changes that updates display settings.
 * 
 * This class listens for changes to a color selector and updates the appropriate
 * color in the display settings when a change is detected. It supports different
 * types of MIDI messages, each with its own color setting.
 */
class ColorChangeListener : public juce::ChangeListener {
public:
    /**
     * @enum ColorType
     * @brief Enumeration of different MIDI message types for color selection.
     * 
     * Each type corresponds to a different kind of MIDI message that can be
     * displayed with a unique color in the log display.
     */
    enum class ColorType {
        // X- Removed Background color type as it's now handled in WindowRoutingComponent
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

    /**
     * @brief Constructor that initializes the listener with settings and selector references.
     * @param settingsPtr Pointer to the display settings to update.
     * @param selector Pointer to the color selector to listen to.
     * @param type The type of MIDI message whose color is being configured.
     */
    ColorChangeListener(DisplaySettingsManager::DisplaySettings* settingsPtr, 
                       juce::ColourSelector* selector,
                       ColorType type)
        : settings(settingsPtr), colourSelector(selector), colorType(type) {}

    /**
     * @brief Handles change notifications from the color selector.
     * @param source The ChangeBroadcaster that triggered the notification.
     * 
     * Updates the appropriate color in the display settings based on the
     * current color of the selector and the configured color type.
     */
    void changeListenerCallback(juce::ChangeBroadcaster*) override {
        if (settings != nullptr && colourSelector != nullptr) {
            auto newColor = colourSelector->getCurrentColour();
            switch (colorType) {
                // X- Removed Background case as it's now handled in WindowRoutingComponent
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
    /**
     * @brief Pointer to the display settings to update.
     */
    DisplaySettingsManager::DisplaySettings* settings;
    
    /**
     * @brief Pointer to the color selector to listen to.
     */
    juce::ColourSelector* colourSelector;
    
    /**
     * @brief The type of MIDI message whose color is being configured.
     */
    ColorType colorType;
};

/**
 * @class MuteButtonListener
 * @brief Listener for mute button changes that updates display settings.
 * 
 * This class listens for changes to a mute button and updates the appropriate
 * mute flag in the display settings when a change is detected.
 */
class MuteButtonListener : public juce::Button::Listener {
public:
    /**
     * @enum MuteType
     * @brief Enumeration of different MIDI message types for muting.
     * 
     * Each type corresponds to a different kind of MIDI message that can be muted.
     */
    enum class MuteType {
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

    /**
     * @brief Constructor that initializes the listener with settings and button references.
     * @param settingsPtr Pointer to the display settings to update.
     * @param button Pointer to the toggle button to listen to.
     * @param type The type of MIDI message to mute.
     */
    MuteButtonListener(DisplaySettingsManager::DisplaySettings* settingsPtr, 
                       juce::ToggleButton* button,
                       MuteType type)
        : settings(settingsPtr), muteButton(button), muteType(type) {}

    /**
     * @brief Handles button state changes.
     * @param button The button that changed state.
     * 
     * Updates the appropriate mute flag in the display settings based on the
     * current state of the button and the configured mute type.
     */
    void buttonClicked(juce::Button* button) override {
        if (settings != nullptr && button == muteButton) {
            bool muted = button->getToggleState();
            switch (muteType) {
                case MuteType::NoteOn:         settings->muteNoteOn = muted; break;
                case MuteType::NoteOff:        settings->muteNoteOff = muted; break;
                case MuteType::Controller:     settings->muteController = muted; break;
                case MuteType::PitchBend:      settings->mutePitchBend = muted; break;
                case MuteType::Pressure:       settings->mutePressure = muted; break;
                case MuteType::ProgramChange:  settings->muteProgramChange = muted; break;
                case MuteType::Clock:          settings->muteClock = muted; break;
                case MuteType::SysEx:          settings->muteSysEx = muted; break;
                case MuteType::Default:        settings->muteDefault = muted; break;
            }
        }
    }

private:
    /**
     * @brief Pointer to the display settings to update.
     */
    DisplaySettingsManager::DisplaySettings* settings;
    
    /**
     * @brief Pointer to the mute button to listen to.
     */
    juce::ToggleButton* muteButton;
    
    /**
     * @brief The type of MIDI message to mute.
     */
    MuteType muteType;
};

/**
 * @class LogDisplaySettingsComponent
 * @brief Component for configuring MIDI log display settings.
 * 
 * This component provides a user interface for configuring the visual appearance
 * of MIDI message logs, including colors for different message types, font size,
 * and other display properties. It works with the DisplaySettingsManager to
 * apply consistent visual styling based on device-specific settings.
 */
class LogDisplaySettingsComponent : public juce::Component,
                                  private juce::ComboBox::Listener
                                  
{
public:
    /**
     * @brief Constructor that takes a reference to the MidiLogDisplay to configure.
     * @param logDisplayToControl Reference to the MidiLogDisplay to configure.
     * 
     * Initializes the component with a reference to the MidiLogDisplay to be configured
     * and sets up the initial UI state.
     */
    LogDisplaySettingsComponent(MidiLogDisplay& logDisplayToControl);
    
    /**
     * @brief Destructor that cleans up resources.
     * 
     * Ensures proper cleanup of resources when the component is destroyed.
     */
    ~LogDisplaySettingsComponent() override;
    
    /**
     * @brief Paints the component.
     * @param g The Graphics context to paint into.
     * 
     * Draws the background and any custom visual elements of the component.
     */
    void paint(juce::Graphics& g) override;
    
    /**
     * @brief Handles component resizing.
     * 
     * Positions all child components based on the new size of the component.
     */
    void resized() override;
    
    /**
     * @brief Updates the device selector with currently enabled MIDI devices.
     * 
     * Refreshes the device selector dropdown to show only currently enabled MIDI devices.
     * This should be called when MIDI device configuration changes.
     */
    void updateDeviceSelector();
    
    /**
     * @brief Handles changes to the device selector.
     * 
     * Updates the UI to reflect the settings for the newly selected device.
     */
    void deviceSelectorChanged();
    
private:
    /**
     * @brief Reference to the log display being controlled.
     * 
     * Used to access and modify the display settings of the log display.
     */
    MidiLogDisplay& logDisplay;
    
    /**
     * @brief Flag to track if the component is being destroyed.
     * 
     * Used to prevent operations during destruction that could cause crashes.
     */
    bool isBeingDestroyed = false;
    
    /**
     * @brief Settings section for device selection.
     * 
     * Contains controls for selecting which device's settings to edit.
     */
    std::unique_ptr<SettingsSection> deviceSection;
    
    /**
     * @brief Settings section for appearance settings.
     * 
     * Contains controls for configuring general appearance settings like font size.
     */
    std::unique_ptr<SettingsSection> appearanceSection;
    
    /**
     * @brief Settings section for color settings.
     * 
     * Contains controls for configuring colors for different MIDI message types.
     */
    std::unique_ptr<SettingsSection> colorSection;
    
    /**
     * @brief Current display settings being edited.
     * 
     * Stores the current state of the display settings being edited.
     */
    DisplaySettingsManager::DisplaySettings currentSettings;
    
    /**
     * @brief Previous display settings for the current device.
     * 
     * Stores the previous state of the display settings for the reset button.
     */
    DisplaySettingsManager::DisplaySettings previousSettings;
    
    /**
     * @brief Map of previous display settings for each device.
     * 
     * Stores the previous state of display settings for each device for reset functionality.
     */
    std::map<juce::String, DisplaySettingsManager::DisplaySettings> devicePreviousSettings;
    
    /**
     * @brief Default display settings.
     * 
     * Stores the default display settings for reference.
     */
    DisplaySettingsManager::DisplaySettings overrideAllDevices;   // Original defaults
    
    /**
     * @brief Name of the current device being edited.
     * 
     * Stores the name of the device whose settings are currently being edited.
     */
    juce::String currentDevice;
    
    /**
     * @brief Flag to track if settings have been applied at least once.
     * 
     * Used to determine if the reset button should be enabled.
     */
    bool hasAppliedOnce = false;
    
    /**
     * @brief Label for the device selector.
     * 
     * Displays a label for the device selector dropdown.
     */
    juce::Label deviceLabel{"Device Label", "Device:"};
    
    /**
     * @brief Dropdown for selecting which device's settings to edit.
     * 
     * Allows the user to select which device's settings to edit.
     */
    juce::ComboBox deviceSelector{"Device Selector"};
    
    /**
     * @brief Label for the font size slider.
     * 
     * Displays a label for the font size slider.
     */
    juce::Label fontSizeLabel{"Font Size Label", "Font Size:"};
    
    /**
     * @brief Slider for adjusting font size.
     * 
     * Allows the user to adjust the font size used in the log display.
     */
    juce::Slider fontSizeSlider{"Font Size Slider"};

    /**
     * @brief Container for color selectors.
     * 
     * Contains all the color selectors for different MIDI message types.
     */
    std::unique_ptr<juce::Component> colorContainer;
    
    /**
     * @brief Viewport for scrolling through color selectors.
     * 
     * Allows scrolling through the color selectors if they don't fit in the visible area.
     */
    juce::Viewport colorViewport;
    
    /**
     * @struct ColorSection
     * @brief Container for a color selector and its associated components.
     * 
     * Groups together a label, color selector, and change listener for a specific MIDI message type.
     */
    struct ColorSection {
        juce::Label label;
        std::unique_ptr<juce::ColourSelector> selector;
        std::unique_ptr<ColorChangeListener> listener;
        // X- Added mute button and its listener
        juce::ToggleButton muteButton;
        std::unique_ptr<MuteButtonListener> muteListener;
    };

    /**
     * @brief Color section for Note On messages.
     */
    ColorSection noteOnColorSection;
    
    /**
     * @brief Color section for Note Off messages.
     */
    ColorSection noteOffColorSection;
    
    /**
     * @brief Color section for Controller Change messages.
     */
    ColorSection controllerColorSection;
    
    /**
     * @brief Color section for Pitch Bend messages.
     */
    ColorSection pitchBendColorSection;
    
    /**
     * @brief Color section for Pressure/Aftertouch messages.
     */
    ColorSection pressureColorSection;
    
    /**
     * @brief Color section for Program Change messages.
     */
    ColorSection programChangeColorSection;
    
    /**
     * @brief Color section for MIDI Clock messages.
     */
    ColorSection clockColorSection;
    
    /**
     * @brief Color section for System Exclusive messages.
     */
    ColorSection sysExColorSection;
    
    /**
     * @brief Color section for other message types.
     */
    ColorSection defaultColorSection;
    
    /**
     * @brief Button to apply the current settings.
     * 
     * When clicked, applies the current settings to the log display.
     */
    juce::TextButton applyButton;
    
    /**
     * @brief Button to reset to the previous settings.
     * 
     * When clicked, resets the settings to their previous state.
     */
    juce::TextButton resetButton;
    
    /**
     * @brief Sets up a color section with a label and color selector.
     * @param section The ColorSection to set up.
     * @param name The name to display in the label.
     * @param initialColor The initial color for the selector.
     * @param initialMute The initial mute state.
     * 
     * Initializes a ColorSection with a label, color selector, mute button, and listeners.
     */
    void setupColorSection(ColorSection& section, const juce::String& name, const juce::Colour& initialColor, bool initialMute);
    
    /**
     * @brief Handles changes to the font size slider.
     * 
     * Updates the current settings with the new font size.
     */
    void fontSizeChanged();
    
    /**
     * @brief Updates all controls to reflect the current settings.
     * 
     * Sets the state of all UI controls based on the current settings.
     */
    void updateControls();
    
    /**
     * @brief Handles changes to combo boxes.
     * @param comboBoxThatHasChanged The combo box that changed.
     * 
     * Called when a combo box value changes, such as the device selector.
     */
    void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) override;
    
    /**
     * @brief Handles clicks on the Apply button.
     * 
     * Applies the current settings to the log display.
     */
    void handleApplyButton();
    
    /**
     * @brief Handles clicks on the Reset button.
     * 
     * Resets the settings to their previous state.
     */
    void handleResetButton();
    
    /**
     * @brief Caches the current settings for later reset.
     * 
     * Stores the current settings so they can be restored if the user clicks Reset.
     */
    void cacheCurrentSettings();
    
    /**
     * @brief Applies settings to the log display.
     * @param settings The settings to apply.
     * 
     * Updates the log display with the specified settings.
     */
    void applySettings(const DisplaySettingsManager::DisplaySettings& settings);
    
    // X- Added override toggle button for ALL device settings
    juce::ToggleButton overrideToggle{"Override Toggle"};
    
    /**
     * @brief Label for the override description.
     * 
     * Displays a label for the override description.
     */
    juce::Label overrideDescription{"Override Description"};
    
    /**
     * @brief Gets the current settings.
     * 
     * Returns the current settings of the component.
     */
    DisplaySettingsManager::DisplaySettings getCurrentSettings();
    
    // Add this member variable
    std::map<juce::String, DisplaySettingsManager::DisplaySettings> deviceOriginalSettings;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LogDisplaySettingsComponent)
};

} // namespace MidiPortal 