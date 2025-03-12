/**
 * @file MidiLogger.cpp
 * @brief Implementation of the MidiLogger class.
 * 
 * This file contains the implementation of the MidiLogger class methods,
 * which handle logging and analysis of MIDI messages.
 */

#include "MidiLogger.h"

namespace MidiPortal {

/**
 * @brief Constructor that initializes the logger.
 * @param logFilePath The path to the log file.
 * 
 * Creates a new logger that writes to the specified log file.
 * The constructor attempts to create a "logs" directory in the current working directory
 * if it doesn't exist, and then opens a log file named "MidiTraffic.log" in that directory.
 * If the log file cannot be opened, an error message is logged to the debug output.
 * 
 * The constructor also starts a timer that periodically flushes the message buffer to the log file.
 */
MidiLogger::MidiLogger(const juce::String& logFilePath) 
    : juce::Timer()  // Initialize base class
{
    // X- Properly ignore unused parameter
    juce::ignoreUnused(logFilePath);
    
    juce::File buildDir = juce::File::getCurrentWorkingDirectory();
    juce::File logDir = buildDir.getChildFile("logs");

    DBG("==== MidiLogger Initialization ====");
    DBG("Build Directory: " + buildDir.getFullPathName());
    DBG("Attempting to create log directory at: " + logDir.getFullPathName());

    if (!logDir.exists()) {
        juce::Result result = logDir.createDirectory();
        if (result.failed()) {
            DBG("Failed to create log directory: " + result.getErrorMessage());
            return;
        }
    }

    if (!logDir.exists() || !logDir.hasWriteAccess()) {
        DBG("Log directory is not writable!");
        return;
    }

    juce::File logFile = logDir.getChildFile("MidiTraffic.log");
    this->logFile.open(logFile.getFullPathName().toStdString(), std::ios::out | std::ios::trunc | std::ios::binary);

    if (!this->logFile.is_open()) {
        DBG("FAILED to open log file!");
    } else {
        DBG("Successfully opened log file");
        this->logFile << "=== MidiPortal Log Started: " 
                     << juce::Time::getCurrentTime().formatted("%Y-%m-%d %H:%M:%S") 
                     << " ===" << std::endl;
        this->logFile.flush();
    }

    // Start flush timer (every 1000ms)
    startTimer(1000);
}

/**
 * @brief Destructor that cleans up resources.
 * 
 * Stops the timer, flushes any remaining messages to the log file,
 * and closes the log file.
 */
MidiLogger::~MidiLogger() {
    stopTimer();  // Stop the timer first
    flushBuffer();  // Flush any remaining messages
    if (logFile.is_open()) {
        logFile.close();
    }
}

/**
 * @brief Resets the timing state.
 * 
 * Clears all timing-related variables and buffers, including the BPM calculation,
 * clock timing, and BPM buffer.
 */
void MidiLogger::resetTiming() {
    timing.lastClockTime = 0.0;
    timing.currentBPM = 0.0;
    timing.isPlaying = false;
    bpmBufferIndex = 0;
    std::fill(bpmBuffer.begin(), bpmBuffer.end(), 0.0);
}

/**
 * @brief Updates the BPM based on a MIDI clock message.
 * @param currentTime The time when the clock message was received, in seconds.
 * 
 * Calculates the BPM based on the time between consecutive MIDI clock messages.
 * MIDI clock messages are sent at a rate of 24 per quarter note, so the BPM
 * can be calculated as 60 / (deltaTime * 24).
 * 
 * The method includes validation to detect and handle clock anomalies, such as
 * very short or very long intervals between clock messages.
 */
void MidiLogger::updateBPM(double currentTime) {
    if (!timing.isPlaying) return;

    if (timing.lastClockTime > 0.0) {
        double deltaTime = currentTime - timing.lastClockTime;
        
        // X- Detect clock anomalies
        if (deltaTime > 2.0 || deltaTime < timing.MIN_CLOCK_DELTA) {
            DBG("MIDI Clock Anomaly Detected! Resetting...");
            resetTiming();
            return;
        }

        double instantBPM = 60.0 / (deltaTime * 24.0);
        
        // X- Apply EMA smoothing
        constexpr double smoothingFactor = 0.05;  // More stable BPM smoothing
        timing.currentBPM = (instantBPM * smoothingFactor) + 
                           (timing.currentBPM * (1.0 - smoothingFactor));
    }
    timing.lastClockTime = currentTime;
}

/**
 * @brief Timer callback that flushes the message buffer.
 * 
 * Called regularly by the timer to flush the message buffer to the log file
 * if new messages have been added since the last flush.
 */
void MidiLogger::timerCallback() {
    if (shouldFlushLogs) {
        flushBuffer();
        shouldFlushLogs = false;
    }
}

/**
 * @brief Flushes the message buffer to the log file.
 * 
 * Writes all messages in the buffer to the log file and clears the buffer.
 * This method uses a separate thread for writing to avoid blocking the main thread,
 * and includes synchronization to prevent concurrent write operations.
 */
void MidiLogger::flushBuffer() {
    if (isWriting.exchange(true)) return;  // Prevent concurrent writes
    
    std::vector<BufferedMessage> tempBuffer;
    {
        std::lock_guard<std::mutex> lock(bufferMutex);
        tempBuffer.swap(messageBuffer);
    }

    if (!tempBuffer.empty() && logFile.is_open()) {
        std::thread([this, tempBuffer = std::move(tempBuffer)]() {
            for (const auto& msg : tempBuffer) {
                logFile << msg.timestamp.formatted("%Y-%m-%d %H:%M:%S.%ms")
                       << " " << msg.description << std::endl;
            }
            logFile.flush();
            isWriting.store(false);
        }).detach();
    } else {
        isWriting.store(false);  // X- Ensure flag is always reset
    }
}

/**
 * @brief Logs a MIDI message.
 * @param message The MIDI message to log.
 * 
 * Formats the message as text and adds it to the message buffer.
 * The message will be written to the log file when the buffer is flushed.
 * 
 * This method also extracts and formats information about the MIDI message,
 * such as the message type, channel, and relevant parameters.
 */
void MidiLogger::logMessage(const juce::MidiMessage& message) {
    try {
        juce::String description;
        auto now = juce::Time::getCurrentTime();

        const uint8_t* rawData = message.getRawData();
        uint8_t statusByte = rawData[0] & 0xF0; // Extract status type
        uint8_t channel = (rawData[0] & 0x0F) + 1; // Extract channel (1-16)

        if (message.isNoteOn()) {
            description << "Note On: " << message.getNoteNumber()
                       << " (" << juce::MidiMessage::getMidiNoteName(message.getNoteNumber(), true, true, 4) << ")"
                       << " Vel=" << message.getVelocity()
                       << " Ch=" << channel;
        }
        else if (message.isNoteOff()) {
            description << "Note Off: " << message.getNoteNumber()
                       << " Velocity: " << message.getVelocity();
        }
        else if (message.isPitchWheel()) {
            int pitchBendValue = message.getPitchWheelValue() - 8192;
            description << "Pitch Bend: " << pitchBendValue << " Ch=" << channel;
        }
        else if (message.isController()) {
            description << "CC " << message.getControllerNumber()
                       << " Value: " << message.getControllerValue()
                       << " Ch=" << channel;
            
            switch (message.getControllerNumber()) {
                case 1:  description << " (Mod Wheel)"; break;
                case 7:  description << " (Volume)"; break;
                case 10: description << " (Pan)"; break;
                case 11: description << " (Expression)"; break;
                case 64: description << " (Sustain Pedal)"; break;
                case 74: description << " (Filter Cutoff)"; break;
            }
        }
        else if (statusByte == 0xA0) { // Polyphonic Aftertouch
            description << "Poly Aftertouch: Note=" << message.getNoteNumber()
                       << " Value=" << message.getAfterTouchValue() 
                       << " Ch=" << channel;
        }
        else if (message.isChannelPressure()) {
            description << "Channel Pressure: " << message.getChannelPressureValue();
        }
        else if (message.isProgramChange()) {
            description << "Program Change: " << message.getProgramChangeNumber();
        }
        else if (message.isMidiClock()) {
            updateBPM(now.toMilliseconds() / 1000.0);
            if (timing.currentBPM > 0.0) {
                description << "MIDI Clock - BPM: " << juce::String(timing.currentBPM, 1);
            }
        }
        else if (message.isSysEx()) {
            description << "SysEx Message: Size=" << message.getRawDataSize() << " bytes";
        }
        else if (rawData[0] == 0xFE) {  // Active Sensing
            description << "Active Sensing Message";
        }
        else if (rawData[0] == 0xFF) {  // System Reset
            description << "System Reset Message";
        }
        else {
            description << "Unknown MIDI Message: Status Byte: " 
                       << juce::String::formatted("0x%X", rawData[0]);
        }

        if (description.isNotEmpty()) {
            description << " (Channel: " << channel << ")";
            BufferedMessage msg{description, now};
            {
                std::lock_guard<std::mutex> lock(bufferMutex);
                messageBuffer.push_back(std::move(msg));
                shouldFlushLogs = true;
            }
        }
    }
    catch (const std::exception& e) {
        DBG("Exception in MidiLogger::logMessage: " + juce::String(e.what()));
    }
}

} // namespace MidiPortal