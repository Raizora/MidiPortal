#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include "WindowManager.h"

namespace MidiPortal {

// Component for managing window routing and background colors
// This component displays a grid showing which MIDI devices are routed to which windows
// and allows changing the background color of each window
class WindowRoutingComponent : public juce::Component,
                             private juce::Button::Listener,
                             private juce::ChangeListener
{
public:
    // Constructor takes a reference to the WindowManager
    // The WindowManager handles the actual window creation and routing
    WindowRoutingComponent(WindowManager& manager);
    
    // Default destructor is sufficient since we use JUCE's OwnedArray for component management
    ~WindowRoutingComponent() override = default;

    // Override paint to draw the grid and colored bars
    void paint(juce::Graphics& g) override;
    
    // Override resized to position all components
    void resized() override;

private:
    // Button click handler for the New Window button and routing cells
    void buttonClicked(juce::Button* button) override;
    
    // Change listener callback for the color selector
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;
    
    // Update the grid with current windows and devices
    void updateGrid();
    
    // Create a new window with the next available letter name (A, B, C, etc.)
    void createNewWindow();
    
    // Show the color selector for a specific window
    void showColorSelectorForWindow(const juce::String& windowName);

    // Reference to the WindowManager
    WindowManager& windowManager;
    
    // Button to create new windows
    juce::TextButton newWindowButton;
    
    // Routing cell for connecting devices to windows
    // This is a toggle button that shows whether a device is routed to a window
    struct RoutingCell : public juce::ToggleButton
    {
        // Constructor takes the device and window names
        RoutingCell(const juce::String& deviceName, const juce::String& windowName)
            : device(deviceName), window(windowName) {}
            
        // Store the device and window names for this cell
        juce::String device;
        juce::String window;
    };
    
    // Color button for changing window background color
    // This is an invisible component that opens the color selector when clicked
    class ColorButton : public juce::Component
    {
    public:
        // Constructor takes the window name and initial color
        ColorButton(const juce::String& windowName, const juce::Colour& initialColor)
            : window(windowName), color(initialColor), button("RGB")
        {
            // Add the button as a child component
            addAndMakeVisible(button);
            
            // Set the button colors
            button.setColour(juce::TextButton::buttonColourId, color);
            button.setColour(juce::TextButton::textColourOffId, getContrastingColor(color));
            
            // Add a listener to the internal button
            button.onClick = [this] {
                if (onClick)
                    onClick();
            };
        }
        
        // Override resized to position the button
        void resized() override
        {
            // Make the button fill the entire component
            button.setBounds(getLocalBounds());
        }
        
        // Method to update the color
        void setColor(const juce::Colour& newColor)
        {
            // Store the new color
            color = newColor;
            
            // Update the button colors
            button.setColour(juce::TextButton::buttonColourId, color);
            button.setColour(juce::TextButton::textColourOffId, getContrastingColor(color));
        }
        
        // Function to get a contrasting text color (black or white) based on background brightness
        juce::Colour getContrastingColor(const juce::Colour& backgroundColor)
        {
            // Calculate perceived brightness (ITU-R BT.709)
            // This formula gives more weight to green since human eyes are more sensitive to it
            float brightness = 0.2126f * backgroundColor.getFloatRed() + 
                              0.7152f * backgroundColor.getFloatGreen() + 
                              0.0722f * backgroundColor.getFloatBlue();
            
            // Return black for bright backgrounds, white for dark backgrounds
            return brightness > 0.5f ? juce::Colours::black : juce::Colours::white;
        }
        
        // Window name for this button
        juce::String window;
        
        // Current color
        juce::Colour color;
        
        // The actual button component
        juce::TextButton button;
        
        // Callback function for when the button is clicked
        std::function<void()> onClick;
    };
    
    // Arrays to store the components
    juce::OwnedArray<juce::Label> deviceLabels;     // Labels for devices
    juce::OwnedArray<juce::Label> windowLabels;     // Labels for windows
    juce::OwnedArray<RoutingCell> routingCells;     // Toggle buttons for routing
    juce::OwnedArray<ColorButton> colorButtons;     // Buttons for color selection
    
    // Color selector popup
    std::unique_ptr<juce::CallOutBox> colorSelectorCallout;
    std::unique_ptr<juce::ColourSelector> currentColorSelector;
    juce::String currentWindowForColorSelection;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WindowRoutingComponent)
};

} // namespace MidiPortal 