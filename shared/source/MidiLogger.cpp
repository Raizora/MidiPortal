//
// Created by Jamie Benchia on 1/21/25.
//

#include "MidiLogger.h"

namespace MidiPortal {


MidiLogger::MidiLogger(const juce::String& logFilePath) {
  // Open the log file for writing, truncating any existing content
  logFile.open(logFilePath.toStdString(), std::ios::out | std::ios::trunc);

  if (!logFile.is_open()) {
    juce::Logger::writeToLog("Failed to open log file: " + logFilePath);
  } else {
    // Log success to help with debugging
    juce::Logger::writeToLog("Successfully opened log file: " + logFilePath);
  }
}

MidiLogger::~MidiLogger() {
  if (logFile.is_open()) {
    logFile.close(); // Ensure the file is closed on destruction
  }
}

void MidiLogger::logMessage(const juce::MidiMessage& message) {
    juce::String description;
    description << "MIDI [";
    
    // Identify message type
    if (message.getRawData()[0] == 0xFE)
        description << "Active Sensing";
    else if (message.getRawData()[0] == 0xF0)
        description << "SysEx";
    else if (message.isNoteOn())
        description << "Note On";
    else if (message.isNoteOff())
        description << "Note Off";
    else
        description << "Other";
    
    description << "] from device: " << deviceName << " - Raw: ";
    
    // Add raw data
    for (int i = 0; i < message.getRawDataSize(); ++i)
        description << juce::String::formatted("%02x ", message.getRawData()[i]);
        
    DBG(description);
}

} // namespace MidiPortal