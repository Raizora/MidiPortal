/**
 * @file LogDisplaySettingsComponent.cpp
 * @brief Implementation of the LogDisplaySettingsComponent class.
 * 
 * This file contains the implementation of the LogDisplaySettingsComponent class methods,
 * which provide a user interface for configuring the visual appearance of MIDI message logs
 * in the MidiPortal application, including colors for different message types, font size,
 * and other display properties.
 */

// LogDisplaySettingsComponent is like a form that lets users view and edit DisplaySettingsManager settings

#include "LogDisplaySettingsComponent.h"

namespace MidiPortal {

// X- Custom LookAndFeel class to increase text size in RGB value boxes
class LargeTextLookAndFeel : public juce::LookAndFeel_V4 {
public:
    juce::Font getLabelFont(juce::Label& label) override {
        // Make the font larger for RGB value boxes
        if (auto* parent = label.getParentComponent()) {
            if (dynamic_cast<juce::ColourSelector*>(parent->getParentComponent()) != nullptr) {
                return juce::Font(12.0f); // X- Reduced from 16.0f to fit better in the boxes
            }
        }
        return LookAndFeel_V4::getLabelFont(label);
    }
};

/*
 * IMPORTANT MEMORY MANAGEMENT NOTE:
 * 
 * This component previously had a segmentation fault when being destroyed. The issue was related
 * to the ownership of the colorContainer component when used with the viewport.
 * 
 * The Problem:
 * - colorContainer is managed by a unique_ptr in this class
 * - When setting it as the viewed component with setViewedComponent(component, true),
 *   JUCE would take ownership and try to delete it when the viewport was cleared
 * - This caused a double-delete: once by JUCE and once by the unique_ptr destructor
 * 
 * The Solution:
 * - Use setViewedComponent(component, false) to tell JUCE NOT to delete the component
 * - Let the unique_ptr handle the deletion when this component is destroyed
 * - Clear the viewport before any other cleanup in the destructor
 * 
 * If you encounter segmentation faults with JUCE components, always check for ownership
 * conflicts between smart pointers and JUCE's internal deletion mechanisms.
 */

/**
 * @brief Constructor that initializes the settings component with all UI elements.
 * @param logDisplayToControl Reference to the MidiLogDisplay that this component will control.
 * 
 * Creates and configures all UI elements including device selector, font size controls,
 * color selectors for different MIDI message types, and Apply/Reset buttons. Sets up
 * the component layout and initializes all settings from the provided log display.
 */
LogDisplaySettingsComponent::LogDisplaySettingsComponent(MidiLogDisplay& logDisplayToControl)
    : logDisplay(logDisplayToControl),
      isBeingDestroyed(false),
      deviceSection(std::make_unique<SettingsSection>("Device")),
      appearanceSection(std::make_unique<SettingsSection>("Appearance")),
      colorSection(std::make_unique<SettingsSection>("Colors")),
      currentSettings(logDisplay.getSettingsManager().getSettings()),
      previousSettings(currentSettings),
      overrideAllDevices(currentSettings),
      currentDevice("ALL"),
      hasAppliedOnce(false),
      colorContainer(std::make_unique<juce::Component>())
{
    // Create sections
    deviceSection = std::make_unique<SettingsSection>("Device Settings");
    appearanceSection = std::make_unique<SettingsSection>("Appearance Settings");
    
    addAndMakeVisible(deviceSection.get());
    addAndMakeVisible(appearanceSection.get());

    // Set up device selector
    deviceLabel.setText("Device:", juce::dontSendNotification);
    deviceSelector.addItem("ALL", 1);  // X- "ALL" represents global settings used when override mode is enabled
    
    // X- Only add enabled MIDI devices to the selector
    auto devices = juce::MidiInput::getAvailableDevices();
    int itemId = 2;
    for (const auto& device : devices) {
        // Add all available devices - we don't have direct access to check if they're enabled
        // The user will see all devices but can only configure those that are enabled
        deviceSelector.addItem(device.name, itemId++);
    }
    
    deviceSelector.setSelectedId(1, juce::dontSendNotification);
    deviceSelector.onChange = [this] { deviceSelectorChanged(); };
    
    // Add components to the device section
    deviceSection->addAndMakeVisible(deviceLabel);
    deviceSection->addAndMakeVisible(deviceSelector);
    
    // X- Set up override toggle button and description
    overrideToggle.setButtonText("Override all device settings");
    overrideToggle.setToggleState(currentSettings.overrideAllDevices, juce::dontSendNotification);
    overrideToggle.onClick = [this] {
        // X- Update the override state in currentSettings
        currentSettings.overrideAllDevices = overrideToggle.getToggleState();
        // X- Remove immediate application - will be applied when user clicks Apply
    };
    deviceSection->addAndMakeVisible(overrideToggle);
    
    // X- Add description label for override toggle
    overrideDescription.setText("When enabled, these settings will override individual device settings",
                              juce::dontSendNotification);
    overrideDescription.setFont(juce::Font(12.0f));
    overrideDescription.setColour(juce::Label::textColourId, juce::Colours::grey);
    deviceSection->addAndMakeVisible(overrideDescription);
    
    // Set up font size controls
    fontSizeLabel.setText("Font Size:", juce::dontSendNotification);
    juce::FontOptions options;
    options = options.withHeight(16.0f);
    fontSizeLabel.setFont(juce::Font(options));
    fontSizeLabel.setJustificationType(juce::Justification::right);
    
    fontSizeSlider.setRange(8.0, 24.0, 1.0);
    fontSizeSlider.setValue(currentSettings.fontSize, juce::dontSendNotification);
    fontSizeSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    fontSizeSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 20);
    fontSizeSlider.setColour(juce::Slider::thumbColourId, juce::Colours::lightblue);
    fontSizeSlider.setColour(juce::Slider::trackColourId, juce::Colours::darkgrey);
    
    // Set up font size slider callback
    fontSizeSlider.onValueChange = [this] {
        fontSizeChanged();
    };
    
    // X- Set up fade rate controls
    fadeRateLabel.setText("Fade Rate:", juce::dontSendNotification);
    fadeRateLabel.setFont(juce::Font(options));
    fadeRateLabel.setJustificationType(juce::Justification::right);
    
    // X- Define range from 0.01 (almost instant) to 1.0 (30 seconds)
    // Use a strong logarithmic scale to match our exponential fade formula
    fadeRateSlider.setRange(0.01, 1.0, 0.001);
    fadeRateSlider.setSkewFactor(0.65); // Stronger logarithmic scale for better control in lower ranges
    fadeRateSlider.setValue(currentSettings.fadeRate, juce::dontSendNotification);
    fadeRateSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    fadeRateSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 20);
    fadeRateSlider.setColour(juce::Slider::thumbColourId, juce::Colours::lightblue);
    fadeRateSlider.setColour(juce::Slider::trackColourId, juce::Colours::darkgrey);
    
    // X- Set up fade rate slider callback
    fadeRateSlider.onValueChange = [this] {
        fadeRateChanged();
    };
    
    // X- Set up fade rate toggle
    fadeRateToggle.setToggleState(currentSettings.fadeRateEnabled, juce::dontSendNotification);
    fadeRateToggle.onClick = [this] {
        // Only update the current settings, don't apply to the display yet
        currentSettings.fadeRateEnabled = fadeRateToggle.getToggleState();
        // Removed immediate application to settings manager
    };
    
    // Add components to the appearance section
    appearanceSection->addAndMakeVisible(fontSizeLabel);
    appearanceSection->addAndMakeVisible(fontSizeSlider);
    appearanceSection->addAndMakeVisible(fadeRateLabel);
    appearanceSection->addAndMakeVisible(fadeRateSlider);
    appearanceSection->addAndMakeVisible(fadeRateToggle);
    
    // Set up all color sections with their initial mute states
    setupColorSection(noteOnColorSection, "Note On Color", currentSettings.noteOnColor, currentSettings.muteNoteOn);
    setupColorSection(noteOffColorSection, "Note Off Color", currentSettings.noteOffColor, currentSettings.muteNoteOff);
    setupColorSection(controllerColorSection, "Controller Color", currentSettings.controllerColor, currentSettings.muteController);
    setupColorSection(pitchBendColorSection, "Pitch Bend Color", currentSettings.pitchBendColor, currentSettings.mutePitchBend);
    setupColorSection(pressureColorSection, "Pressure Color", currentSettings.pressureColor, currentSettings.mutePressure);
    setupColorSection(programChangeColorSection, "Program Change Color", currentSettings.programChangeColor, currentSettings.muteProgramChange);
    setupColorSection(clockColorSection, "Clock Color", currentSettings.clockColor, currentSettings.muteClock);
    setupColorSection(sysExColorSection, "SysEx Color", currentSettings.sysExColor, currentSettings.muteSysEx);
    setupColorSection(defaultColorSection, "Default Color", currentSettings.defaultColor, currentSettings.muteDefault);
    
    // Add color sections to the container
    colorContainer->addAndMakeVisible(noteOnColorSection.label);
    colorContainer->addAndMakeVisible(*noteOnColorSection.selector);
    colorContainer->addAndMakeVisible(noteOnColorSection.muteButton);
    
    colorContainer->addAndMakeVisible(noteOffColorSection.label);
    colorContainer->addAndMakeVisible(*noteOffColorSection.selector);
    colorContainer->addAndMakeVisible(noteOffColorSection.muteButton);
    
    colorContainer->addAndMakeVisible(controllerColorSection.label);
    colorContainer->addAndMakeVisible(*controllerColorSection.selector);
    colorContainer->addAndMakeVisible(controllerColorSection.muteButton);
    
    colorContainer->addAndMakeVisible(pitchBendColorSection.label);
    colorContainer->addAndMakeVisible(*pitchBendColorSection.selector);
    colorContainer->addAndMakeVisible(pitchBendColorSection.muteButton);
    
    colorContainer->addAndMakeVisible(pressureColorSection.label);
    colorContainer->addAndMakeVisible(*pressureColorSection.selector);
    colorContainer->addAndMakeVisible(pressureColorSection.muteButton);
    
    colorContainer->addAndMakeVisible(programChangeColorSection.label);
    colorContainer->addAndMakeVisible(*programChangeColorSection.selector);
    colorContainer->addAndMakeVisible(programChangeColorSection.muteButton);
    
    colorContainer->addAndMakeVisible(clockColorSection.label);
    colorContainer->addAndMakeVisible(*clockColorSection.selector);
    colorContainer->addAndMakeVisible(clockColorSection.muteButton);
    
    colorContainer->addAndMakeVisible(sysExColorSection.label);
    colorContainer->addAndMakeVisible(*sysExColorSection.selector);
    colorContainer->addAndMakeVisible(sysExColorSection.muteButton);
    
    colorContainer->addAndMakeVisible(defaultColorSection.label);
    colorContainer->addAndMakeVisible(*defaultColorSection.selector);
    colorContainer->addAndMakeVisible(defaultColorSection.muteButton);

    // Set up the viewport with the color container
    colorViewport.setViewedComponent(colorContainer.get(), false); // X- JUCE won't delete colorContainer
    colorViewport.setScrollBarsShown(true, false);  // Disable horizontal scrollbar, keep vertical
    addAndMakeVisible(colorViewport);
    
    // Set up Apply, Reset, and Clear buttons
    applyButton.setButtonText("Apply Settings");
    applyButton.onClick = [this] { handleApplyButton(); };
    addAndMakeVisible(applyButton);
    
    resetButton.setButtonText("Reset");
    resetButton.onClick = [this] { handleResetButton(); };
    addAndMakeVisible(resetButton);
    
    // Add the Clear button
    clearButton.setButtonText("Clear Messages");
    clearButton.onClick = [this] { 
        if (logDisplay.getSettingsManager().getSettings("ALL").overrideAllDevices) {
            // Show confirmation dialog
            juce::AlertWindow::showOkCancelBox(
                juce::AlertWindow::QuestionIcon,
                "Clear Messages",
                "This will clear all messages from all displays. Continue?",
                "Clear",
                "Cancel",
                nullptr,
                juce::ModalCallbackFunction::create([this](int result) {
                    if (result == 1) { // OK was clicked
                        logDisplay.clear();
                    }
                })
            );
        } else {
            logDisplay.clear();
        }
    };
    addAndMakeVisible(clearButton);
    
    // Initial update
    updateControls();
}

/**
 * @brief Destructor that carefully cleans up resources to prevent memory leaks and crashes.
 * 
 * Implements a specific destruction sequence to prevent double-deletion issues with the
 * colorContainer component. Sets a flag to prevent callbacks during destruction, clears
 * the viewport, removes all listeners, and then resets all smart pointers in a specific order.
 */
LogDisplaySettingsComponent::~LogDisplaySettingsComponent()
{
    // Set the flag to indicate we're being destroyed
    isBeingDestroyed = true;
    
    // CRITICAL: Clear the viewport's viewed component FIRST to prevent double-delete issues
    // This must be done before any other cleanup to avoid accessing components after deletion
    // The 'false' parameter ensures JUCE doesn't try to delete the component itself
    colorViewport.setViewedComponent(nullptr, false);
    
    // X- Remove custom LookAndFeel from all selectors before they are destroyed
    for (auto* selector : {
        noteOnColorSection.selector.get(),
        noteOffColorSection.selector.get(),
        controllerColorSection.selector.get(),
        pitchBendColorSection.selector.get(),
        pressureColorSection.selector.get(),
        programChangeColorSection.selector.get(),
        clockColorSection.selector.get(),
        sysExColorSection.selector.get(),
        defaultColorSection.selector.get()
    }) {
        if (selector != nullptr) {
            selector->setLookAndFeel(nullptr);
            selector->removeAllChangeListeners();
        }
    }
    
    // Remove all button listeners
    noteOnColorSection.muteButton.removeListener(noteOnColorSection.muteListener.get());
    noteOffColorSection.muteButton.removeListener(noteOffColorSection.muteListener.get());
    controllerColorSection.muteButton.removeListener(controllerColorSection.muteListener.get());
    pitchBendColorSection.muteButton.removeListener(pitchBendColorSection.muteListener.get());
    pressureColorSection.muteButton.removeListener(pressureColorSection.muteListener.get());
    programChangeColorSection.muteButton.removeListener(programChangeColorSection.muteListener.get());
    clockColorSection.muteButton.removeListener(clockColorSection.muteListener.get());
    sysExColorSection.muteButton.removeListener(sysExColorSection.muteListener.get());
    defaultColorSection.muteButton.removeListener(defaultColorSection.muteListener.get());
    
    // Reset all listeners first
    noteOnColorSection.listener.reset();
    noteOffColorSection.listener.reset();
    controllerColorSection.listener.reset();
    pitchBendColorSection.listener.reset();
    pressureColorSection.listener.reset();
    programChangeColorSection.listener.reset();
    clockColorSection.listener.reset();
    sysExColorSection.listener.reset();
    defaultColorSection.listener.reset();
    
    // Reset all mute listeners
    noteOnColorSection.muteListener.reset();
    noteOffColorSection.muteListener.reset();
    controllerColorSection.muteListener.reset();
    pitchBendColorSection.muteListener.reset();
    pressureColorSection.muteListener.reset();
    programChangeColorSection.muteListener.reset();
    clockColorSection.muteListener.reset();
    sysExColorSection.muteListener.reset();
    defaultColorSection.muteListener.reset();
    
    // Reset all selectors next
    noteOnColorSection.selector.reset();
    noteOffColorSection.selector.reset();
    controllerColorSection.selector.reset();
    pitchBendColorSection.selector.reset();
    pressureColorSection.selector.reset();
    programChangeColorSection.selector.reset();
    clockColorSection.selector.reset();
    sysExColorSection.selector.reset();
    defaultColorSection.selector.reset();

    // Finally, reset the container
    colorContainer.reset();
}

/**
 * @brief Paints the component background and adds a border around the color viewport.
 * @param g The Graphics context to paint into.
 * 
 * Fills the component background with the default window background color and
 * draws a border around the color viewport for visual separation.
 */
void LogDisplaySettingsComponent::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    // X- Draw an outline around the color viewport
    g.setColour(juce::Colours::darkgrey); // or whatever color you'd like
    g.drawRect(colorViewport.getBounds().reduced(-1), 2); // 2px thick border
}

/**
 * @brief Handles component resizing and positions all child components.
 * 
 * Positions all sections, controls, and the viewport based on the new size of the component.
 * Uses a sophisticated layout system to ensure all elements are properly positioned and sized.
 */
void LogDisplaySettingsComponent::resized()
{
    // Start with the entire component bounds, reduced by 10px on all sides
    auto bounds = getLocalBounds().reduced(10);

    // Spacing between major sections
    const int sectionSpacing = 20;
    const int buttonHeight = 40;
    
    // Device section
    auto deviceBounds = bounds.removeFromTop(80); // X- Increased from 60 to 80 for more padding
    deviceSection->setBounds(deviceBounds);
    auto innerDeviceBounds = deviceBounds.reduced(10);
    
    // X- Create a smaller area for the actual controls, positioned at the top
    auto deviceControlArea = innerDeviceBounds.removeFromTop(40);
    
    // X- Position components in the control area
    deviceLabel.setBounds(deviceControlArea.removeFromLeft(100));
    deviceSelector.setBounds(deviceControlArea.removeFromLeft(200).reduced(5, 0));
    
    // X- Position override toggle button to the right of the device selector
    overrideToggle.setBounds(deviceControlArea.removeFromLeft(200).reduced(5, 0));
    // Only show the override toggle when "ALL" is selected
    overrideToggle.setVisible(deviceSelector.getItemText(deviceSelector.getSelectedItemIndex()) == "ALL");
    
    // X- Add description under the controls with better positioning
    overrideDescription.setBounds(innerDeviceBounds);
    
    bounds.removeFromTop(sectionSpacing);
    
    // Appearance section
    auto appearanceBounds = bounds.removeFromTop(120); // Reduced from 140px to eliminate excess space
    appearanceSection->setBounds(appearanceBounds);
    
    // X- Create more precisely spaced control areas
    auto innerAppearanceBounds = appearanceSection->getLocalBounds().reduced(10);
    
    // Add spacing between section title and first control row
    innerAppearanceBounds.removeFromTop(15); // Add 15px padding after the section title
    
    // Create first row for font size
    auto fontSizeRow = innerAppearanceBounds.removeFromTop(40);
    fontSizeLabel.setBounds(fontSizeRow.removeFromLeft(100).withHeight(30).withY(fontSizeRow.getY() + 5));
    fontSizeSlider.setBounds(fontSizeRow.withHeight(30).withY(fontSizeRow.getY() + 5));
    
    // Add spacing between rows
    innerAppearanceBounds.removeFromTop(10);
    
    // Create second row for fade rate
    auto fadeRateRow = innerAppearanceBounds.removeFromTop(40);
    fadeRateLabel.setBounds(fadeRateRow.removeFromLeft(100).withHeight(30).withY(fadeRateRow.getY() + 5));
    
    // Split remaining area between slider and toggle
    auto fadeRateControlArea = fadeRateRow.withHeight(30).withY(fadeRateRow.getY() + 5);
    int toggleWidth = 120;
    fadeRateSlider.setBounds(fadeRateControlArea.withTrimmedRight(toggleWidth + 5));
    fadeRateToggle.setBounds(fadeRateControlArea.removeFromRight(toggleWidth));
    
    bounds.removeFromTop(sectionSpacing);
    
    // BUTTON ROW - At bottom, Height: 40px
    auto buttonRow = bounds.removeFromBottom(buttonHeight);
    
    // Split button row into 3 equal parts
    int buttonWidth = (buttonRow.getWidth() - 40) / 3; // 40px for gaps (20px each)
    
    applyButton.setBounds(buttonRow.removeFromLeft(buttonWidth));
    buttonRow.removeFromLeft(20); // 20px gap
    
    resetButton.setBounds(buttonRow.removeFromLeft(buttonWidth));
    buttonRow.removeFromLeft(20); // 20px gap
    
    clearButton.setBounds(buttonRow.removeFromLeft(buttonWidth));
    
    bounds.removeFromBottom(sectionSpacing); // 20px gap above buttons
    
    // COLOR VIEWPORT - Takes remaining vertical space
    colorViewport.setBounds(bounds);
    
    // COLOR SECTIONS CONTAINER
    const int colorSectionHeight = 240;
    // X- Adjust the total height to match the actual number of color sections (9 instead of 10)
    const int totalColorSectionsHeight = 9 * colorSectionHeight;
    // X- Expand colorContainer to fill the full width of the viewport to eliminate the dead zone
    colorContainer->setBounds(0, 0, colorViewport.getWidth(), totalColorSectionsHeight);
    
    // COLOR SECTIONS LAYOUT
    auto containerBounds = colorContainer->getLocalBounds().reduced(15);
    
    // Each color section layout:
    auto positionColorSection = [](ColorSection& section, juce::Rectangle<int>& area) {
        // X- Increased total section height to give more room for the RGB value boxes
        auto sectionArea = area.removeFromTop(220);  // Increased from 180
        
        // Label row: 24px height at top with mute button on the right
        auto labelRow = sectionArea.removeFromTop(24);
        section.label.setBounds(labelRow.removeFromLeft(labelRow.getWidth() - 80)); // Leave space for mute button
        section.muteButton.setBounds(labelRow.reduced(0, 2)); // Reduce height slightly for better appearance
        
        // X- Increased gap between label and color selector for better spacing
        sectionArea.removeFromTop(16);  // Increased from 8
        
        // X- Make the color selector taller to accommodate the larger text boxes
        section.selector->setBounds(sectionArea.withHeight(180));  // Increased from 148
        
        // 20px gap between sections
        area.removeFromTop(20);
    };
    
    // Position all 9 color sections sequentially
    // X- Update section height calculation to match new size
    positionColorSection(noteOnColorSection, containerBounds);
    positionColorSection(noteOffColorSection, containerBounds);
    positionColorSection(controllerColorSection, containerBounds);
    positionColorSection(pitchBendColorSection, containerBounds);
    positionColorSection(pressureColorSection, containerBounds);
    positionColorSection(programChangeColorSection, containerBounds);
    positionColorSection(clockColorSection, containerBounds);
    positionColorSection(sysExColorSection, containerBounds);
    positionColorSection(defaultColorSection, containerBounds);
}

/**
 * @brief Handles changes to the device selector dropdown.
 * 
 * Updates the current settings to reflect the selected device and refreshes
 * all UI controls to show the device-specific settings.
 */
void LogDisplaySettingsComponent::deviceSelectorChanged()
{
    // Get the selected device name
    juce::String selectedDevice = deviceSelector.getItemText(deviceSelector.getSelectedItemIndex());
    
    // Cache current settings before switching
    if (hasAppliedOnce) {
        cacheCurrentSettings();
    }
    
    // Update current device
    currentDevice = selectedDevice;
    
    // Get settings for the selected device
    currentSettings = logDisplay.getSettingsManager().getSettings(selectedDevice);
    
    // X- Show/hide override toggle only for "ALL" device and update its state
    overrideToggle.setVisible(selectedDevice == "ALL");
    if (selectedDevice == "ALL") {
        overrideToggle.setToggleState(currentSettings.overrideAllDevices, juce::dontSendNotification);
    }
    
    // Update all controls to reflect the new device's settings
    updateControls();
}

/**
 * @brief Handles changes to the font size slider.
 * 
 * Updates the current settings with the new font size and applies the change
 * to the settings manager for the selected device.
 */
void LogDisplaySettingsComponent::fontSizeChanged()
{
    // Don't access logDisplay if we're being destroyed
    if (!isBeingDestroyed) {
        // Only update the current settings, don't apply to the display yet
        currentSettings.fontSize = static_cast<float>(fontSizeSlider.getValue());
        // Removed immediate application to settings manager
    }
}

/**
 * @brief Handles changes to the fade rate slider.
 * 
 * Updates the current settings with the new fade rate and applies the change
 * to the settings manager for the selected device.
 */
void LogDisplaySettingsComponent::fadeRateChanged()
{
    // Don't access logDisplay if we're being destroyed
    if (!isBeingDestroyed) {
        // Only update the current settings, don't apply to the display yet
        currentSettings.fadeRate = static_cast<float>(fadeRateSlider.getValue());
        // Removed immediate application to settings manager
    }
}

/**
 * @brief Updates all UI controls to reflect the current settings.
 * 
 * Sets the values of all sliders, color selectors, and other controls
 * based on the current settings for the selected device.
 */
void LogDisplaySettingsComponent::updateControls()
{
    fontSizeSlider.setValue(currentSettings.fontSize, juce::dontSendNotification);
    
    // X- Update fade rate controls
    fadeRateSlider.setValue(currentSettings.fadeRate, juce::dontSendNotification);
    fadeRateToggle.setToggleState(currentSettings.fadeRateEnabled, juce::dontSendNotification);
    
    // X- Update override toggle state
    overrideToggle.setToggleState(currentSettings.overrideAllDevices, juce::dontSendNotification);
    overrideToggle.setVisible(deviceSelector.getItemText(deviceSelector.getSelectedItemIndex()) == "ALL");
    
    noteOnColorSection.selector->setCurrentColour(currentSettings.noteOnColor, juce::dontSendNotification);
    noteOnColorSection.muteButton.setToggleState(currentSettings.muteNoteOn, juce::dontSendNotification);
    
    noteOffColorSection.selector->setCurrentColour(currentSettings.noteOffColor, juce::dontSendNotification);
    noteOffColorSection.muteButton.setToggleState(currentSettings.muteNoteOff, juce::dontSendNotification);
    
    controllerColorSection.selector->setCurrentColour(currentSettings.controllerColor, juce::dontSendNotification);
    controllerColorSection.muteButton.setToggleState(currentSettings.muteController, juce::dontSendNotification);
    
    pitchBendColorSection.selector->setCurrentColour(currentSettings.pitchBendColor, juce::dontSendNotification);
    pitchBendColorSection.muteButton.setToggleState(currentSettings.mutePitchBend, juce::dontSendNotification);
    
    pressureColorSection.selector->setCurrentColour(currentSettings.pressureColor, juce::dontSendNotification);
    pressureColorSection.muteButton.setToggleState(currentSettings.mutePressure, juce::dontSendNotification);
    
    programChangeColorSection.selector->setCurrentColour(currentSettings.programChangeColor, juce::dontSendNotification);
    programChangeColorSection.muteButton.setToggleState(currentSettings.muteProgramChange, juce::dontSendNotification);
    
    clockColorSection.selector->setCurrentColour(currentSettings.clockColor, juce::dontSendNotification);
    clockColorSection.muteButton.setToggleState(currentSettings.muteClock, juce::dontSendNotification);
    
    sysExColorSection.selector->setCurrentColour(currentSettings.sysExColor, juce::dontSendNotification);
    sysExColorSection.muteButton.setToggleState(currentSettings.muteSysEx, juce::dontSendNotification);
    
    defaultColorSection.selector->setCurrentColour(currentSettings.defaultColor, juce::dontSendNotification);
    defaultColorSection.muteButton.setToggleState(currentSettings.muteDefault, juce::dontSendNotification);
}

/**
 * @brief Handles changes to combo boxes.
 * @param comboBoxThatHasChanged The combo box that changed.
 * 
 * Called when a combo box value changes, such as the device selector.
 * Delegates to the appropriate handler method based on which combo box changed.
 */
void LogDisplaySettingsComponent::comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged)
{
    if (comboBoxThatHasChanged == &deviceSelector)
    {
        deviceSelectorChanged();
    }
}

/**
 * @brief Sets up a color section with a label, color selector, and change listener.
 * @param section The ColorSection to set up.
 * @param name The name to display in the label.
 * @param initialColor The initial color for the selector.
 * @param initialMute The initial mute state for the mute button.
 * 
 * Creates and configures a ColorSection with a label, color selector, and change listener
 * for a specific MIDI message type. The change listener updates the appropriate color
 * in the current settings when the user selects a new color.
 */
void LogDisplaySettingsComponent::setupColorSection(ColorSection& section, const juce::String& name, const juce::Colour& initialColor, bool initialMute)
{
    section.label.setText(name, juce::dontSendNotification);
    addAndMakeVisible(section.label);
    
    section.selector = std::make_unique<juce::ColourSelector>();
    section.selector->setCurrentColour(initialColor);
    // X- Apply custom LookAndFeel to increase text size in RGB value boxes
    static LargeTextLookAndFeel largeTextLookAndFeel;
    section.selector->setLookAndFeel(&largeTextLookAndFeel);
    
    // Setup mute button
    section.muteButton.setButtonText("Mute");
    section.muteButton.setToggleState(initialMute, juce::dontSendNotification);
    addAndMakeVisible(section.muteButton);

    // Determine the color type based on the section name
    ColorChangeListener::ColorType colorType;
    MuteButtonListener::MuteType muteType;
    
    if (name == "Note On Color") {
        colorType = ColorChangeListener::ColorType::NoteOn;
        muteType = MuteButtonListener::MuteType::NoteOn;
    }
    else if (name == "Note Off Color") {
        colorType = ColorChangeListener::ColorType::NoteOff;
        muteType = MuteButtonListener::MuteType::NoteOff;
    }
    else if (name == "Controller Color") {
        colorType = ColorChangeListener::ColorType::Controller;
        muteType = MuteButtonListener::MuteType::Controller;
    }
    else if (name == "Pitch Bend Color") {
        colorType = ColorChangeListener::ColorType::PitchBend;
        muteType = MuteButtonListener::MuteType::PitchBend;
    }
    else if (name == "Pressure Color") {
        colorType = ColorChangeListener::ColorType::Pressure;
        muteType = MuteButtonListener::MuteType::Pressure;
    }
    else if (name == "Program Change Color") {
        colorType = ColorChangeListener::ColorType::ProgramChange;
        muteType = MuteButtonListener::MuteType::ProgramChange;
    }
    else if (name == "Clock Color") {
        colorType = ColorChangeListener::ColorType::Clock;
        muteType = MuteButtonListener::MuteType::Clock;
    }
    else if (name == "SysEx Color") {
        colorType = ColorChangeListener::ColorType::SysEx;
        muteType = MuteButtonListener::MuteType::SysEx;
    }
    else {
        colorType = ColorChangeListener::ColorType::Default;
        muteType = MuteButtonListener::MuteType::Default;
    }

    section.listener = std::make_unique<ColorChangeListener>(&currentSettings, section.selector.get(), colorType);
    section.selector->addChangeListener(section.listener.get());
    
    // Setup mute button listener
    section.muteListener = std::make_unique<MuteButtonListener>(&currentSettings, &section.muteButton, muteType);
    section.muteButton.addListener(section.muteListener.get());
    
    addAndMakeVisible(section.selector.get());
}

/**
 * @brief Handles clicks on the Apply button.
 * 
 * Applies the current settings to the log display by updating the settings manager
 * with the values from all UI controls. Caches the settings for potential reset.
 */
void LogDisplaySettingsComponent::handleApplyButton()
{
    // Get current override state from previous settings
    bool wasOverrideEnabled = previousSettings.overrideAllDevices;
    
    // Get new settings from UI
    DisplaySettingsManager::DisplaySettings newSettings = getCurrentSettings();
    
    // If override setting is changing
    if (currentDevice == "ALL" && newSettings.overrideAllDevices != wasOverrideEnabled) {
        // If enabling override, store device settings
        if (newSettings.overrideAllDevices) {
            logDisplay.getSettingsManager().storeDeviceSettingsBeforeOverride();
        }
        // If disabling override, restore device settings after applying new "ALL" settings
        else {
            // Apply the ALL settings first without override
            newSettings.overrideAllDevices = false;
            logDisplay.getSettingsManager().setSettings(newSettings, currentDevice);
            
            // Then restore device-specific settings
            logDisplay.getSettingsManager().restoreDeviceSettingsAfterOverride();
            return; // Return here as we've already done the updates
        }
    }
    
    // Normal apply behavior
    logDisplay.getSettingsManager().setSettings(newSettings, currentDevice);
    
    // Update previousSettings for potential resets
    previousSettings = newSettings;
    // Also update device-specific previous settings map
    devicePreviousSettings[currentDevice] = newSettings;
    hasAppliedOnce = true;
}

/**
 * @brief Handles clicks on the Reset button.
 * 
 * Resets the settings to either the previous settings (if Apply has been clicked)
 * or the default settings (if Apply has not been clicked). Updates the UI controls
 * and applies the reset settings to the log display.
 */
void LogDisplaySettingsComponent::handleResetButton()
{
    // Don't access logDisplay if we're being destroyed
    if (!isBeingDestroyed) {
        // X- Get the current background color to preserve it
        juce::Colour currentBgColor = currentSettings.backgroundColor;
        
        // Get the current device name
        juce::String deviceName = deviceSelector.getItemText(deviceSelector.getSelectedItemIndex());
        
        // Check if we have previous settings for this specific device in the map
        auto it = devicePreviousSettings.find(deviceName);
        
        if (hasAppliedOnce) {
            // If device-specific settings exist in the map, use those
            if (it != devicePreviousSettings.end()) {
                currentSettings = it->second;
            } else {
                // Otherwise fall back to the general previousSettings
                currentSettings = previousSettings;
            }
        } else {
            // Otherwise reset to default settings
            if (deviceName == "Default") {
                // For "Default", use the default settings
                currentSettings = overrideAllDevices;
            } else {
                // For specific devices, get fresh settings from the manager
                currentSettings = logDisplay.getSettingsManager().getSettings(deviceName);
            }
        }
        
        // X- Preserve the background color
        currentSettings.backgroundColor = currentBgColor;
        
        // Update UI to reflect the reset settings
        updateControls();
        
        // Apply the reset settings
        logDisplay.getSettingsManager().setSettings(currentSettings, deviceName);
    }
}

/**
 * @brief Applies settings to the log display.
 * @param settings The settings to apply.
 * 
 * Updates the current settings with the specified settings, updates the UI controls,
 * and applies the settings to the log display for the selected device.
 */
void LogDisplaySettingsComponent::applySettings(const DisplaySettingsManager::DisplaySettings& settings)
{
    currentSettings = settings;
    updateControls();
    logDisplay.getSettingsManager().setSettings(currentSettings, deviceSelector.getItemText(deviceSelector.getSelectedItemIndex()));
}

/**
 * @brief Updates the device selector with currently enabled MIDI devices.
 * 
 * Refreshes the device selector dropdown to show only currently enabled MIDI devices.
 * This should be called when MIDI device configuration changes.
 */
void LogDisplaySettingsComponent::updateDeviceSelector()
{
    // Remember the currently selected device
    juce::String currentlySelectedDevice = deviceSelector.getItemText(deviceSelector.getSelectedItemIndex());
    
    // Clear the selector
    deviceSelector.clear(juce::dontSendNotification);
    
    // Add the Default option
    deviceSelector.addItem("Default", 1);
    
    // Add all available MIDI devices
    auto devices = juce::MidiInput::getAvailableDevices();
    int itemId = 2;
    for (const auto& device : devices) {
        // Add all available devices - we don't have direct access to check if they're enabled
        deviceSelector.addItem(device.name, itemId++);
    }
    
    // Try to reselect the previously selected device
    int indexToSelect = deviceSelector.getNumItems() > 0 ? 0 : -1;
    for (int i = 0; i < deviceSelector.getNumItems(); ++i) {
        if (deviceSelector.getItemText(i) == currentlySelectedDevice) {
            indexToSelect = i;
            break;
        }
    }
    
    // Select the appropriate item
    if (indexToSelect >= 0) {
        deviceSelector.setSelectedItemIndex(indexToSelect, juce::dontSendNotification);
    } else {
        // If the previously selected device is no longer available, select Default
        deviceSelector.setSelectedItemIndex(0, juce::dontSendNotification);
    }
    
    // Update the current settings based on the selected device
    deviceSelectorChanged();
}

/**
 * @brief Caches the current settings for later reset.
 * 
 * Stores the current settings so they can be restored if the user clicks Reset.
 */
void LogDisplaySettingsComponent::cacheCurrentSettings()
{
    // X- Cache the current settings before switching devices
    previousSettings = currentSettings;
    // Also update device-specific previous settings map
    devicePreviousSettings[currentDevice] = currentSettings;
}

DisplaySettingsManager::DisplaySettings LogDisplaySettingsComponent::getCurrentSettings()
{
    DisplaySettingsManager::DisplaySettings settings = currentSettings;
    
    // Update with current UI state
    settings.fontSize = fontSizeSlider.getValue();
    
    // X- Get fade rate settings
    settings.fadeRate = fadeRateSlider.getValue();
    settings.fadeRateEnabled = fadeRateToggle.getToggleState();
    
    // Get colors from color selectors
    settings.noteOnColor = noteOnColorSection.selector->getCurrentColour();
    settings.noteOffColor = noteOffColorSection.selector->getCurrentColour();
    settings.controllerColor = controllerColorSection.selector->getCurrentColour();
    settings.pitchBendColor = pitchBendColorSection.selector->getCurrentColour();
    settings.pressureColor = pressureColorSection.selector->getCurrentColour();
    settings.programChangeColor = programChangeColorSection.selector->getCurrentColour();
    settings.clockColor = clockColorSection.selector->getCurrentColour();
    settings.sysExColor = sysExColorSection.selector->getCurrentColour();
    settings.defaultColor = defaultColorSection.selector->getCurrentColour();
    
    // Get mute states
    settings.muteNoteOn = noteOnColorSection.muteButton.getToggleState();
    settings.muteNoteOff = noteOffColorSection.muteButton.getToggleState();
    settings.muteController = controllerColorSection.muteButton.getToggleState();
    settings.mutePitchBend = pitchBendColorSection.muteButton.getToggleState();
    settings.mutePressure = pressureColorSection.muteButton.getToggleState();
    settings.muteProgramChange = programChangeColorSection.muteButton.getToggleState();
    settings.muteClock = clockColorSection.muteButton.getToggleState();
    settings.muteSysEx = sysExColorSection.muteButton.getToggleState();
    settings.muteDefault = defaultColorSection.muteButton.getToggleState();
    
    // For "ALL" device, check the override setting
    if (currentDevice == "ALL") {
        settings.overrideAllDevices = overrideToggle.getToggleState();
    }
    
    return settings;
}

} // namespace MidiPortal 