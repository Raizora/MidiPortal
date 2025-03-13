/**
 * @file SettingsManager.h
 * @brief Defines the SettingsManager class.
 * 
 * This file defines the SettingsManager class, which manages application settings.
 */

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <memory>
#include <map>
#include <string>
#include "DisplaySettingsManager.h"

namespace MidiPortal {

/**
 * @class SettingsManager
 * @brief Manages application settings.
 * 
 * This class manages application settings, including audio device settings
 * and display settings.
 */
class SettingsManager
{
public:
    /**
     * @brief Default constructor.
     * 
     * Initializes the SettingsManager with default settings.
     */
    SettingsManager()
    {
        // Initialize the audio device manager
        deviceManager.initialiseWithDefaultDevices(0, 0);
    }
    
    /**
     * @brief Destructor.
     * 
     * Cleans up resources used by the SettingsManager.
     */
    ~SettingsManager() = default;
    
    /**
     * @brief Gets the audio device manager.
     * @return Reference to the audio device manager.
     * 
     * Gets the audio device manager used by the application.
     */
    juce::AudioDeviceManager& getAudioDeviceManager()
    {
        return deviceManager;
    }
    
    /**
     * @brief Gets the display settings manager.
     * @return Reference to the display settings manager.
     * 
     * Gets the display settings manager used by the application.
     */
    DisplaySettingsManager& getDisplaySettingsManager()
    {
        return displaySettingsManager;
    }
    
private:
    /**
     * @brief The audio device manager.
     * 
     * Manages audio devices used by the application.
     */
    juce::AudioDeviceManager deviceManager;
    
    /**
     * @brief The display settings manager.
     * 
     * Manages display settings for the application.
     */
    DisplaySettingsManager displaySettingsManager;
};

} // namespace MidiPortal 