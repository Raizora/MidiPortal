/**
 * @file SettingsComponent.h
 * @brief Component for configuring audio and MIDI device settings.
 * 
 * This file defines the SettingsComponent class, which provides a user interface
 * for configuring audio and MIDI device settings in the MidiPortal application.
 * It wraps JUCE's AudioDeviceSelectorComponent and adds custom controls for
 * MIDI channel selection and activity indication.
 */

#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include "MidiActivityIndicator.h"
#include "MidiChannelSelector.h"

// X- Remove forward declaration since we need the full definition

namespace MidiPortal {

/**
 * @class SettingsComponent
 * @brief Component for configuring audio and MIDI device settings.
 * 
 * This component provides a user interface for configuring audio and MIDI device
 * settings in the MidiPortal application. It wraps JUCE's AudioDeviceSelectorComponent
 * and adds custom controls for MIDI channel selection and activity indication.
 * 
 * The component listens for changes to the audio device manager and updates
 * the MIDI channel selectors accordingly.
 */
class SettingsComponent : public juce::Component,
                         public juce::ChangeListener
{
public:
    /**
     * @brief Constructor that takes a reference to the shared AudioDeviceManager.
     * @param deviceManager Reference to the application's AudioDeviceManager.
     * 
     * Initializes the SettingsComponent with a reference to the shared AudioDeviceManager
     * and sets up the initial UI state.
     */
    SettingsComponent(juce::AudioDeviceManager& deviceManager);
    
    /**
     * @brief Destructor that cleans up resources.
     * 
     * Stops listening for changes to the audio device manager and cleans up resources.
     */
    ~SettingsComponent() override;

    /**
     * @brief Handles component resizing.
     * 
     * Positions all child components based on the new size of the component.
     */
    void resized() override;
    
    /**
     * @brief Triggers the activity indicator for a specific device.
     * @param deviceName The name of the device to trigger activity for.
     * 
     * Causes the activity indicator for the specified device to flash,
     * indicating that MIDI activity has been detected on that device.
     */
    void triggerActivityForDevice(const juce::String& deviceName);
    
    /**
     * @brief Handles change notifications from the AudioDeviceManager.
     * @param source The ChangeBroadcaster that triggered the notification.
     * 
     * Called when the audio device configuration changes, such as when
     * devices are enabled or disabled. Updates the MIDI channel selectors.
     */
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

private:
    /**
     * @brief Updates the MIDI channel selectors based on enabled devices.
     * 
     * Creates or updates MIDI channel selectors for each enabled MIDI input device.
     * This method is called when the audio device configuration changes.
     */
    void updateMidiChannelSelectors();
    
    /**
     * @brief Reference to the application's AudioDeviceManager.
     * 
     * Used to access and modify audio and MIDI device settings.
     */
    juce::AudioDeviceManager& audioDeviceManager;

    /**
     * @brief The built-in JUCE component that shows audio+MIDI device controls.
     * 
     * This component provides the standard JUCE interface for selecting
     * audio and MIDI devices and configuring their settings.
     */
    std::unique_ptr<juce::AudioDeviceSelectorComponent> deviceSelector;

    /**
     * @brief Array of available MIDI input devices.
     * 
     * Stores information about available MIDI input devices for display.
     */
    juce::Array<juce::MidiDeviceInfo> midiInputs;
    
    /**
     * @brief Array of MIDI channel selector components.
     * 
     * These components allow selecting which MIDI channels to listen to
     * for each enabled MIDI input device.
     */
    juce::OwnedArray<MidiChannelSelector> midiChannelSelectors;
    
    /**
     * @brief Array of MIDI activity indicator components.
     * 
     * These components display visual feedback when MIDI activity is
     * detected on a specific device.
     */
    juce::OwnedArray<MidiActivityIndicator> midiActivityIndicators;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SettingsComponent)
};

} // namespace MidiPortal