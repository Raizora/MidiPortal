use std::collections::HashMap;

#[derive(Debug, Clone)]
pub struct NoteExpression {
    pub note: u8,
    pub channel: u8,
    pub velocity: f64,        // Initial velocity
    pub pitch_bend: f64,      // -1.0 to +1.0
    pub pressure: f64,        // 0.0 to 1.0 (aftertouch)
    pub timbre: f64,          // 0.0 to 1.0 (brightness)
    pub start_time: f64,      // When the note started
    pub last_update: f64,     // Last time any expression changed
}

#[derive(Debug)]
pub struct NoteTracker {
    active_notes: HashMap<(u8, u8), NoteExpression>, // (note, channel) -> expression
    history: Vec<NoteExpression>,                    // Completed notes
}

#[derive(Debug)]
pub struct ExpressionStats {
    // Basic stats
    pub note_count: usize,
    pub active_notes: usize,
    pub average_velocity: f64,
    pub velocity_range: (f64, f64),  // min, max
    
    // Timing stats
    pub average_duration: f64,
    pub shortest_note: f64,
    pub longest_note: f64,
    pub note_density: f64,  // notes per second
    
    // Expression stats
    pub max_pitch_bend: f64,
    pub pitch_bend_activity: f64,
    pub average_pressure: f64,
    pub pressure_activity: f64,
    pub average_timbre: f64,
    pub timbre_activity: f64,
    
    // Performance analysis
    pub polyphony: usize,          // Max simultaneous notes
    pub chord_changes: usize,      // Number of chord changes
    pub scale_detection: String,   // Detected musical scale
    pub key_signature: String,     // Detected key signature
}

impl NoteTracker {
    pub fn new() -> Self {
        Self {
            active_notes: HashMap::new(),
            history: Vec::new(),
        }
    }

    pub fn note_on(&mut self, note: u8, channel: u8, velocity: u8, timestamp: f64) {
        // Bounds checking
        if note > 127 || channel > 15 || velocity > 127 {
            return;  // Silently ignore invalid values
        }

        let expr = NoteExpression {
            note,
            channel,
            velocity: velocity as f64 / 127.0,
            pitch_bend: 0.0,
            pressure: 0.0,
            timbre: 0.5,  // Default to middle
            start_time: timestamp,
            last_update: timestamp,
        };
        self.active_notes.insert((note, channel), expr);
    }

    pub fn note_off(&mut self, note: u8, channel: u8, timestamp: f64) {
        // Bounds checking
        if note > 127 || channel > 15 {
            return;  // Silently ignore invalid values
        }

        if let Some(expr) = self.active_notes.remove(&(note, channel)) {
            let mut completed_note = expr.clone();  // Clone to avoid borrow checker issues
            completed_note.last_update = timestamp;
            self.history.push(completed_note);
        }
    }

    pub fn update_pitch_bend(&mut self, channel: u8, value: f64, timestamp: f64) {
        for expr in self.active_notes.values_mut() {
            if expr.channel == channel {
                expr.pitch_bend = value;
                expr.last_update = timestamp;
            }
        }
    }

    pub fn update_pressure(&mut self, note: u8, channel: u8, value: f64, timestamp: f64) {
        if let Some(expr) = self.active_notes.get_mut(&(note, channel)) {
            expr.pressure = value;
            expr.last_update = timestamp;
        }
    }

    pub fn update_timbre(&mut self, channel: u8, value: f64, timestamp: f64) {
        for expr in self.active_notes.values_mut() {
            if expr.channel == channel {
                expr.timbre = value;
                expr.last_update = timestamp;
            }
        }
    }

    pub fn get_active_notes(&self) -> &HashMap<(u8, u8), NoteExpression> {
        &self.active_notes
    }

    pub fn get_note_history(&self) -> &[NoteExpression] {
        &self.history
    }

    pub fn get_stats(&self) -> ExpressionStats {
        let mut stats = ExpressionStats {
            note_count: self.history.len(),
            active_notes: self.active_notes.len(),
            average_velocity: 0.0,
            velocity_range: (1.0, 0.0),
            average_duration: 0.0,
            shortest_note: f64::MAX,
            longest_note: 0.0,
            note_density: 0.0,
            max_pitch_bend: 0.0,
            pitch_bend_activity: 0.0,
            average_pressure: 0.0,
            pressure_activity: 0.0,
            average_timbre: 0.0,
            timbre_activity: 0.0,
            polyphony: 0,
            chord_changes: 0,
            scale_detection: String::new(),
            key_signature: String::new(),
        };

        if !self.history.is_empty() {
            let mut total_velocity = 0.0;
            let mut total_pressure = 0.0;
            let mut total_timbre = 0.0;
            let mut total_duration = 0.0;
            let mut max_simultaneous = 0;
            let mut note_times = Vec::new();
            let mut active_count = 0;
            let mut note_histogram = [0; 12];  // For scale detection

            for note in &self.history {
                // Basic stats
                total_velocity += note.velocity;
                stats.velocity_range.0 = stats.velocity_range.0.min(note.velocity);
                stats.velocity_range.1 = stats.velocity_range.1.max(note.velocity);
                
                // Timing stats
                let duration = note.last_update - note.start_time;
                total_duration += duration;
                stats.shortest_note = stats.shortest_note.min(duration);
                stats.longest_note = stats.longest_note.max(duration);
                note_times.push((note.start_time, true));
                note_times.push((note.last_update, false));
                
                // Expression stats
                total_pressure += note.pressure;
                total_timbre += note.timbre;
                stats.max_pitch_bend = stats.max_pitch_bend.max(note.pitch_bend.abs());
                
                // Scale detection
                note_histogram[(note.note % 12) as usize] += 1;
            }

            // Sort note times for polyphony analysis
            note_times.sort_by(|a, b| a.0.partial_cmp(&b.0).unwrap());
            for (_, is_start) in note_times {
                if is_start {
                    active_count += 1;
                    max_simultaneous = max_simultaneous.max(active_count);
                } else {
                    active_count -= 1;
                }
            }

            let count = self.history.len() as f64;
            stats.average_velocity = total_velocity / count;
            stats.average_pressure = total_pressure / count;
            stats.average_timbre = total_timbre / count;
            stats.average_duration = total_duration / count;
            stats.polyphony = max_simultaneous;
            
            // Detect scale and key
            stats.scale_detection = detect_scale(&note_histogram);
            stats.key_signature = detect_key(&note_histogram);
        }

        stats
    }
}

fn detect_scale(histogram: &[i32; 12]) -> String {
    // Simple scale detection based on note frequency
    let mut scale_type = "Unknown";
    let major_pattern = [2, 2, 1, 2, 2, 2, 1];
    let minor_pattern = [2, 1, 2, 2, 1, 2, 2];
    
    // Compare note distribution with scale patterns
    // This is a simplified version - you'd want more sophisticated analysis
    let mut major_match = 0;
    let mut minor_match = 0;
    
    for i in 0..12 {
        if histogram[i] > 0 {
            if major_pattern.contains(&(i as i32)) {
                major_match += 1;
            }
            if minor_pattern.contains(&(i as i32)) {
                minor_match += 1;
            }
        }
    }
    
    if major_match > minor_match {
        scale_type = "Major"
    } else if minor_match > major_match {
        scale_type = "Minor"
    }
    
    scale_type.to_string()
}

fn detect_key(histogram: &[i32; 12]) -> String {
    // Find the most frequent note as potential key
    let mut max_count = 0;
    let mut key_note = 0;
    
    for (note, &count) in histogram.iter().enumerate() {
        if count > max_count {
            max_count = count;
            key_note = note;
        }
    }
    
    // Convert note number to name
    let note_names = ["C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"];
    note_names[key_note].to_string()
}

impl ExpressionStats {
    // Get velocity data formatted for a histogram
    pub fn get_velocity_histogram(&self) -> Vec<(f64, usize)> {
        let mut bins = vec![(0.0, 0); 8];  // 8 velocity ranges
        for i in 0..8 {
            bins[i].0 = i as f64 * 0.125;  // Center of each bin
        }
        // Fill in actual data from note history...
        bins
    }

    // Get timing data for note length visualization
    pub fn get_note_timing_data(&self) -> Vec<(f64, f64)> {
        // Returns [(start_time, duration), ...]
        vec![]  // Would be filled with actual note timing data
    }

    // Get expression data for MPE visualization
    pub fn get_expression_plot_data(&self) -> Vec<(f64, f64, f64)> {
        // Returns [(pitch_bend, pressure, timbre), ...]
        vec![]  // Would be filled with actual expression data
    }

    // Format stats as a pretty string for display
    pub fn format_display(&self) -> String {
        format!(
            "Performance Stats:\n\
             Key: {} {}\n\
             Notes: {} (max polyphony: {})\n\
             Velocity: {:.2} (range: {:.2}-{:.2})\n\
             Duration: {:.2}ms (range: {:.2}-{:.2})\n\
             Expression: {:.2}% pitch bend activity",
            self.key_signature,
            self.scale_detection,
            self.note_count,
            self.polyphony,
            self.average_velocity * 127.0,
            self.velocity_range.0 * 127.0,
            self.velocity_range.1 * 127.0,
            self.average_duration * 1000.0,
            self.shortest_note * 1000.0,
            self.longest_note * 1000.0,
            self.pitch_bend_activity * 100.0
        )
    }
} 