/**
 * @file MidiAIManager.cpp
 * @brief Implementation of the MidiAIManager class.
 * 
 * This file contains the implementation of the MidiAIManager class methods,
 * which manage AI/ML models for MIDI processing.
 */

#include "MidiAIManager.h"

namespace MidiPortal {

/**
 * @brief Default constructor.
 * 
 * Initializes the MidiAIManager with default settings.
 */
MidiAIManager::MidiAIManager()
    : mlContext(nullptr),
      lastInsightTime(0)
{
    // Initialize the ML context
    mlContext = create_ml_context();
    
    // Start the timer to generate insights periodically
    startTimer(1000); // Generate insights every second
    
    // Load default models
    loadModel("models/pattern_recognition.model");
}

/**
 * @brief Destructor.
 * 
 * Cleans up resources used by the MidiAIManager.
 */
MidiAIManager::~MidiAIManager()
{
    // Stop the timer
    stopTimer();
    
    // Clean up the ML context
    if (mlContext != nullptr)
    {
        destroy_ml_context(mlContext);
        mlContext = nullptr;
    }
}

/**
 * @brief Processes a MIDI message.
 * @param message The MIDI message to process.
 * @param deviceName The name of the device that sent the message.
 * 
 * Processes a MIDI message, updating the AI models with the new data.
 */
void MidiAIManager::processMidiMessage(const juce::MidiMessage& message, const juce::String& deviceName)
{
    // Process the message with the ML context
    if (mlContext != nullptr)
    {
        // Convert the MIDI message to a format that the Rust code can understand
        uint8_t buffer[3];
        int size = 0;
        
        if (message.getRawData() != nullptr)
        {
            size = message.getRawDataSize();
            if (size > 3) size = 3; // Limit to 3 bytes for now
            
            // Copy the raw data
            for (int i = 0; i < size; ++i)
            {
                buffer[i] = message.getRawData()[i];
            }
            
            // Process the message
            process_midi_message_ml(mlContext, buffer, size, deviceName.toRawUTF8());
        }
    }
}

/**
 * @brief Gets insights from the AI models.
 * @return A vector of insights.
 * 
 * Gets insights from the AI models based on the processed MIDI data.
 */
std::vector<MidiAIManager::AIInsight> MidiAIManager::getInsights()
{
    // Return a copy of the insights
    return insights;
}

/**
 * @brief Loads a model from a file.
 * @param filePath The path to the model file.
 * @return true if the model was loaded successfully, false otherwise.
 * 
 * Loads a model from a file, making it available for use.
 */
bool MidiAIManager::loadModel(const juce::String& filePath)
{
    // Load the model
    if (mlContext != nullptr)
    {
        int modelId = load_model_ml(mlContext, filePath.toRawUTF8());
        
        if (modelId >= 0)
        {
            // Store the model ID and name
            loadedModels[modelId] = filePath;
            return true;
        }
    }
    
    return false;
}

/**
 * @brief Unloads a model.
 * @param modelId The ID of the model to unload.
 * @return true if the model was unloaded successfully, false otherwise.
 * 
 * Unloads a model, freeing up resources.
 */
bool MidiAIManager::unloadModel(int modelId)
{
    // Unload the model
    if (mlContext != nullptr)
    {
        if (unload_model(mlContext, modelId))
        {
            // Remove the model from the map
            loadedModels.erase(modelId);
            return true;
        }
    }
    
    return false;
}

/**
 * @brief Gets the IDs of all loaded models.
 * @return A vector of model IDs.
 * 
 * Gets the IDs of all loaded models.
 */
std::vector<int> MidiAIManager::getLoadedModelIds() const
{
    // Get the IDs of all loaded models
    std::vector<int> ids;
    
    for (const auto& pair : loadedModels)
    {
        ids.push_back(pair.first);
    }
    
    return ids;
}

/**
 * @brief Gets the name of a model.
 * @param modelId The ID of the model.
 * @return The name of the model.
 * 
 * Gets the name of a model.
 */
juce::String MidiAIManager::getModelName(int modelId) const
{
    // Get the name of a model
    auto it = loadedModels.find(modelId);
    
    if (it != loadedModels.end())
    {
        return it->second;
    }
    
    return {};
}

/**
 * @brief Gets the description of a model.
 * @param modelId The ID of the model.
 * @return The description of the model.
 * 
 * Gets the description of a model.
 */
juce::String MidiAIManager::getModelDescription(int modelId) const
{
    // Get the description of a model
    if (mlContext != nullptr)
    {
        const char* description = get_model_description(mlContext, modelId);
        
        if (description != nullptr)
        {
            return juce::String(description);
        }
    }
    
    return {};
}

/**
 * @brief Gets the version of a model.
 * @param modelId The ID of the model.
 * @return The version of the model.
 * 
 * Gets the version of a model.
 */
juce::String MidiAIManager::getModelVersion(int modelId) const
{
    // Get the version of a model
    if (mlContext != nullptr)
    {
        const char* version = get_model_version(mlContext, modelId);
        
        if (version != nullptr)
        {
            return juce::String(version);
        }
    }
    
    return {};
}

/**
 * @brief Gets the author of a model.
 * @param modelId The ID of the model.
 * @return The author of the model.
 * 
 * Gets the author of a model.
 */
juce::String MidiAIManager::getModelAuthor(int modelId) const
{
    // Get the author of a model
    if (mlContext != nullptr)
    {
        const char* author = get_model_author(mlContext, modelId);
        
        if (author != nullptr)
        {
            return juce::String(author);
        }
    }
    
    return {};
}

/**
 * @brief Gets the license of a model.
 * @param modelId The ID of the model.
 * @return The license of the model.
 * 
 * Gets the license of a model.
 */
juce::String MidiAIManager::getModelLicense(int modelId) const
{
    // Get the license of a model
    if (mlContext != nullptr)
    {
        const char* license = get_model_license(mlContext, modelId);
        
        if (license != nullptr)
        {
            return juce::String(license);
        }
    }
    
    return {};
}

/**
 * @brief Timer callback.
 * 
 * Called periodically to generate insights.
 */
void MidiAIManager::timerCallback()
{
    // Generate insights
    if (mlContext != nullptr)
    {
        // Get the current time
        juce::int64 currentTime = juce::Time::currentTimeMillis();
        
        // Only generate insights if enough time has passed
        if (currentTime - lastInsightTime > 1000)
        {
            // Update the last insight time
            lastInsightTime = currentTime;
            
            // Clear the insights
            insights.clear();
            
            // Generate insights for each model
            for (const auto& pair : loadedModels)
            {
                int modelId = pair.first;
                
                // Get the number of insights
                int numInsights = get_num_insights(mlContext, modelId);
                
                // Get each insight
                for (int i = 0; i < numInsights; ++i)
                {
                    // Get the insight
                    const char* description = get_insight_description(mlContext, modelId, i);
                    float score = get_insight_score(mlContext, modelId, i);
                    int type = get_insight_type(mlContext, modelId, i);
                    
                    // Create an insight
                    AIInsight insight;
                    insight.description = juce::String(description);
                    insight.score = score;
                    
                    // Convert the type
                    switch (type)
                    {
                        case 0:
                            insight.type = AIInsight::Type::Pattern;
                            break;
                        case 1:
                            insight.type = AIInsight::Type::Performance;
                            break;
                        case 2:
                            insight.type = AIInsight::Type::Style;
                            break;
                        default:
                            insight.type = AIInsight::Type::Pattern;
                            break;
                    }
                    
                    // Add the insight
                    insights.push_back(insight);
                }
            }
        }
    }
}

} // namespace MidiPortal 