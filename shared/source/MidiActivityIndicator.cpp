/**
 * @file MidiActivityIndicator.cpp
 * @brief Implementation of the MidiActivityIndicator class.
 * 
 * This file contains the implementation of the MidiActivityIndicator class methods,
 * which provide a visual indicator that flashes when MIDI activity is detected on a device.
 */

#include "MidiActivityIndicator.h"

namespace MidiPortal {

/**
 * @brief Default constructor.
 * 
 * Initializes the MidiActivityIndicator and starts a timer that updates
 * the indicator at 30 frames per second. This timer is used to handle
 * the fade-out effect after activity is detected.
 */
MidiActivityIndicator::MidiActivityIndicator()
{
    // Start the timer to update the indicator
    startTimerHz(30); // Update at 30fps
}

/**
 * @brief Paints the indicator.
 * @param g The Graphics context to paint into.
 * 
 * Draws the indicator as a circle that is bright green when active and
 * dark grey when inactive. The indicator has a grey outline to make it
 * visible against different backgrounds.
 */
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

/**
 * @brief Triggers the activity indicator to flash.
 * 
 * Called when MIDI activity is detected on the associated device.
 * Sets the indicator to its active state, records the current time
 * for the fade-out effect, and triggers a repaint to update the display.
 */
void MidiActivityIndicator::triggerActivity()
{
    active = true;
    lastActivityTime = juce::Time::currentTimeMillis();
    repaint();
}

/**
 * @brief Timer callback for handling the fade-out effect.
 * 
 * Called regularly by the timer (30 times per second) to check if the
 * indicator should be deactivated. If the indicator is active and more
 * than 150 milliseconds have passed since the last activity, the indicator
 * is deactivated and a repaint is triggered to update the display.
 */
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