/**
 * @file MidiChannelSelector.h
 * @brief Defines a UI component for selecting MIDI channels.
 * 
 * This file defines the MidiChannelSelector class, which provides a user interface
 * for selecting which MIDI channels to listen to for a specific device. It displays
 * a set of toggle buttons for each of the 16 MIDI channels.
 */

#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_devices/juce_audio_devices.h>

namespace MidiPortal {

/**
 * @class MidiChannelSelector
 * @brief A component for selecting which MIDI channels to listen to.
 * 
 * This component displays a set of toggle buttons for each of the 16 MIDI channels,
 * allowing users to select which channels to listen to for a specific MIDI device.
 * It also displays the name of the device and provides callbacks for when channel
 * selections change.
 */
class MidiChannelSelector : public juce::Component
{
public:
    /**
     * @brief Callback function type for channel toggle events.
     * 
     * This callback is triggered when a channel toggle button is clicked,
     * providing the channel number (0-15) and the new state (true=enabled, false=disabled).
     */
    std::function<void(int, bool)> onChannelToggled;
    
    /**
     * @brief Constructor that takes the name of the MIDI device.
     * @param name The name of the MIDI device.
     * 
     * Creates a new MidiChannelSelector for the specified MIDI device,
     * initializing the toggle buttons for all 16 MIDI channels.
     */
    MidiChannelSelector(const juce::String& name);
    
    /**
     * @brief Checks if a specific MIDI channel is enabled.
     * @param channel The MIDI channel to check (0-15).
     * @return true if the channel is enabled, false otherwise.
     * 
     * Returns the current state of the toggle button for the specified channel.
     * If the channel is out of range, returns false.
     */
    bool isChannelEnabled(int channel) const
    {
        return channel >= 0 && channel < 16 && channelToggles[channel]->getToggleState();
    }
    
    /**
     * @brief Gets the name of the MIDI device.
     * @return The name of the MIDI device.
     */
    const juce::String& getDeviceName() const { return deviceName; }

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
     * Positions the device name label and channel toggle buttons based on
     * the new size of the component.
     */
    void resized() override;

private:
    /**
     * @brief The name of the MIDI device.
     * 
     * Displayed in the component and used to identify which device
     * the channel selections apply to.
     */
    juce::String deviceName;
    
    /**
     * @brief Array of toggle buttons for each MIDI channel.
     * 
     * Contains 16 toggle buttons, one for each MIDI channel (0-15).
     */
    juce::OwnedArray<juce::ToggleButton> channelToggles;
    
    /**
     * @brief Label for displaying the device name.
     * 
     * Displays the name of the MIDI device at the top of the component.
     */
    juce::Label nameLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiChannelSelector)
};

} // namespace MidiPortal 