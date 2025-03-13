/**
 * @file AIInsightComponent.cpp
 * @brief Implementation of the AIInsightComponent class.
 * 
 * This file contains the implementation of the AIInsightComponent class methods,
 * which display AI insights in the UI.
 */

#include "AIInsightComponent.h"

namespace MidiPortal {

/**
 * @brief Constructor.
 * 
 * Creates a new AIInsightComponent.
 */
AIInsightComponent::AIInsightComponent()
{
    // Start the timer to update the opacity of insights
    startTimer(50); // Update every 50ms
    
    // Set the component to be opaque
    setOpaque(true);
}

/**
 * @brief Destructor.
 * 
 * Destroys the AIInsightComponent.
 */
AIInsightComponent::~AIInsightComponent()
{
    // Stop the timer
    stopTimer();
}

/**
 * @brief Paints the component.
 * @param g The graphics context to paint into.
 * 
 * Paints the component with the current insights.
 */
void AIInsightComponent::paint(juce::Graphics& g)
{
    // Fill the background
    g.fillAll(juce::Colours::black);
    
    // Draw the insights
    const int padding = 10;
    const int insightHeight = 30;
    int y = getHeight() - padding;
    
    // Draw from bottom to top (most recent at the bottom)
    for (auto it = insights.rbegin(); it != insights.rend(); ++it)
    {
        const auto& insight = *it;
        
        // Skip if fully transparent
        if (insight.opacity <= 0.0f)
            continue;
        
        // Calculate position
        y -= insightHeight;
        if (y < padding)
            break;
        
        // Draw background
        juce::Colour color = getColorForType(insight.type).withAlpha(insight.opacity * 0.3f);
        g.setColour(color);
        g.fillRoundedRectangle(padding, y, getWidth() - padding * 2, insightHeight, 5.0f);
        
        // Draw border
        g.setColour(color.brighter(0.5f).withAlpha(insight.opacity));
        g.drawRoundedRectangle(padding, y, getWidth() - padding * 2, insightHeight, 5.0f, 1.0f);
        
        // Draw text
        g.setColour(juce::Colours::white.withAlpha(insight.opacity));
        g.setFont(juce::Font(14.0f));
        
        // Draw description
        juce::String text = insight.description;
        if (insight.score > 0.0f)
            text += juce::String::formatted(" (%.0f%%)", insight.score * 100.0f);
        
        g.drawText(text, padding + 5, y, getWidth() - padding * 2 - 10, insightHeight, juce::Justification::centredLeft, true);
    }
}

/**
 * @brief Handles component resizing.
 * 
 * Positions child components based on the new size of the component.
 */
void AIInsightComponent::resized()
{
    // Nothing to do here
}

/**
 * @brief Timer callback.
 * 
 * Called periodically to update the opacity of insights.
 */
void AIInsightComponent::timerCallback()
{
    bool needsRepaint = false;
    
    // Update the opacity of insights
    for (auto& insight : insights)
    {
        // Calculate how long the insight has been displayed
        const double age = juce::Time::getMillisecondCounterHiRes() / 1000.0 - insight.timestamp.toMilliseconds() / 1000.0;
        
        // Start fading after 5 seconds
        if (age > 5.0)
        {
            insight.opacity -= fadeRate;
            needsRepaint = true;
        }
    }
    
    // Remove fully transparent insights
    while (!insights.empty() && insights.front().opacity <= 0.0f)
    {
        insights.pop_front();
        needsRepaint = true;
    }
    
    // Repaint if needed
    if (needsRepaint)
        repaint();
}

/**
 * @brief Adds insights to display.
 * @param newInsights The insights to add.
 * 
 * Adds insights to the display. The insights will fade out over time.
 */
void AIInsightComponent::addInsights(const std::vector<MidiAIManager::AIInsight>& newInsights)
{
    // Add new insights
    for (const auto& newInsight : newInsights)
    {
        // Create a displayed insight
        DisplayedInsight insight;
        insight.description = newInsight.description;
        insight.score = newInsight.score;
        insight.type = newInsight.type;
        insight.opacity = 1.0f;
        insight.timestamp = juce::Time::getCurrentTime();
        
        // Add to the queue
        insights.push_back(insight);
        
        // Limit the number of insights
        while (insights.size() > maxInsights)
            insights.pop_front();
    }
    
    // Repaint to show the new insights
    repaint();
}

/**
 * @brief Gets the color for an insight type.
 * @param type The type of insight.
 * @return The color for the insight type.
 * 
 * Gets the color for an insight type. Pattern insights are green,
 * performance insights are blue, and style insights are purple.
 */
juce::Colour AIInsightComponent::getColorForType(MidiAIManager::AIInsight::Type type) const
{
    switch (type)
    {
        case MidiAIManager::AIInsight::Type::Pattern:
            return juce::Colours::green;
        case MidiAIManager::AIInsight::Type::Performance:
            return juce::Colours::blue;
        case MidiAIManager::AIInsight::Type::Style:
            return juce::Colours::purple;
        default:
            return juce::Colours::white;
    }
}

} // namespace MidiPortal 