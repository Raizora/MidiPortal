#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include "WindowManager.h"

namespace MidiPortal {

class WindowRoutingComponent : public juce::Component,
                             private juce::Button::Listener
{
public:
    WindowRoutingComponent(WindowManager& manager);
    ~WindowRoutingComponent() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void buttonClicked(juce::Button* button) override;
    void updateGrid();
    void createNewWindow();

    WindowManager& windowManager;
    juce::TextButton newWindowButton;
    
    struct RoutingCell : public juce::ToggleButton
    {
        RoutingCell(const juce::String& deviceName, const juce::String& windowName)
            : device(deviceName), window(windowName) {}
            
        juce::String device;
        juce::String window;
    };
    
    juce::OwnedArray<juce::Label> deviceLabels;
    juce::OwnedArray<juce::Label> windowLabels;
    juce::OwnedArray<RoutingCell> routingCells;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WindowRoutingComponent)
};

} // namespace MidiPortal 