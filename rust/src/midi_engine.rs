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
<<<<<<< HEAD
=======
}

/// Statistics for MIDI timing analysis
#[derive(Debug, Default)]
pub struct MidiStats {
    // Timing stats
    pub current_bpm: f64,
    pub average_bpm: f64,
    pub jitter: f64,
    pub clock_count: i32,
    pub last_clock_time: f64,
    
    // MTC stats
    pub mtc_hours: i32,
    pub mtc_minutes: i32,
    pub mtc_seconds: i32,
    pub mtc_frames: i32,
    pub mtc_frame_rate: f64,
    
    // SPP stats
    pub current_beat: i16,
    pub sysex_in_progress: bool,
>>>>>>> cursor-main
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
<<<<<<< HEAD
    pub fn process_message(&mut self, data: &[u8], timestamp: f64) {
        let evt = MidiEvent {
            data: data.to_vec(),
            timestamp,
        };
        self.messages.push(evt);
=======
    pub fn process_message(&mut self, data: &[u8], timestamp: f64) -> MidiStats {
        let mut stats = MidiStats::default();
        
        // Validate message size
        if data.is_empty() {
            return stats;
        }
        
        match data[0] {
            0xF8 => self.update_timing(timestamp, &mut stats),
            0xF1 if data.len() >= 2 => self.update_mtc(data[1], &mut stats),
            0xF2 if data.len() >= 3 => self.update_spp(data[1], data[2], &mut stats),
            0xF0 => {
                // Handle SysEx start - could buffer for reassembly
                stats.sysex_in_progress = true;
            },
            0xF7 => {
                // Handle SysEx end
                stats.sysex_in_progress = false;
            },
            _ => {}  // Other message types
        }
        
        stats
    }

    fn update_timing(&mut self, timestamp: f64, stats: &mut MidiStats) {
        if let Some(last_event) = self.messages.last() {
            let delta = timestamp - last_event.timestamp;
            if delta > 0.0 && delta < 2.0 {  // Ignore gaps > 2 seconds
                stats.current_bpm = 60.0 / (delta * 24.0);  // 24 PPQN
                stats.jitter = delta - (60.0 / (stats.current_bpm * 24.0));
            }
        }
        stats.clock_count += 1;
        stats.last_clock_time = timestamp;
    }

    fn update_mtc(&mut self, data: u8, stats: &mut MidiStats) {
        let mtc_type = (data >> 4) & 0x7;
        let value = data & 0x0F;
        
        match mtc_type {
            0 => stats.mtc_frames = (stats.mtc_frames & 0xF0) | value as i32,
            1 => stats.mtc_frames = (stats.mtc_frames & 0x0F) | ((value as i32) << 4),
            2 => stats.mtc_seconds = (stats.mtc_seconds & 0xF0) | value as i32,
            3 => stats.mtc_seconds = (stats.mtc_seconds & 0x0F) | ((value as i32) << 4),
            4 => stats.mtc_minutes = (stats.mtc_minutes & 0xF0) | value as i32,
            5 => stats.mtc_minutes = (stats.mtc_minutes & 0x0F) | ((value as i32) << 4),
            6 => stats.mtc_hours = (stats.mtc_hours & 0xF0) | value as i32,
            7 => {
                stats.mtc_hours = (stats.mtc_hours & 0x0F) | ((value as i32 & 0x1) << 4);
                stats.mtc_frame_rate = match (value >> 1) & 0x3 {
                    0 => 24.0,
                    1 => 25.0,
                    2 => 29.97,
                    _ => 30.0,
                };
            }
            _ => {}
        }
    }

    fn update_spp(&mut self, lsb: u8, msb: u8, stats: &mut MidiStats) {
        stats.current_beat = ((msb as i16) << 7) | (lsb as i16);
>>>>>>> cursor-main
    }

    /// Clear all stored messages (if you want a "reset" feature).
    pub fn clear(&mut self) {
        self.messages.clear();
    }
}