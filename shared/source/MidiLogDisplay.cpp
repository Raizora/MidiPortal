#include "MidiLogDisplay.h"

namespace MidiPortal {

MidiLogDisplay::MidiLogDisplay(DisplaySettingsManager& manager)
    : settingsManager(manager)
{
    // Initialize default settings
    settingsManager.addSettings("Default", DisplaySettingsManager::DisplaySettings());
    setSize(800, 600);
    
    // X- Start the timer to update animations (30fps)
    startTimerHz(30);
    
    // X- Set background color
    setOpaque(true);
    
    // X- Register with settings manager
    settingsManager.registerDisplay(this);
    settingsManager.addChangeListener(this);
}

MidiLogDisplay::~MidiLogDisplay()
{
    // X- Stop the timer
    stopTimer();
    
    // X- Unregister from settings manager
    settingsManager.unregisterDisplay(this);
    settingsManager.removeChangeListener(this);
}

void MidiLogDisplay::paint(juce::Graphics& g)
{
    // Get default settings for background
    const auto& defaultSettings = settingsManager.getSettings("Default");
    g.fillAll(defaultSettings.backgroundColor);
    
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

void MidiLogDisplay::resized()
{
    // Nothing needed here as we handle layout in paint()
}

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

void MidiLogDisplay::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &settingsManager)
    {
        // Settings have changed, repaint with new settings
        repaint();
    }
}

void MidiLogDisplay::addMessage(const juce::MidiMessage& message, const juce::String& deviceName)
{
    auto text = formatMidiMessage(message, deviceName);
    auto color = getColorForMessage(message, deviceName);
    
    logEntries.add(LogEntryData(text, color, deviceName));
    if (logEntries.size() > maxEntries)
        logEntries.remove(0);
    
    messages.emplace_back(text, color, juce::Time::getCurrentTime(), deviceName);
    if (messages.size() > maxMessages)
        messages.pop_front();
    
    repaint();
}

void MidiLogDisplay::clear()
{
    messages.clear();
    logEntries.clear();
    repaint();
}

void MidiLogDisplay::setMaxMessages(size_t max)
{
    maxMessages = max;
    while (messages.size() > maxMessages)
        messages.pop_front();
}

void MidiLogDisplay::settingsChanged(const juce::String& /*deviceName*/)
{
    // Settings have changed, repaint with new settings
    repaint();
}

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

juce::Colour MidiLogDisplay::getColorForMessage(const juce::MidiMessage& message, const juce::String& deviceName)
{
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