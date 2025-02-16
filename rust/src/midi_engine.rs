// midi_engine.rs

/// Maximum allowed MIDI message size (including SysEx).
pub const MAX_MIDI_MESSAGE_SIZE: usize = 1024;

/// Holds a single MIDI message + timestamp.
#[derive(Debug, Clone)]
pub struct MidiEvent {
    /// The raw bytes of the MIDI message (up to MAX_MIDI_MESSAGE_SIZE).
    pub data: Vec<u8>,
    /// Timestamp in seconds (e.g., from Time::getMillisecondCounterHiRes() / 1000.0).
    pub timestamp: f64,
}

/// The main engine that stores or observes incoming MIDI traffic.
/// If you want advanced storage (ring buffer, etc.), add it here.
#[derive(Debug)]
pub struct MidiEngine {
    // If you need concurrency, store data in an Arc<Mutex<...>> or similar.
    // For now, we just keep a list for demonstration.
    pub messages: Vec<MidiEvent>,
}

impl MidiEngine {
    /// Create a brand-new engine instance.
    pub fn new() -> Self {
        MidiEngine {
            messages: Vec::new(),
        }
    }

    /// Process a new incoming MIDI message (already validated).
    /// For now, we just store it in `messages`. 
    /// In a real-time scenario, you might want a lock-free ring buffer
    /// or immediately forward it to C++ instead.
    pub fn process_message(&mut self, data: &[u8], timestamp: f64) {
        let evt = MidiEvent {
            data: data.to_vec(),
            timestamp,
        };
        self.messages.push(evt);
    }

    /// Clear all stored messages (if you want a "reset" feature).
    pub fn clear(&mut self) {
        self.messages.clear();
    }
}