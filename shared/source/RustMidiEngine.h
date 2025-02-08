#pragma once

extern "C" {
    // Match Rust's MidiStats struct
    struct RustMidiStats {
        double current_bpm;
        double average_bpm;
        double jitter;
        int32_t clock_count;
        double last_clock_time;
    };

    struct ErrorInfo {
        int32_t code;
        char* message;
    };

    struct ProcessResult {
        bool success;
        ErrorInfo error;
    };

    // Updated function signature
    void process_midi_message(const uint8_t* data, size_t len, 
                            double timestamp, RustMidiStats* stats,
                            ProcessResult* result);
    
    // Function to free error message memory
    void free_error_message(char* message);
} 