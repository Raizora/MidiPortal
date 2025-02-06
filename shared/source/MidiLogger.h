//
// Created by Jamie Benchia on 1/21/25.
//

#pragma once

#include <juce_core/juce_core.h>
#include <fstream>
#include <string>
#include <juce_audio_basics/juce_audio_basics.h> // Broader module inclusion

namespace MidiPortal { // Wrap in the project namespace

class MidiLogger {
public:
  // Constructor and Destructor
  explicit MidiLogger(const juce::String& logFilePath);
  ~MidiLogger();

  // Public method to log MIDI messages
  void logMessage(const juce::MidiMessage& message);

  void setDeviceName(const juce::String& name) { deviceName = name; }

  struct TimingStats {
    double currentBPM;
    double averageBPM;
    double jitter;  // timing variation
    int clockCount;
    double lastClockTime;
  };

  // Get current timing statistics
  TimingStats getTimingStats() const { return stats; }

private:
  std::ofstream logFile; // File stream for logging
  juce::String deviceName;
  TimingStats stats;
  
  // Helper method to update timing statistics
  void updateTimingStats(const juce::MidiMessage& message, double timeStamp);
};

} // namespace MidiPortal