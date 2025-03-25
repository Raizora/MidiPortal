/**
 * @file WindowRoutingComponent.h
 * @brief Component for managing window routing and background colors.
 * 
 * This file defines the WindowRoutingComponent class, which provides a user interface
 * for managing the routing of MIDI devices to display windows and customizing the
 * background colors of those windows.
 * 
 * The component displays a grid showing which MIDI devices are routed to which windows,
 * with toggle buttons for enabling/disabling routing and color buttons for customizing
 * window background colors.
 */

#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include "WindowManager.h"

namespace MidiPortal {

/**
 * @class WindowRoutingComponent
 * @brief Component for managing window routing and background colors.
 * 
 * This component displays a grid showing which MIDI devices are routed to which windows
 * and allows changing the background color of each window. It provides a user interface for:
 * - Creating new display windows
 * - Routing MIDI devices to specific windows
 * - Customizing window background colors
 * - Removing and recreating windows
 * 
 * The component works with the WindowManager to create and manage windows and
 * update routing information.
 */
class WindowRoutingComponent : public juce::Component,
                             private juce::Button::Listener,
                             private juce::Slider::Listener,
                             private juce::ChangeListener
{
public:
    /**
     * @brief Constructor that takes a reference to the WindowManager.
     * @param manager Reference to the WindowManager that will be used for window management.
     * 
     * Initializes the WindowRoutingComponent with a reference to the WindowManager
     * and sets up the initial UI state.
     */
    WindowRoutingComponent(WindowManager& manager);
    
    /**
     * @brief Default destructor.
     * 
     * The default destructor is sufficient since we use JUCE's OwnedArray for component management,
     * which automatically deletes the components when the array is destroyed.
     */
    ~WindowRoutingComponent() override = default;

    /**
     * @brief Paints the component.
     * @param g The Graphics context to paint into.
     * 
     * Draws the grid and colored bars representing the routing of MIDI devices to windows.
     */
    void paint(juce::Graphics& g) override;
    
    /**
     * @brief Handles component resizing.
     * 
     * Positions all child components based on the new size of the component.
     */
    void resized() override;

private:
    /**
     * @brief Handles button clicks.
     * @param button Pointer to the button that was clicked.
     * 
     * Called when a button is clicked, such as the New Window button or a routing cell.
     * Updates the routing information in the WindowManager accordingly.
     */
    void buttonClicked(juce::Button* button) override;
    
    /**
     * @brief Handles slider value changes.
     * @param slider Pointer to the slider that was changed.
     * 
     * Called when a slider value is changed, such as the RGB sliders for window colors.
     * Updates the window background color accordingly.
     */
    void sliderValueChanged(juce::Slider* slider) override;
    
    /**
     * @brief Handles change notifications.
     * @param source The ChangeBroadcaster that triggered the notification.
     * 
     * Called when a change notification is received, such as from the color selector.
     * Updates the window background color accordingly.
     */
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;
    
    /**
     * @brief Updates the grid with current windows and devices.
     * 
     * Rebuilds the grid of routing cells and color buttons based on the current
     * windows and devices in the WindowManager.
     */
    void updateGrid();
    
    /**
     * @brief Creates a new window with the next available letter name.
     * 
     * Creates a new window with a name like "A", "B", "C", etc., based on
     * the next available letter.
     */
    void createNewWindow();
    
    /**
     * @brief Shows the color selector for a specific window.
     * @param windowName The name of the window to show the color selector for.
     * 
     * Opens a color selector popup for changing the background color of the specified window.
     */
    void showColorSelectorForWindow(const juce::String& windowName);
    
    /**
     * @brief Applies the RGB slider values to update a window's background color.
     * @param windowName The name of the window to update the color for.
     * 
     * Gets the current RGB values from the sliders and updates the background color
     * of the specified window.
     */
    void applyRGBSlidersToWindow(const juce::String& windowName);

    /**
     * @brief Closes a window while preserving its settings.
     * @param windowName The name of the window to close.
     * 
     * Closes the specified window without removing it from the grid or forgetting
     * its settings. The window can be recreated later with the same name and settings.
     */
    void removeWindow(const juce::String& windowName);
    
    /**
     * @brief Recreates a previously closed window.
     * @param windowName The name of the window to recreate.
     * 
     * Recreates a window that was previously closed, using the same name and settings.
     */
    void recreateWindow(const juce::String& windowName);

    /**
     * @brief Reference to the WindowManager.
     * 
     * Used to create and manage windows and update routing information.
     */
    WindowManager& windowManager;
    
    /**
     * @brief Button to create new windows.
     * 
     * When clicked, creates a new window with the next available letter name.
     */
    juce::TextButton newWindowButton;
    
    /**
     * @class RoutingCell
     * @brief Toggle button for connecting devices to windows.
     * 
     * This class represents a cell in the routing grid that shows whether a device
     * is routed to a window. It's a toggle button that can be clicked to enable
     * or disable routing between a specific device and window.
     */
    struct RoutingCell : public juce::ToggleButton
    {
        /**
         * @brief Constructor that initializes a routing cell.
         * @param deviceName The name of the device for this cell.
         * @param windowName The name of the window for this cell.
         * 
         * Creates a new routing cell for the specified device and window.
         */
        RoutingCell(const juce::String& deviceName, const juce::String& windowName)
            : device(deviceName), window(windowName) {}
            
        /**
         * @brief The name of the device for this cell.
         * 
         * Used to identify which device this cell represents in the routing grid.
         */
        juce::String device;
        
        /**
         * @brief The name of the window for this cell.
         * 
         * Used to identify which window this cell represents in the routing grid.
         */
        juce::String window;
    };
    
    /**
     * @class RGBSliders
     * @brief Component for adjusting RGB color values.
     * 
     * This component contains three sliders (R, G, B) and an Apply button for
     * changing the background color of a window. Each slider controls one of
     * the RGB color components.
     */
    class RGBSliders : public juce::Component
    {
    public:
        /**
         * @brief Constructor that initializes the RGB sliders.
         * @param windowName The name of the window these sliders control.
         * @param initialColor The initial color to set the sliders to.
         * 
         * Creates a new set of RGB sliders for the specified window with the
         * specified initial color.
         */
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
        
        /**
         * @brief Handles component resizing.
         * 
         * Positions the sliders and button based on the new size of the component.
         */
        void resized() override
        {
            auto bounds = getLocalBounds();
            
            // Calculate heights to ensure Apply button is at the bottom
            int buttonHeight = 25;
            int sliderHeight = 20;
            int gap = 5;
            int totalSliderAreaHeight = bounds.getHeight() - buttonHeight - gap;
            
            // Ensure there's space for all sliders with equal gaps
            int availableHeight = totalSliderAreaHeight;
            int sliderSectionHeight = sliderHeight + gap;
            
            // Position the sliders vertically
            redSlider.setBounds(bounds.getX(), bounds.getY(), bounds.getWidth(), sliderHeight);
            availableHeight -= sliderSectionHeight;
            
            greenSlider.setBounds(bounds.getX(), bounds.getY() + sliderSectionHeight, 
                                 bounds.getWidth(), sliderHeight);
            availableHeight -= sliderSectionHeight;
            
            blueSlider.setBounds(bounds.getX(), bounds.getY() + 2 * sliderSectionHeight, 
                                bounds.getWidth(), sliderHeight);
            
            // Position the Apply button at the exact bottom
            applyButton.setBounds(bounds.getX(), bounds.getBottom() - buttonHeight, 
                                 bounds.getWidth(), buttonHeight);
        }
        
        /**
         * @brief Updates the color of the sliders.
         * @param newColor The new color to set the sliders to.
         * 
         * Updates the slider values to match the specified color.
         */
        void setColor(const juce::Colour& newColor)
        {
            // Store the new color
            color = newColor;
            
            // Update the slider values
            redSlider.setValue(newColor.getRed(), juce::dontSendNotification);
            greenSlider.setValue(newColor.getGreen(), juce::dontSendNotification);
            blueSlider.setValue(newColor.getBlue(), juce::dontSendNotification);
        }
        
        /**
         * @brief Gets the current color from the sliders.
         * @return The current color based on the slider values.
         * 
         * Creates a color from the current RGB slider values.
         */
        juce::Colour getCurrentColor() const
        {
            return juce::Colour(
                static_cast<juce::uint8>(redSlider.getValue()),
                static_cast<juce::uint8>(greenSlider.getValue()),
                static_cast<juce::uint8>(blueSlider.getValue())
            );
        }
        
        /**
         * @brief The name of the window these sliders control.
         * 
         * Used to identify which window's color is being adjusted.
         */
        juce::String window;
        
        /**
         * @brief The current color of the sliders.
         * 
         * Stores the current color represented by the slider values.
         */
        juce::Colour color;
        
        /**
         * @brief Slider for the red component of the color.
         * 
         * Controls the red component of the RGB color (0-255).
         */
        juce::Slider redSlider;
        
        /**
         * @brief Slider for the green component of the color.
         * 
         * Controls the green component of the RGB color (0-255).
         */
        juce::Slider greenSlider;
        
        /**
         * @brief Slider for the blue component of the color.
         * 
         * Controls the blue component of the RGB color (0-255).
         */
        juce::Slider blueSlider;
        
        /**
         * @brief Button to apply the color changes.
         * 
         * When clicked, applies the current slider values to the window's background color.
         */
        juce::TextButton applyButton;
    };
    
    /**
     * @class ColorButton
     * @brief Button for changing window background color.
     * 
     * This component displays a button with the current background color of a window
     * and opens a color selector when clicked.
     */
    class ColorButton : public juce::Component
    {
    public:
        /**
         * @brief Constructor that initializes a color button.
         * @param windowName The name of the window this button controls.
         * @param initialColor The initial color of the button.
         * 
         * Creates a new color button for the specified window with the specified initial color.
         */
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
        
        /**
         * @brief Handles component resizing.
         * 
         * Makes the button fill the entire component.
         */
        void resized() override
        {
            // Make the button fill the entire component
            button.setBounds(getLocalBounds());
        }
        
        /**
         * @brief Updates the color of the button.
         * @param newColor The new color to set the button to.
         * 
         * Updates the button's color and text color to match the specified color.
         */
        void setColor(const juce::Colour& newColor)
        {
            // Store the new color
            color = newColor;
            
            // Update the button colors
            button.setColour(juce::TextButton::buttonColourId, color);
            button.setColour(juce::TextButton::textColourOffId, getContrastingColor(color));
        }
        
        /**
         * @brief Gets a contrasting text color based on background brightness.
         * @param backgroundColor The background color to get a contrasting color for.
         * @return Black for bright backgrounds, white for dark backgrounds.
         * 
         * Calculates the perceived brightness of the background color and returns
         * a contrasting text color (black or white) for optimal readability.
         */
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
        
        /**
         * @brief The name of the window this button controls.
         * 
         * Used to identify which window's color is being adjusted.
         */
        juce::String window;
        
        /**
         * @brief The current color of the button.
         * 
         * Stores the current color of the button, which represents the window's background color.
         */
        juce::Colour color;
        
        /**
         * @brief The actual button component.
         * 
         * The TextButton that is displayed and can be clicked to open the color selector.
         */
        juce::TextButton button;
        
        /**
         * @brief Callback function for when the button is clicked.
         * 
         * This callback is triggered when the button is clicked, allowing
         * the parent component to show the color selector.
         */
        std::function<void()> onClick;
    };
    
    /**
     * @brief Array of labels for device names.
     * 
     * These labels display the names of MIDI devices in the routing grid.
     */
    juce::OwnedArray<juce::Label> deviceLabels;
    
    /**
     * @brief Array of labels for window names.
     * 
     * These labels display the names of windows in the routing grid.
     */
    juce::OwnedArray<juce::Label> windowLabels;
    
    /**
     * @brief Array of routing cells.
     * 
     * These toggle buttons represent the routing of devices to windows in the grid.
     */
    juce::OwnedArray<RoutingCell> routingCells;
    
    /**
     * @brief Array of color buttons.
     * 
     * These buttons display and allow changing the background colors of windows.
     */
    juce::OwnedArray<ColorButton> colorButtons;
    
    /**
     * @brief Array of RGB sliders.
     * 
     * These components allow adjusting the RGB values of window background colors.
     */
    juce::OwnedArray<RGBSliders> rgbSliders;
    
    /**
     * @brief Popup for the color selector.
     * 
     * This callout box contains the color selector when it's shown.
     */
    std::unique_ptr<juce::CallOutBox> colorSelectorCallout;
    
    /**
     * @brief Color selector for choosing window background colors.
     * 
     * This component allows selecting a color using various color models.
     */
    std::unique_ptr<juce::ColourSelector> currentColorSelector;
    
    /**
     * @brief Name of the window currently being color-selected.
     * 
     * Stores the name of the window whose color is currently being adjusted
     * with the color selector.
     */
    juce::String currentWindowForColorSelection;
    
    /**
     * @class WindowRemoveButton
     * @brief Button for removing or recreating windows.
     * 
     * This component displays a button that allows removing (closing) a window
     * or recreating a previously closed window. The button's text and function
     * toggle between "Remove" and "Recreate" based on whether the window is currently open.
     */
    class WindowRemoveButton : public juce::TextButton
    {
    public:
        /**
         * @brief Constructor that initializes a window remove/recreate button.
         * @param windowName The name of the window this button controls.
         * @param isWindowOpen Whether the window is currently open.
         * 
         * Creates a new button for removing or recreating the specified window.
         */
        WindowRemoveButton(const juce::String& windowName, bool isWindowOpen)
            : window(windowName), isOpen(isWindowOpen)
        {
            updateButtonText();
        }
        
        /**
         * @brief Updates the button text based on the window state.
         * 
         * Sets the button text to "Remove" if the window is open, or "Recreate" if it's closed.
         */
        void updateButtonText()
        {
            setButtonText(isOpen ? "Remove" : "Recreate");
        }
        
        /**
         * @brief Toggles the window state between open and closed.
         * 
         * Changes the window state and updates the button text accordingly.
         */
        void toggleWindowState()
        {
            isOpen = !isOpen;
            updateButtonText();
        }
        
        /**
         * @brief Checks if the window is currently open.
         * @return true if the window is open, false if it's closed.
         * 
         * Returns the current state of the window.
         */
        bool isWindowOpen() const
        {
            return isOpen;
        }
        
        /**
         * @brief The name of the window this button controls.
         * 
         * Used to identify which window to remove or recreate.
         */
        juce::String window;
        
    private:
        /**
         * @brief Whether the window is currently open.
         * 
         * Tracks whether the window is open (true) or closed (false).
         */
        bool isOpen;
    };
    
    /**
     * @brief Array of remove/recreate buttons.
     * 
     * These buttons allow removing and recreating windows in the grid.
     */
    juce::OwnedArray<WindowRemoveButton> windowRemoveButtons;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WindowRoutingComponent)
};

} // namespace MidiPortal 