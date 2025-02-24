#include "SettingsComponent.h" // ✅ Include statement must be correct
#include <juce_audio_devices/juce_audio_devices.h> // ✅ Required for MIDI device functionality

void SettingsComponent::showDeviceStatus()
{
    juce::String message;
    auto devices = juce::MidiInput::getAvailableDevices();
    
    if (devices.isEmpty()) {
        message = "No MIDI devices found";
    } else {
        message = "Connected MIDI devices:\n\n";
        for (auto& device : devices) {
            message += "- " + device.name + "\n";
        }
    }
    
    juce::MessageManager::callAsync([message]() {
        juce::AlertWindow::showMessageBoxAsync(
            juce::MessageBoxIconType::InfoIcon,
            "MIDI Device Status",
            message
        );
    });
}