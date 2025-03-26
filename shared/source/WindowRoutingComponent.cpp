/**
 * @file WindowRoutingComponent.cpp
 * @brief Implementation of the WindowRoutingComponent class.
 * 
 * This file contains the implementation of the WindowRoutingComponent class methods,
 * which provide a user interface for managing the routing of MIDI devices to display
 * windows and customizing the background colors of those windows.
 * 
 * The component displays a grid showing which MIDI devices are routed to which windows,
 * with toggle buttons for enabling/disabling routing and RGB sliders for customizing
 * window background colors.
 */

#include "WindowRoutingComponent.h"

namespace MidiPortal {

/**
 * @brief Constructor that initializes the component with a reference to the WindowManager.
 * @param manager Reference to the WindowManager that will be used for window management.
 * 
 * Sets up the "New Window" button and initializes the grid with current windows and devices.
 * The grid displays which MIDI devices are routed to which windows and provides controls
 * for changing routing and window background colors.
 */
WindowRoutingComponent::WindowRoutingComponent(WindowManager& manager)
    : windowManager(manager)
{
    // Set up the component with a reasonable initial size
    // This will be the size of the window when it first opens
    setSize(600, 400);
    
    // Create and configure the "New Window" button
    // This button will create new windows (A, B, C, etc.) when clicked
    newWindowButton.setButtonText("New Window");
    newWindowButton.addListener(this); // Register this class as a listener for button clicks
    addAndMakeVisible(newWindowButton); // Make the button visible in the component
    
    // Initialize the grid with current windows and devices
    updateGrid();
}

/**
 * @brief Paints the component background and grid lines.
 * @param g The Graphics context to paint into.
 * 
 * Fills the background with the default window background color and draws
 * grid lines to separate the cells in the routing grid. The grid has one column
 * for each window and one row for each MIDI device.
 */
void WindowRoutingComponent::paint(juce::Graphics& g)
{
    // Fill the background with the default window background color
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    
    // Draw grid lines in grey
    g.setColour(juce::Colours::grey);
    
    // Get the component bounds, reduced by 10 pixels on all sides for padding
    auto bounds = getLocalBounds().reduced(10);
    
    // The remaining area is for the grid
    auto gridArea = bounds;
    gridArea.removeFromTop(40); // 30px for button + 10px spacing
    
    // Calculate grid dimensions based on number of windows and devices
    int numDevices = deviceLabels.size();
    int numWindows = windowLabels.size();
    
    // If there are no windows or devices, don't draw the grid
    if (numDevices == 0 || numWindows == 0)
        return;
    
    // Calculate cell dimensions
    // The grid has numWindows columns and numDevices+1 rows (extra row for window names)
    int cellWidth = gridArea.getWidth() / numWindows;
    int cellHeight = gridArea.getHeight() / (numDevices + 1);
    
    // Reserve space for window labels and RGB sliders
    int windowLabelHeight = 30;
    int windowLabelPadding = 5;
    int totalWindowLabelHeight = windowLabelHeight + windowLabelPadding;
    
    int rgbSliderHeight = 100;
    int rgbSliderPadding = 10;
    int totalRgbHeight = rgbSliderHeight + rgbSliderPadding;
    
    // Define areas for different sections
    auto windowLabelArea = gridArea.removeFromTop(totalWindowLabelHeight);
    auto rgbArea = gridArea.removeFromTop(totalRgbHeight);
    auto deviceArea = gridArea;
    
    // Draw vertical grid lines - these span the entire height
    for (int i = 0; i <= numWindows; ++i)
    {
        int x = gridArea.getX() + i * cellWidth;
        g.drawLine(x, bounds.getY() + 40, x, bounds.getBottom()); // Start after button area
    }
    
    // Draw horizontal grid lines with proper section spacing
    
    // Line below "New Window" button (already handled by gridArea starting point)
    
    // Line below window labels
    g.drawLine(gridArea.getX(), windowLabelArea.getBottom(), gridArea.getRight(), windowLabelArea.getBottom());
    
    // Line below RGB sliders - this is the line that should align with Apply buttons
    g.drawLine(gridArea.getX(), rgbArea.getBottom(), gridArea.getRight(), rgbArea.getBottom());
    
    // Device section grid lines
    if (numDevices > 0) {
        int deviceCellHeight = deviceArea.getHeight() / numDevices;
        for (int i = 1; i <= numDevices; ++i) {
            int y = deviceArea.getY() + i * deviceCellHeight;
            g.drawLine(deviceArea.getX(), y, deviceArea.getRight(), y);
        }
    }
}

/**
 * @brief Handles component resizing and positions all child components.
 * 
 * Positions the "New Window" button, window labels, remove/recreate buttons,
 * device labels, routing cells, and RGB sliders based on the new size of the component.
 */
void WindowRoutingComponent::resized()
{
    // Get the component bounds, reduced by 10 pixels on all sides for padding
    auto bounds = getLocalBounds().reduced(10);
    
    // Position the "New Window" button at the top
    // The button takes up 30 pixels in height
    newWindowButton.setBounds(bounds.removeFromTop(30));
    
    // Add 10 pixels of space below the button
    bounds.removeFromTop(10);
    
    // Calculate grid dimensions
    int numDevices = deviceLabels.size();
    int numWindows = windowLabels.size();
    
    // If there are no windows or devices, don't position anything
    if (numDevices == 0 || numWindows == 0)
        return;
        
    // Calculate cell dimensions
    // The grid has numWindows columns and numDevices+1 rows (extra row for window names)
    int cellWidth = bounds.getWidth() / numWindows;
    
    // Reserve space for window labels (30px height) and some padding
    int windowLabelHeight = 30;
    int windowLabelPadding = 5;
    int totalWindowLabelHeight = windowLabelHeight + windowLabelPadding;
    
    // Reserve space for RGB sliders (100px height) and some padding
    int rgbSliderHeight = 100;
    int rgbSliderPadding = 10;
    int totalRgbHeight = rgbSliderHeight + rgbSliderPadding;
    
    // Create section areas
    auto windowLabelArea = bounds.removeFromTop(totalWindowLabelHeight);
    auto rgbArea = bounds.removeFromTop(totalRgbHeight);
    auto deviceArea = bounds;
    
    // Calculate the cell height for device rows - ensure it fills the remaining space evenly
    int deviceCellHeight = numDevices > 0 ? deviceArea.getHeight() / numDevices : 0;
    
    // Position window labels (column headers)
    for (int i = 0; i < numWindows; ++i)
    {
        // Position the window label, leaving space for remove button if needed
        if (i == 0) // MAIN window
        {
            // Full width for MAIN
            windowLabels[i]->setBounds(windowLabelArea.getX() + i * cellWidth,
                                     windowLabelArea.getY(),
                                     cellWidth,
                                     windowLabelHeight);
        }
        else // Other windows (A, B, C, etc.)
        {
            // Reduced width to make room for remove button
            int labelWidth = cellWidth - 90; // Leave 90px for button (increased from 75px)
            windowLabels[i]->setBounds(windowLabelArea.getX() + i * cellWidth,
                                     windowLabelArea.getY(),
                                     labelWidth,
                                     windowLabelHeight);
                                     
            // Position the remove/recreate button next to the label
            int buttonIndex = i - 1; // Adjust index since MAIN has no button
            if (buttonIndex < windowRemoveButtons.size())
            {
                windowRemoveButtons[buttonIndex]->setBounds(
                    windowLabelArea.getX() + i * cellWidth + labelWidth,
                    windowLabelArea.getY() + 1, // Reduced vertical offset from 2 to 1
                    80, // 80px width (increased from 65px)
                    windowLabelHeight - 2 // Less vertical padding (reduced from 4 to 2)
                );
            }
        }
    }
    
    // Position device labels (row headers)
    for (int i = 0; i < numDevices; ++i)
    {
        deviceLabels[i]->setBounds(deviceArea.getX(),
                                  deviceArea.getY() + i * deviceCellHeight,
                                  cellWidth,
                                  deviceCellHeight);
    }
    
    // Position routing cells (toggle buttons)
    for (auto* cell : routingCells)
    {
        // Find the window and device indices for this cell
        int windowIndex = -1;
        int deviceIndex = -1;
        
        // Find the window index by matching the window name
        for (int i = 0; i < windowLabels.size(); ++i)
        {
            if (windowLabels[i]->getText() == cell->window)
            {
                windowIndex = i;
                break;
            }
        }
        
        // Find the device index by matching the device name
        for (int i = 0; i < deviceLabels.size(); ++i)
        {
            if (deviceLabels[i]->getText() == cell->device)
            {
                deviceIndex = i;
                break;
            }
        }
        
        // If both window and device were found, position the cell
        if (windowIndex >= 0 && deviceIndex >= 0)
        {
            cell->setBounds(deviceArea.getX() + windowIndex * cellWidth,
                           deviceArea.getY() + deviceIndex * deviceCellHeight,
                           cellWidth,
                           deviceCellHeight);
        }
    }
    
    // Position RGB sliders with proper alignment to ensure Apply buttons are at the exact bottom of their section
    for (int i = 0; i < rgbSliders.size(); ++i)
    {
        int windowIndex = -1;
        for (int j = 0; j < windowLabels.size(); ++j)
        {
            if (windowLabels[j]->getText() == rgbSliders[i]->window)
            {
                windowIndex = j;
                break;
            }
        }
        
        if (windowIndex >= 0)
        {
            // Position the sliders in the RGB area, ensuring the Apply button will align with the bottom grid line
            int x = rgbArea.getX() + windowIndex * cellWidth + 10; // Add 10px padding
            int y = rgbArea.getY();
            int width = cellWidth - 20;
            
            // Make sure the RGB slider component is exactly the right height to position the Apply button at the bottom
            rgbSliders[i]->setBounds(x, y, width, rgbSliderHeight);
        }
    }
    
    // We no longer need the color buttons
    for (auto* btn : colorButtons)
    {
        btn->setBounds(-1000, -1000, 1, 1);
    }
    
    // Repaint to ensure grid lines are redrawn in the correct positions
    repaint();
}

/**
 * @brief Handles button clicks from buttons in the routing grid.
 * @param button Pointer to the button that was clicked.
 * 
 * Handles different actions based on the button that was clicked:
 * - If the "New Window" button was clicked, creates a new window
 * - If a routing cell was clicked, updates the routing between the device and window
 * - If an Apply button from an RGB slider was clicked, applies the color to the window
 * - If a remove/recreate button was clicked, removes or recreates the associated window
 */
void WindowRoutingComponent::buttonClicked(juce::Button* button)
{
    // Handle button clicks based on the button type
    
    if (button == &newWindowButton)
    {
        // If the "New Window" button was clicked, create a new window
        createNewWindow();
    }
    else if (auto* cell = dynamic_cast<RoutingCell*>(button))
    {
        // If a routing cell was clicked, update the routing
        if (cell->getToggleState())
        {
            // If the toggle is now on, route the device to the window
            windowManager.routeDeviceToWindow(cell->device, cell->window);
        }
        else
        {
            // If the toggle is now off, unroute the device from the window
            windowManager.unrouteDeviceFromWindow(cell->device, cell->window);
        }
    }
    else if (auto* removeButton = dynamic_cast<WindowRemoveButton*>(button))
    {
        // Handle remove/recreate button clicks
        if (removeButton->isWindowOpen())
        {
            // Remove (close) the window
            removeWindow(removeButton->window);
        }
        else
        {
            // Recreate the window
            recreateWindow(removeButton->window);
        }
        
        // Animation is handled inside removeWindow/recreateWindow methods
    }
    else
    {
        // Check if it's an Apply button from one of the RGB sliders
        for (auto* slider : rgbSliders)
        {
            if (button == &slider->applyButton)
            {
                // Apply the RGB slider values to update the window's background color
                applyRGBSlidersToWindow(slider->window);
                break;
            }
        }
    }
}

/**
 * @brief Handles slider value changes from the RGB sliders.
 * @param slider Pointer to the slider that changed.
 * 
 * This method is called when one of the RGB sliders is adjusted.
 * No action is taken here since colors are only applied when the Apply button is clicked.
 */
void WindowRoutingComponent::sliderValueChanged(juce::Slider* slider)
{
    // X- Intentionally unused parameter as we only apply color changes when the Apply button is clicked
    juce::ignoreUnused(slider);
    
    // Handle slider value changes
    // We don't need to do anything here since we only apply the color when the Apply button is clicked
}

/**
 * @brief Handles change notifications from the color selector.
 * @param source The ChangeBroadcaster that triggered the notification.
 * 
 * Called when the color selector's color changes. Updates the color button and RGB sliders
 * for the selected window, and applies the new color to the window's background.
 */
void WindowRoutingComponent::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    // Handle changes from the color selector
    if (currentColorSelector.get() == source && !currentWindowForColorSelection.isEmpty())
    {
        // Get the selected color from the color selector
        juce::Colour newColor = currentColorSelector->getCurrentColour();
        
        // Update the color button for the selected window
        for (auto* btn : colorButtons)
        {
            if (btn->window == currentWindowForColorSelection)
            {
                btn->setColor(newColor);
                break;
            }
        }
        
        // Update the RGB sliders for the selected window
        for (auto* slider : rgbSliders)
        {
            if (slider->window == currentWindowForColorSelection)
            {
                slider->setColor(newColor);
                break;
            }
        }
        
        // Update the window background color in the settings manager
        auto& settingsManager = windowManager.getSettingsManager();
        
        if (currentWindowForColorSelection == "MAIN")
        {
            // For the MAIN window, update only the Default settings
            // This affects only the main application window's background color
            auto defaultSettings = settingsManager.getSettings("Default");
            defaultSettings.backgroundColor = newColor;
            settingsManager.setSettings(defaultSettings, "Default");
        }
        else
        {
            // For other windows, just update their specific settings
            auto settings = settingsManager.getSettings(currentWindowForColorSelection);
            settings.backgroundColor = newColor;
            settingsManager.setSettings(settings, currentWindowForColorSelection);
        }
    }
}

/**
 * @brief Updates the grid with current windows and devices.
 * 
 * Clears all existing components and rebuilds the grid based on the current
 * windows and MIDI devices. Creates labels for windows and devices, routing cells
 * for device-window combinations, RGB sliders for window background colors, and
 * remove/recreate buttons for closing and reopening windows.
 */
void WindowRoutingComponent::updateGrid()
{
    // Clear all existing components
    // This is necessary when rebuilding the grid
    deviceLabels.clear();
    windowLabels.clear();
    routingCells.clear();
    colorButtons.clear();
    rgbSliders.clear();
    windowRemoveButtons.clear();
    
    // Get current windows and devices from the window manager
    auto windows = windowManager.getWindowNames();
    auto midiInputs = juce::MidiInput::getAvailableDevices();
    
    // Ensure MAIN is the first window
    // This is important for the layout
    if (windows.size() > 0 && windows[0] != "MAIN")
    {
        windows.move(windows.indexOf("MAIN"), 0);
    }
    
    // Create window labels for each window
    for (const auto& window : windows)
    {
        // Create a new label for the window
        auto* label = new juce::Label();
        label->setText(window, juce::dontSendNotification);
        label->setJustificationType(juce::Justification::centred);
        addAndMakeVisible(label);
        windowLabels.add(label);
        
        // Create remove/recreate buttons for each window except MAIN
        if (window != "MAIN")
        {
            // Check if the window is currently open (visible)
            bool isWindowOpen = windowManager.isWindowOpen(window);
            
            // Create a new remove/recreate button for this window
            auto* removeButton = new WindowRemoveButton(window, isWindowOpen);
            removeButton->addListener(this);
            addAndMakeVisible(removeButton);
            windowRemoveButtons.add(removeButton);
        }
    }
    
    // Create device labels for each MIDI input device
    for (const auto& device : midiInputs)
    {
        // Create a new label for the device
        auto* label = new juce::Label();
        label->setText(device.name, juce::dontSendNotification);
        label->setJustificationType(juce::Justification::centred);
        addAndMakeVisible(label);
        deviceLabels.add(label);
    }
    
    // Create routing cells for each device-window combination
    // Skip creating toggle buttons for the MAIN window since all devices
    // are automatically routed to it
    for (const auto& device : midiInputs)
    {
        for (int i = 0; i < windows.size(); ++i)
        {
            const auto& window = windows[i];
            
            // Skip creating toggle buttons for the MAIN window (index 0)
            if (window == "MAIN")
                continue;
                
            // Create a new toggle button for this device-window combination
            auto* cell = new RoutingCell(device.name, window);
            
            // Set the initial state based on current routing
            cell->setToggleState(windowManager.isDeviceRoutedToWindow(device.name, window),
                               juce::dontSendNotification);
            
            // Register this class as a listener for button clicks
            cell->addListener(this);
            
            // Make the cell visible
            addAndMakeVisible(cell);
            
            // Add the cell to the list
            routingCells.add(cell);
        }
    }
    
    // Create RGB sliders for each window
    auto& settingsManager = windowManager.getSettingsManager();
    for (const auto& window : windows)
    {
        // Get the current background color for this window
        juce::Colour bgColor;
        
        if (window == "MAIN")
        {
            // For MAIN window, use the Default settings
            // This is the main application window's background color
            bgColor = settingsManager.getSettings("Default").backgroundColor;
        }
        else
        {
            // For other windows, use their specific settings
            bgColor = settingsManager.getSettings(window).backgroundColor;
        }
        
        // Create a new RGB sliders component for this window
        auto* rgbSlider = new RGBSliders(window, bgColor);
        
        // Register this class as a listener for the sliders
        rgbSlider->redSlider.addListener(this);
        rgbSlider->greenSlider.addListener(this);
        rgbSlider->blueSlider.addListener(this);
        
        // Register this class as a listener for the Apply button
        rgbSlider->applyButton.addListener(this);
        
        // Make the component visible
        addAndMakeVisible(rgbSlider);
        
        // Add the component to the list
        rgbSliders.add(rgbSlider);
    }
    
    // Create color buttons for each window
    for (const auto& window : windows)
    {
        // Get the current background color for this window
        juce::Colour bgColor;
        
        if (window == "MAIN")
        {
            // For MAIN window, use the Default settings
            // This is the main application window's background color
            bgColor = settingsManager.getSettings("Default").backgroundColor;
        }
        else
        {
            // For other windows, use their specific settings
            bgColor = settingsManager.getSettings(window).backgroundColor;
        }
        
        // Create a new color button for this window
        auto* colorBtn = new ColorButton(window, bgColor);
        
        // Set up the click handler using a lambda function
        colorBtn->onClick = [this, window]() {
            showColorSelectorForWindow(window);
        };
        
        // Make the button visible
        addAndMakeVisible(colorBtn);
        
        // Add the button to the list
        colorButtons.add(colorBtn);
    }
    
    // Update the layout
    resized();
}

/**
 * @brief Creates a new window with a unique name.
 * 
 * Generates a new window name (A, B, C, etc.) that isn't already in use,
 * creates the window using the WindowManager, and updates the grid to include
 * the new window. Preserves existing window colors during the update.
 */
void WindowRoutingComponent::createNewWindow()
{
    // Generate next window name (A, B, C, etc.)
    auto windows = windowManager.getWindowNames();
    juce::String newName;
    
    // Try each letter from A to Z
    for (char c = 'A'; c <= 'Z'; ++c)
    {
        // Convert the character to a string
        newName = juce::String::charToString(c);
        
        // If this name is not already used, use it
        if (!windows.contains(newName))
            break;
    }
    
    // If all letters are used, don't create a new window
    if (newName.isEmpty())
        return;
        
    // Store existing window colors before creating a new window
    std::map<juce::String, juce::Colour> existingColors;
    auto& settingsManager = windowManager.getSettingsManager();
    
    for (const auto& window : windows)
    {
        if (window == "MAIN")
        {
            existingColors[window] = settingsManager.getSettings("Default").backgroundColor;
        }
        else
        {
            existingColors[window] = settingsManager.getSettings(window).backgroundColor;
        }
    }
    
    // Create the new window using the window manager
    windowManager.createWindow(newName);
    
    // Give a short delay to let the window creation finish
    juce::Thread::sleep(50);
    
    // Update the grid to include the new window
    updateGrid();
    
    // Restore existing window colors
    for (const auto& [window, color] : existingColors)
    {
        // Find the RGB sliders for this window
        for (auto* slider : rgbSliders)
        {
            if (slider->window == window)
            {
                // Set the color in the sliders
                slider->setColor(color);
                
                // Apply the color to the window
                applyRGBSlidersToWindow(window);
                break;
            }
        }
    }
    
    // Force a resize to ensure all components are properly positioned
    resized();
}

/**
 * @brief Shows a color selector for a specific window.
 * @param windowName The name of the window to show the color selector for.
 * 
 * Creates a color selector popup next to the color button for the specified window.
 * The color selector allows the user to choose a new background color for the window.
 */
void WindowRoutingComponent::showColorSelectorForWindow(const juce::String& windowName)
{
    // Find the color button for this window
    ColorButton* targetButton = nullptr;
    for (auto* btn : colorButtons)
    {
        if (btn->window == windowName)
        {
            targetButton = btn;
            break;
        }
    }
    
    // If the button wasn't found, don't show the color selector
    if (targetButton == nullptr)
        return;
    
    // Create a color selector with RGB sliders and color space
    currentColorSelector = std::make_unique<juce::ColourSelector>(
        juce::ColourSelector::showColourAtTop | 
        juce::ColourSelector::showSliders | 
        juce::ColourSelector::showColourspace);
    
    // Set the initial color to the current window background color
    currentColorSelector->setCurrentColour(targetButton->color);
    
    // Set the size of the color selector
    currentColorSelector->setSize(300, 400);
    
    // Register this class as a listener for color changes
    currentColorSelector->addChangeListener(this);
    
    // Store the window name for later use
    currentWindowForColorSelection = windowName;
    
    // Create a callout box to display the color selector
    // This creates a popup that appears next to the button
    colorSelectorCallout = std::make_unique<juce::CallOutBox>(
        *currentColorSelector, 
        targetButton->getBounds(), 
        this);
    
    // Ensure clicks on the callout are consumed
    // This prevents clicks from passing through to components underneath
    colorSelectorCallout->setDismissalMouseClicksAreAlwaysConsumed(true);
}

/**
 * @brief Applies the RGB slider values to a window's background color.
 * @param windowName The name of the window to apply the color to.
 * 
 * Gets the current color from the RGB sliders for the specified window,
 * updates the color button to match, and applies the color to the window's
 * background by updating the settings in the DisplaySettingsManager.
 */
void WindowRoutingComponent::applyRGBSlidersToWindow(const juce::String& windowName)
{
    // Find the RGB sliders for this window
    RGBSliders* targetSliders = nullptr;
    for (auto* slider : rgbSliders)
    {
        if (slider->window == windowName)
        {
            targetSliders = slider;
            break;
        }
    }
    
    // If the sliders weren't found, don't apply the color
    if (targetSliders == nullptr)
        return;
    
    // Get the current color from the sliders
    juce::Colour newColor = targetSliders->getCurrentColor();
    
    // Update the color button for this window
    for (auto* btn : colorButtons)
    {
        if (btn->window == windowName)
        {
            btn->setColor(newColor);
            break;
        }
    }
    
    // Update the window background color in the settings manager
    auto& settingsManager = windowManager.getSettingsManager();
    
    if (windowName == "MAIN")
    {
        // For the MAIN window, update only the Default settings
        // This affects only the main application window's background color
        auto defaultSettings = settingsManager.getSettings("Default");
        defaultSettings.backgroundColor = newColor;
        settingsManager.setSettings(defaultSettings, "Default");
    }
    else
    {
        // For other windows, just update their specific settings
        auto settings = settingsManager.getSettings(windowName);
        settings.backgroundColor = newColor;
        settingsManager.setSettings(settings, windowName);
    }
}

/**
 * @brief Closes a window while preserving its settings.
 * @param windowName The name of the window to close.
 * 
 * Closes the specified window without removing it from the grid or forgetting
 * its settings. The window can be recreated later with the same name and settings.
 */
void WindowRoutingComponent::removeWindow(const juce::String& windowName)
{
    // Don't allow removing the MAIN window
    if (windowName == "MAIN")
        return;
        
    // Close the window but keep its settings
    windowManager.closeWindow(windowName);
    
    // Update button appearance in the UI - find and update the button
    for (auto* btn : windowRemoveButtons)
    {
        if (btn->window == windowName)
        {
            btn->toggleStateWithAnimation();
            break;
        }
    }
}

/**
 * @brief Recreates a previously closed window.
 * @param windowName The name of the window to recreate.
 * 
 * Recreates a window that was previously closed, using the same name and settings.
 */
void WindowRoutingComponent::recreateWindow(const juce::String& windowName)
{
    // Don't attempt to recreate the MAIN window
    if (windowName == "MAIN")
        return;
        
    // Recreate the window with the same name and settings
    windowManager.reopenWindow(windowName);
    
    // Update button appearance in the UI - find and update the button
    for (auto* btn : windowRemoveButtons)
    {
        if (btn->window == windowName)
        {
            btn->toggleStateWithAnimation();
            break;
        }
    }
}

} // namespace MidiPortal 