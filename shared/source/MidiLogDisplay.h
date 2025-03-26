/**
 * @file MidiLogDisplay.h
 * @brief Displays a scrolling log of MIDI messages with visual formatting.
 * 
 * The MidiLogDisplay component is responsible for visually displaying MIDI messages
 * in a scrolling log format. It applies different colors to different types of MIDI
 * messages and supports fading effects for a more dynamic display.
 * 
 * This component works with the DisplaySettingsManager to apply consistent visual
 * styling based on device-specific or window-specific settings.
 */

#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include "DisplaySettingsManager.h"
#include <deque>
#include <memory>
#include <map>
#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_events/juce_events.h>

namespace MidiPortal {

/**
 * @class MidiLogDisplay
 * @brief Displays a scrolling log of MIDI messages with visual formatting.
 * 
 * This component displays a scrolling log of MIDI messages with customizable
 * visual formatting. Each message is displayed with a color based on its type
 * and fades out over time.
 */
class MidiLogDisplay : public juce::AnimatedAppComponent,
                      public juce::ChangeListener
{
public:
    /**
     * @brief Constructor that takes a reference to the DisplaySettingsManager.
     * @param manager Reference to the DisplaySettingsManager that will be used for display settings.
     * 
     * Initializes the MidiLogDisplay with a reference to the DisplaySettingsManager,
     * sets up default visual properties, and starts the animation.
     */
    MidiLogDisplay(DisplaySettingsManager& manager);
    
    /**
     * @brief Destructor that cleans up resources.
     * 
     * Stops the animation and cleans up resources.
     */
    ~MidiLogDisplay() override;
    
    /**
     * @brief Paints the component with the current MIDI message log.
     * @param g The Graphics context to paint into.
     * 
     * Draws all visible MIDI messages with their appropriate colors and opacity,
     * based on the current display settings.
     */
    void paint(juce::Graphics& g) override;
    
    /**
     * @brief Handles component resizing.
     * 
     * Called when the component is resized, allowing layout adjustments.
     */
    void resized() override;
    
    /**
     * @brief Animation update callback that updates message animations.
     * 
     * Called regularly by the AnimatedAppComponent to update message opacities
     * and scroll position, creating a fading effect for older messages.
     */
    void update() override;
    
    /**
     * @brief Handles change notifications from the DisplaySettingsManager.
     * @param source The ChangeBroadcaster that triggered the notification.
     * 
     * Called when display settings change, triggering a repaint with the new settings.
     */
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;
    
    /**
     * @brief Adds a new MIDI message to the display.
     * @param message The MIDI message to add.
     * @param deviceName The name of the device that sent the message.
     * 
     * Formats the message as text, assigns an appropriate color based on the
     * message type and device settings, and adds it to the display.
     */
    void addMessage(const juce::MidiMessage& message, const juce::String& deviceName);
    
    /**
     * @brief Clears all messages from the display.
     * 
     * Removes all messages from the display and triggers a repaint.
     */
    void clear();
    
    /**
     * @brief Sets the maximum number of messages to display.
     * @param maxMessages The maximum number of messages to keep in the display.
     * 
     * Limits the number of messages stored in memory to prevent excessive
     * memory usage. Older messages are removed when the limit is reached.
     */
    void setMaxMessages(size_t maxMessages);
    
    /**
     * @brief Called when display settings change for a specific device.
     * @param deviceName The name of the device whose settings changed.
     * 
     * Triggers a repaint with the new settings for the specified device.
     */
    void settingsChanged(const juce::String& deviceName);
    
    /**
     * @brief Gets a reference to the DisplaySettingsManager.
     * @return A reference to the DisplaySettingsManager.
     * 
     * Provides access to the DisplaySettingsManager for other components.
     */
    DisplaySettingsManager& getSettingsManager() { return settingsManager; }
    
    /**
     * @brief Sets the window name that this display belongs to.
     * @param name The name of the window.
     * 
     * This allows the display to use the correct settings for its window
     * rather than always using the default settings.
     */
    void setWindowName(const juce::String& name) { windowName = name; }
    
    /**
     * @brief Gets the window name that this display belongs to.
     * @return The name of the window.
     */
    const juce::String& getWindowName() const { return windowName; }
    
private:
    /**
     * @struct LogEntry
     * @brief Represents a single MIDI message in the display.
     * 
     * Stores the text, color, opacity, timestamp, and device name for a MIDI message.
     * The opacity is used for the fading effect as messages age.
     */
    struct LogEntry {
        juce::String text;       ///< Formatted text representation of the MIDI message
        juce::Colour color;      ///< Color to display the message in
        float opacity;           ///< Current opacity of the message (1.0 = fully opaque)
        juce::Time timestamp;    ///< When the message was received
        juce::String deviceName; ///< Name of the device that sent the message
        juce::String uniqueId;   ///< Unique identifier to link with LogEntryData
        bool shouldFade;         ///< Whether this message should fade (set at creation time)
        float fadeRate;          ///< The fade rate for this message (set at creation time)
        
        /**
         * @brief Constructor that initializes a LogEntry.
         * @param textIn The formatted text of the MIDI message.
         * @param colorIn The color to display the message in.
         * @param timestampIn When the message was received.
         * @param deviceNameIn The name of the device that sent the message.
         */
        LogEntry(const juce::String& textIn, const juce::Colour& colorIn, const juce::Time& timestampIn, const juce::String& deviceNameIn)
            : text(textIn), color(colorIn), opacity(1.0f), timestamp(timestampIn), deviceName(deviceNameIn), 
              uniqueId(""), shouldFade(false), fadeRate(0.5f) {}
    };
    
    /**
     * @brief Formats a MIDI message as text.
     * @param message The MIDI message to format.
     * @param deviceName The name of the device that sent the message.
     * @return A formatted string representation of the MIDI message.
     * 
     * Creates a human-readable text representation of a MIDI message,
     * including the device name and relevant message parameters.
     */
    juce::String formatMidiMessage(const juce::MidiMessage& message, const juce::String& deviceName);
    
    /**
     * @brief Gets the appropriate color for a MIDI message based on its type.
     * @param message The MIDI message to get the color for.
     * @param deviceName The name of the device that sent the message.
     * @return The color to display the message in.
     * 
     * Determines the appropriate color for a MIDI message based on its type
     * and the current display settings for the specified device.
     */
    juce::Colour getColorForMessage(const juce::MidiMessage& message, const juce::String& deviceName);
    
    /**
     * @brief Queue of active messages with fading effect.
     * 
     * These messages are actively being animated with the fading effect.
     */
    std::deque<LogEntry> messages;
    
    /**
     * @brief Maximum number of messages to keep in the display.
     * 
     * Limits the number of messages stored in memory to prevent excessive
     * memory usage. Older messages are removed when the limit is reached.
     */
    size_t maxMessages = 100;
    
    /**
     * @brief Speed at which the display scrolls.
     * 
     * Controls how quickly the display scrolls when new messages are added.
     * Higher values make the display scroll faster.
     */
    float scrollSpeed = 0.5f;
    
    /**
     * @brief Current vertical scroll position.
     * 
     * Tracks the current vertical scroll position of the display.
     */
    float yOffset = 0.0f;
    
    /**
     * @brief Reference to the DisplaySettingsManager.
     * 
     * Used to get display settings for different devices and to register
     * for change notifications when settings change.
     */
    DisplaySettingsManager& settingsManager;
    
    /**
     * @struct LogEntryData
     * @brief Simplified version of LogEntry for persistent storage.
     * 
     * Stores the text, color, and device name for a MIDI message,
     * but without the animation-related properties.
     */
    struct LogEntryData {
        juce::String text;       ///< Formatted text representation of the MIDI message
        juce::Colour color;      ///< Color to display the message in
        juce::String deviceName; ///< Name of the device that sent the message
        juce::String uniqueId;   ///< Unique identifier to link with LogEntry
        
        /**
         * @brief Default constructor.
         */
        LogEntryData() : text(), color(juce::Colours::white), deviceName(), uniqueId("") {}
        
        /**
         * @brief Constructor that initializes a LogEntryData.
         * @param t The formatted text of the MIDI message.
         * @param c The color to display the message in.
         * @param d The name of the device that sent the message.
         */
        LogEntryData(const juce::String& t, const juce::Colour& c, const juce::String& d)
            : text(t), color(c), deviceName(d), uniqueId("") {}
    };
    
    /**
     * @brief Array of all log entries for persistent display.
     * 
     * Stores all log entries that should be displayed, even those
     * that are no longer being animated with the fading effect.
     */
    juce::Array<LogEntryData> logEntries;
    
    /**
     * @brief Maximum number of entries to store.
     * 
     * Limits the number of entries stored in memory to prevent excessive
     * memory usage. Older entries are removed when the limit is reached.
     */
    static constexpr size_t maxEntries = 1000;
    
    /**
     * @brief Name of the window that this display belongs to.
     * 
     * Used to get the correct settings for the display's background color.
     * If empty, the display will use the default settings.
     */
    juce::String windowName;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiLogDisplay)
};

} // namespace MidiPortal 