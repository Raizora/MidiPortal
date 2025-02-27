#include "MidiActivityIndicator.h"

namespace MidiPortal {

MidiActivityIndicator::MidiActivityIndicator()
{
    // Start the timer to update the indicator
    startTimerHz(30); // Update at 30fps
}

void MidiActivityIndicator::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().reduced(2);
    
    // Draw background
    g.setColour(juce::Colours::darkgrey);
    g.fillEllipse(bounds.toFloat());
    
    // Draw active indicator
    if (active)
    {
        g.setColour(juce::Colours::green);
        g.fillEllipse(bounds.toFloat());
    }
    
    // Draw outline
    g.setColour(juce::Colours::grey);
    g.drawEllipse(bounds.toFloat(), 1.0f);
}

void MidiActivityIndicator::triggerActivity()
{
    active = true;
    lastActivityTime = juce::Time::currentTimeMillis();
    repaint();
}

void MidiActivityIndicator::timerCallback()
{
    // Check if we need to turn off the indicator
    if (active && juce::Time::currentTimeMillis() - lastActivityTime > 150)
    {
        active = false;
        repaint();
    }
}

} // namespace MidiPortal 