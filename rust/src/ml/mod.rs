/**
 * @file mod.rs
 * @brief Exports the ML modules.
 * 
 * This file exports the ML modules, including the context module.
 */

pub mod context;
pub mod pattern;

use std::collections::HashMap;
use crate::shared_buffer::MidiEvent;
use self::context::{ModelContext, MidiModel, Insight, ModelError};
use self::pattern::PatternRecognitionModel;

/// Available model types
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum ModelType {
    /// Pattern recognition model
    PatternRecognition,
    /// Style classification model
    StyleClassification,
    /// Performance analysis model
    PerformanceAnalysis,
}

/// The main model context protocol that manages models and insights
pub struct ModelContextProtocol {
    /// Current context
    context: ModelContext,
    /// Available models
    models: HashMap<String, Box<dyn MidiModel>>,
    /// Active model
    active_model: Option<String>,
}

impl ModelContextProtocol {
    /// Creates a new model context protocol
    pub fn new() -> Self {
        Self {
            context: ModelContext::new(),
            models: HashMap::new(),
            active_model: None,
        }
    }
    
    /// Registers a model with the protocol
    pub fn register_model(&mut self, name: &str, model: Box<dyn MidiModel>) {
        self.models.insert(name.to_string(), model);
    }
    
    /// Activates a model by name
    pub fn activate_model(&mut self, name: &str) -> Result<(), ModelError> {
        if self.models.contains_key(name) {
            self.active_model = Some(name.to_string());
            Ok(())
        } else {
            Err(ModelError::ModelNotFound)
        }
    }
    
    /// Loads a model by type
    pub fn load_model(&mut self, model_type: ModelType) -> Result<(), ModelError> {
        match model_type {
            ModelType::PatternRecognition => {
                let model = PatternRecognitionModel::new();
                self.register_model("pattern_recognition", Box::new(model));
                self.activate_model("pattern_recognition")
            },
            ModelType::StyleClassification => {
                // Not implemented yet
                Err(ModelError::LoadFailed("Style classification not implemented yet".to_string()))
            },
            ModelType::PerformanceAnalysis => {
                // Not implemented yet
                Err(ModelError::LoadFailed("Performance analysis not implemented yet".to_string()))
            },
        }
    }
    
    /// Processes a MIDI event
    pub fn process_event(&mut self, event: MidiEvent) {
        // Update context with new event
        self.context.add_event(event);
    }
    
    /// Generates insights from the current context
    pub fn generate_insights(&self) -> Vec<Insight> {
        self.context.generate_insights()
    }
} 