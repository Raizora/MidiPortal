#include "WindowRoutingComponent.h"

namespace MidiPortal {

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

void WindowRoutingComponent::paint(juce::Graphics& g)
{
    // Fill the background with the default window background color
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    
    // Draw grid lines in grey
    g.setColour(juce::Colours::grey);
    
    // Get the component bounds, reduced by 10 pixels on all sides for padding
    auto bounds = getLocalBounds().reduced(10);
    
    // Reserve space at the top for the header (New Window button)
    auto headerArea = bounds.removeFromTop(40);
    
    // The remaining area is for the grid
    auto gridArea = bounds;
    
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
    
    // Draw vertical grid lines
    for (int i = 0; i <= numWindows; ++i)
    {
        int x = gridArea.getX() + i * cellWidth;
        g.drawLine(x, gridArea.getY(), x, gridArea.getBottom());
    }
    
    // Draw horizontal grid lines
    for (int i = 0; i <= numDevices + 1; ++i)
    {
        int y = gridArea.getY() + i * cellHeight;
        g.drawLine(gridArea.getX(), y, gridArea.getRight(), y);
    }
    
    // Draw colored bars below each window name
    // These bars represent the window's color scheme
    for (int i = 0; i < numWindows; ++i)
    {
        // Calculate the position for the color bars
        int x = gridArea.getX() + i * cellWidth;
        int y = gridArea.getY() + cellHeight - 30; // Position below the window name
        int barWidth = cellWidth - 20; // Make it slightly narrower than the cell
        int barHeight = 5; // Height of each color bar
        
        // Draw red bar
        g.setColour(juce::Colours::red);
        g.fillRect(x + 10, y, barWidth, barHeight);
        
        // Draw yellow bar below red
        g.setColour(juce::Colours::yellow);
        g.fillRect(x + 10, y + barHeight, barWidth, barHeight);
        
        // Draw green bar below yellow
        g.setColour(juce::Colours::green);
        g.fillRect(x + 10, y + barHeight * 2, barWidth, barHeight);
        
        // Draw blue bar below green
        g.setColour(juce::Colours::blue);
        g.fillRect(x + 10, y + barHeight * 3, barWidth, barHeight);
    }
    
}

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
    int cellHeight = bounds.getHeight() / (numDevices + 1);
    
    // Position window labels (column headers)
    // Each window gets its own column
    for (int i = 0; i < numWindows; ++i)
    {
        // Position the window label in the center of its cell
        windowLabels[i]->setBounds(bounds.getX() + i * cellWidth,
                                 bounds.getY(),
                                 cellWidth,
                                 cellHeight);
    }
    
    // Position device labels (row headers)
    // Each device gets its own row
    for (int i = 0; i < numDevices; ++i)
    {
        // Position the device label in the first column of its row
        deviceLabels[i]->setBounds(bounds.getX(),
                                 bounds.getY() + (i + 1) * cellHeight,
                                 cellWidth,
                                 cellHeight);
    }
    
    // Position routing cells (toggle buttons)
    // Each cell represents a connection between a device and a window
    // Note: We don't have toggle buttons for the MAIN window
    int cellIndex = 0;
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
            // Position the cell at the intersection of the window column and device row
            cell->setBounds(bounds.getX() + windowIndex * cellWidth,
                          bounds.getY() + (deviceIndex + 1) * cellHeight,
                          cellWidth,
                          cellHeight);
        }
        
        cellIndex++;
    }
    
    // Position color buttons below the grid
    // Each window gets a color button for changing its background color
    for (int i = 0; i < colorButtons.size(); ++i)
    {
        // Find the window index for this color button
        int windowIndex = -1;
        for (int j = 0; j < windowLabels.size(); ++j)
        {
            if (windowLabels[j]->getText() == colorButtons[i]->window)
            {
                windowIndex = j;
                break;
            }
        }
        
        // If the window was found, position the color button
        if (windowIndex >= 0)
        {
            // Position the button below the grid, in the same column as its window
            int x = bounds.getX() + windowIndex * cellWidth + 10; // Add 10px padding
            int y = bounds.getY() + (numDevices + 1) * cellHeight + 30; // Add 30px space
            
            // Make the button slightly narrower than the cell
            colorButtons[i]->setBounds(x, y, cellWidth - 20, 30);
        }
    }
}

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
    // Note: ColorButton clicks are now handled through the onClick lambda
}

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
        
        // Update the window background color in the settings manager
        auto& settingsManager = windowManager.getSettingsManager();
        
        if (currentWindowForColorSelection == "MAIN")
        {
            // For the MAIN window, update the Default settings
            // This affects the main application window's background color
            auto defaultSettings = settingsManager.getSettings("Default");
            defaultSettings.backgroundColor = newColor;
            settingsManager.setSettings(defaultSettings, "Default");
            
            // Also update any existing windows that might be using the Default settings
            for (const auto& windowName : windowManager.getWindowNames())
            {
                if (windowName != "MAIN")
                {
                    auto windowSettings = settingsManager.getSettings(windowName);
                    // Only update if this window is using the default background color
                    if (windowSettings.backgroundColor == defaultSettings.backgroundColor)
                    {
                        windowSettings.backgroundColor = newColor;
                        settingsManager.setSettings(windowSettings, windowName);
                    }
                }
            }
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

void WindowRoutingComponent::updateGrid()
{
    // Clear all existing components
    // This is necessary when rebuilding the grid
    deviceLabels.clear();
    windowLabels.clear();
    routingCells.clear();
    colorButtons.clear();
    
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
    
    // Create color buttons for each window
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
        
    // Create the new window using the window manager
    windowManager.createWindow(newName);
    
    // Update the grid to include the new window
    updateGrid();
}

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

} // namespace MidiPortal 