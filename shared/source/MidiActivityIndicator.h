/**
 * @file MidiActivityIndicator.h
 * @brief Defines a visual indicator for MIDI activity.
 * 
 * This file defines the MidiActivityIndicator class, which provides a visual
 * indicator that flashes when MIDI activity is detected on a device. This helps
 * users see which MIDI devices are actively sending or receiving data.
 */

#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

namespace MidiPortal {

/**
 * @class MidiActivityIndicator
 * @brief A visual indicator that flashes when MIDI activity is detected.
 * 
 * This component displays a small indicator that flashes when MIDI activity
 * is detected on a device. It uses a timer to fade out the indicator after
 * a short period of inactivity.
 * 
 * The indicator is typically placed next to a device name in the settings
 * interface to show which devices are actively sending or receiving MIDI data.
 */
class MidiActivityIndicator : public juce::Component,
                             public juce::Timer
{
public:
    /**
     * @brief Default constructor.
     * 
     * Initializes the MidiActivityIndicator and starts the timer for
     * handling the fade-out effect.
     */
    MidiActivityIndicator();
    
    /**
     * @brief Paints the indicator.
     * @param g The Graphics context to paint into.
     * 
     * Draws the indicator as a circle that is bright when active and
     * dimmed when inactive.
     */
    void paint(juce::Graphics& g) override;
    
    /**
     * @brief Timer callback for handling the fade-out effect.
     * 
     * Called regularly by the timer to check if the indicator should
     * be deactivated after a period of inactivity.
     */
    void timerCallback() override;
    
    /**
     * @brief Triggers the activity indicator to flash.
     * 
     * Called when MIDI activity is detected on the associated device,
     * causing the indicator to flash and then fade out after a short delay.
     */
    void triggerActivity();
    
private:
    /**
     * @brief Flag indicating whether the indicator is currently active.
     * 
     * When true, the indicator is drawn in its active (bright) state.
     */
    bool active = false;
    
    /**
     * @brief Timestamp of the last detected activity.
     * 
     * Used to determine when to deactivate the indicator after a period of inactivity.
     */
    juce::int64 lastActivityTime = 0;
};

} // namespace MidiPortal 