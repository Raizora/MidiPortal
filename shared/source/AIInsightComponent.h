/**
 * @file AIInsightComponent.h
 * @brief Defines the AIInsightComponent class.
 * 
 * This file defines the AIInsightComponent class, which displays AI insights in the UI.
 */

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_events/juce_events.h>
#include <deque>
#include "MidiAIManager.h"

namespace MidiPortal {

/**
 * @class AIInsightComponent
 * @brief Displays AI insights in the UI.
 * 
 * This class displays AI insights in the UI, including insights about patterns,
 * performance, and style.
 */
class AIInsightComponent : public juce::Component,
                          public juce::Timer
{
public:
    /**
     * @brief Constructor.
     * 
     * Creates a new AIInsightComponent.
     */
    AIInsightComponent();
    
    /**
     * @brief Destructor.
     * 
     * Destroys the AIInsightComponent.
     */
    ~AIInsightComponent() override;
    
    /**
     * @brief Paints the component.
     * @param g The graphics context to paint into.
     * 
     * Paints the component with the current insights.
     */
    void paint(juce::Graphics& g) override;
    
    /**
     * @brief Handles component resizing.
     * 
     * Positions child components based on the new size of the component.
     */
    void resized() override;
    
    /**
     * @brief Timer callback.
     * 
     * Called periodically to update the opacity of insights.
     */
    void timerCallback() override;
    
    /**
     * @brief Adds insights to display.
     * @param newInsights The insights to add.
     * 
     * Adds insights to the display. The insights will fade out over time.
     */
    void addInsights(const std::vector<MidiAIManager::AIInsight>& newInsights);
    
private:
    /**
     * @struct DisplayedInsight
     * @brief Represents an insight displayed in the UI.
     * 
     * This struct represents an insight displayed in the UI, including
     * the type of insight, a description, a confidence score, and display properties.
     */
    struct DisplayedInsight
    {
        /**
         * @brief The type of insight.
         * 
         * The type of insight, such as pattern, performance, or style.
         */
        MidiAIManager::AIInsight::Type type;
        
        /**
         * @brief The description of the insight.
         * 
         * A human-readable description of the insight.
         */
        juce::String description;
        
        /**
         * @brief The confidence score of the insight.
         * 
         * A value between 0.0 and 1.0 indicating the confidence of the insight.
         */
        float score;
        
        /**
         * @brief The opacity of the insight.
         * 
         * A value between 0.0 and 1.0 indicating the opacity of the insight.
         */
        float opacity;
        
        /**
         * @brief The timestamp of the insight.
         * 
         * The time when the insight was added to the display.
         */
        juce::Time timestamp;
    };
    
    /**
     * @brief Gets the color for an insight type.
     * @param type The type of insight.
     * @return The color for the insight type.
     * 
     * Gets the color for an insight type. Pattern insights are green,
     * performance insights are blue, and style insights are purple.
     */
    juce::Colour getColorForType(MidiAIManager::AIInsight::Type type) const;
    
    /**
     * @brief The insights to display.
     * 
     * A deque of insights to display, ordered by time (oldest first).
     */
    std::deque<DisplayedInsight> insights;
    
    /**
     * @brief The maximum number of insights to display.
     * 
     * The maximum number of insights to keep in the deque.
     */
    static constexpr int maxInsights = 10;
    
    /**
     * @brief The fade rate of insights.
     * 
     * The rate at which insights fade out, in opacity units per timer callback.
     */
    static constexpr float fadeRate = 0.05f;
};

} // namespace MidiPortal 