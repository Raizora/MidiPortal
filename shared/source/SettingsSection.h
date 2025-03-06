#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

namespace MidiPortal {

class SettingsSection : public juce::Component
{
public:
    SettingsSection(const juce::String& titleText)
    {
        // Create and set up the header label
        title.setText(titleText, juce::dontSendNotification);
        
        // Use a simple approach that works with JUCE 8
        auto font = juce::Font(16.0f);
        font = font.boldened();
        title.setFont(font);
        
        addAndMakeVisible(title);
        
        // Add a separator line below the title
        addAndMakeVisible(separator);
    }
    
    void resized() override
    {
        auto bounds = getLocalBounds().reduced(5);
        title.setBounds(bounds.removeFromTop(24));
        bounds.removeFromTop(5);
        separator.setBounds(bounds.removeFromTop(1));
    }
    
    void paint(juce::Graphics& g) override
    {
        auto backgroundColor = getLookAndFeel().findColour(
            juce::ResizableWindow::backgroundColourId).brighter(0.1f);
        
        g.setColour(backgroundColor);
        g.fillRoundedRectangle(getLocalBounds().toFloat(), 5.0f);
    }
    
    // Get the content area where child components should be placed
    juce::Rectangle<int> getContentBounds()
    {
        return getLocalBounds().reduced(5)
                             .withTrimmedTop(35);
    }
    
private:
    juce::Label title;
    juce::DrawablePath separator;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SettingsSection)
};

} // namespace MidiPortal 