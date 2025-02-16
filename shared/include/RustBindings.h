#pragma once
#include <cstdint>   // for int32_t, uint8_t
#include <cstddef>   // for size_t

#ifdef __cplusplus
extern "C" {
#endif

// Struct definitions first
struct RustMidiStats {
    // Timing stats
    double current_bpm;
    double average_bpm;
    double jitter;
    int32_t clock_count;
    double last_clock_time;

    // Note tracking
    size_t active_notes;
    size_t total_notes;
    double average_velocity;
    double velocity_range[2];  // min, max

    // Expression tracking
    double max_pitch_bend;
    double pitch_bend_activity;
    double average_pressure;
    double pressure_activity;

    // Opaque pointers to internal state
    void* _reserved1;  // mpe_config
    void* _reserved2;  // note_tracker
    void* _reserved3;  // mpe_init
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

// Create and destroy engine
void* create_midi_engine(void);
void destroy_midi_engine(void* handle);

// Process MIDI messages
bool process_midi_message(
    void* handle,
    const uint8_t* data,
    size_t len,
    double timestamp
);

void clear_midi_messages(void* handle);

// Function to free error message memory
void free_error_message(char* message);

ColorWithOpacity midi_note_to_color_with_opacity(uint8_t note, uint8_t velocity);
Position generate_position(void);

#ifdef __cplusplus
}
#endif 