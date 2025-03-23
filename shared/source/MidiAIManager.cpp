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
      lastInsightTime(0),
      midiEventWritten(false)
{
    // Initialize the ML context
    mlContext = create_ml_context();
    
    // Start the timer to generate insights periodically
    startTimer(1000); // Generate insights every second
    
    // Load default models
    loadModel("models/pattern_recognition.model");
    
    // X- Set default file path in user documents folder
    juce::File appDataDir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                               .getChildFile("MidiPortal");
    
    // Ensure directory exists
    appDataDir.createDirectory();
    
    // Set the file path
    setMidiDataFilePath(appDataDir.getChildFile("MidiData.json").getFullPathName());
    
    // Open the file
    openMidiDataFile();
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
    
    // Close the MIDI data file
    if (midiDataFile != nullptr)
    {
        // Write the JSON footer for MIDI events
        midiDataFile->writeText("\n  ],\n", false, false, nullptr);
        
        // Write the insights section if we have any
        midiDataFile->writeText("  \"insights\": [\n", false, false, nullptr);
        
        bool firstInsight = true;
        for (const auto& insight : insightsToRecord)
        {
            if (!firstInsight)
                midiDataFile->writeText(",\n", false, false, nullptr);
            else
                firstInsight = false;
                
            // Start insight JSON object
            midiDataFile->writeText("    {\n", false, false, nullptr);
            
            // Write insight type
            midiDataFile->writeText("      \"type\": \"", false, false, nullptr);
            switch (insight.type)
            {
                case AIInsight::Type::Pattern:
                    midiDataFile->writeText("Pattern", false, false, nullptr);
                    break;
                case AIInsight::Type::Performance:
                    midiDataFile->writeText("Performance", false, false, nullptr);
                    break;
                case AIInsight::Type::Style:
                    midiDataFile->writeText("Style", false, false, nullptr);
                    break;
            }
            midiDataFile->writeText("\",\n", false, false, nullptr);
            
            // Write description
            midiDataFile->writeText("      \"description\": \"", false, false, nullptr);
            midiDataFile->writeText(insight.description, false, false, nullptr);
            midiDataFile->writeText("\",\n", false, false, nullptr);
            
            // Write score
            midiDataFile->writeText("      \"score\": ", false, false, nullptr);
            midiDataFile->writeText(juce::String(insight.score), false, false, nullptr);
            midiDataFile->writeText("\n", false, false, nullptr);
            
            // Close insight JSON object
            midiDataFile->writeText("    }", false, false, nullptr);
        }
        
        // Close the insights section and the entire JSON document
        midiDataFile->writeText("\n  ]\n}", false, false, nullptr);
        midiDataFile->flush();
        midiDataFile.reset();
    }
    
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
    // Process with ML context as before
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
            
            // Process the message with the ML context
            process_midi_message_ml(mlContext, buffer, size, deviceName.toRawUTF8());
        }
    }
    
    // X- Record the MIDI data to file
    recordMidiData(message, deviceName);
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
                    
                    // X- Write insight to file
                    recordInsight(insight);
                }
            }
        }
    }
}

// New method to set file path
void MidiAIManager::setMidiDataFilePath(const juce::String& path)
{
    // Set the file path
    midiDataFilePath = path;
    
    // Close existing file if open
    midiDataFile.reset();
    
    // Open the new file
    openMidiDataFile();
}

// New method to get file path
juce::String MidiAIManager::getMidiDataFilePath() const
{
    return midiDataFilePath;
}

// New method to open/create the file
bool MidiAIManager::openMidiDataFile()
{
    // Create the file
    juce::File file(midiDataFilePath);
    
    // Create parent directory if it doesn't exist
    file.getParentDirectory().createDirectory();
    
    // Open the file for writing (overwrite existing file)
    midiDataFile = std::make_unique<juce::FileOutputStream>(file);
    
    // Check if the file was opened successfully
    if (midiDataFile == nullptr || !midiDataFile->openedOk())
    {
        DBG("Failed to open MIDI data file: " + midiDataFilePath);
        midiDataFile.reset();
        return false;
    }
    
    // Write the JSON header
    midiDataFile->writeText("{\n  \"midi_events\": [\n", false, false, nullptr);
    
    // Add flag to track if we've written any events
    midiEventWritten = false;
    
    return true;
}

// New method to record MIDI data
void MidiAIManager::recordMidiData(const juce::MidiMessage& message, const juce::String& deviceName)
{
    // Check if the file is open
    if (midiDataFile == nullptr || !midiDataFile->openedOk())
    {
        return;
    }
    
    // Format the MIDI data as JSON
    juce::String jsonEvent;
    
    // Add comma if this isn't the first event
    if (midiEventWritten)
    {
        jsonEvent = ",\n";
    }
    else
    {
        midiEventWritten = true;
    }
    
    // Get the current time
    juce::Time now = juce::Time::getCurrentTime();
    
    // Start the JSON object
    jsonEvent += "    {\n";
    
    // Add timestamp
    jsonEvent += "      \"timestamp\": \"" + now.formatted("%Y-%m-%d %H:%M:%S.%ms") + "\",\n";
    
    // Add device name
    jsonEvent += "      \"device\": \"" + deviceName + "\",\n";
    
    // Add message type
    if (message.isNoteOn())
        jsonEvent += "      \"type\": \"NoteOn\",\n";
    else if (message.isNoteOff())
        jsonEvent += "      \"type\": \"NoteOff\",\n";
    else if (message.isController())
        jsonEvent += "      \"type\": \"ControlChange\",\n";
    else if (message.isPitchWheel())
        jsonEvent += "      \"type\": \"PitchBend\",\n";
    else if (message.isProgramChange())
        jsonEvent += "      \"type\": \"ProgramChange\",\n";
    else if (message.isChannelPressure() || message.isAftertouch())
        jsonEvent += "      \"type\": \"Aftertouch\",\n";
    else if (message.isMidiClock())
        jsonEvent += "      \"type\": \"Clock\",\n";
    else if (message.isSysEx())
        jsonEvent += "      \"type\": \"SysEx\",\n";
    else
        jsonEvent += "      \"type\": \"Other\",\n";
    
    // Add channel
    jsonEvent += "      \"channel\": " + juce::String(message.getChannel()) + ",\n";
    
    // Add message-specific data
    if (message.isNoteOn() || message.isNoteOff())
    {
        jsonEvent += "      \"note\": " + juce::String(message.getNoteNumber()) + ",\n";
        jsonEvent += "      \"velocity\": " + juce::String(message.getVelocity()) + ",\n";
        
        // Add note name
        jsonEvent += "      \"noteName\": \"" + juce::MidiMessage::getMidiNoteName(message.getNoteNumber(), true, true, 4) + "\",\n";
    }
    else if (message.isController())
    {
        jsonEvent += "      \"controller\": " + juce::String(message.getControllerNumber()) + ",\n";
        jsonEvent += "      \"value\": " + juce::String(message.getControllerValue()) + ",\n";
    }
    else if (message.isPitchWheel())
    {
        jsonEvent += "      \"value\": " + juce::String(message.getPitchWheelValue()) + ",\n";
    }
    else if (message.isProgramChange())
    {
        jsonEvent += "      \"program\": " + juce::String(message.getProgramChangeNumber()) + ",\n";
    }
    else if (message.isChannelPressure())
    {
        jsonEvent += "      \"pressure\": " + juce::String(message.getChannelPressureValue()) + ",\n";
    }
    else if (message.isAftertouch())
    {
        jsonEvent += "      \"note\": " + juce::String(message.getNoteNumber()) + ",\n";
        jsonEvent += "      \"pressure\": " + juce::String(message.getAfterTouchValue()) + ",\n";
    }
    
    // Add raw data
    jsonEvent += "      \"rawData\": [";
    
    for (int i = 0; i < message.getRawDataSize(); ++i)
    {
        if (i > 0) jsonEvent += ", ";
        jsonEvent += juce::String(message.getRawData()[i]);
    }
    
    jsonEvent += "]\n";
    
    // Close the JSON object
    jsonEvent += "    }";
    
    // Write to file
    midiDataFile->writeText(jsonEvent, false, false, nullptr);
    
    // Flush to ensure data is written
    midiDataFile->flush();
}

// New method to record insights
void MidiAIManager::recordInsight(const AIInsight& insight)
{
    // Check if the file is open
    if (midiDataFile == nullptr || !midiDataFile->openedOk())
    {
        return;
    }
    
    // We won't include insights in the main MIDI event array
    // Instead, we'll add them to a separate section at the end of the file
    // This will happen when the file is closed in the destructor
    
    // For now, just store the insights
    insightsToRecord.push_back(insight);
}

} // namespace MidiPortal 