#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

namespace MidiPortal {

class MidiActivityIndicator : public juce::Component,
                             public juce::Timer
{
public:
    MidiActivityIndicator();
    
    void paint(juce::Graphics& g) override;
    void timerCallback() override;
    void triggerActivity();
    
private:
    bool active = false;
    juce::int64 lastActivityTime = 0;
};

} // namespace MidiPortal 