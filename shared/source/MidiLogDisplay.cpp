/**
 * @file MidiLogDisplay.cpp
 * @brief Implementation of the MidiLogDisplay class.
 * 
 * This file contains the implementation of the MidiLogDisplay class methods,
 * which display a scrolling log of MIDI messages with visual formatting.
 */

#include "MidiLogDisplay.h"

namespace MidiPortal {

/**
 * @brief Constructor that takes a reference to the DisplaySettingsManager.
 * @param manager Reference to the DisplaySettingsManager that will be used for display settings.
 * 
 * Initializes the MidiLogDisplay with a reference to the DisplaySettingsManager,
 * sets up default visual properties, and starts the animation timer.
 * Also registers with the DisplaySettingsManager to receive settings updates.
 */
MidiLogDisplay::MidiLogDisplay(DisplaySettingsManager& manager)
    : settingsManager(manager)
{
    // Initialize default settings
    settingsManager.addSettings("ALL", DisplaySettingsManager::DisplaySettings());
    setSize(800, 600);
    
    // X- Set animation frame rate (30fps)
    setFramesPerSecond(30);
    
    // X- Set background color
    setOpaque(true);
    
    // X- Register with settings manager
    settingsManager.registerDisplay(this);
    settingsManager.addChangeListener(this);
}

/**
 * @brief Destructor that cleans up resources and unregisters from the DisplaySettingsManager.
 * 
 * Stops the timer and unregisters from the DisplaySettingsManager to prevent
 * memory leaks and dangling references.
 */
MidiLogDisplay::~MidiLogDisplay()
{
    // X- Unregister from settings manager
    settingsManager.unregisterDisplay(this);
    settingsManager.removeChangeListener(this);
}

/**
 * @brief Paints the component with the current MIDI message log.
 * @param g The Graphics context to paint into.
 * 
 * Draws all visible MIDI messages with their appropriate colors and opacity,
 * based on the current display settings. Messages can be drawn either from bottom to top
 * (standard) or from top to bottom (reverse direction) based on settings.
 */
void MidiLogDisplay::paint(juce::Graphics& g)
{
    // Get the correct settings for background based on window name
    juce::Colour backgroundColor;
    bool reverseDirection = false;
    
    // First check if ALL settings exist and have override enabled
    const auto& allSettings = settingsManager.getSettings("ALL");
    if (allSettings.overrideAllDevices) {
        // Use ALL settings if override is enabled
        backgroundColor = allSettings.backgroundColor;
        reverseDirection = allSettings.reverseDirection;
    } else {
        // For background color, use window-specific settings
        if (windowName.isEmpty() || windowName == "MAIN") {
            const auto& defaultSettings = settingsManager.getSettings("Default");
            backgroundColor = defaultSettings.backgroundColor;
        } else {
            const auto& windowSettings = settingsManager.getSettings(windowName);
            backgroundColor = windowSettings.backgroundColor;
        }
        
        // For reverse direction, use device-specific settings
        // If we have messages, use the settings from the first message's device
        if (!messages.empty()) {
            const auto& deviceSettings = settingsManager.getSettings(messages.front().deviceName);
            reverseDirection = deviceSettings.reverseDirection;
        }
    }
    
    // Fill the background with the correct color
    g.fillAll(backgroundColor);
    
    // Collect visible messages with their y-positions
    std::vector<std::pair<LogEntry*, float>> visibleEntries;
    
    if (!reverseDirection) {
        // STANDARD DIRECTION: Bottom-to-top (newest at bottom)
        float y = getHeight() - 10.0f;  // Start from bottom
        
        // First, collect all visible messages with their y-positions
        for (auto& msg : messages)
        {
            // Get settings for this specific device
            const auto& settings = settingsManager.getSettings(msg.deviceName);
            float messageHeight = juce::Font(settings.fontSize).getHeight();
            
            y -= messageHeight;
            
            // If we've reached the top of the screen, stop adding entries
            if (y < 0)
                break;
                
            visibleEntries.push_back(std::make_pair(&msg, y));
        }
    } else {
        // REVERSE DIRECTION: Top-to-bottom (newest at top)
        float y = 10.0f;  // Start from top
        
        // First, collect all visible messages with their y-positions
        for (auto& msg : messages)
        {
            // Get settings for this specific device
            const auto& settings = settingsManager.getSettings(msg.deviceName);
            float messageHeight = juce::Font(settings.fontSize).getHeight();
            
            // If we've reached the bottom of the screen, stop adding entries
            if (y + messageHeight > getHeight())
                break;
                
            visibleEntries.push_back(std::make_pair(&msg, y));
            y += messageHeight;
        }
    }
    
    // Now draw the visible messages
    for (const auto& entry : visibleEntries)
    {
        LogEntry* msg = entry.first;
        float yPos = entry.second;
        
        // Get settings for this message's device
        const auto& settings = settingsManager.getSettings(msg->deviceName);
        g.setFont(settings.fontSize);
        
        // Apply opacity to the color
        juce::Colour colorWithOpacity = msg->color.withAlpha(msg->opacity);
        g.setColour(colorWithOpacity);
        
        float messageHeight = g.getCurrentFont().getHeight();
        
        g.drawText(msg->text, 10.0f, yPos, getWidth() - 20.0f, messageHeight, 
                  juce::Justification::left, true);
    }
}

/**
 * @brief Handles component resizing.
 * 
 * Currently empty as the layout is handled in the paint method.
 */
void MidiLogDisplay::resized()
{
    // Nothing needed here as we handle layout in paint()
}

/**
 * @brief Animation update callback that updates message animations.
 * 
 * Called regularly by the AnimatedAppComponent to update message opacities
 * and scroll position, creating a fading effect for older messages.
 */
void MidiLogDisplay::update()
{
    // Update message opacities and remove fully faded messages
    for (auto it = messages.begin(); it != messages.end();)
    {
        // Each message carries its own fade settings from when it was created
        // If it was created with fading enabled, it will always fade
        if (it->shouldFade) {
            // Calculate fade amount - smaller fadeRate = faster fade
            // 0.01 = very fast fade, 1.0 = very slow fade
            float fadeAmount = 0.33f * std::exp(-6.5f * it->fadeRate);
            
            it->opacity -= fadeAmount;
            
            // If opacity reaches 0, message is gone forever
            if (it->opacity <= 0.0f) {
                // Remove from logEntries too to ensure it's completely gone
                for (int i = 0; i < logEntries.size(); ++i) {
                    if (logEntries[i].uniqueId == it->uniqueId) {
                        logEntries.remove(i);
                        break;
                    }
                }
                
                it = messages.erase(it);
                continue;
            }
        }
        
        ++it;
    }
    
    // Update scroll position
    if (yOffset > 0.0f)
    {
        yOffset = std::max(0.0f, yOffset - scrollSpeed);
    }
    
    // No need to call repaint() - AnimatedAppComponent does this automatically
}

/**
 * @brief Handles change notifications from the DisplaySettingsManager.
 * @param source The ChangeBroadcaster that triggered the notification.
 * 
 * Called when display settings change, triggering a repaint with the new settings.
 * Also applies fade settings to existing messages when fade rate is enabled.
 */
void MidiLogDisplay::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &settingsManager)
    {
        // Get the ALL settings to check fade rate state
        const auto& allSettings = settingsManager.getSettings("ALL");
        
        // If fade rate is enabled, apply it to all existing messages
        if (allSettings.fadeRateEnabled) {
            for (auto& msg : messages) {
                // Apply the new fade settings to existing messages
                msg.shouldFade = true;
                msg.fadeRate = allSettings.fadeRate;
            }
        }
        
        // Settings have changed, repaint with new settings
        repaint();
    }
}

/**
 * @brief Adds a new MIDI message to the display.
 * @param message The MIDI message to add.
 * @param deviceName The name of the device that sent the message.
 * 
 * Formats the message as text, assigns an appropriate color based on the
 * message type and device settings, and adds it to both the animated messages
 * queue and the persistent log entries array.
 * Respects mute settings for each message type.
 */
void MidiLogDisplay::addMessage(const juce::MidiMessage& message, const juce::String& deviceName)
{
    // Get settings for this device
    const auto& settings = settingsManager.getSettings(deviceName);

    // Check mute flags based on message type
    if ((message.isNoteOn() && settings.muteNoteOn) ||
        (message.isNoteOff() && settings.muteNoteOff) ||
        (message.isController() && settings.muteController) ||
        (message.isPitchWheel() && settings.mutePitchBend) ||
        ((message.isChannelPressure() || message.isAftertouch()) && settings.mutePressure) ||
        (message.isProgramChange() && settings.muteProgramChange) ||
        ((message.isMidiClock() || message.isMidiStart() || message.isMidiStop() || message.isMidiContinue()) && settings.muteClock) ||
        (message.isSysEx() && settings.muteSysEx))
    {
        return; // Skip muted message types
    }

    auto text = formatMidiMessage(message, deviceName);
    auto color = getColorForMessage(message, deviceName);
    
    // Use a unique ID to link entries in logEntries with messages in the deque
    juce::String uniqueId = juce::String(juce::Time::getMillisecondCounter()) + "_" + deviceName;
    
    // Create a log entry with the unique ID
    LogEntryData entry(text, color, deviceName);
    entry.uniqueId = uniqueId;
    logEntries.add(entry);
    
    // Create a message with the same unique ID and full opacity
    LogEntry msg(text, color, juce::Time::getCurrentTime(), deviceName);
    msg.uniqueId = uniqueId;
    
    // Store current fade settings with the message at creation time
    // These will be used regardless of later settings changes
    msg.shouldFade = settings.fadeRateEnabled;
    msg.fadeRate = settings.fadeRate;
    
    // Add to front of deque so newest messages are drawn first
    messages.push_front(msg);
    
    // Fixed queue size - just make sure it's large enough for display
    maxMessages = 1000; // Plenty for any display
    
    // Remove oldest messages if we exceed the max queue size
    while (messages.size() > maxMessages)
        messages.pop_back();
    
    repaint();
}

/**
 * @brief Clears all messages from the display.
 * 
 * Removes all messages from both the animated messages queue and
 * the persistent log entries array, and triggers a repaint.
 */
void MidiLogDisplay::clear()
{
    messages.clear();
    logEntries.clear();
    repaint();
}

/**
 * @brief Sets the maximum number of messages to display.
 * @param max The maximum number of messages to keep in the display.
 * 
 * Limits the number of messages stored in memory to prevent excessive
 * memory usage. Older messages are removed when the limit is reached.
 */
void MidiLogDisplay::setMaxMessages(size_t max)
{
    maxMessages = max;
    while (messages.size() > maxMessages)
        messages.pop_back();
}

/**
 * @brief Called when display settings change for a specific device.
 * @param deviceName The name of the device whose settings changed.
 * 
 * Triggers a repaint with the new settings for the specified device.
 * This method is currently not used directly, as changes are handled
 * through the ChangeListener interface.
 */
void MidiLogDisplay::settingsChanged(const juce::String& /*deviceName*/)
{
    // Settings have changed, repaint with new settings
    repaint();
}

/**
 * @brief Formats a MIDI message as text.
 * @param message The MIDI message to format.
 * @param deviceName The name of the device that sent the message.
 * @return A formatted string representation of the MIDI message.
 * 
 * Creates a human-readable text representation of a MIDI message,
 * including the device name and relevant message parameters.
 */
juce::String MidiLogDisplay::formatMidiMessage(const juce::MidiMessage& message, const juce::String& deviceName)
{
    juce::String text = deviceName + ": ";
    
    if (message.isNoteOn())
        text += "Note On: " + juce::String(message.getNoteNumber()) + " Vel: " + juce::String(message.getVelocity());
    else if (message.isNoteOff())
        text += "Note Off: " + juce::String(message.getNoteNumber());
    else if (message.isPitchWheel())
        text += "Pitch Bend: " + juce::String(message.getPitchWheelValue());
    else if (message.isController())
        text += "CC: " + juce::String(message.getControllerNumber()) + " Val: " + juce::String(message.getControllerValue());
    else if (message.isChannelPressure())
        text += "Channel Pressure: " + juce::String(message.getChannelPressureValue());
    else if (message.isAftertouch())
        text += "Aftertouch: " + juce::String(message.getAfterTouchValue());
    else if (message.isProgramChange())
        text += "Program Change: " + juce::String(message.getProgramChangeNumber());
    else if (message.isMidiClock())
        text += "MIDI Clock";
    else if (message.isMidiStart())
        text += "MIDI Start";
    else if (message.isMidiStop())
        text += "MIDI Stop";
    else if (message.isMidiContinue())
        text += "MIDI Continue";
    else if (message.isSysEx())
        text += "SysEx: " + juce::String(message.getSysExDataSize()) + " bytes";
    else
        text += "Other MIDI Event";
    
    return text;
}

/**
 * @brief Gets the appropriate color for a MIDI message based on its type.
 * @param message The MIDI message to get the color for.
 * @param deviceName The name of the device that sent the message.
 * @return The color to display the message in.
 * 
 * Determines the appropriate color for a MIDI message based on its type
 * and the current display settings for the specified device.
 */
juce::Colour MidiLogDisplay::getColorForMessage(const juce::MidiMessage& message, const juce::String& deviceName)
{
    // X- Get settings for this device (override will be handled by DisplaySettingsManager::getSettings)
    const auto& settings = settingsManager.getSettings(deviceName);
    
    if (message.isNoteOn())
        return settings.noteOnColor;
    else if (message.isNoteOff())
        return settings.noteOffColor;
    else if (message.isPitchWheel())
        return settings.pitchBendColor;
    else if (message.isController())
        return settings.controllerColor;
    else if (message.isChannelPressure() || message.isAftertouch())
        return settings.pressureColor;
    else if (message.isProgramChange())
        return settings.programChangeColor;
    else if (message.isMidiClock() || message.isMidiStart() || message.isMidiStop() || message.isMidiContinue())
        return settings.clockColor;
    else if (message.isSysEx())
        return settings.sysExColor;
    else
        return settings.defaultColor;
}

} // namespace MidiPortal 