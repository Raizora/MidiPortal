#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <map>

class SettingsComponent : public juce::Component {
public:
    struct MidiFilterSettings {
        bool logNotes = true;
        bool logCC = true;
        bool logClock = true;
        bool logSysEx = true;
        // etc...
    };

    struct DeviceFilterSettings {
        std::map<juce::String, bool> deviceEnabled;  // device name -> enabled
        std::map<juce::String, bool> deviceLogging;  // device name -> logging enabled
    };

    SettingsComponent()
    {
        // Input Devices Section
        addAndMakeVisible(inputDevicesGroup);
        inputDevicesGroup.setText("MIDI Input Devices");
        
        // Logging Devices Section
        addAndMakeVisible(loggingDevicesGroup);
        loggingDevicesGroup.setText("Logging Devices");
        
        // Get available MIDI devices and create checkboxes
        auto devices = juce::MidiInput::getAvailableDevices();
        for (const auto& device : devices)
        {
            // Device listening checkbox
            auto* listenBox = deviceListenBoxes.add(new juce::ToggleButton(device.name));
            listenBox->setToggleState(true, juce::dontSendNotification);
            addAndMakeVisible(listenBox);
            
            // Device logging checkbox
            auto* logBox = deviceLogBoxes.add(new juce::ToggleButton(device.name));
            logBox->setToggleState(true, juce::dontSendNotification);
            addAndMakeVisible(logBox);
        }
        
        setSize(600, 400);
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced(10);
        
        // Layout groups
        auto inputArea = area.removeFromTop(200);
        inputDevicesGroup.setBounds(inputArea);
        
        auto loggingArea = area.removeFromTop(200);
        loggingDevicesGroup.setBounds(loggingArea);
        
        // Layout checkboxes
        auto inputInnerArea = inputArea.reduced(10);
        auto loggingInnerArea = loggingArea.reduced(10);
        
        for (int i = 0; i < deviceListenBoxes.size(); ++i)
        {
            deviceListenBoxes[i]->setBounds(inputInnerArea.removeFromTop(24));
            deviceLogBoxes[i]->setBounds(loggingInnerArea.removeFromTop(24));
        }
    }

private:
    juce::GroupComponent inputDevicesGroup;
    juce::GroupComponent loggingDevicesGroup;
    
    juce::OwnedArray<juce::ToggleButton> deviceListenBoxes;
    juce::OwnedArray<juce::ToggleButton> deviceLogBoxes;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SettingsComponent)
}; 