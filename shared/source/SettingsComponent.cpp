/**
 * @file SettingsComponent.cpp
 * @brief Implementation of the SettingsComponent class.
 * 
 * This file contains the implementation of the SettingsComponent class methods,
 * which provide a user interface for configuring audio and MIDI device settings
 * in the MidiPortal application. It wraps JUCE's AudioDeviceSelectorComponent
 * and adds custom controls for MIDI channel selection and activity indication.
 */

#include "SettingsComponent.h"

namespace MidiPortal {

/**
 * @brief Constructor that initializes the settings component with a reference to the AudioDeviceManager.
 * @param deviceManager Reference to the application's AudioDeviceManager.
 * 
 * Sets up the AudioDeviceSelectorComponent with appropriate configuration for the MidiPortal
 * application (no audio inputs/outputs, MIDI inputs only), registers as a listener for
 * device changes, and initializes the MIDI channel selectors.
 */
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

/**
 * @brief Destructor that cleans up resources.
 * 
 * Stops listening for changes to the audio device manager to prevent
 * callbacks to a deleted object. The rest of the resources are cleaned up
 * automatically by the unique_ptr and OwnedArray destructors.
 */
SettingsComponent::~SettingsComponent()
{
    audioDeviceManager.removeChangeListener(this);
}

/**
 * @brief Updates the MIDI channel selectors based on enabled devices.
 * 
 * Clears existing selectors and creates new ones for each enabled MIDI input device.
 * Each device gets a MidiChannelSelector for selecting which MIDI channels to listen to
 * and a MidiActivityIndicator for displaying visual feedback when MIDI activity is detected.
 */
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

/**
 * @brief Handles change notifications from the AudioDeviceManager.
 * @param source The ChangeBroadcaster that triggered the notification.
 * 
 * Called when the audio device configuration changes, such as when
 * devices are enabled or disabled. Updates the MIDI channel selectors
 * to reflect the current state of enabled devices.
 */
void SettingsComponent::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &audioDeviceManager)
    {
        // X- Update MIDI channel selectors when device selection changes
        updateMidiChannelSelectors();
    }
}

/**
 * @brief Handles component resizing and positions all child components.
 * 
 * Positions the AudioDeviceSelectorComponent and all MIDI channel selectors
 * and activity indicators based on the new size of the component. Adjusts
 * the component's size to fit all content if it's inside a viewport.
 */
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

/**
 * @brief Triggers the activity indicator for a specific device.
 * @param deviceName The name of the device to trigger activity for.
 * 
 * Causes the activity indicator for the specified device to flash,
 * indicating that MIDI activity has been detected on that device.
 * This method is called when a MIDI message is received from a device.
 */
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