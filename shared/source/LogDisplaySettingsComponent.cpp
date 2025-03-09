// LogDisplaySettingsComponent is like a form that lets users view and edit DisplaySettingsManager settings

#include "LogDisplaySettingsComponent.h"

namespace MidiPortal {

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
LogDisplaySettingsComponent::LogDisplaySettingsComponent(MidiLogDisplay& logDisplayToControl)
    : logDisplay(logDisplayToControl),
      currentSettings(logDisplay.getSettingsManager().getSettings()),
      defaultSettings(currentSettings),
      deviceSection(std::make_unique<SettingsSection>("Device")),
      appearanceSection(std::make_unique<SettingsSection>("Appearance")),
      colorSection(std::make_unique<SettingsSection>("Colors")),
      colorContainer(std::make_unique<juce::Component>())
{
    // Create sections
    deviceSection = std::make_unique<SettingsSection>("Device Settings");
    appearanceSection = std::make_unique<SettingsSection>("Appearance Settings");
    
    addAndMakeVisible(deviceSection.get());
    addAndMakeVisible(appearanceSection.get());

    // Set up device selector
    deviceLabel.setText("Device:", juce::dontSendNotification);
    deviceSelector.addItem("Default", 1);
    auto devices = juce::MidiInput::getAvailableDevices();
    int itemId = 2;
    for (const auto& device : devices)
        deviceSelector.addItem(device.name, itemId++);
    deviceSelector.setSelectedId(1, juce::dontSendNotification);
    deviceSelector.onChange = [this] { deviceSelectorChanged(); };
    
    // Add components to the device section
    deviceSection->addAndMakeVisible(deviceLabel);
    deviceSection->addAndMakeVisible(deviceSelector);
    
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
    fontSizeSlider.onValueChange = [this] { fontSizeChanged(); };
    
    // Add components to the appearance section
    appearanceSection->addAndMakeVisible(fontSizeLabel);
    appearanceSection->addAndMakeVisible(fontSizeSlider);
    
    // Set up all color sections
    setupColorSection(backgroundColorSection, "Background Color", currentSettings.backgroundColor);
    setupColorSection(noteOnColorSection, "Note On Color", currentSettings.noteOnColor);
    setupColorSection(noteOffColorSection, "Note Off Color", currentSettings.noteOffColor);
    setupColorSection(controllerColorSection, "Controller Color", currentSettings.controllerColor);
    setupColorSection(pitchBendColorSection, "Pitch Bend Color", currentSettings.pitchBendColor);
    setupColorSection(pressureColorSection, "Pressure Color", currentSettings.pressureColor);
    setupColorSection(programChangeColorSection, "Program Change Color", currentSettings.programChangeColor);
    setupColorSection(clockColorSection, "Clock Color", currentSettings.clockColor);
    setupColorSection(sysExColorSection, "SysEx Color", currentSettings.sysExColor);
    setupColorSection(defaultColorSection, "Default Color", currentSettings.defaultColor);
    
    // Add color sections to the container
    colorContainer->addAndMakeVisible(backgroundColorSection.label);
    colorContainer->addAndMakeVisible(*backgroundColorSection.selector);
    colorContainer->addAndMakeVisible(noteOnColorSection.label);
    colorContainer->addAndMakeVisible(*noteOnColorSection.selector);
    colorContainer->addAndMakeVisible(noteOffColorSection.label);
    colorContainer->addAndMakeVisible(*noteOffColorSection.selector);
    colorContainer->addAndMakeVisible(controllerColorSection.label);
    colorContainer->addAndMakeVisible(*controllerColorSection.selector);
    colorContainer->addAndMakeVisible(pitchBendColorSection.label);
    colorContainer->addAndMakeVisible(*pitchBendColorSection.selector);
    colorContainer->addAndMakeVisible(pressureColorSection.label);
    colorContainer->addAndMakeVisible(*pressureColorSection.selector);
    colorContainer->addAndMakeVisible(programChangeColorSection.label);
    colorContainer->addAndMakeVisible(*programChangeColorSection.selector);
    colorContainer->addAndMakeVisible(clockColorSection.label);
    colorContainer->addAndMakeVisible(*clockColorSection.selector);
    colorContainer->addAndMakeVisible(sysExColorSection.label);
    colorContainer->addAndMakeVisible(*sysExColorSection.selector);
    colorContainer->addAndMakeVisible(defaultColorSection.label);
    colorContainer->addAndMakeVisible(*defaultColorSection.selector);

    // Set up the viewport with the color container
    colorViewport.setViewedComponent(colorContainer.get(), false); // X- JUCE won't delete colorContainer
    colorViewport.setScrollBarsShown(true, false);  // Disable horizontal scrollbar, keep vertical
    addAndMakeVisible(colorViewport);
    
    // Set up Apply and Reset buttons
    applyButton.setButtonText("Apply Settings");
    applyButton.onClick = [this] { handleApplyButton(); };
    addAndMakeVisible(applyButton);
    
    resetButton.setButtonText("Reset");
    resetButton.onClick = [this] { handleResetButton(); };
    addAndMakeVisible(resetButton);
    
    // Initial update
    updateControls();
}

LogDisplaySettingsComponent::~LogDisplaySettingsComponent()
{
    // Set the flag to indicate we're being destroyed
    isBeingDestroyed = true;
    
    // CRITICAL: Clear the viewport's viewed component FIRST to prevent double-delete issues
    // This must be done before any other cleanup to avoid accessing components after deletion
    // The 'false' parameter ensures JUCE doesn't try to delete the component itself
    colorViewport.setViewedComponent(nullptr, false);
    
    // Remove all listeners from selectors
    for (auto* selector : {
        backgroundColorSection.selector.get(),
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
            selector->removeAllChangeListeners();
        }
    }
    
    // Reset all listeners first
    backgroundColorSection.listener.reset();
    noteOnColorSection.listener.reset();
    noteOffColorSection.listener.reset();
    controllerColorSection.listener.reset();
    pitchBendColorSection.listener.reset();
    pressureColorSection.listener.reset();
    programChangeColorSection.listener.reset();
    clockColorSection.listener.reset();
    sysExColorSection.listener.reset();
    defaultColorSection.listener.reset();
    
    // Reset all selectors next
    backgroundColorSection.selector.reset();
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

void LogDisplaySettingsComponent::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    // X- Draw an outline around the color viewport
    g.setColour(juce::Colours::darkgrey); // or whatever color you'd like
    g.drawRect(colorViewport.getBounds().reduced(-1), 2); // 2px thick border
}

void LogDisplaySettingsComponent::resized()
{
    // Start with the entire component bounds, reduced by 10px on all sides
    auto bounds = getLocalBounds().reduced(10);

    // Spacing between major sections
    const int sectionSpacing = 20;
    const int buttonHeight = 40;
    
    // Device section
    auto deviceBounds = bounds.removeFromTop(80);
    deviceSection->setBounds(deviceBounds);
    auto innerDeviceBounds = deviceBounds.reduced(10);
    deviceLabel.setBounds(innerDeviceBounds.removeFromLeft(100));  // Increased from 80 to 100
    deviceSelector.setBounds(innerDeviceBounds.reduced(5, 0));  // Add 5px padding from label
    bounds.removeFromTop(sectionSpacing);
    
    // Appearance section
    auto appearanceBounds = bounds.removeFromTop(80);
    appearanceSection->setBounds(appearanceBounds);
    
    // X- Completely revised font size control layout to ensure visibility
    // Get the inner bounds of the appearance section for our controls
    auto innerAppearanceBounds = appearanceSection->getLocalBounds().reduced(10);
    
    // Position the label on the left side
    fontSizeLabel.setBounds(innerAppearanceBounds.removeFromLeft(100).withHeight(30).withY(innerAppearanceBounds.getY() + 25));
    
    // Position the slider to fill the remaining width
    fontSizeSlider.setBounds(innerAppearanceBounds.withHeight(30).withY(innerAppearanceBounds.getY() + 25));
    
    bounds.removeFromTop(sectionSpacing);
    
    // BUTTON ROW - At bottom, Height: 40px
    // Example: If window width is 600px, button area is 580px wide (after 10px padding each side)
    auto buttonRow = bounds.removeFromBottom(buttonHeight);
    // Each button width = (580 - 20) / 2 = 280px
    auto buttonWidth = (buttonRow.getWidth() - 20) / 2; // 20px gap between buttons
    applyButton.setBounds(buttonRow.removeFromLeft(buttonWidth));  // Left button: 280x40
    buttonRow.removeFromLeft(20);  // 20px gap between buttons
    resetButton.setBounds(buttonRow);  // Right button: 280x40
    bounds.removeFromBottom(sectionSpacing); // 20px gap above buttons
    
    // COLOR VIEWPORT - Takes remaining vertical space
    // If window is 800px tall:
    // 800 - (10+10) padding - 80 device - 20 gap - 80 appearance - 20 gap - 40 buttons - 20 gap
    // = ~520px height for viewport
    colorViewport.setBounds(bounds);
    
    // COLOR SECTIONS CONTAINER
    const int colorSectionHeight = 200;
    const int totalColorSectionsHeight = 10 * colorSectionHeight;
    // X- Expand colorContainer to fill the full width of the viewport to eliminate the dead zone
    colorContainer->setBounds(0, 0, colorViewport.getWidth(), totalColorSectionsHeight);
    
    
    
    // COLOR SECTIONS LAYOUT
    auto containerBounds = colorContainer->getLocalBounds().reduced(15);
    
    // Each color section layout:
    auto positionColorSection = [](ColorSection& section, juce::Rectangle<int>& area) {
        // Total section height: 180px
        auto sectionArea = area.removeFromTop(180);
        
        // Label: 24px height at top
        section.label.setBounds(sectionArea.removeFromTop(24));
        
        // 8px gap between label and color selector
        sectionArea.removeFromTop(8);
        
        // Color selector gets remaining height (148px)
        // This gives more room for RGB value boxes and labels
        section.selector->setBounds(sectionArea);
        
        // 20px gap between sections
        area.removeFromTop(20);
    };
    
    // Position all 10 color sections sequentially
    // Each takes up 180px height + 20px gap = 200px total
    positionColorSection(backgroundColorSection, containerBounds);
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

void LogDisplaySettingsComponent::deviceSelectorChanged()
{
    // Don't access logDisplay if we're being destroyed
    if (!isBeingDestroyed) {
        currentSettings = logDisplay.getSettingsManager().getSettings(deviceSelector.getItemText(deviceSelector.getSelectedItemIndex()));
        updateControls();
    }
}

void LogDisplaySettingsComponent::fontSizeChanged()
{
    // Don't access logDisplay if we're being destroyed
    if (!isBeingDestroyed) {
        currentSettings.fontSize = static_cast<float>(fontSizeSlider.getValue());
        logDisplay.getSettingsManager().setSettings(currentSettings, deviceSelector.getItemText(deviceSelector.getSelectedItemIndex()));
    }
}

void LogDisplaySettingsComponent::updateControls()
{
    fontSizeSlider.setValue(currentSettings.fontSize, juce::dontSendNotification);
    backgroundColorSection.selector->setCurrentColour(currentSettings.backgroundColor, juce::dontSendNotification);
    noteOnColorSection.selector->setCurrentColour(currentSettings.noteOnColor, juce::dontSendNotification);
    noteOffColorSection.selector->setCurrentColour(currentSettings.noteOffColor, juce::dontSendNotification);
    controllerColorSection.selector->setCurrentColour(currentSettings.controllerColor, juce::dontSendNotification);
    pitchBendColorSection.selector->setCurrentColour(currentSettings.pitchBendColor, juce::dontSendNotification);
    pressureColorSection.selector->setCurrentColour(currentSettings.pressureColor, juce::dontSendNotification);
    programChangeColorSection.selector->setCurrentColour(currentSettings.programChangeColor, juce::dontSendNotification);
    clockColorSection.selector->setCurrentColour(currentSettings.clockColor, juce::dontSendNotification);
    sysExColorSection.selector->setCurrentColour(currentSettings.sysExColor, juce::dontSendNotification);
    defaultColorSection.selector->setCurrentColour(currentSettings.defaultColor, juce::dontSendNotification);
}

void LogDisplaySettingsComponent::comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged)
{
    if (comboBoxThatHasChanged == &deviceSelector)
    {
        deviceSelectorChanged();
    }
}

void LogDisplaySettingsComponent::setupColorSection(ColorSection& section, const juce::String& name, const juce::Colour& initialColor)
{
    section.label.setText(name, juce::dontSendNotification);
    addAndMakeVisible(section.label);
    
    section.selector = std::make_unique<juce::ColourSelector>();
    section.selector->setCurrentColour(initialColor);

    // Determine the color type based on the section name
    ColorChangeListener::ColorType colorType;
    if (name == "Background Color")        colorType = ColorChangeListener::ColorType::Background;
    else if (name == "Note On Color")      colorType = ColorChangeListener::ColorType::NoteOn;
    else if (name == "Note Off Color")     colorType = ColorChangeListener::ColorType::NoteOff;
    else if (name == "Controller Color")   colorType = ColorChangeListener::ColorType::Controller;
    else if (name == "Pitch Bend Color")   colorType = ColorChangeListener::ColorType::PitchBend;
    else if (name == "Pressure Color")     colorType = ColorChangeListener::ColorType::Pressure;
    else if (name == "Program Change Color") colorType = ColorChangeListener::ColorType::ProgramChange;
    else if (name == "Clock Color")        colorType = ColorChangeListener::ColorType::Clock;
    else if (name == "SysEx Color")       colorType = ColorChangeListener::ColorType::SysEx;
    else                                   colorType = ColorChangeListener::ColorType::Default;

    section.listener = std::make_unique<ColorChangeListener>(&currentSettings, section.selector.get(), colorType);
    section.selector->addChangeListener(section.listener.get());
    addAndMakeVisible(section.selector.get());
}

void LogDisplaySettingsComponent::handleApplyButton()
{
    // Don't access logDisplay if we're being destroyed
    if (!isBeingDestroyed) {
        // Cache the previous settings before applying new ones
        if (hasAppliedOnce) {
            previousSettings = currentSettings;
        }
        
        // Update current settings from all controls
        currentSettings.fontSize = static_cast<float>(fontSizeSlider.getValue());
        currentSettings.backgroundColor = backgroundColorSection.selector->getCurrentColour();
        currentSettings.noteOnColor = noteOnColorSection.selector->getCurrentColour();
        currentSettings.noteOffColor = noteOffColorSection.selector->getCurrentColour();
        currentSettings.controllerColor = controllerColorSection.selector->getCurrentColour();
        currentSettings.pitchBendColor = pitchBendColorSection.selector->getCurrentColour();
        currentSettings.pressureColor = pressureColorSection.selector->getCurrentColour();
        currentSettings.programChangeColor = programChangeColorSection.selector->getCurrentColour();
        currentSettings.clockColor = clockColorSection.selector->getCurrentColour();
        currentSettings.sysExColor = sysExColorSection.selector->getCurrentColour();
        currentSettings.defaultColor = defaultColorSection.selector->getCurrentColour();
        
        // Apply the settings
        logDisplay.getSettingsManager().setSettings(currentSettings, deviceSelector.getItemText(deviceSelector.getSelectedItemIndex()));
        hasAppliedOnce = true;
    }
}

void LogDisplaySettingsComponent::handleResetButton()
{
    // Don't access logDisplay if we're being destroyed
    if (!isBeingDestroyed) {
        // If we've applied settings before, reset to the previous settings
        if (hasAppliedOnce) {
            currentSettings = previousSettings;
        } else {
            // Otherwise reset to default settings
            currentSettings = defaultSettings;
        }
        
        // Update UI to reflect the reset settings
        updateControls();
        
        // Apply the reset settings
        logDisplay.getSettingsManager().setSettings(currentSettings, deviceSelector.getItemText(deviceSelector.getSelectedItemIndex()));
    }
}

void LogDisplaySettingsComponent::applySettings(const DisplaySettingsManager::DisplaySettings& settings)
{
    currentSettings = settings;
    updateControls();
    logDisplay.getSettingsManager().setSettings(currentSettings, deviceSelector.getItemText(deviceSelector.getSelectedItemIndex()));
}

} // namespace MidiPortal 