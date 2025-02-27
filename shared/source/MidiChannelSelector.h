#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_devices/juce_audio_devices.h>

namespace MidiPortal {

class MidiChannelSelector : public juce::Component
{
public:
    // Add a callback for when channel states change
    std::function<void(int, bool)> onChannelToggled;
    
    // Constructor takes just the device name
    MidiChannelSelector(const juce::String& name);
    
    // Add method to check if a channel is enabled
    bool isChannelEnabled(int channel) const
    {
        return channel >= 0 && channel < 16 && channelToggles[channel]->getToggleState();
    }
    
    // Add method to get the device name
    const juce::String& getDeviceName() const { return deviceName; }

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    juce::String deviceName;
    juce::OwnedArray<juce::ToggleButton> channelToggles;
    juce::Label nameLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiChannelSelector)
};

} // namespace MidiPortal 