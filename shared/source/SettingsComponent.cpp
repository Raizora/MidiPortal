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