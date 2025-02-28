#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <deque>
#include <memory>

namespace MidiPortal {

// X- A component that displays a scrolling log of MIDI messages with fading effect
class MidiLogDisplay : public juce::Component,
                      public juce::Timer
{
public:
    MidiLogDisplay();
    ~MidiLogDisplay() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;
    
    // X- Add a new message to the display
    void addMessage(const juce::MidiMessage& message, const juce::String& deviceName);
    
    // X- Clear all messages
    void clear();
    
    // X- Set maximum number of messages to display
    void setMaxMessages(size_t maxMessages);
    
private:
    struct LogEntry {
        juce::String text;
        juce::Colour color;
        float opacity;
        juce::Time timestamp;
        
        LogEntry(const juce::String& text, const juce::Colour& color, const juce::Time& timestamp)
            : text(text), color(color), opacity(1.0f), timestamp(timestamp) {}
    };
    
    // X- Format a MIDI message as text
    juce::String formatMidiMessage(const juce::MidiMessage& message, const juce::String& deviceName);
    
    // X- Get color for a MIDI message based on its type
    juce::Colour getColorForMessage(const juce::MidiMessage& message);
    
    std::deque<LogEntry> messages;
    size_t maxMessages = 100;
    float fadeRate = 0.02f;  // How quickly messages fade out
    float scrollSpeed = 0.5f; // Pixels per frame to scroll
    float yOffset = 0.0f;     // Current scroll position
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiLogDisplay)
};

} // namespace MidiPortal 