#[repr(C)]
pub struct RustMidiStats {
    pub current_bpm: f64,
    pub average_bpm: f64,
    pub jitter: f64,
    pub clock_count: i32,
    pub last_clock_time: f64,
}

#[repr(C)]
pub struct ProcessResult {
    pub success: bool,
    pub error: ErrorInfo,
}

#[repr(C)]
pub struct ErrorInfo {
    pub code: i32,
    pub message: *mut i8,
}

#[no_mangle]
pub extern "C" fn process_midi_message(
    _data: *const u8,
    _len: usize,
    _timestamp: f64,
    _stats: *mut RustMidiStats,
    result: *mut ProcessResult,
) {
    // Basic implementation
    unsafe {
        (*result).success = true;
    }
} 