#include "SettingsComponent.h"

namespace MidiPortal {

SettingsComponent::SettingsComponent(juce::AudioDeviceManager& deviceManager)
    : audioDeviceManager(deviceManager)
{
    // X- Create the AudioDeviceSelectorComponent with scrolling enabled
    deviceSelector = std::make_unique<juce::AudioDeviceSelectorComponent>(
        audioDeviceManager,
        0, 2,  // audio inputs
        0, 2,  // audio outputs
        true,  // show MIDI inputs
        false, // show MIDI outputs
        false, // stereo pairs
        false  // hide advanced options
    );
    
    addAndMakeVisible(deviceSelector.get());

    // X- Set up a callback to be notified when MIDI devices change
    audioDeviceManager.addChangeListener(this);
    
    // X- Initial update of MIDI channel selectors
    updateMidiChannelSelectors();
}

SettingsComponent::~SettingsComponent()
{
    audioDeviceManager.removeChangeListener(this);
}

void SettingsComponent::updateMidiChannelSelectors()
{
    // X- Clear existing selectors
    midiChannelSelectors.clear();
    midiActivityIndicators.clear();
    
    // X- Get list of all available MIDI input devices
    auto midiInputs = juce::MidiInput::getAvailableDevices();
    
    // X- Create selectors only for enabled devices
    for (auto& device : midiInputs)
    {
        // Only create selectors for enabled devices
        if (audioDeviceManager.isMidiInputDeviceEnabled(device.identifier))
        {
            auto* selector = new MidiChannelSelector(device.name);
            midiChannelSelectors.add(selector);
            addAndMakeVisible(selector);
            
            auto* indicator = new MidiActivityIndicator();
            midiActivityIndicators.add(indicator);
            addAndMakeVisible(indicator);
        }
    }
    
    // X- Update layout
    resized();
}

void SettingsComponent::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &audioDeviceManager)
    {
        // X- Update MIDI channel selectors when device selection changes
        updateMidiChannelSelectors();
    }
}

void SettingsComponent::resized()
{
    // X- Get the parent viewport's width if we're in one
    int componentWidth = getWidth();
    if (auto* viewport = dynamic_cast<juce::Viewport*>(getParentComponent()))
    {
        componentWidth = viewport->getWidth();
        // Update our width to match the viewport
        setSize(componentWidth, getHeight());
    }
    
    auto bounds = getLocalBounds().reduced(10);
    
    // X- Get the actual height needed by the device selector based on its content
    deviceSelector->setBounds(0, 0, bounds.getWidth(), 1000);
    int requiredHeight = deviceSelector->getHeight();
    
    // X- Position the device selector with its required height
    deviceSelector->setBounds(bounds.removeFromTop(requiredHeight));
    
    // X- Add a small gap
    bounds.removeFromTop(5);
    
    // X- Track total height used
    int totalUsedHeight = requiredHeight + 5;
    
    // X- Position MIDI channel selectors below the device selector
    for (int i = 0; i < midiChannelSelectors.size(); ++i)
    {
        auto rowArea = bounds.removeFromTop(80);
        totalUsedHeight += 80;
        
        // X- Make the activity indicator a small circle
        int indicatorSize = 15;
        midiActivityIndicators[i]->setBounds(
            rowArea.getX(), 
            rowArea.getY() + (rowArea.getHeight() - indicatorSize) / 2,
            indicatorSize, 
            indicatorSize
        );
        
        // X- Position the channel selector to the right of the indicator
        midiChannelSelectors[i]->setBounds(
            rowArea.getX() + indicatorSize + 5, 
            rowArea.getY(),
            rowArea.getWidth() - indicatorSize - 10,
            rowArea.getHeight()
        );
    }
    
    // X- Update our height to match the content
    setSize(componentWidth, totalUsedHeight + 20); // +20 for bottom margin
}

// X- Implement the method to trigger activity for a specific device
void SettingsComponent::triggerActivityForDevice(const juce::String& deviceName)
{
    for (int i = 0; i < midiChannelSelectors.size(); ++i)
    {
        if (midiChannelSelectors[i]->getDeviceName() == deviceName)
        {
            midiActivityIndicators[i]->triggerActivity();
            break;
        }
    }
}

} // namespace MidiPortal