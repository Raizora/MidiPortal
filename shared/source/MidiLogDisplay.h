#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include "DisplaySettingsManager.h"
#include <deque>
#include <memory>
#include <map>

namespace MidiPortal {

// X- A component that displays a scrolling log of MIDI messages with fading effect
class MidiLogDisplay : public juce::Component,
                      public juce::Timer,
                      public juce::ChangeListener
{
public:
    explicit MidiLogDisplay(DisplaySettingsManager& manager);
    ~MidiLogDisplay() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;
    
    // X- Add a new message to the display
    void addMessage(const juce::MidiMessage& message, const juce::String& deviceName);
    
    // X- Clear all messages
    void clear();
    
    // X- Set maximum number of messages to display
    void setMaxMessages(size_t maxMessages);
    
    // X- Called by DisplaySettingsManager when settings change
    void settingsChanged(const juce::String& deviceName);
    
    // X- Get reference to settings manager
    DisplaySettingsManager& getSettingsManager() { return settingsManager; }
    
private:
    struct LogEntry {
        juce::String text;
        juce::Colour color;
        float opacity;
        juce::Time timestamp;
        juce::String deviceName;
        
        LogEntry(const juce::String& textIn, const juce::Colour& colorIn, const juce::Time& timestampIn, const juce::String& deviceNameIn)
            : text(textIn), color(colorIn), opacity(1.0f), timestamp(timestampIn), deviceName(deviceNameIn) {}
    };
    
    // X- Format a MIDI message as text
    juce::String formatMidiMessage(const juce::MidiMessage& message, const juce::String& deviceName);
    
    // X- Get color for a MIDI message based on its type
    juce::Colour getColorForMessage(const juce::MidiMessage& message, const juce::String& deviceName);
    
    std::deque<LogEntry> messages;
    size_t maxMessages = 100;
    float fadeRate = 0.02f;  // How quickly messages fade out
    float scrollSpeed = 0.5f; // Pixels per frame to scroll
    float yOffset = 0.0f;     // Current scroll position
    
    // X- Reference to the settings manager
    DisplaySettingsManager& settingsManager;
    
    // X- Store log entries
    struct LogEntryData {
        juce::String text;
        juce::Colour color;
        juce::String deviceName;
        
        // Add default constructor
        LogEntryData() : text(), color(juce::Colours::white), deviceName() {}
        
        LogEntryData(const juce::String& t, const juce::Colour& c, const juce::String& d)
            : text(t), color(c), deviceName(d) {}
    };
    
    juce::Array<LogEntryData> logEntries;
    static constexpr size_t maxEntries = 1000;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiLogDisplay)
};

} // namespace MidiPortal 