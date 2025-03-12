/**
 * @file SettingsSection.h
 * @brief Defines a reusable UI component for settings sections.
 * 
 * This file defines the SettingsSection class, which provides a consistent
 * visual container for groups of related settings in the MidiPortal application.
 * Each section has a title, a separator line, and a content area where child
 * components can be placed.
 */

#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

namespace MidiPortal {

/**
 * @class SettingsSection
 * @brief A container component for grouping related settings.
 * 
 * This component provides a consistent visual style for sections of settings
 * in the MidiPortal application. Each section has a title, a separator line,
 * and a content area where child components can be placed.
 * 
 * The component handles its own layout and styling, providing a clean API
 * for adding settings controls to the content area.
 */
class SettingsSection : public juce::Component
{
public:
    /**
     * @brief Constructor that takes a title for the section.
     * @param titleText The text to display as the section title.
     * 
     * Creates a new settings section with the specified title and
     * sets up the visual appearance of the section.
     */
    SettingsSection(const juce::String& titleText)
    {
        // Create and set up the header label
        title.setText(titleText, juce::dontSendNotification);
        
        // X- Use the correct approach for JUCE 8
        juce::FontOptions options;
        options = options.withHeight(16.0f);
        juce::Font font(options);
        font = font.boldened();
        title.setFont(font);
        
        addAndMakeVisible(title);
        
        // Add a separator line below the title
        addAndMakeVisible(separator);
    }
    
    /**
     * @brief Handles component resizing.
     * 
     * Positions the title and separator based on the new size of the component.
     */
    void resized() override
    {
        auto bounds = getLocalBounds().reduced(5);
        title.setBounds(bounds.removeFromTop(24));
        bounds.removeFromTop(5);
        separator.setBounds(bounds.removeFromTop(1));
    }
    
    /**
     * @brief Paints the component.
     * @param g The Graphics context to paint into.
     * 
     * Draws the background of the section with a rounded rectangle and
     * a slightly brighter color than the window background.
     */
    void paint(juce::Graphics& g) override
    {
        auto backgroundColor = getLookAndFeel().findColour(
            juce::ResizableWindow::backgroundColourId).brighter(0.1f);
        
        g.setColour(backgroundColor);
        g.fillRoundedRectangle(getLocalBounds().toFloat(), 5.0f);
    }
    
    /**
     * @brief Gets the bounds of the content area.
     * @return A Rectangle representing the bounds of the content area.
     * 
     * Returns the bounds of the area where child components should be placed,
     * accounting for the space taken by the title and separator.
     */
    juce::Rectangle<int> getContentBounds()
    {
        return getLocalBounds().reduced(5)
                             .withTrimmedTop(35);
    }
    
private:
    /**
     * @brief Label for the section title.
     * 
     * Displays the title of the section at the top.
     */
    juce::Label title;
    
    /**
     * @brief Separator line below the title.
     * 
     * Provides a visual separation between the title and the content area.
     */
    juce::DrawablePath separator;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SettingsSection)
};

} // namespace MidiPortal 