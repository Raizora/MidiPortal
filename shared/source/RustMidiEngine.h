#pragma once
#include "../include/RustBindings.h"

class RustMidiEngine {
public:
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
    void* rustEngine = nullptr; // FFI Handle for Rust Engine
};