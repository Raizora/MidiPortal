#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Match the Rust RustMidiStats struct layout
typedef struct RustMidiStats {
    double current_bpm;
    double average_bpm;
    double jitter;
    int32_t clock_count;
    double last_clock_time;
} RustMidiStats;

// Function declarations for Rust FFI
bool process_midi_message(const uint8_t* data, size_t len, double timestamp, RustMidiStats* stats);

// Function to free error message memory
void free_error_message(char* message);

#ifdef __cplusplus
}
#endif 