/**
 * @file SharedMidiBuffer.cpp
 * @brief Implementation of the SharedMidiBuffer class.
 * 
 * This file contains the implementation of the SharedMidiBuffer class methods,
 * which provide a lock-free ring buffer for sharing MIDI events between C++ and Rust.
 */

#include "SharedMidiBuffer.h"
#include <chrono>

// FFI functions from Rust
extern "C" {
    void* create_shared_midi_buffer(size_t capacity);
    void destroy_shared_midi_buffer(void* handle);
    bool write_midi_event(void* handle, const uint8_t* data, size_t len, uint64_t timestamp, const char* device_name);
    
    // Structure matching the Rust CMidiEvent
    struct CMidiEvent {
        uint8_t* data;
        size_t data_len;
        uint64_t timestamp;
        char* device_name;
    };
    
    CMidiEvent* read_midi_event(void* handle);
    void free_midi_event(CMidiEvent* event);
    uint64_t get_current_timestamp();
}

namespace MidiPortal {

/**
 * @brief Constructor that creates a new shared buffer with the specified capacity.
 * @param capacity The capacity of the buffer in bytes.
 * 
 * Creates a new shared buffer with the specified capacity. The buffer
 * is allocated in memory and can be accessed by both C++ and Rust code.
 */
SharedMidiBuffer::SharedMidiBuffer(size_t capacity)
    : rustHandle(nullptr)
{
    // Create the Rust shared buffer
    rustHandle = create_shared_midi_buffer(capacity);
}

/**
 * @brief Destructor that cleans up resources.
 * 
 * Destroys the shared buffer and frees any allocated memory.
 */
SharedMidiBuffer::~SharedMidiBuffer()
{
    if (rustHandle != nullptr)
    {
        destroy_shared_midi_buffer(rustHandle);
        rustHandle = nullptr;
    }
}

/**
 * @brief Writes a MIDI event to the buffer.
 * @param message The MIDI message to write.
 * @param deviceName The name of the device that generated the message.
 * @return true if the write was successful, false if the buffer is full.
 * 
 * Writes a MIDI event to the buffer. The event includes the raw MIDI data,
 * a timestamp, and the name of the device that generated the event.
 */
bool SharedMidiBuffer::write(const juce::MidiMessage& message, const juce::String& deviceName)
{
    if (rustHandle == nullptr)
        return false;
    
    // Get the raw MIDI data
    const uint8_t* data = message.getRawData();
    size_t len = message.getRawDataSize();
    
    // Get the current timestamp
    uint64_t timestamp = getCurrentTimestamp();
    
    // Write the event to the Rust buffer
    return write_midi_event(rustHandle, data, len, timestamp, deviceName.toRawUTF8());
}

/**
 * @brief Reads a MIDI event from the buffer.
 * @param outEvent Reference to a MidiEvent to store the read event.
 * @return true if an event was read, false if the buffer is empty.
 * 
 * Reads a MIDI event from the buffer. The event includes the raw MIDI data,
 * a timestamp, and the name of the device that generated the event.
 */
bool SharedMidiBuffer::read(MidiEvent& outEvent)
{
    if (rustHandle == nullptr)
        return false;
    
    // Read an event from the Rust buffer
    CMidiEvent* event = read_midi_event(rustHandle);
    if (event == nullptr)
        return false;
    
    // Copy the data to the output event
    outEvent.data.resize(event->data_len);
    std::memcpy(outEvent.data.data(), event->data, event->data_len);
    outEvent.timestamp = event->timestamp;
    outEvent.deviceName = juce::String(event->device_name);
    
    // Free the event
    free_midi_event(event);
    
    return true;
}

/**
 * @brief Gets the current timestamp in microseconds.
 * @return The current timestamp in microseconds since epoch.
 * 
 * This method is used to generate timestamps for MIDI events.
 */
uint64_t SharedMidiBuffer::getCurrentTimestamp()
{
    return get_current_timestamp();
}

} // namespace MidiPortal 