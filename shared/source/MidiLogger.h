//
// Created by Jamie Benchia on 1/21/25.
//

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

// X- Simplified MidiLogger to focus on pure logging
class MidiLogger : public juce::Timer {
public:
    // MTC tracking with frame rate detection
    struct MTCTime {
        int hours = 0;
        int minutes = 0;
        int seconds = 0;
        int frames = 0;
        double frameRate = 24.0;
        bool isValid() const {
            return hours >= 0 && hours < 24 &&
                   minutes >= 0 && minutes < 60 &&
                   seconds >= 0 && seconds < 60 &&
                   frames >= 0 && frames < 30;
        }
    };
    
    explicit MidiLogger(const juce::String& logFilePath);
    ~MidiLogger() override;

    void logMessage(const juce::MidiMessage& message);
    void setDeviceName(const juce::String& name) { deviceName = name; }
    const juce::String& getDeviceName() const { return deviceName; }
    void timerCallback() override;

    // Fix getters to use struct members
    double getCurrentBPM() const { return timing.currentBPM; }
    const struct MTCTime& getMTCTime() const { return mtcTime; }
    double getCurrentBeat() const { return currentBeat / 4.0; }

private:
    std::ofstream logFile;
    juce::String deviceName;
    
    // MIDI timing tracking with validation
    struct TimingState {
        double lastClockTime = 0.0;
        double currentBPM = 0.0;
        bool isPlaying = false;
        static constexpr double MIN_VALID_BPM = 30.0;
        static constexpr double MAX_VALID_BPM = 300.0;
        static constexpr double SMOOTHING_FACTOR = 0.1;
        static constexpr double MIN_CLOCK_DELTA = 0.002;  // 2ms minimum between clocks
    } timing;

    MTCTime mtcTime;  // Just keep the member variable
    
    // SPP tracking with validation
    int16_t currentBeat = 0;
    static constexpr int16_t MAX_BEATS = 16384;  // 14-bit max
    
    // BPM smoothing
    static constexpr int BPM_BUFFER_SIZE = 48;  // Two quarter notes worth
    std::array<double, BPM_BUFFER_SIZE> bpmBuffer;
    int bpmBufferIndex = 0;
    
    // Message buffering
    struct BufferedMessage {
        juce::String description;
        juce::Time timestamp;
    };
    
    static constexpr size_t BUFFER_SIZE = 1024;
    std::vector<BufferedMessage> messageBuffer;
    std::mutex bufferMutex;
    
    // Async logging control
    bool shouldFlushLogs = false;
    std::atomic<bool> isWriting{false};
    
    void resetTiming();
    void updateBPM(double currentTime);
    void updateMTC(const juce::MidiMessage& message);
    void updateSPP(const juce::MidiMessage& message);
    
    // Helper for Rust integration
    void processTimingFromRust(const RustMidiStats& stats);
    
    void flushBuffer();
};

} // namespace MidiPortal