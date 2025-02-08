//! MidiPortal Rust Engine v0.1.1
//! Core MIDI processing with safety features
use std::ffi::CString;
use thiserror::Error;

// First declare the error type that other modules need
#[derive(Error, Debug)]
pub enum MidiError {
    #[error("Invalid MIDI data: {0}")]
    InvalidData(String),
    #[error("Processing error: {0}")]
    ProcessingError(String),
    #[error("Timing error: {0}")]
    TimingError(String),
}

// Then declare modules
mod mpe;
mod note_tracker;
mod midi_processor;
mod sysex;

// Then imports
use note_tracker::{NoteTracker, ExpressionStats};
use midi_processor::process_message;
use mpe::MpeConfiguration;
use sysex::MpeInitTracker;

use std::panic::catch_unwind;

// Error info struct that can cross FFI boundary
#[repr(C)]
pub struct ErrorInfo {
    code: i32,
    message: *mut i8,  // C-string
}

#[repr(C)]
pub struct ProcessResult {
    success: bool,
    error: ErrorInfo,
}

#[repr(C)]
pub struct RustMidiStats {  // Rename to match C++ side
    current_bpm: f64,
    average_bpm: f64,
    jitter: f64,
    clock_count: i32,
    last_clock_time: f64,
    mpe_config: MpeConfiguration,
    note_tracker: NoteTracker,
    mpe_init: MpeInitTracker,
}

impl RustMidiStats {
    pub fn new() -> Self {
        Self {
            current_bpm: 0.0,
            average_bpm: 0.0,
            jitter: 0.0,
            clock_count: 0,
            last_clock_time: 0.0,
            mpe_config: MpeConfiguration::new(),
            note_tracker: NoteTracker::new(),
            mpe_init: MpeInitTracker::new(),
        }
    }

    pub fn get_expression_stats(&self) -> ExpressionStats {
        self.note_tracker.get_stats()
    }
}

// Safe resource cleanup
impl Drop for RustMidiStats {
    fn drop(&mut self) {
        // Ensure any resources are properly cleaned up
        self.note_tracker.clear();
        // Add any other cleanup needed
    }
}

// Safe cloning
impl Clone for RustMidiStats {
    fn clone(&self) -> Self {
        Self {
            current_bpm: self.current_bpm,
            average_bpm: self.average_bpm,
            jitter: self.jitter,
            clock_count: self.clock_count,
            last_clock_time: self.last_clock_time,
            note_tracker: self.note_tracker.clone(),
            mpe_config: self.mpe_config.clone(),
            mpe_init: self.mpe_init.clone(),
        }
    }
}

// Add these constants at the top
const MAX_MIDI_MESSAGE_SIZE: usize = 1024;  // Maximum sysex size
const MIN_MIDI_MESSAGE_SIZE: usize = 1;     // Smallest possible MIDI message

#[no_mangle]
pub extern "C" fn process_midi_message(
    data: *const u8,
    len: usize,
    timestamp: f64,
    stats: *mut RustMidiStats,
) -> bool {
    // 1. Null pointer checks
    if data.is_null() {
        log::error!("Null MIDI data pointer provided");
        return false;
    }
    if stats.is_null() {
        log::error!("Null stats pointer provided");
        return false;
    }

    // 2. Length validation
    if len < MIN_MIDI_MESSAGE_SIZE {
        log::error!("MIDI message too short: {}", len);
        return false;
    }
    if len > MAX_MIDI_MESSAGE_SIZE {
        log::error!("MIDI message too long: {}", len);
        return false;
    }

    // 3. Timestamp validation
    if timestamp < 0.0 {
        log::error!("Invalid negative timestamp: {}", timestamp);
        return false;
    }

    // If we pass all checks, process the message
    let process_result = catch_unwind(|| {
        let data_slice = unsafe { std::slice::from_raw_parts(data, len) };
        let stats = unsafe { &mut *stats };
        process_message(data_slice, timestamp, stats)
    });

    match process_result {
        Ok(Ok(_)) => true,
        Ok(Err(e)) => {
            log::error!("MIDI processing error: {:?}", e);
            false
        }
        Err(_) => {
            log::error!("MIDI processing panic");
            false
        }
    }
}

// Free error message memory
#[no_mangle]
pub extern "C" fn free_error_message(message: *mut i8) {
    if !message.is_null() {
        unsafe {
            let _ = CString::from_raw(message);
        }
    }
}

pub fn add(left: u64, right: u64) -> u64 {
    left + right
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn it_works() {
        let result = add(2, 2);
        assert_eq!(result, 4);
    }

    #[test]
    fn test_mpe_zone_configuration() {
        let mut config = MpeConfiguration::new();
        config.configure_zone(true, 5, 48.0);  // Lower zone, 5 channels
        assert!(config.is_mpe_channel(2));
        assert!(config.is_mpe_channel(6));
        assert!(!config.is_mpe_channel(7));
    }

    #[test]
    fn test_note_tracking() {
        let mut tracker = NoteTracker::new();
        tracker.note_on(60, 2, 100, 0.0);  // Middle C, channel 2, velocity 100
        tracker.update_pitch_bend(2, 0.5, 0.1);  // Add some pitch bend
        tracker.note_off(60, 2, 1.0);  // Note duration = 1.0s
        
        let stats = tracker.get_stats();
        assert_eq!(stats.note_count, 1);
        assert!(stats.max_pitch_bend > 0.0);
    }

    #[test]
    fn test_midi_processing() {
        let mut stats = RustMidiStats::new();
        
        // Test MIDI Clock
        let clock = [0xF8];  // MIDI Clock message
        process_midi_message(clock.as_ptr(), 1, 0.0, &mut stats);
        assert_eq!(stats.clock_count, 1);
        
        // Test Note tracking
        let note_on = [0x90, 60, 100];
        process_midi_message(note_on.as_ptr(), 3, 0.0, &mut stats);
        assert!(stats.note_tracker.get_stats().active_notes > 0);
    }
}
