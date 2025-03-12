/**
 * @file MidiChannelSelector.cpp
 * @brief Implementation of the MidiChannelSelector class.
 * 
 * This file contains the implementation of the MidiChannelSelector class methods,
 * which provide a user interface for selecting which MIDI channels to listen to
 * for a specific device.
 */

#include "MidiChannelSelector.h"

namespace MidiPortal {

/**
 * @brief Constructor that takes the name of the MIDI device.
 * @param name The name of the MIDI device.
 * 
 * Creates a new MidiChannelSelector for the specified MIDI device.
 * Initializes the device name label and creates toggle buttons for all 16 MIDI channels.
 * Each toggle button is initially set to the enabled state and has a callback
 * that triggers the onChannelToggled function when clicked.
 */
MidiChannelSelector::MidiChannelSelector(const juce::String& name)
    : deviceName(name)
{
    // Add device name label
    nameLabel.setText(deviceName, juce::dontSendNotification);
    nameLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(nameLabel);
    
    // Create toggle buttons for each MIDI channel
    for (int i = 0; i < 16; ++i)
    {
        auto* toggle = new juce::ToggleButton("Ch " + juce::String(i + 1));
        toggle->setToggleState(true, juce::dontSendNotification); // Default to enabled
        toggle->onClick = [this, i] {
            if (onChannelToggled)
                onChannelToggled(i, channelToggles[i]->getToggleState());
        };
        channelToggles.add(toggle);
        addAndMakeVisible(toggle);
    }
}

/**
 * @brief Paints the component.
 * @param g The Graphics context to paint into.
 * 
 * Fills the background with the default window background color and
 * draws a grey border around the component to visually separate it
 * from other components.
 */
void MidiChannelSelector::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    
    g.setColour(juce::Colours::grey);
    g.drawRect(getLocalBounds(), 1);
}

/**
 * @brief Handles component resizing.
 * 
 * Positions the device name label at the top of the component and
 * arranges the channel toggle buttons in a grid below it. The grid
 * has 8 columns, and the toggle buttons are sized to fit the available
 * space while maintaining a minimum width of 45 pixels.
 */
void MidiChannelSelector::resized()
{
    auto area = getLocalBounds().reduced(4);
    
    // Position the device name label
    nameLabel.setBounds(area.removeFromTop(24));
    
    // X- Position the channel toggles in a grid with properly calculated sizes
    // Calculate toggle width based on available space
    int columns = 8;
    int toggleWidth = juce::jmax(45, (area.getWidth() - 10) / columns);
    int toggleHeight = 24;
    
    for (int i = 0; i < channelToggles.size(); ++i)
    {
        int row = i / columns;
        int col = i % columns;
        
        channelToggles[i]->setBounds(
            area.getX() + col * toggleWidth,
            area.getY() + row * toggleHeight,
            toggleWidth,
            toggleHeight
        );
    }
}

} // namespace MidiPortal 