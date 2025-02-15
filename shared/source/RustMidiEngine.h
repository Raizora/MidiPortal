#pragma once
#include "../include/RustBindings.h"

class RustMidiEngine {
public:
    bool processMidiMessage(const uint8_t* data, size_t len, double timestamp) {
        RustMidiStats stats = {};
        ProcessResult result = {};
        process_midi_message(data, len, timestamp, &stats, &result);
        return result.success;
    }
}; 