//
// Created by Jamie Benchia on 1/21/25.
//

#include "MidiLogger.h"

namespace MidiPortal {


MidiLogger::MidiLogger(const juce::String& logFilePath) {
    // Create log directory in build folder
    juce::File buildDir = juce::File::getCurrentWorkingDirectory();
    juce::File logDir = buildDir.getChildFile("logs");
    
    DBG("==== MidiLogger Initialization ====");
    DBG("Build Directory: " + buildDir.getFullPathName());
    DBG("Attempting to create log directory at: " + logDir.getFullPathName());
    
    // Create directory and check result
    if (!logDir.exists()) {
        juce::Result result = logDir.createDirectory();
        if (result.failed()) {
            DBG("Failed to create log directory: " + result.getErrorMessage());
            return;
        }
    }
    
    // Verify directory exists and is writable
    if (!logDir.exists() || !logDir.hasWriteAccess()) {
        DBG("Log directory doesn't exist or isn't writable!");
        DBG("Directory exists: " + juce::String(logDir.exists() ? "YES" : "NO"));
        DBG("Write access: " + juce::String(logDir.hasWriteAccess() ? "YES" : "NO"));
        return;
    }
    
    juce::File logFile = logDir.getChildFile("MidiTraffic.log");
    DBG("Log File Path: " + logFile.getFullPathName());
    
    // Open the log file for writing
    this->logFile.open(logFile.getFullPathName().toStdString(), 
                      std::ios::out | std::ios::trunc | std::ios::binary);

    if (!this->logFile.is_open()) {
        DBG("FAILED to open log file!");
        DBG("Error: " + juce::String(strerror(errno)));
    } else {
        DBG("Successfully opened log file");
        this->logFile << "=== MidiPortal Log Started at: " 
                     << juce::Time::getCurrentTime().formatted("%Y-%m-%d %H:%M:%S") 
                     << " ===" << std::endl;
        this->logFile.flush();
        
        DBG("File exists: " + juce::String(logFile.exists() ? "YES" : "NO"));
        DBG("File size: " + juce::String(logFile.getSize()));
    }
    DBG("================================");
}

MidiLogger::~MidiLogger() {
  if (logFile.is_open()) {
    logFile.close(); // Ensure the file is closed on destruction
  }
}

void MidiLogger::logMessage(const juce::MidiMessage& message) {
    static double lastClockTime = 0.0;
    static int clockCount = 0;
    static double currentBPM = 0.0;
    
    juce::String description;
    description << "MIDI [";
    
    // Get high-precision timestamp
    auto now = juce::Time::getHighResolutionTicks();
    auto nowSeconds = juce::Time::highResolutionTicksToSeconds(now);
    
    if (message.isMidiClock()) {
        // MIDI Clock is 24 pulses per quarter note
        clockCount++;
        double currentTime = nowSeconds;
        
        if (lastClockTime > 0.0) {
            // Calculate BPM from clock intervals
            double interval = currentTime - lastClockTime;
            double instantBPM = 60.0 / (interval * 24.0);
            
            // Smooth BPM calculation
            currentBPM = (currentBPM * 0.9) + (instantBPM * 0.1);
            
            description << "MIDI Clock - BPM: " << juce::String(currentBPM, 2) 
                       << " (Pulse " << clockCount << ")";
        }
        lastClockTime = currentTime;
    }
    else if (message.isMidiStart())
        description << "Transport Start";
    else if (message.isMidiStop())
        description << "Transport Stop";
    else if (message.isMidiContinue())
        description << "Transport Continue";
    else if (message.isTempoMetaEvent())
        description << "Tempo Change: " << message.getTempoSecondsPerQuarterNote() * 60.0 << " BPM";
    else if (message.isTimeSignatureMetaEvent()) {
        int numerator, denominator;
        message.getTimeSignatureInfo(numerator, denominator);
        description << "Time Signature: " << numerator << "/" << denominator;
    }
    else if (message.getRawData()[0] == 0xFE)
        description << "Active Sensing";
    else if (message.getRawData()[0] == 0xF0)
        description << "SysEx";
    else if (message.isNoteOn())
        description << "Note On: Note=" << message.getNoteNumber() 
                   << " (" << juce::MidiMessage::getMidiNoteName(message.getNoteNumber(), true, true, 4) << ")"
                   << " Vel=" << message.getVelocity() 
                   << " Ch=" << message.getChannel();
    else if (message.isNoteOff())
        description << "Note Off: Note=" << message.getNoteNumber()
                   << " (" << juce::MidiMessage::getMidiNoteName(message.getNoteNumber(), true, true, 4) << ")"
                   << " Ch=" << message.getChannel();
    else if (message.isController()) {
        description << "CC: Number=" << message.getControllerNumber();
        // Add common CC names
        switch (message.getControllerNumber()) {
            case 1:  description << " (Modulation Wheel)"; break;
            case 7:  description << " (Volume)"; break;
            case 10: description << " (Pan)"; break;
            case 11: description << " (Expression)"; break;
            case 64: description << " (Sustain Pedal)"; break;
            case 74: description << " (Filter Cutoff)"; break;
            // Add more CC names as needed
        }
        description << " Value=" << message.getControllerValue() 
                   << " Ch=" << message.getChannel();
    }
    else if (message.isPitchWheel())
        description << "Pitch Wheel: Value=" << message.getPitchWheelValue() 
                   << " Ch=" << message.getChannel();
    else if (message.isProgramChange())
        description << "Program Change: Program=" << message.getProgramChangeNumber() 
                   << " Ch=" << message.getChannel();
    else if (message.isAftertouch())
        description << "Aftertouch: Note=" << message.getNoteNumber() 
                   << " Value=" << message.getAfterTouchValue() 
                   << " Ch=" << message.getChannel();
    else if (message.isChannelPressure())
        description << "Channel Pressure: Value=" << message.getChannelPressureValue() 
                   << " Ch=" << message.getChannel();
    
    description << "] from device: " << deviceName << " - Raw: ";
    
    // Add raw data
    for (int i = 0; i < message.getRawDataSize(); ++i)
        description << juce::String::formatted("%02x ", message.getRawData()[i]);
        
    // Add system timing info to all messages
    description << " [System Time: " << juce::Time::getCurrentTime().formatted("%H:%M:%S.%ms")
                << ", Delta: " << juce::String(nowSeconds - lastClockTime, 4) << "s]";
    
    DBG(description);
    
    if (logFile.is_open()) {
        logFile << juce::Time::getCurrentTime().formatted("%Y-%m-%d %H:%M:%S.%ms")
                << " " << description << std::endl;
        logFile.flush();
    }
}

} // namespace MidiPortal