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
                             private juce::Slider::Listener,
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
    
    // Slider value changed handler for RGB sliders
    void sliderValueChanged(juce::Slider* slider) override;
    
    // Change listener callback for the color selector
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;
    
    // Update the grid with current windows and devices
    void updateGrid();
    
    // Create a new window with the next available letter name (A, B, C, etc.)
    void createNewWindow();
    
    // Show the color selector for a specific window
    void showColorSelectorForWindow(const juce::String& windowName);
    
    // Apply the RGB slider values to update a window's background color
    void applyRGBSlidersToWindow(const juce::String& windowName);

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
    
    // RGB Sliders component for changing window background color
    // This component contains three sliders (R, G, B) and an Apply button
    class RGBSliders : public juce::Component
    {
    public:
        // Constructor takes the window name and initial color
        RGBSliders(const juce::String& windowName, const juce::Colour& initialColor)
            : window(windowName), color(initialColor)
        {
            // Create and configure the red slider
            redSlider.setRange(0.0, 255.0, 1.0);
            redSlider.setValue(initialColor.getRed(), juce::dontSendNotification);
            redSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 20);
            redSlider.setColour(juce::Slider::thumbColourId, juce::Colours::red);
            redSlider.setColour(juce::Slider::trackColourId, juce::Colours::red.withAlpha(0.5f));
            redSlider.setTextValueSuffix(" R");
            addAndMakeVisible(redSlider);
            
            // Create and configure the green slider
            greenSlider.setRange(0.0, 255.0, 1.0);
            greenSlider.setValue(initialColor.getGreen(), juce::dontSendNotification);
            greenSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 20);
            greenSlider.setColour(juce::Slider::thumbColourId, juce::Colours::green);
            greenSlider.setColour(juce::Slider::trackColourId, juce::Colours::green.withAlpha(0.5f));
            greenSlider.setTextValueSuffix(" G");
            addAndMakeVisible(greenSlider);
            
            // Create and configure the blue slider
            blueSlider.setRange(0.0, 255.0, 1.0);
            blueSlider.setValue(initialColor.getBlue(), juce::dontSendNotification);
            blueSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 20);
            blueSlider.setColour(juce::Slider::thumbColourId, juce::Colours::blue);
            blueSlider.setColour(juce::Slider::trackColourId, juce::Colours::blue.withAlpha(0.5f));
            blueSlider.setTextValueSuffix(" B");
            addAndMakeVisible(blueSlider);
            
            // Create and configure the Apply button
            applyButton.setButtonText("Apply");
            addAndMakeVisible(applyButton);
        }
        
        // Override resized to position the sliders and button
        void resized() override
        {
            auto bounds = getLocalBounds();
            
            // Position the sliders vertically, one below the other
            redSlider.setBounds(bounds.removeFromTop(20));
            bounds.removeFromTop(5); // Add a small gap
            greenSlider.setBounds(bounds.removeFromTop(20));
            bounds.removeFromTop(5); // Add a small gap
            blueSlider.setBounds(bounds.removeFromTop(20));
            bounds.removeFromTop(5); // Add a small gap
            
            // Position the Apply button at the bottom
            applyButton.setBounds(bounds.removeFromTop(25));
        }
        
        // Method to update the color
        void setColor(const juce::Colour& newColor)
        {
            // Store the new color
            color = newColor;
            
            // Update the slider values
            redSlider.setValue(newColor.getRed(), juce::dontSendNotification);
            greenSlider.setValue(newColor.getGreen(), juce::dontSendNotification);
            blueSlider.setValue(newColor.getBlue(), juce::dontSendNotification);
        }
        
        // Method to get the current color from the sliders
        juce::Colour getCurrentColor() const
        {
            return juce::Colour(
                static_cast<juce::uint8>(redSlider.getValue()),
                static_cast<juce::uint8>(greenSlider.getValue()),
                static_cast<juce::uint8>(blueSlider.getValue())
            );
        }
        
        // Window name for this component
        juce::String window;
        
        // Current color
        juce::Colour color;
        
        // The sliders for R, G, B values
        juce::Slider redSlider;
        juce::Slider greenSlider;
        juce::Slider blueSlider;
        
        // The Apply button
        juce::TextButton applyButton;
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
    juce::OwnedArray<RGBSliders> rgbSliders;        // RGB sliders for color adjustment
    
    // Color selector popup
    std::unique_ptr<juce::CallOutBox> colorSelectorCallout;
    std::unique_ptr<juce::ColourSelector> currentColorSelector;
    juce::String currentWindowForColorSelection;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WindowRoutingComponent)
};

} // namespace MidiPortal 