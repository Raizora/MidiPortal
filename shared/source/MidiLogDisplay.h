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
    // X- Settings struct to store user preferences for colors
    struct DisplaySettings {
        juce::Colour noteOnColor = juce::Colours::lightgreen;
        juce::Colour noteOffColor = juce::Colours::indianred;
        juce::Colour pitchBendColor = juce::Colours::deepskyblue;
        juce::Colour controllerColor = juce::Colours::orange;
        juce::Colour pressureColor = juce::Colours::mediumpurple;
        juce::Colour programChangeColor = juce::Colours::yellow;
        juce::Colour clockColor = juce::Colours::lightgrey;
        juce::Colour sysExColor = juce::Colours::hotpink;
        juce::Colour defaultColor = juce::Colours::white;
        
        // X- Font size for the log display
        float fontSize = 14.0f;
        
        // X- Background color
        juce::Colour backgroundColor = juce::Colours::black;
    };
    
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
    
    // X- Get current settings
    const DisplaySettings& getSettings() const { return settings; }
    
    // X- Update settings
    void setSettings(const DisplaySettings& newSettings);
    
private:
    struct LogEntry {
        juce::String text;
        juce::Colour color;
        float opacity;
        juce::Time timestamp;
        
        LogEntry(const juce::String& textIn, const juce::Colour& colorIn, const juce::Time& timestampIn)
            : text(textIn), color(colorIn), opacity(1.0f), timestamp(timestampIn) {}
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
    
    // X- User settings
    DisplaySettings settings;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiLogDisplay)
};

} // namespace MidiPortal 