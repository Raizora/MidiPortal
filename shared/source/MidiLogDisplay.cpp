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
    
    // X- Start the timer to update animations (30fps)
    startTimerHz(30);
    
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
    // X- Stop the timer
    stopTimer();
    
    // X- Unregister from settings manager
    settingsManager.unregisterDisplay(this);
    settingsManager.removeChangeListener(this);
}

/**
 * @brief Paints the component with the current MIDI message log.
 * @param g The Graphics context to paint into.
 * 
 * Draws all visible MIDI messages with their appropriate colors and opacity,
 * based on the current display settings. Messages are drawn from bottom to top,
 * with the most recent messages at the bottom.
 */
void MidiLogDisplay::paint(juce::Graphics& g)
{
    // Get the correct settings for background based on window name
    juce::Colour backgroundColor;
    
    if (windowName.isEmpty() || windowName == "MAIN")
    {
        // For MAIN window or if no window name is set, use the Default settings
        const auto& defaultSettings = settingsManager.getSettings("Default");
        backgroundColor = defaultSettings.backgroundColor;
    }
    else
    {
        // For other windows, use their specific settings
        const auto& windowSettings = settingsManager.getSettings(windowName);
        backgroundColor = windowSettings.backgroundColor;
    }
    
    // Fill the background with the correct color
    g.fillAll(backgroundColor);
    
    float y = getHeight() - 10.0f;  // Start from bottom
    
    // Draw messages from bottom to top
    for (int i = logEntries.size() - 1; i >= 0; --i)
    {
        const auto& entry = logEntries[i];
        
        // Get settings for this message's device
        const auto& settings = settingsManager.getSettings(entry.deviceName);
        
        g.setFont(settings.fontSize);
        g.setColour(entry.color);
        
        float messageHeight = g.getCurrentFont().getHeight();
        y -= messageHeight;
        
        if (y < 0)  // Stop if we've reached the top
            break;
            
        g.drawText(entry.text, 10.0f, y, getWidth() - 20.0f, messageHeight, 
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
 * @brief Timer callback that updates message animations.
 * 
 * Called regularly by the timer to update message opacities and scroll position,
 * creating a fading effect for older messages and smooth scrolling.
 * Triggers a repaint when animations are updated.
 */
void MidiLogDisplay::timerCallback()
{
    bool needsRepaint = false;
    
    // Update message opacities and remove fully faded messages
    for (auto it = messages.begin(); it != messages.end();)
    {
        it->opacity -= fadeRate;
        if (it->opacity <= 0.0f)
            it = messages.erase(it);
        else
            ++it;
        needsRepaint = true;
    }
    
    // Update scroll position
    if (yOffset > 0.0f)
    {
        yOffset = std::max(0.0f, yOffset - scrollSpeed);
        needsRepaint = true;
    }
    
    if (needsRepaint)
        repaint();
}

/**
 * @brief Handles change notifications from the DisplaySettingsManager.
 * @param source The ChangeBroadcaster that triggered the notification.
 * 
 * Called when display settings change, triggering a repaint with the new settings.
 */
void MidiLogDisplay::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &settingsManager)
    {
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
    // X- Get settings for this device
    const auto& settings = settingsManager.getSettings(deviceName);

    // Debug pitch bend messages
    if (message.isPitchWheel()) {
        DBG("PitchBend message received, mute state: " + juce::String(settings.mutePitchBend ? "true" : "false"));
    }
    
    // X- Check mute flags based on message type - ensure correct check for PitchBend
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
    
    logEntries.add(LogEntryData(text, color, deviceName));
    if (static_cast<size_t>(logEntries.size()) > maxEntries)
        logEntries.remove(0);
    
    messages.emplace_back(text, color, juce::Time::getCurrentTime(), deviceName);
    if (static_cast<size_t>(messages.size()) > maxMessages)
        messages.pop_front();
    
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
        messages.pop_front();
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