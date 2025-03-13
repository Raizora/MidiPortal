/**
 * @file SharedMidiBuffer.h
 * @brief Defines a shared memory buffer for MIDI events between C++ and Rust.
 * 
 * This file defines the SharedMidiBuffer class, which provides a lock-free
 * ring buffer for sharing MIDI events between C++ and Rust. This allows
 * both languages to access the same MIDI data without copying, reducing
 * latency and overhead.
 */

#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <atomic>
#include <vector>
#include <string>
#include <memory>

namespace MidiPortal {

/**
 * @class SharedMidiBuffer
 * @brief A lock-free ring buffer for sharing MIDI events between C++ and Rust.
 * 
 * This class provides a shared memory buffer that can be accessed by both
 * C++ and Rust code. It uses atomic operations to ensure thread safety
 * without locks, making it suitable for real-time audio processing.
 * 
 * The buffer stores MIDI events with timestamps and device information,
 * allowing both languages to process the same MIDI data without copying.
 */
class SharedMidiBuffer
{
public:
    /**
     * @struct MidiEvent
     * @brief Represents a MIDI event with timestamp and device information.
     * 
     * This structure is used to pass MIDI events between C++ and Rust.
     * It includes the raw MIDI data, a timestamp, and the name of the
     * device that generated the event.
     */
    struct MidiEvent
    {
        /**
         * @brief Raw MIDI data.
         */
        std::vector<uint8_t> data;
        
        /**
         * @brief Timestamp in microseconds since epoch.
         */
        uint64_t timestamp;
        
        /**
         * @brief Name of the device that generated this event.
         */
        juce::String deviceName;
    };
    
    /**
     * @brief Constructor that creates a new shared buffer with the specified capacity.
     * @param capacity The capacity of the buffer in bytes.
     * 
     * Creates a new shared buffer with the specified capacity. The buffer
     * is allocated in memory and can be accessed by both C++ and Rust code.
     */
    SharedMidiBuffer(size_t capacity);
    
    /**
     * @brief Destructor that cleans up resources.
     * 
     * Destroys the shared buffer and frees any allocated memory.
     */
    ~SharedMidiBuffer();
    
    /**
     * @brief Writes a MIDI event to the buffer.
     * @param message The MIDI message to write.
     * @param deviceName The name of the device that generated the message.
     * @return true if the write was successful, false if the buffer is full.
     * 
     * Writes a MIDI event to the buffer. The event includes the raw MIDI data,
     * a timestamp, and the name of the device that generated the event.
     */
    bool write(const juce::MidiMessage& message, const juce::String& deviceName);
    
    /**
     * @brief Reads a MIDI event from the buffer.
     * @param outEvent Reference to a MidiEvent to store the read event.
     * @return true if an event was read, false if the buffer is empty.
     * 
     * Reads a MIDI event from the buffer. The event includes the raw MIDI data,
     * a timestamp, and the name of the device that generated the event.
     */
    bool read(MidiEvent& outEvent);
    
    /**
     * @brief Gets the current timestamp in microseconds.
     * @return The current timestamp in microseconds since epoch.
     * 
     * This method is used to generate timestamps for MIDI events.
     */
    static uint64_t getCurrentTimestamp();

private:
    /**
     * @brief Pointer to the Rust SharedMidiBufferHandle.
     * 
     * This is an opaque pointer to the Rust side of the shared buffer.
     * It is used to call Rust functions for reading and writing events.
     */
    void* rustHandle;
};

} // namespace MidiPortal 