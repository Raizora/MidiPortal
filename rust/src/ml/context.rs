/**
 * @file context.rs
 * @brief Defines the musical context for ML models.
 * 
 * This file defines the musical context for ML models, including
 * MIDI message types and the musical context struct.
 */

use std::collections::{VecDeque, HashMap};
use crate::shared_buffer::MidiEvent;

/// MIDI message types
#[derive(Debug, Clone)]
pub enum MidiMessage {
    /// Note on message
    NoteOn {
        /// MIDI channel (0-15)
        channel: u8,
        /// MIDI note number (0-127)
        note: u8,
        /// Velocity (0-127)
        velocity: u8,
    },
    /// Note off message
    NoteOff {
        /// MIDI channel (0-15)
        channel: u8,
        /// MIDI note number (0-127)
        note: u8,
        /// Velocity (0-127)
        velocity: u8,
    },
    /// Polyphonic aftertouch message
    PolyphonicAftertouch {
        /// MIDI channel (0-15)
        channel: u8,
        /// MIDI note number (0-127)
        note: u8,
        /// Pressure (0-127)
        pressure: u8,
    },
    /// Control change message
    ControlChange {
        /// MIDI channel (0-15)
        channel: u8,
        /// Controller number (0-127)
        controller: u8,
        /// Controller value (0-127)
        value: u8,
    },
    /// Program change message
    ProgramChange {
        /// MIDI channel (0-15)
        channel: u8,
        /// Program number (0-127)
        program: u8,
    },
    /// Channel aftertouch message
    ChannelAftertouch {
        /// MIDI channel (0-15)
        channel: u8,
        /// Pressure (0-127)
        pressure: u8,
    },
    /// Pitch bend message
    PitchBend {
        /// MIDI channel (0-15)
        channel: u8,
        /// Pitch bend value (0-16383)
        value: u16,
    },
    /// Other MIDI message
    Other,
}

/// Represents the type of a MIDI message
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum MidiMessageType {
    NoteOn,
    NoteOff,
    ControlChange,
    PitchBend,
    ProgramChange,
    Aftertouch,
    PolyAftertouch,
    Clock,
    Start,
    Stop,
    Continue,
    SysEx,
    Other,
}

/// Represents a musical note with timing information
#[derive(Debug, Clone)]
pub struct Note {
    /// MIDI note number (0-127)
    pub note: u8,
    /// MIDI velocity (0-127)
    pub velocity: u8,
    /// Channel (0-15)
    pub channel: u8,
    /// Start time in microseconds
    pub start_time: u64,
    /// End time in microseconds (None if note is still active)
    pub end_time: Option<u64>,
    /// Duration in microseconds (None if note is still active)
    pub duration: Option<u64>,
}

/// Musical context for ML models
pub struct MusicalContext {
    /// Recent MIDI messages
    messages: VecDeque<MidiMessage>,
    /// Maximum number of messages to store
    max_messages: usize,
    /// Active notes (note number -> velocity)
    active_notes: [Option<u8>; 128],
    /// Current tempo (beats per minute)
    tempo: f32,
    /// Current time signature (numerator, denominator)
    time_signature: (u8, u8),
    /// Current key signature (0 = C, 1 = C#, etc.)
    key_signature: u8,
}

impl MusicalContext {
    /// Creates a new musical context
    pub fn new() -> Self {
        Self {
            messages: VecDeque::new(),
            max_messages: 1000,
            active_notes: [None; 128],
            tempo: 120.0,
            time_signature: (4, 4),
            key_signature: 0,
        }
    }
    
    /// Updates the context with a new MIDI message
    pub fn update(&mut self, message: MidiMessage) {
        // Add the message to the queue
        self.messages.push_back(message.clone());
        
        // Remove old messages if the queue is too large
        while self.messages.len() > self.max_messages {
            self.messages.pop_front();
        }
        
        // Update the active notes
        match message {
            MidiMessage::NoteOn { note, velocity, .. } => {
                if velocity > 0 {
                    self.active_notes[note as usize] = Some(velocity);
                } else {
                    self.active_notes[note as usize] = None;
                }
            }
            MidiMessage::NoteOff { note, .. } => {
                self.active_notes[note as usize] = None;
            }
            _ => {}
        }
    }
    
    /// Gets the active notes
    pub fn active_notes(&self) -> &[Option<u8>; 128] {
        &self.active_notes
    }
    
    /// Gets the recent messages
    pub fn messages(&self) -> &VecDeque<MidiMessage> {
        &self.messages
    }
    
    /// Gets the current tempo
    pub fn tempo(&self) -> f32 {
        self.tempo
    }
    
    /// Sets the current tempo
    pub fn set_tempo(&mut self, tempo: f32) {
        self.tempo = tempo;
    }
    
    /// Gets the current time signature
    pub fn time_signature(&self) -> (u8, u8) {
        self.time_signature
    }
    
    /// Sets the current time signature
    pub fn set_time_signature(&mut self, numerator: u8, denominator: u8) {
        self.time_signature = (numerator, denominator);
    }
    
    /// Gets the current key signature
    pub fn key_signature(&self) -> u8 {
        self.key_signature
    }
    
    /// Sets the current key signature
    pub fn set_key_signature(&mut self, key: u8) {
        self.key_signature = key;
    }
    
    /// Determines the type of a MIDI event
    pub fn get_message_type(event: &MidiEvent) -> MidiMessageType {
        // Check the status byte (first byte of MIDI message)
        if event.data.is_empty() {
            return MidiMessageType::Other;
        }
        
        let status = event.data[0];
        
        // Extract the high nibble (message type)
        match status & 0xF0 {
            0x80 => MidiMessageType::NoteOff,
            0x90 => {
                // Note On with velocity 0 is treated as Note Off
                if event.data.len() > 2 && event.data[2] == 0 {
                    MidiMessageType::NoteOff
                } else {
                    MidiMessageType::NoteOn
                }
            },
            0xA0 => MidiMessageType::PolyAftertouch,
            0xB0 => MidiMessageType::ControlChange,
            0xC0 => MidiMessageType::ProgramChange,
            0xD0 => MidiMessageType::Aftertouch,
            0xE0 => MidiMessageType::PitchBend,
            0xF0 => {
                // System messages
                match status {
                    0xF0 => MidiMessageType::SysEx,
                    0xF8 => MidiMessageType::Clock,
                    0xFA => MidiMessageType::Start,
                    0xFB => MidiMessageType::Continue,
                    0xFC => MidiMessageType::Stop,
                    _ => MidiMessageType::Other,
                }
            },
            _ => MidiMessageType::Other,
        }
    }
}

/// Represents a detected pattern in MIDI data
#[derive(Debug, Clone)]
pub struct Pattern {
    /// Unique identifier for the pattern
    pub id: u64,
    /// Sequence of MIDI events that make up the pattern
    pub events: Vec<MidiEvent>,
    /// Number of times this pattern has been observed
    pub occurrence_count: u32,
    /// Significance score (0.0 - 1.0)
    pub significance_score: f64,
    /// Pattern type (e.g., "chord progression", "rhythm", "melody")
    pub pattern_type: String,
}

impl Pattern {
    /// Creates a new pattern from a sequence of MIDI events
    pub fn new(events: Vec<MidiEvent>) -> Self {
        let id = Self::calculate_hash(&events);
        Self {
            id,
            events,
            occurrence_count: 1,
            significance_score: 0.0,
            pattern_type: "unknown".to_string(),
        }
    }

    /// Calculates a hash for the pattern based on its events
    fn calculate_hash(events: &[MidiEvent]) -> u64 {
        use std::hash::{Hash, Hasher};
        use std::collections::hash_map::DefaultHasher;
        
        let mut hasher = DefaultHasher::new();
        for event in events {
            event.data.hash(&mut hasher);
        }
        hasher.finish()
    }

    /// Gets the hash of this pattern
    pub fn hash(&self) -> u64 {
        self.id
    }
}

/// Represents an insight generated from MIDI analysis
#[derive(Debug, Clone)]
pub enum Insight {
    /// A detected pattern
    Pattern(Pattern),
    /// Performance analysis
    Performance {
        /// Description of the insight
        description: String,
        /// Score (0.0 - 1.0)
        score: f64,
        /// Suggestions for improvement
        suggestions: Vec<String>,
    },
    /// Style classification
    Style {
        /// Detected style
        style: String,
        /// Confidence (0.0 - 1.0)
        confidence: f64,
    },
}

/// Error type for model operations
#[derive(Debug, thiserror::Error)]
pub enum ModelError {
    #[error("Model not found")]
    ModelNotFound,
    #[error("Failed to load model: {0}")]
    LoadFailed(String),
    #[error("Invalid input: {0}")]
    InvalidInput(String),
}

/// Trait for MIDI models
pub trait MidiModel {
    /// Processes a MIDI event
    fn process_event(&mut self, event: &MidiEvent, context: &MusicalContext);
    
    /// Generates insights based on the current context
    fn generate_insights(&self, context: &MusicalContext) -> Vec<Insight>;
}

/// The main model context that manages MIDI data and models
pub struct ModelContext {
    /// Recent MIDI events
    pub recent_events: VecDeque<MidiEvent>,
    /// Detected patterns
    pub patterns: Vec<Pattern>,
    /// Current musical context
    pub musical_context: MusicalContext,
    /// Active model
    pub model: Option<Box<dyn MidiModel>>,
}

impl ModelContext {
    /// Creates a new model context
    pub fn new() -> Self {
        Self {
            recent_events: VecDeque::with_capacity(1000),
            patterns: Vec::new(),
            musical_context: MusicalContext::new(),
            model: None,
        }
    }
    
    /// Adds a MIDI event to the context
    pub fn add_event(&mut self, event: MidiEvent) {
        // Add to recent events
        self.recent_events.push_back(event.clone());
        if self.recent_events.len() > 1000 {
            self.recent_events.pop_front();
        }
        
        // Update musical context
        self.musical_context.update(MidiMessage::NoteOn {
            channel: event.data[0] & 0x0F,
            note: event.data[1],
            velocity: event.data[2],
        });
        
        // Process with model if available
        if let Some(model) = &mut self.model {
            model.process_event(&event, &self.musical_context);
        }
    }
    
    /// Generates insights from the current context
    pub fn generate_insights(&self) -> Vec<Insight> {
        let mut insights = Vec::new();
        
        // Add pattern-based insights
        for pattern in &self.patterns {
            if pattern.significance_score > 0.5 {
                insights.push(Insight::Pattern(pattern.clone()));
            }
        }
        
        // Add model-based insights if available
        if let Some(model) = &self.model {
            insights.extend(model.generate_insights(&self.musical_context));
        }
        
        insights
    }
} 