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
mod shared_buffer;
mod ml;

use crate::midi_engine::MidiEngine;
use crate::shared_buffer::{SharedMidiBuffer, MidiEvent};
use crate::ml::{ModelContextProtocol, ModelType};
use crate::ml::context::Insight;
use std::slice;
use std::ffi::{CStr, CString};
use std::os::raw::{c_char, c_void};

/// If you want an error enum, define one. But for a minimal skeleton, we skip it.

// Opaque pointer to our MidiEngine.
#[repr(C)]
pub struct RustMidiEngineHandle {
    // Boxing so we can pass it as a raw pointer over FFI
    pub engine: Box<MidiEngine>,
}

// Opaque pointer to our SharedMidiBuffer
#[repr(C)]
pub struct SharedMidiBufferHandle {
    // Boxing so we can pass it as a raw pointer over FFI
    pub buffer: Box<SharedMidiBuffer>,
}

// Opaque pointer to our ModelContextProtocol
#[repr(C)]
pub struct ModelContextHandle {
    // Boxing so we can pass it as a raw pointer over FFI
    pub context: Box<ModelContextProtocol>,
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

/// Creates a new SharedMidiBuffer with the specified capacity.
/// Returns an opaque pointer to the buffer.
#[no_mangle]
pub extern "C" fn create_shared_midi_buffer(capacity: usize) -> *mut SharedMidiBufferHandle {
    let buffer = SharedMidiBuffer::new(capacity);
    let handle = SharedMidiBufferHandle {
        buffer: Box::new(buffer),
    };
    Box::into_raw(Box::new(handle))
}

/// Creates a SharedMidiBuffer from an existing memory address.
/// This is useful for sharing memory between C++ and Rust.
/// 
/// # Safety
/// 
/// The caller must ensure that:
/// - The pointer points to a valid memory region of at least `capacity` bytes
/// - The memory remains valid for the lifetime of this object
/// - No other code will free this memory while this object exists
#[no_mangle]
pub unsafe extern "C" fn create_shared_midi_buffer_from_raw(
    buffer_ptr: *mut u8,
    capacity: usize,
) -> *mut SharedMidiBufferHandle {
    let buffer = SharedMidiBuffer::from_raw(buffer_ptr, capacity);
    let handle = SharedMidiBufferHandle {
        buffer: Box::new(buffer),
    };
    Box::into_raw(Box::new(handle))
}

/// Destroys a SharedMidiBuffer.
#[no_mangle]
pub extern "C" fn destroy_shared_midi_buffer(handle: *mut SharedMidiBufferHandle) {
    if handle.is_null() {
        return;
    }
    unsafe {
        drop(Box::from_raw(handle));
    }
}

/// Gets the raw pointer to the buffer.
/// This is useful for sharing memory between C++ and Rust.
#[no_mangle]
pub extern "C" fn get_shared_midi_buffer_ptr(handle: *const SharedMidiBufferHandle) -> *const u8 {
    if handle.is_null() {
        return std::ptr::null();
    }
    unsafe {
        let buffer_handle = &*handle;
        buffer_handle.buffer.as_ptr()
    }
}

/// Gets the mutable raw pointer to the buffer.
/// This is useful for sharing memory between C++ and Rust.
#[no_mangle]
pub extern "C" fn get_shared_midi_buffer_mut_ptr(handle: *mut SharedMidiBufferHandle) -> *mut u8 {
    if handle.is_null() {
        return std::ptr::null_mut();
    }
    unsafe {
        let buffer_handle = &mut *handle;
        buffer_handle.buffer.as_mut_ptr()
    }
}

/// Writes a MIDI event to the buffer.
/// Returns true if the write was successful, false if the buffer is full.
#[no_mangle]
pub extern "C" fn write_midi_event(
    handle: *mut SharedMidiBufferHandle,
    data: *const u8,
    len: usize,
    timestamp: u64,
    device_name: *const c_char,
) -> bool {
    if handle.is_null() || data.is_null() || device_name.is_null() || len == 0 {
        return false;
    }
    
    unsafe {
        let buffer_handle = &mut *handle;
        
        // Convert C string to Rust string
        let device_name_cstr = CStr::from_ptr(device_name);
        let device_name_str = match device_name_cstr.to_str() {
            Ok(s) => s,
            Err(_) => return false,
        };
        
        // Convert data pointer to Vec<u8>
        let data_slice = slice::from_raw_parts(data, len);
        let data_vec = data_slice.to_vec();
        
        // Create MidiEvent
        let event = MidiEvent {
            data: data_vec,
            timestamp,
            device_name: device_name_str.to_string(),
        };
        
        // Write to buffer
        buffer_handle.buffer.write(&event)
    }
}

/// Reads a MIDI event from the buffer.
/// Returns a pointer to a newly allocated MidiEvent if successful, null if the buffer is empty.
/// The caller is responsible for freeing the returned MidiEvent using free_midi_event.
#[repr(C)]
pub struct CMidiEvent {
    pub data: *mut u8,
    pub data_len: usize,
    pub timestamp: u64,
    pub device_name: *mut c_char,
}

#[no_mangle]
pub extern "C" fn read_midi_event(handle: *mut SharedMidiBufferHandle) -> *mut CMidiEvent {
    if handle.is_null() {
        return std::ptr::null_mut();
    }
    
    unsafe {
        let buffer_handle = &mut *handle;
        
        // Try to read an event
        match buffer_handle.buffer.read() {
            Some(event) => {
                // Allocate memory for data
                let data_len = event.data.len();
                let data = libc::malloc(data_len) as *mut u8;
                if data.is_null() {
                    return std::ptr::null_mut();
                }
                
                // Copy data
                std::ptr::copy_nonoverlapping(event.data.as_ptr(), data, data_len);
                
                // Allocate memory for device name
                let device_name_len = event.device_name.len() + 1; // +1 for null terminator
                let device_name = libc::malloc(device_name_len) as *mut c_char;
                if device_name.is_null() {
                    libc::free(data as *mut libc::c_void);
                    return std::ptr::null_mut();
                }
                
                // Copy device name
                std::ptr::copy_nonoverlapping(
                    event.device_name.as_ptr() as *const c_char,
                    device_name,
                    event.device_name.len()
                );
                // Add null terminator
                *device_name.add(event.device_name.len()) = 0;
                
                // Allocate memory for CMidiEvent
                let c_event = libc::malloc(std::mem::size_of::<CMidiEvent>()) as *mut CMidiEvent;
                if c_event.is_null() {
                    libc::free(data as *mut libc::c_void);
                    libc::free(device_name as *mut libc::c_void);
                    return std::ptr::null_mut();
                }
                
                // Initialize CMidiEvent
                (*c_event).data = data;
                (*c_event).data_len = data_len;
                (*c_event).timestamp = event.timestamp;
                (*c_event).device_name = device_name;
                
                c_event
            },
            None => std::ptr::null_mut(),
        }
    }
}

/// Frees a MidiEvent that was returned by read_midi_event.
#[no_mangle]
pub extern "C" fn free_midi_event(event: *mut CMidiEvent) {
    if event.is_null() {
        return;
    }
    
    unsafe {
        // Free data
        if !(*event).data.is_null() {
            libc::free((*event).data as *mut libc::c_void);
        }
        
        // Free device name
        if !(*event).device_name.is_null() {
            libc::free((*event).device_name as *mut libc::c_void);
        }
        
        // Free event
        libc::free(event as *mut libc::c_void);
    }
}

/// Gets the current timestamp in microseconds.
#[no_mangle]
pub extern "C" fn get_current_timestamp() -> u64 {
    SharedMidiBuffer::current_timestamp()
}

/// Creates a new ModelContext.
/// Returns an opaque pointer to the context.
#[no_mangle]
pub extern "C" fn create_model_context() -> *mut ModelContextHandle {
    let context = ModelContextProtocol::new();
    let handle = ModelContextHandle {
        context: Box::new(context),
    };
    Box::into_raw(Box::new(handle))
}

/// Destroys a ModelContext.
#[no_mangle]
pub extern "C" fn destroy_model_context(handle: *mut ModelContextHandle) {
    if handle.is_null() {
        return;
    }
    unsafe {
        drop(Box::from_raw(handle));
    }
}

/// Loads a model into the context.
/// Returns true if successful, false otherwise.
#[no_mangle]
pub extern "C" fn load_model(handle: *mut ModelContextHandle, model_type: i32) -> bool {
    if handle.is_null() {
        return false;
    }
    
    let model_type = match model_type {
        0 => ModelType::PatternRecognition,
        1 => ModelType::StyleClassification,
        2 => ModelType::PerformanceAnalysis,
        _ => return false,
    };
    
    unsafe {
        let context_handle = &mut *handle;
        context_handle.context.load_model(model_type).is_ok()
    }
}

/// Processes a MIDI event in the model context.
#[no_mangle]
pub extern "C" fn process_model_event(
    handle: *mut ModelContextHandle,
    data: *const u8,
    len: usize,
    timestamp: u64,
    device_name: *const c_char,
) -> bool {
    if handle.is_null() || data.is_null() || device_name.is_null() || len == 0 {
        return false;
    }
    
    unsafe {
        let context_handle = &mut *handle;
        
        // Convert C string to Rust string
        let device_name_cstr = CStr::from_ptr(device_name);
        let device_name_str = match device_name_cstr.to_str() {
            Ok(s) => s,
            Err(_) => return false,
        };
        
        // Convert data pointer to Vec<u8>
        let data_slice = slice::from_raw_parts(data, len);
        let data_vec = data_slice.to_vec();
        
        // Create MidiEvent
        let event = MidiEvent {
            data: data_vec,
            timestamp,
            device_name: device_name_str.to_string(),
        };
        
        // Process event
        context_handle.context.process_event(event);
        true
    }
}

/// Represents an insight from the model context.
#[repr(C)]
pub struct CInsight {
    /// Type of insight (0 = Pattern, 1 = Performance, 2 = Style)
    pub insight_type: i32,
    /// Description of the insight
    pub description: *mut c_char,
    /// Score or confidence (0.0 - 1.0)
    pub score: f64,
}

/// Generates insights from the model context.
/// Returns an array of insights and sets the count.
/// The caller is responsible for freeing the returned insights using free_insights.
#[no_mangle]
pub extern "C" fn generate_insights(
    handle: *mut ModelContextHandle,
    count: *mut usize,
) -> *mut CInsight {
    if handle.is_null() || count.is_null() {
        return std::ptr::null_mut();
    }
    
    unsafe {
        let context_handle = &*handle;
        
        // Generate insights
        let insights = context_handle.context.generate_insights();
        
        // Set count
        *count = insights.len();
        
        if insights.is_empty() {
            return std::ptr::null_mut();
        }
        
        // Allocate memory for insights
        let c_insights = libc::malloc(insights.len() * std::mem::size_of::<CInsight>()) as *mut CInsight;
        if c_insights.is_null() {
            return std::ptr::null_mut();
        }
        
        // Convert insights to C format
        for (i, insight) in insights.iter().enumerate() {
            let (insight_type, description, score) = match insight {
                Insight::Pattern(pattern) => {
                    let desc = format!("Pattern detected with {} events, occurred {} times", 
                                      pattern.events.len(), pattern.occurrence_count);
                    (0, desc, pattern.significance_score)
                },
                Insight::Performance { description, score, .. } => {
                    (1, description.clone(), *score)
                },
                Insight::Style { style, confidence } => {
                    let desc = format!("Style detected: {}", style);
                    (2, desc, *confidence)
                },
            };
            
            // Convert description to C string
            let c_description = match CString::new(description) {
                Ok(s) => s.into_raw(),
                Err(_) => std::ptr::null_mut(),
            };
            
            // Initialize CInsight
            let c_insight = c_insights.add(i);
            (*c_insight).insight_type = insight_type;
            (*c_insight).description = c_description;
            (*c_insight).score = score;
        }
        
        c_insights
    }
}

/// Frees insights that were returned by generate_insights.
#[no_mangle]
pub extern "C" fn free_insights(insights: *mut CInsight, count: usize) {
    if insights.is_null() || count == 0 {
        return;
    }
    
    unsafe {
        // Free each insight
        for i in 0..count {
            let insight = insights.add(i);
            
            // Free description
            if !(*insight).description.is_null() {
                let _ = CString::from_raw((*insight).description);
            }
        }
        
        // Free insights array
        libc::free(insights as *mut libc::c_void);
    }
}

// ML FFI functions
#[no_mangle]
pub extern "C" fn create_ml_context() -> *mut c_void {
    // Create a new ML context
    let context = Box::new(ml::context::MusicalContext::new());
    Box::into_raw(context) as *mut c_void
}

#[no_mangle]
pub extern "C" fn destroy_ml_context(context: *mut c_void) {
    // Safety: This function should only be called with a valid context pointer
    if !context.is_null() {
        unsafe {
            let _ = Box::from_raw(context as *mut ml::context::MusicalContext);
        }
    }
}

#[no_mangle]
pub extern "C" fn load_model_ml(context: *mut c_void, file_path: *const c_char) -> i32 {
    // Safety: This function should only be called with a valid context pointer
    if context.is_null() || file_path.is_null() {
        return -1;
    }
    
    // Convert the file path to a Rust string
    let file_path = unsafe {
        CStr::from_ptr(file_path).to_string_lossy().into_owned()
    };
    
    // Load the model
    unsafe {
        let context = &mut *(context as *mut ml::context::MusicalContext);
        // For now, just return a dummy model ID
        1
    }
}

#[no_mangle]
pub extern "C" fn unload_model(context: *mut c_void, model_id: i32) -> bool {
    // Safety: This function should only be called with a valid context pointer
    if context.is_null() {
        return false;
    }
    
    // Unload the model
    unsafe {
        let context = &mut *(context as *mut ml::context::MusicalContext);
        // For now, just return true
        true
    }
}

#[no_mangle]
pub extern "C" fn process_midi_message_ml(context: *mut c_void, data: *const u8, size: i32, device_name: *const c_char) {
    // Safety: This function should only be called with a valid context pointer
    if context.is_null() || data.is_null() || size <= 0 || device_name.is_null() {
        return;
    }
    
    // Convert the device name to a Rust string
    let device_name = unsafe {
        CStr::from_ptr(device_name).to_string_lossy().into_owned()
    };
    
    // Convert the MIDI data to a Rust slice
    let data = unsafe {
        std::slice::from_raw_parts(data, size as usize)
    };
    
    // Process the MIDI message
    unsafe {
        let context = &mut *(context as *mut ml::context::MusicalContext);
        
        // Create a MIDI message from the data
        let message = match data[0] & 0xF0 {
            0x80 => ml::context::MidiMessage::NoteOff {
                channel: (data[0] & 0x0F) as u8,
                note: data[1],
                velocity: data[2],
            },
            0x90 => ml::context::MidiMessage::NoteOn {
                channel: (data[0] & 0x0F) as u8,
                note: data[1],
                velocity: data[2],
            },
            0xA0 => ml::context::MidiMessage::PolyphonicAftertouch {
                channel: (data[0] & 0x0F) as u8,
                note: data[1],
                pressure: data[2],
            },
            0xB0 => ml::context::MidiMessage::ControlChange {
                channel: (data[0] & 0x0F) as u8,
                controller: data[1],
                value: data[2],
            },
            0xC0 => ml::context::MidiMessage::ProgramChange {
                channel: (data[0] & 0x0F) as u8,
                program: data[1],
            },
            0xD0 => ml::context::MidiMessage::ChannelAftertouch {
                channel: (data[0] & 0x0F) as u8,
                pressure: data[1],
            },
            0xE0 => ml::context::MidiMessage::PitchBend {
                channel: (data[0] & 0x0F) as u8,
                value: ((data[2] as u16) << 7) | (data[1] as u16),
            },
            _ => ml::context::MidiMessage::Other,
        };
        
        // Update the context with the message
        context.update(message);
    }
}

#[no_mangle]
pub extern "C" fn get_num_insights(context: *mut c_void, model_id: i32) -> i32 {
    // Safety: This function should only be called with a valid context pointer
    if context.is_null() {
        return 0;
    }
    
    // Get the number of insights
    unsafe {
        let context = &mut *(context as *mut ml::context::MusicalContext);
        // For now, just return a dummy value
        1
    }
}

#[no_mangle]
pub extern "C" fn get_insight_description(context: *mut c_void, model_id: i32, insight_index: i32) -> *const c_char {
    // Safety: This function should only be called with a valid context pointer
    if context.is_null() || insight_index < 0 {
        return std::ptr::null();
    }
    
    // Get the insight description
    unsafe {
        let context = &mut *(context as *mut ml::context::MusicalContext);
        // For now, just return a dummy description
        CString::new("Example insight").unwrap().into_raw()
    }
}

#[no_mangle]
pub extern "C" fn get_insight_score(context: *mut c_void, model_id: i32, insight_index: i32) -> f32 {
    // Safety: This function should only be called with a valid context pointer
    if context.is_null() || insight_index < 0 {
        return 0.0;
    }
    
    // Get the insight score
    unsafe {
        let context = &mut *(context as *mut ml::context::MusicalContext);
        // For now, just return a dummy score
        0.75
    }
}

#[no_mangle]
pub extern "C" fn get_insight_type(context: *mut c_void, model_id: i32, insight_index: i32) -> i32 {
    // Safety: This function should only be called with a valid context pointer
    if context.is_null() || insight_index < 0 {
        return 0;
    }
    
    // Get the insight type
    unsafe {
        let context = &mut *(context as *mut ml::context::MusicalContext);
        // For now, just return a dummy type (0 = Pattern)
        0
    }
}

#[no_mangle]
pub extern "C" fn get_model_description(context: *mut c_void, model_id: i32) -> *const c_char {
    // Safety: This function should only be called with a valid context pointer
    if context.is_null() {
        return std::ptr::null();
    }
    
    // Get the model description
    unsafe {
        let context = &mut *(context as *mut ml::context::MusicalContext);
        // For now, just return a dummy description
        CString::new("Example model").unwrap().into_raw()
    }
}

#[no_mangle]
pub extern "C" fn get_model_version(context: *mut c_void, model_id: i32) -> *const c_char {
    // Safety: This function should only be called with a valid context pointer
    if context.is_null() {
        return std::ptr::null();
    }
    
    // Get the model version
    unsafe {
        let context = &mut *(context as *mut ml::context::MusicalContext);
        // For now, just return a dummy version
        CString::new("1.0.0").unwrap().into_raw()
    }
}

#[no_mangle]
pub extern "C" fn get_model_author(context: *mut c_void, model_id: i32) -> *const c_char {
    // Safety: This function should only be called with a valid context pointer
    if context.is_null() {
        return std::ptr::null();
    }
    
    // Get the model author
    unsafe {
        let context = &mut *(context as *mut ml::context::MusicalContext);
        // For now, just return a dummy author
        CString::new("Example author").unwrap().into_raw()
    }
}

#[no_mangle]
pub extern "C" fn get_model_license(context: *mut c_void, model_id: i32) -> *const c_char {
    // Safety: This function should only be called with a valid context pointer
    if context.is_null() {
        return std::ptr::null();
    }
    
    // Get the model license
    unsafe {
        let context = &mut *(context as *mut ml::context::MusicalContext);
        // For now, just return a dummy license
        CString::new("MIT").unwrap().into_raw()
    }
}