/**
 * @file MidiLogger.h
 * @brief Handles logging and analysis of MIDI messages.
 * 
 * The MidiLogger class is responsible for logging MIDI messages to a file and
 * analyzing timing-related information such as BPM (beats per minute) from MIDI clock
 * messages, MIDI Time Code (MTC), and Song Position Pointer (SPP).
 * 
 * It provides methods for logging MIDI messages and retrieving timing information,
 * and uses a buffered approach to minimize I/O overhead when writing to the log file.
 */

#pragma once

#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>
#include "../include/RustBindings.h"  // Add this for RustMidiStats
#include <fstream>
#include <string>
#include <juce_audio_basics/juce_audio_basics.h>
#include <array>
#include <vector>
#include <mutex>
#include <atomic>

namespace MidiPortal {

/**
 * @class MidiLogger
 * @brief Logs and analyzes MIDI messages.
 * 
 * This class logs MIDI messages to a file and analyzes timing-related information
 * such as BPM from MIDI clock messages, MIDI Time Code (MTC), and Song Position Pointer (SPP).
 * 
 * It uses a buffered approach to minimize I/O overhead when writing to the log file,
 * and provides methods for retrieving timing information extracted from the MIDI messages.
 */
class MidiLogger : public juce::Timer {
public:
    /**
     * @struct MTCTime
     * @brief Represents MIDI Time Code (MTC) timing information.
     * 
     * Stores hours, minutes, seconds, frames, and frame rate information
     * extracted from MIDI Time Code messages.
     */
    struct MTCTime {
        int hours = 0;      ///< Hours component of the time code (0-23)
        int minutes = 0;    ///< Minutes component of the time code (0-59)
        int seconds = 0;    ///< Seconds component of the time code (0-59)
        int frames = 0;     ///< Frames component of the time code (0-29, depending on frame rate)
        double frameRate = 24.0;  ///< Frame rate in frames per second (typically 24, 25, 29.97, or 30)
        
        /**
         * @brief Checks if the time code is valid.
         * @return true if the time code is valid, false otherwise.
         * 
         * A valid time code has hours in the range 0-23, minutes in the range 0-59,
         * seconds in the range 0-59, and frames in the range 0-29.
         */
        bool isValid() const {
            return hours >= 0 && hours < 24 &&
                   minutes >= 0 && minutes < 60 &&
                   seconds >= 0 && seconds < 60 &&
                   frames >= 0 && frames < 30;
        }
    };
    
    /**
     * @brief Constructor that initializes the logger.
     * @param logFilePath The path to the log file.
     * 
     * Creates a new logger that writes to the specified log file.
     * If the log file cannot be opened, an error message is logged to the debug output.
     */
    explicit MidiLogger(const juce::String& logFilePath);
    
    /**
     * @brief Destructor that cleans up resources.
     * 
     * Stops the timer, flushes any remaining messages to the log file,
     * and closes the log file.
     */
    ~MidiLogger() override;

    /**
     * @brief Logs a MIDI message.
     * @param message The MIDI message to log.
     * 
     * Formats the message as text and adds it to the message buffer.
     * The message will be written to the log file when the buffer is flushed.
     */
    void logMessage(const juce::MidiMessage& message);
    
    /**
     * @brief Sets the name of the device that messages are being logged from.
     * @param name The name of the device.
     * 
     * This name is included in the log entries to identify which device
     * each message came from.
     */
    void setDeviceName(const juce::String& name) { deviceName = name; }
    
    /**
     * @brief Gets the name of the device that messages are being logged from.
     * @return The name of the device.
     */
    const juce::String& getDeviceName() const { return deviceName; }
    
    /**
     * @brief Timer callback that flushes the message buffer.
     * 
     * Called regularly by the timer to flush the message buffer to the log file.
     */
    void timerCallback() override;

    /**
     * @brief Gets the current BPM (beats per minute) calculated from MIDI clock messages.
     * @return The current BPM.
     */
    double getCurrentBPM() const { return timing.currentBPM; }
    
    /**
     * @brief Gets the current MIDI Time Code (MTC) time.
     * @return The current MTC time.
     */
    const struct MTCTime& getMTCTime() const { return mtcTime; }
    
    /**
     * @brief Gets the current beat position calculated from Song Position Pointer (SPP) messages.
     * @return The current beat position in beats (quarter notes).
     */
    double getCurrentBeat() const { return currentBeat / 4.0; }

private:
    /**
     * @brief Output file stream for writing log messages.
     * 
     * This stream is used to write log messages to the log file.
     */
    std::ofstream logFile;
    
    /**
     * @brief Name of the device that messages are being logged from.
     * 
     * This name is included in the log entries to identify which device
     * each message came from.
     */
    juce::String deviceName;
    
    /**
     * @struct TimingState
     * @brief Stores state for MIDI timing analysis.
     * 
     * Contains variables and constants for analyzing MIDI timing information,
     * particularly for calculating BPM from MIDI clock messages.
     */
    struct TimingState {
        double lastClockTime = 0.0;  ///< Time of the last MIDI clock message in seconds
        double currentBPM = 0.0;     ///< Current BPM calculated from MIDI clock messages
        bool isPlaying = false;      ///< Whether MIDI clock messages are being received
        
        static constexpr double MIN_VALID_BPM = 30.0;     ///< Minimum valid BPM (below this is considered an error)
        static constexpr double MAX_VALID_BPM = 300.0;    ///< Maximum valid BPM (above this is considered an error)
        static constexpr double SMOOTHING_FACTOR = 0.1;   ///< Factor for smoothing BPM calculations
        static constexpr double MIN_CLOCK_DELTA = 0.002;  ///< Minimum time between clock messages (2ms)
    } timing;

    /**
     * @brief Current MIDI Time Code (MTC) time.
     * 
     * Stores the current time extracted from MIDI Time Code messages.
     */
    MTCTime mtcTime;
    
    /**
     * @brief Current beat position from Song Position Pointer (SPP) messages.
     * 
     * Stores the current beat position in 16th notes (0-16383).
     */
    int16_t currentBeat = 0;
    
    /**
     * @brief Maximum number of beats in a Song Position Pointer message.
     * 
     * SPP messages use 14 bits to represent the beat position, giving a range of 0-16383.
     */
    static constexpr int16_t MAX_BEATS = 16384;
    
    /**
     * @brief Buffer size for BPM smoothing.
     * 
     * The number of MIDI clock messages to use for smoothing BPM calculations.
     * 48 clock messages correspond to two quarter notes (24 clock messages per quarter note).
     */
    static constexpr int BPM_BUFFER_SIZE = 48;
    
    /**
     * @brief Buffer for BPM smoothing.
     * 
     * Stores the BPM calculated from recent MIDI clock messages for smoothing.
     */
    std::array<double, BPM_BUFFER_SIZE> bpmBuffer;
    
    /**
     * @brief Current index in the BPM buffer.
     * 
     * Points to the next position in the BPM buffer to write to.
     */
    int bpmBufferIndex = 0;
    
    /**
     * @struct BufferedMessage
     * @brief Represents a MIDI message in the buffer.
     * 
     * Stores the formatted description of a MIDI message and its timestamp
     * for later writing to the log file.
     */
    struct BufferedMessage {
        juce::String description;  ///< Formatted description of the MIDI message
        juce::Time timestamp;      ///< Time when the message was received
    };
    
    /**
     * @brief Maximum size of the message buffer.
     * 
     * The maximum number of messages to store in the buffer before flushing to the log file.
     */
    static constexpr size_t BUFFER_SIZE = 1024;
    
    /**
     * @brief Buffer for MIDI messages.
     * 
     * Stores MIDI messages before they are written to the log file.
     */
    std::vector<BufferedMessage> messageBuffer;
    
    /**
     * @brief Mutex for protecting the message buffer.
     * 
     * Ensures thread-safe access to the message buffer.
     */
    std::mutex bufferMutex;
    
    /**
     * @brief Flag indicating whether the buffer should be flushed.
     * 
     * Set to true when new messages are added to the buffer,
     * and reset to false after the buffer is flushed.
     */
    bool shouldFlushLogs = false;
    
    /**
     * @brief Flag indicating whether a write operation is in progress.
     * 
     * Used to prevent concurrent write operations to the log file.
     */
    std::atomic<bool> isWriting{false};
    
    /**
     * @brief Resets the timing state.
     * 
     * Clears all timing-related variables and buffers.
     */
    void resetTiming();
    
    /**
     * @brief Updates the BPM based on a MIDI clock message.
     * @param currentTime The time when the clock message was received, in seconds.
     * 
     * Calculates the BPM based on the time between consecutive MIDI clock messages.
     */
    void updateBPM(double currentTime);
    
    /**
     * @brief Updates the MTC time based on a MIDI Time Code message.
     * @param message The MIDI message containing MTC information.
     * 
     * Extracts time code information from MIDI Time Code messages.
     */
    void updateMTC(const juce::MidiMessage& message);
    
    /**
     * @brief Updates the beat position based on a Song Position Pointer message.
     * @param message The MIDI message containing SPP information.
     * 
     * Extracts beat position information from Song Position Pointer messages.
     */
    void updateSPP(const juce::MidiMessage& message);
    
    /**
     * @brief Processes timing information from the Rust MIDI engine.
     * @param stats The MIDI statistics from the Rust engine.
     * 
     * Updates timing information based on statistics from the Rust MIDI engine.
     */
    void processTimingFromRust(const RustMidiStats& stats);
    
    /**
     * @brief Flushes the message buffer to the log file.
     * 
     * Writes all messages in the buffer to the log file and clears the buffer.
     */
    void flushBuffer();
};

} // namespace MidiPortal