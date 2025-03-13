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

// Rust FFI functions
extern "C" {
    // MIDI Engine functions
    void* create_midi_engine();
    void destroy_midi_engine(void* engine);
    bool process_midi_message_engine(void* engine, const unsigned char* data, int size);
    void set_midi_channel_enabled(void* engine, int channel, bool enabled);
    bool is_midi_channel_enabled(void* engine, int channel);
    void set_midi_device_enabled(void* engine, const char* device_name, bool enabled);
    bool is_midi_device_enabled(void* engine, const char* device_name);
    
    // Shared MIDI Buffer functions
    void* create_shared_midi_buffer(size_t capacity);
    void destroy_shared_midi_buffer(void* buffer);
    bool write_midi_event(void* buffer, const unsigned char* data, size_t size, uint64_t timestamp, const char* device_name);
    bool read_midi_event(void* buffer, unsigned char* data, size_t* size, uint64_t* timestamp, char* device_name, size_t device_name_size);
    uint64_t get_current_timestamp_us();
    
    // ML functions
    void* create_ml_context();
    void destroy_ml_context(void* context);
    int load_model_ml(void* context, const char* file_path);
    bool unload_model(void* context, int model_id);
    void process_midi_message_ml(void* context, const unsigned char* data, int size, const char* device_name);
    int get_num_insights(void* context, int model_id);
    const char* get_insight_description(void* context, int model_id, int insight_index);
    float get_insight_score(void* context, int model_id, int insight_index);
    int get_insight_type(void* context, int model_id, int insight_index);
    const char* get_model_description(void* context, int model_id);
    const char* get_model_version(void* context, int model_id);
    const char* get_model_author(void* context, int model_id);
    const char* get_model_license(void* context, int model_id);
}

#ifdef __cplusplus
}
#endif 