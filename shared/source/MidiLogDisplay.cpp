#include "MidiLogDisplay.h"

namespace MidiPortal {

MidiLogDisplay::MidiLogDisplay()
{
    // X- Start the timer to update animations (30fps)
    startTimerHz(30);
    
    // X- Set background color
    setOpaque(true);
}

MidiLogDisplay::~MidiLogDisplay()
{
    // X- Stop the timer
    stopTimer();
}

void MidiLogDisplay::paint(juce::Graphics& g)
{
    // X- Fill background
    g.fillAll(juce::Colours::black);
    
    // X- Get the visible area
    auto bounds = getLocalBounds();

    // Create a FontOptions object with the desired typeface name and height
    auto fontOptions = juce::FontOptions()
                           .withName(juce::Font::getDefaultMonospacedFontName())
                           .withHeight(14.0f);

    // Create a Font object using the FontOptions
    juce::Font customFont(fontOptions);

    // Set the font to the Graphics context
    g.setFont(customFont);

    // Calculate line height
    float lineHeight = customFont.getHeight() * 1.5f;

    // Start position for drawing (bottom of the component)
    float y = bounds.getHeight() - yOffset;

    // Draw each message from newest to oldest
    for (auto it = messages.rbegin(); it != messages.rend(); ++it)
    {
        // Skip if fully transparent
        if (it->opacity <= 0.01f)
            continue;

        // Set color with opacity
        g.setColour(it->color.withAlpha(it->opacity));

        // Draw text
        g.drawText(it->text, bounds.getX() + 10, static_cast<int>(y - lineHeight),
                   bounds.getWidth() - 20, static_cast<int>(lineHeight),
                   juce::Justification::left, true);

        // Move up for next message
        y -= lineHeight;

        // Stop if we're off the top of the component
        if (y < 0)
            break;
    }
}

void MidiLogDisplay::resized()
{
    // X- Nothing to do here
}

void MidiLogDisplay::timerCallback()
{
    bool needsRepaint = false;
    
    // X- Update opacity for each message
    for (auto& entry : messages)
    {
        // X- Fade out messages over time
        entry.opacity -= fadeRate;
        if (entry.opacity < 0.0f)
            entry.opacity = 0.0f;
            
        needsRepaint = true;
    }
    
    // X- Remove fully faded messages from the front
    while (!messages.empty() && messages.front().opacity <= 0.0f)
    {
        messages.pop_front();
        needsRepaint = true;
    }
    
    // X- Update scroll position
    if (!messages.empty())
    {
        yOffset += scrollSpeed;
        needsRepaint = true;
    }
    else
    {
        // X- Reset scroll position when no messages
        yOffset = 0.0f;
    }
    
    // X- Repaint if needed
    if (needsRepaint)
        repaint();
}

void MidiLogDisplay::addMessage(const juce::MidiMessage& message, const juce::String& deviceName)
{
    // X- Format the message
    juce::String text = formatMidiMessage(message, deviceName);
    
    // X- Get color for this message type
    juce::Colour color = getColorForMessage(message);
    
    // X- Add to the queue
    messages.emplace_back(text, color, juce::Time::getCurrentTime());
    
    // X- Limit the number of messages
    while (messages.size() > maxMessages)
        messages.pop_front();
    
    // X- Reset scroll position for new message
    yOffset = 0.0f;
    
    // X- Trigger repaint
    repaint();
}

void MidiLogDisplay::clear()
{
    // X- Clear all messages
    messages.clear();
    yOffset = 0.0f;
    repaint();
}

void MidiLogDisplay::setMaxMessages(size_t max)
{
    // X- Set maximum number of messages
    maxMessages = max;
    
    // X- Trim if needed
    while (messages.size() > maxMessages)
        messages.pop_front();
}

juce::String MidiLogDisplay::formatMidiMessage(const juce::MidiMessage& message, const juce::String& deviceName)
{
    juce::String formattedText;
    
    // X- Add timestamp
    formattedText << juce::Time::getCurrentTime().formatted("%H:%M:%S.%03d") << " | ";
    
    // X- Add device name (truncated if too long)
    if (deviceName.length() > 15)
        formattedText << deviceName.substring(0, 12) << "... | ";
    else
        formattedText << deviceName.paddedLeft(' ', 15) << " | ";
    
    // X- Format based on message type
    if (message.isNoteOn())
    {
        formattedText << "Note On:  " << juce::MidiMessage::getMidiNoteName(message.getNoteNumber(), true, true, 4)
             << " (" << message.getNoteNumber() << ") Vel:" << message.getVelocity()
             << " Ch:" << message.getChannel();
    }
    else if (message.isNoteOff())
    {
        formattedText << "Note Off: " << juce::MidiMessage::getMidiNoteName(message.getNoteNumber(), true, true, 4)
             << " (" << message.getNoteNumber() << ") Vel:" << message.getVelocity()
             << " Ch:" << message.getChannel();
    }
    else if (message.isPitchWheel())
    {
        int value = message.getPitchWheelValue() - 8192;
        formattedText << "Pitch Bend: " << value << " Ch:" << message.getChannel();
    }
    else if (message.isController())
    {
        formattedText << "CC " << message.getControllerNumber() << ": " << message.getControllerValue()
             << " Ch:" << message.getChannel();
             
        // X- Add common controller names
        switch (message.getControllerNumber())
        {
            case 1:  formattedText << " (Mod Wheel)"; break;
            case 7:  formattedText << " (Volume)"; break;
            case 10: formattedText << " (Pan)"; break;
            case 11: formattedText << " (Expression)"; break;
            case 64: formattedText << " (Sustain)"; break;
            case 74: formattedText << " (Cutoff)"; break;
        }
    }
    else if (message.isChannelPressure())
    {
        formattedText << "Channel Pressure: " << message.getChannelPressureValue()
             << " Ch:" << message.getChannel();
    }
    else if (message.isAftertouch())
    {
        formattedText << "Aftertouch: " << juce::MidiMessage::getMidiNoteName(message.getNoteNumber(), true, true, 4)
             << " (" << message.getNoteNumber() << ") Value:" << message.getAfterTouchValue()
             << " Ch:" << message.getChannel();
    }
    else if (message.isProgramChange())
    {
        formattedText << "Program Change: " << message.getProgramChangeNumber()
             << " Ch:" << message.getChannel();
    }
    else if (message.isMidiClock())
    {
        formattedText << "MIDI Clock";
    }
    else if (message.isMidiStart())
    {
        formattedText << "MIDI Start";
    }
    else if (message.isMidiStop())
    {
        formattedText << "MIDI Stop";
    }
    else if (message.isMidiContinue())
    {
        formattedText << "MIDI Continue";
    }
    else if (message.isSysEx())
    {
        formattedText << "SysEx: " << message.getSysExDataSize() << " bytes";
    }
    else
    {
        // X- Generic fallback for other message types
        formattedText << "MIDI Message: ";
        for (int i = 0; i < message.getRawDataSize(); ++i)
        {
            formattedText << juce::String::formatted("%02X ", message.getRawData()[i]);
        }
    }
    
    return formattedText;
}

juce::Colour MidiLogDisplay::getColorForMessage(const juce::MidiMessage& message)
{
    // X- Color-code by message type
    if (message.isNoteOn())
        return juce::Colours::lightgreen;
    else if (message.isNoteOff())
        return juce::Colours::indianred;
    else if (message.isPitchWheel())
        return juce::Colours::deepskyblue;
    else if (message.isController())
        return juce::Colours::orange;
    else if (message.isChannelPressure() || message.isAftertouch())
        return juce::Colours::mediumpurple;
    else if (message.isProgramChange())
        return juce::Colours::yellow;
    else if (message.isMidiClock() || message.isMidiStart() || message.isMidiStop() || message.isMidiContinue())
        return juce::Colours::lightgrey;
    else if (message.isSysEx())
        return juce::Colours::hotpink;
    else
        return juce::Colours::white;
}

} // namespace MidiPortal 