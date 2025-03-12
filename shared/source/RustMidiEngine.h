/**
 * @file RustMidiEngine.h
 * @brief Interface to the Rust-based MIDI processing engine.
 * 
 * This file defines the RustMidiEngine class, which provides a C++ interface
 * to the Rust-based MIDI processing engine. It handles the communication between
 * the C++ application and the Rust code through FFI (Foreign Function Interface).
 */

#pragma once
#include "../include/RustBindings.h"

/**
 * @class RustMidiEngine
 * @brief Provides an interface to the Rust-based MIDI processing engine.
 * 
 * This class encapsulates the communication with the Rust-based MIDI processing
 * engine, allowing MIDI messages to be passed to Rust for processing.
 */
class RustMidiEngine {
public:
    /**
     * @brief Processes a MIDI message through the Rust engine.
     * @param data Pointer to the raw MIDI message data.
     * @param len Length of the MIDI message in bytes.
     * @param timestamp Timestamp of the MIDI message in seconds.
     * @return true if the message was processed successfully, false otherwise.
     * 
     * Passes a MIDI message to the Rust engine for processing. The Rust engine
     * may analyze the message for timing information or perform other operations.
     */
    bool processMidiMessage(const uint8_t* data, size_t len, double timestamp) {
        // Cast const uint8_t* to void* as required by FFI
        return process_midi_message(
            rustEngine,  // Handle from RustBindings.h
            const_cast<uint8_t*>(data), // Cast to remove const qualifier
            len,                        // Pass length as expected
            timestamp                   // Pass timestamp directly
        );
    }

private:
    /**
     * @brief Handle to the Rust engine instance.
     * 
     * This pointer is used by the Rust FFI functions to identify the specific
     * instance of the Rust engine to operate on.
     */
    void* rustEngine = nullptr; // FFI Handle for Rust Engine
};