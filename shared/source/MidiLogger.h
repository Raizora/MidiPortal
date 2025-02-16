//
// Created by Jamie Benchia on 1/21/25.
//

#pragma once

#include <juce_core/juce_core.h>
#include <fstream>
#include <string>
#include <juce_audio_basics/juce_audio_basics.h>

namespace MidiPortal {

// X- Simplified MidiLogger to focus on pure logging
class MidiLogger {
public:
    explicit MidiLogger(const juce::String& logFilePath);
    ~MidiLogger();

    void logMessage(const juce::MidiMessage& message);
    void setDeviceName(const juce::String& name) { deviceName = name; }

private:
    std::ofstream logFile;
    juce::String deviceName;
};

} // namespace MidiPortal