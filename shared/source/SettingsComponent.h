#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include "MidiActivityIndicator.h"
#include "MidiChannelSelector.h"

// X- Remove forward declaration since we need the full definition

namespace MidiPortal {

// X- SettingsComponent that wraps an AudioDeviceSelectorComponent and shows MIDI channel selectors
class SettingsComponent : public juce::Component,
                         public juce::ChangeListener
{
public:
    // X- Constructor takes a reference to the shared AudioDeviceManager
    SettingsComponent(juce::AudioDeviceManager& deviceManager);
    ~SettingsComponent() override;

    // Called when the component needs to lay out its child components
    void resized() override;
    
    // X- Method to trigger activity indicator for a specific device
    void triggerActivityForDevice(const juce::String& deviceName);
    
    // X- ChangeListener implementation
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

private:

    // X- Method to update MIDI channel selectors based on enabled devices
    void updateMidiChannelSelectors();
    
    // X- We'll store a reference to the device manager (owned by MainComponent or elsewhere)
    juce::AudioDeviceManager& audioDeviceManager;

    // X- The built-in JUCE component that shows audio+MIDI device controls
    std::unique_ptr<juce::AudioDeviceSelectorComponent> deviceSelector;

    // X- Store MIDI inputs for display
    juce::Array<juce::MidiDeviceInfo> midiInputs;
    
    // X- Arrays to hold our custom UI components
    juce::OwnedArray<MidiChannelSelector> midiChannelSelectors;
    juce::OwnedArray<MidiActivityIndicator> midiActivityIndicators;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SettingsComponent)
};

} // namespace MidiPortal