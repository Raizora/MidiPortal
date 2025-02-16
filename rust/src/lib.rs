// lib.rs
//! A minimal "MIDI observer" Rust library for MidiPortal.
//! This just collects raw MIDI messages, with no note tracking or analysis.
//! 
//! FFI boundary: 
//!  - create_engine -> returns pointer to new MidiEngine
//!  - destroy_engine -> free the MidiEngine
//!  - process_midi_message -> feed raw data to the engine
//! 
//! Expand or modify as needed for ring buffers, real-time safe data structures, etc.

mod midi_engine;

use crate::midi_engine::MidiEngine;
use std::slice;

/// If you want an error enum, define one. But for a minimal skeleton, we skip it.

// Opaque pointer to our MidiEngine.
#[repr(C)]
pub struct RustMidiEngineHandle {
    // Boxing so we can pass it as a raw pointer over FFI
    pub engine: Box<MidiEngine>,
}

/// Creates a new MidiEngine and returns an opaque pointer. 
/// The C++ side can store this pointer in a `void*` or similar.
#[no_mangle]
pub extern "C" fn create_midi_engine() -> *mut RustMidiEngineHandle {
    let engine = MidiEngine::new();
    let handle = RustMidiEngineHandle {
        engine: Box::new(engine),
    };
    // Box that handle on the heap so we can safely return a raw pointer
    Box::into_raw(Box::new(handle))
}

/// Destroys the MidiEngine pointer previously created by `create_midi_engine`.
#[no_mangle]
pub extern "C" fn destroy_midi_engine(handle: *mut RustMidiEngineHandle) {
    if handle.is_null() {
        return;
    }
    unsafe {
        drop(Box::from_raw(handle));
    }
}

/// Processes a MIDI message by copying it into the engine's storage.
/// Returns `false` if arguments are invalid (e.g., null pointer, out of range).
#[no_mangle]
pub extern "C" fn process_midi_message(
    handle: *mut RustMidiEngineHandle,
    data: *const u8,
    len: usize,
    timestamp: f64,
) -> bool {
    // Basic validation
    if handle.is_null() || data.is_null() || len == 0 || len > midi_engine::MAX_MIDI_MESSAGE_SIZE {
        return false;
    }

    // Convert the raw pointer to a slice for safe read
    let slice = unsafe { slice::from_raw_parts(data, len) };

    // Access the engine
    let engine_handle = unsafe { &mut *handle };
    engine_handle.engine.process_message(slice, timestamp);

    true
}

/// Clears all stored messages (optional utility).
#[no_mangle]
pub extern "C" fn clear_midi_messages(handle: *mut RustMidiEngineHandle) {
    if handle.is_null() {
        return;
    }
    unsafe {
        let engine_handle = &mut *handle;
        engine_handle.engine.clear();
    }
}