/**
 * @file pattern.rs
 * @brief Defines the pattern recognition model.
 * 
 * This file defines the pattern recognition model, which detects
 * patterns in MIDI data.
 */

use std::collections::{HashMap, VecDeque};
use crate::ml::context::{MidiModel, MusicalContext, Insight, Pattern, MidiMessageType, MidiMessage};
use crate::shared_buffer::MidiEvent;

/// A trie node for pattern matching
struct TrieNode {
    /// Children nodes
    children: HashMap<Vec<u8>, TrieNode>,
    /// Whether this node represents the end of a pattern
    is_pattern: bool,
    /// Pattern ID if this is the end of a pattern
    pattern_id: Option<u64>,
    /// Count of occurrences
    count: u32,
}

impl TrieNode {
    /// Creates a new trie node
    fn new() -> Self {
        Self {
            children: HashMap::new(),
            is_pattern: false,
            pattern_id: None,
            count: 0,
        }
    }
}

/// A trie for efficient pattern matching
pub struct PatternTrie {
    /// Root node
    root: TrieNode,
    /// Patterns by ID
    patterns: HashMap<u64, Pattern>,
}

impl PatternTrie {
    /// Creates a new pattern trie
    pub fn new() -> Self {
        Self {
            root: TrieNode::new(),
            patterns: HashMap::new(),
        }
    }
    
    /// Adds a sequence of events to the trie
    pub fn add_sequence(&mut self, events: &[MidiEvent]) {
        if events.is_empty() {
            return;
        }
        
        // Create a new pattern
        let pattern = Pattern::new(events.to_vec());
        let pattern_id = pattern.id;
        
        // Add to patterns map
        let existing = self.patterns.get_mut(&pattern_id);
        if let Some(existing) = existing {
            existing.occurrence_count += 1;
            // Update significance score based on occurrence count
            existing.significance_score = (existing.occurrence_count as f64).min(10.0) / 10.0;
            return;
        }
        
        // Add new pattern
        self.patterns.insert(pattern_id, pattern);
        
        // Add to trie
        let mut current = &mut self.root;
        for event in events {
            let key = event.data.clone();
            current = current.children.entry(key).or_insert_with(TrieNode::new);
        }
        
        current.is_pattern = true;
        current.pattern_id = Some(pattern_id);
        current.count += 1;
    }
    
    /// Finds patterns in a sequence of events
    pub fn find_patterns(&self, events: &[MidiEvent]) -> Vec<Pattern> {
        let mut result = Vec::new();
        
        // Try all possible subsequences
        for start in 0..events.len() {
            let mut current = &self.root;
            for i in start..events.len() {
                let key = &events[i].data;
                if let Some(next) = current.children.get(key) {
                    current = next;
                    if current.is_pattern && current.pattern_id.is_some() {
                        if let Some(pattern) = self.patterns.get(&current.pattern_id.unwrap()) {
                            result.push(pattern.clone());
                        }
                    }
                } else {
                    break;
                }
            }
        }
        
        result
    }
}

/// A pattern recognition model
pub struct PatternRecognitionModel {
    /// The minimum length of a pattern
    min_pattern_length: usize,
    /// The maximum length of a pattern
    max_pattern_length: usize,
    /// The detected patterns
    patterns: Vec<Pattern>,
    /// The recent notes
    recent_notes: VecDeque<MidiMessage>,
    /// Current sequence of events
    current_sequence: VecDeque<MidiEvent>,
    /// Pattern trie
    trie: PatternTrie,
}

impl PatternRecognitionModel {
    /// Creates a new pattern recognition model
    pub fn new() -> Self {
        Self {
            min_pattern_length: 3,
            max_pattern_length: 16,
            patterns: Vec::new(),
            recent_notes: VecDeque::new(),
            current_sequence: VecDeque::new(),
            trie: PatternTrie::new(),
        }
    }
    
    /// Updates the model with a new musical context
    pub fn update(&mut self, context: &MusicalContext) {
        // Get the recent messages
        let messages = context.messages();
        
        // Update the recent notes
        self.recent_notes.clear();
        for message in messages {
            match message {
                MidiMessage::NoteOn { .. } | MidiMessage::NoteOff { .. } => {
                    self.recent_notes.push_back(message.clone());
                }
                _ => {}
            }
        }
        
        // Detect patterns
        self.detect_patterns();
    }
    
    /// Detects patterns in the recent notes
    fn detect_patterns(&mut self) {
        // Clear the patterns
        self.patterns.clear();
        
        // For now, just add a dummy pattern
        let dummy_events = Vec::new();
        let dummy_pattern = Pattern::new(dummy_events);
        self.patterns.push(dummy_pattern);
    }
    
    /// Gets the detected patterns
    pub fn patterns(&self) -> &[Pattern] {
        &self.patterns
    }
    
    /// Sets the minimum pattern length
    pub fn set_min_pattern_length(&mut self, length: usize) {
        self.min_pattern_length = length;
    }
    
    /// Sets the maximum pattern length
    pub fn set_max_pattern_length(&mut self, length: usize) {
        self.max_pattern_length = length;
    }
}

impl MidiModel for PatternRecognitionModel {
    fn process_event(&mut self, event: &MidiEvent, _context: &MusicalContext) {
        // Only process note events for pattern recognition
        let message_type = MusicalContext::get_message_type(event);
        match message_type {
            MidiMessageType::NoteOn | MidiMessageType::NoteOff => {
                // Add to current sequence
                self.current_sequence.push_back(event.clone());
                
                // Maintain a reasonable window
                if self.current_sequence.len() > 100 {
                    self.current_sequence.pop_front();
                }
                
                // Detect patterns
                self.detect_patterns();
            },
            _ => {
                // Ignore other message types for pattern recognition
            }
        }
    }
    
    fn generate_insights(&self, _context: &MusicalContext) -> Vec<Insight> {
        let mut insights = Vec::new();
        
        // Find patterns in current sequence
        let events: Vec<_> = self.current_sequence.iter().cloned().collect();
        let patterns = self.trie.find_patterns(&events);
        
        // Add significant patterns as insights
        for pattern in patterns {
            if pattern.significance_score > 0.5 {
                insights.push(Insight::Pattern(pattern));
            }
        }
        
        insights
    }
} 