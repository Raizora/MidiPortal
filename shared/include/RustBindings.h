#pragma once
#include <cstdint>   // for int32_t, uint8_t
#include <cstddef>   // for size_t

// Struct definitions first
struct RustMidiStats {
    double current_bpm;
    double average_bpm;
    double jitter;
    int32_t clock_count;
    double last_clock_time;
};

struct ProcessResult {
    bool success;
    struct ErrorInfo {
        int32_t code;
        char* message;
    } error;
};

struct ColorWithOpacity {
    float hue;
    float saturation;
    float value;
    float opacity;
};

struct Position {
    float x;
    float y;
};

// Then extern "C" block with function declarations
#ifdef __cplusplus
extern "C" {
#endif

// Add explicit declaration with all parameter names
void process_midi_message(
    const uint8_t* data,      // MIDI message data
    size_t len,               // Length of data
    double timestamp,         // Message timestamp
    RustMidiStats* stats,     // Stats structure
    ProcessResult* result     // Result structure
);

// Function to free error message memory
void free_error_message(char* message);

ColorWithOpacity midi_note_to_color_with_opacity(uint8_t note, uint8_t velocity);
Position generate_position(void);

#ifdef __cplusplus
}
#endif 