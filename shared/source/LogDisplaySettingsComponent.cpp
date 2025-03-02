#include "LogDisplaySettingsComponent.h"

namespace MidiPortal {

LogDisplaySettingsComponent::LogDisplaySettingsComponent(MidiLogDisplay& logDisplayToControl)
    : logDisplay(logDisplayToControl),
      currentSettings(logDisplayToControl.getSettings())
{
    // X- Set up title
    titleLabel.setText("MIDI Log Display Settings", juce::dontSendNotification);
    
    // X- Create a font using the modern approach (JUCE 8)
    // First create FontOptions, then use withName() and withStyle() to configure it
    // Finally create a Font with the options
    juce::Font titleFont(juce::FontOptions().withName(juce::Font::getDefaultSansSerifFontName())
                                           .withStyle("Bold")
                                           .withHeight(18.0f));
    titleLabel.setFont(titleFont);
    
    titleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(titleLabel);
    
    // X- Set up font size slider
    fontSizeSlider.setRange(8.0, 24.0, 1.0);
    fontSizeSlider.setValue(currentSettings.fontSize, juce::dontSendNotification);
    fontSizeSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 20);
    fontSizeSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    fontSizeSlider.onValueChange = [this] {
        currentSettings.fontSize = static_cast<float>(fontSizeSlider.getValue());
    };
    addAndMakeVisible(fontSizeSlider);
    
    fontSizeLabel.setText("Font Size:", juce::dontSendNotification);
    fontSizeLabel.attachToComponent(&fontSizeSlider, true);
    addAndMakeVisible(fontSizeLabel);
    
    // X- Set up background color selector
    backgroundLabel.setText("Background:", juce::dontSendNotification);
    addAndMakeVisible(backgroundLabel);
    
    backgroundSelector.setCurrentColour(currentSettings.backgroundColor);
    backgroundSelector.setColour(juce::ColourSelector::backgroundColourId, juce::Colours::darkgrey);
    backgroundSelector.setSize(200, 200);
    
    // X- Add the change listener
    backgroundChangeListener = std::make_unique<BackgroundColorChangeListener>(currentSettings, backgroundSelector);
    backgroundSelector.addChangeListener(backgroundChangeListener.get());
    addAndMakeVisible(backgroundSelector);
    
    resetBackgroundButton.setButtonText("Reset");
    resetBackgroundButton.onClick = [this] {
        backgroundSelector.setCurrentColour(juce::Colours::black);
        currentSettings.backgroundColor = juce::Colours::black;
    };
    addAndMakeVisible(resetBackgroundButton);
    
    // X- Create color selectors for each message type
    // These will allow users to customize colors for different MIDI message types
    noteOnSelector = createColorSelector("Note On", currentSettings.noteOnColor);
    noteOffSelector = createColorSelector("Note Off", currentSettings.noteOffColor);
    pitchBendSelector = createColorSelector("Pitch Bend", currentSettings.pitchBendColor);
    controllerSelector = createColorSelector("Controller", currentSettings.controllerColor);
    pressureSelector = createColorSelector("Pressure", currentSettings.pressureColor);
    programChangeSelector = createColorSelector("Program Change", currentSettings.programChangeColor);
    clockSelector = createColorSelector("Clock", currentSettings.clockColor);
    sysExSelector = createColorSelector("SysEx", currentSettings.sysExColor);
    defaultSelector = createColorSelector("Default", currentSettings.defaultColor);
    
    // X- Set up Apply and Reset buttons
    applyButton.setButtonText("Apply Changes");
    applyButton.onClick = [this] {
        logDisplay.setSettings(currentSettings);
    };
    addAndMakeVisible(applyButton);
    
    resetAllButton.setButtonText("Reset All");
    resetAllButton.onClick = [this] {
        // Reset to default settings
        currentSettings = MidiLogDisplay::DisplaySettings();
        fontSizeSlider.setValue(currentSettings.fontSize, juce::dontSendNotification);
        backgroundSelector.setCurrentColour(currentSettings.backgroundColor);
        
        // Reset all color selectors
        noteOnSelector->selector.setCurrentColour(currentSettings.noteOnColor);
        noteOffSelector->selector.setCurrentColour(currentSettings.noteOffColor);
        pitchBendSelector->selector.setCurrentColour(currentSettings.pitchBendColor);
        controllerSelector->selector.setCurrentColour(currentSettings.controllerColor);
        pressureSelector->selector.setCurrentColour(currentSettings.pressureColor);
        programChangeSelector->selector.setCurrentColour(currentSettings.programChangeColor);
        clockSelector->selector.setCurrentColour(currentSettings.clockColor);
        sysExSelector->selector.setCurrentColour(currentSettings.sysExColor);
        defaultSelector->selector.setCurrentColour(currentSettings.defaultColor);
    };
    addAndMakeVisible(resetAllButton);
    
    // X- Set size to accommodate all controls
    setSize(600, 800);  // Increased height to fit all color selectors
}

void LogDisplaySettingsComponent::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    
    g.setColour(juce::Colours::white);
    g.drawRect(getLocalBounds(), 1);
}

void LogDisplaySettingsComponent::resized()
{
    auto area = getLocalBounds().reduced(10);
    
    // X- Position title at top
    titleLabel.setBounds(area.removeFromTop(30));
    
    // X- Add spacing
    area.removeFromTop(10);
    
    // X- Position font size slider
    auto sliderArea = area.removeFromTop(30);
    fontSizeSlider.setBounds(sliderArea.withTrimmedLeft(100));
    
    // X- Add spacing
    area.removeFromTop(10);
    
    // X- Position background color selector
    backgroundLabel.setBounds(area.removeFromTop(20).withWidth(100));
    backgroundSelector.setBounds(area.removeFromTop(200));
    
    // X- Position Reset and Apply buttons side by side for background color
    auto buttonRow = area.removeFromTop(30);
    resetBackgroundButton.setBounds(buttonRow.removeFromLeft(100));
    applyButton.setBounds(buttonRow.removeFromLeft(150).withTrimmedLeft(10));
    
    // X- Add spacing
    area.removeFromTop(20);
    
    // X- Create a scrollable area for color selectors
    const int selectorHeight = 150;
    const int labelHeight = 20;
    const int resetButtonHeight = 25;
    const int selectorSpacing = 10;
    const int totalSelectorHeight = labelHeight + selectorHeight + resetButtonHeight + selectorSpacing;
    
    // X- Calculate positions for each color selector
    // We'll arrange them in a grid with 3 columns
    int numSelectors = 9; // 9 different message types
    int columns = 3;
    int rows = (numSelectors + columns - 1) / columns; // Ceiling division
    
    int selectorWidth = (area.getWidth() - (columns - 1) * selectorSpacing) / columns;
    
    // X- Create an array of all selectors for easier iteration
    std::array<std::unique_ptr<ColorSelector>*, 9> selectors = {
        &noteOnSelector, &noteOffSelector, &pitchBendSelector,
        &controllerSelector, &pressureSelector, &programChangeSelector,
        &clockSelector, &sysExSelector, &defaultSelector
    };
    
    // X- Position each selector in the grid
    for (int i = 0; i < numSelectors; ++i) {
        int row = i / columns;
        int col = i % columns;
        
        int x = area.getX() + col * (selectorWidth + selectorSpacing);
        int y = area.getY() + row * totalSelectorHeight;
        
        auto* selector = selectors[i]->get();
        
        // Position the label
        selector->nameLabel.setBounds(x, y, selectorWidth, labelHeight);
        
        // Position the color selector
        selector->selector.setBounds(x, y + labelHeight, selectorWidth, selectorHeight);
        
        // Position the reset button
        selector->resetButton.setBounds(x, y + labelHeight + selectorHeight, selectorWidth, resetButtonHeight);
    }
    
    // X- Update area to position after all selectors
    area.removeFromTop(rows * totalSelectorHeight + 20);
    
    // X- Position Reset All button at bottom
    resetAllButton.setBounds(area.removeFromBottom(40).reduced(5));
}

// X- Helper method to create a color selector
std::unique_ptr<LogDisplaySettingsComponent::ColorSelector> LogDisplaySettingsComponent::createColorSelector(const juce::String& name, juce::Colour& targetColor)
{
    // X- Create a new ColorSelector struct
    auto selector = std::make_unique<ColorSelector>();
    
    // X- Set up the name label
    selector->nameLabel.setText(name + ":", juce::dontSendNotification);
    selector->nameLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(selector->nameLabel);
    
    // X- Set up the color selector
    selector->selector.setCurrentColour(targetColor);
    selector->selector.setColour(juce::ColourSelector::backgroundColourId, juce::Colours::darkgrey);
    selector->selector.setSize(150, 150);
    addAndMakeVisible(selector->selector);
    
    // X- Create a custom change listener that updates the target color
    class ColorChangeListener : public juce::ChangeListener
    {
    public:
        ColorChangeListener(juce::Colour& colorToUpdate, juce::ColourSelector& selector)
            : targetColor(colorToUpdate), colourSelector(selector) {}
            
        void changeListenerCallback(juce::ChangeBroadcaster* source) override
        {
            if (source == &colourSelector)
                targetColor = colourSelector.getCurrentColour();
        }
        
    private:
        juce::Colour& targetColor;
        juce::ColourSelector& colourSelector;
    };
    
    // X- Add the change listener to the selector
    auto* listener = new ColorChangeListener(targetColor, selector->selector);
    selector->selector.addChangeListener(listener);
    selector->changeListener.reset(listener); // Store the listener to manage its lifetime
    
    // X- Set up reset button
    selector->resetButton.setButtonText("Reset");
    selector->resetButton.onClick = [&targetColor, &selector = selector->selector, originalColor = targetColor]() {
        selector.setCurrentColour(originalColor);
        targetColor = originalColor;
    };
    addAndMakeVisible(selector->resetButton);
    
    return selector;
}

} // namespace MidiPortal 