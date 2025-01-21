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
  if (logFile.is_open()) {
    // Get the current timestamp
    auto timestamp = juce::Time::getCurrentTime().toString(true, true);

    // Write the timestamp and MIDI message description to the log file
    logFile << "[" << timestamp.toStdString() << "] "
            << message.getDescription().toStdString() << "\n";

    // Flush the stream to ensure data is immediately written to the file
    logFile.flush(); // X - Ensures logs are not buffered in memory
  } else {
    juce::Logger::writeToLog("Failed to write MIDI message: Log file not open");
  }
}

} // namespace MidiPortal