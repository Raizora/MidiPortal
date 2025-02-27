#include "MidiChannelSelector.h"

namespace MidiPortal {

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

void MidiChannelSelector::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    
    g.setColour(juce::Colours::grey);
    g.drawRect(getLocalBounds(), 1);
}

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