#include "WindowRoutingComponent.h"

namespace MidiPortal {

WindowRoutingComponent::WindowRoutingComponent(WindowManager& manager)
    : windowManager(manager)
{
    // Set up the component
    setSize(400, 300);
    
    newWindowButton.setButtonText("New Window");
    newWindowButton.addListener(this);
    addAndMakeVisible(newWindowButton);
    
    updateGrid();
}

void WindowRoutingComponent::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    
    // Draw grid lines
    g.setColour(juce::Colours::grey);
    
    auto bounds = getLocalBounds().reduced(10);
    auto gridArea = bounds.removeFromBottom(bounds.getHeight() - 40);
    
    int cellWidth = gridArea.getWidth() / (windowLabels.size() + 1);
    int cellHeight = gridArea.getHeight() / (deviceLabels.size() + 1);
    
    // Draw vertical lines
    for (int i = 0; i <= windowLabels.size() + 1; ++i)
    {
        int x = gridArea.getX() + i * cellWidth;
        g.drawLine(x, gridArea.getY(), x, gridArea.getBottom());
    }
    
    // Draw horizontal lines
    for (int i = 0; i <= deviceLabels.size() + 1; ++i)
    {
        int y = gridArea.getY() + i * cellHeight;
        g.drawLine(gridArea.getX(), y, gridArea.getRight(), y);
    }
}

void WindowRoutingComponent::resized()
{
    auto bounds = getLocalBounds().reduced(10);
    
    // Position new window button at top
    newWindowButton.setBounds(bounds.removeFromTop(30));
    bounds.removeFromTop(10);
    
    // Calculate grid dimensions
    int numWindows = windowLabels.size();
    int numDevices = deviceLabels.size();
    
    if (numWindows == 0 || numDevices == 0)
        return;
        
    int cellWidth = bounds.getWidth() / (numWindows + 1);
    int cellHeight = bounds.getHeight() / (numDevices + 1);
    
    // Position window labels (columns)
    for (int i = 0; i < numWindows; ++i)
    {
        windowLabels[i]->setBounds(bounds.getX() + (i + 1) * cellWidth,
                                 bounds.getY(),
                                 cellWidth,
                                 cellHeight);
    }
    
    // Position device labels (rows)
    for (int i = 0; i < numDevices; ++i)
    {
        deviceLabels[i]->setBounds(bounds.getX(),
                                 bounds.getY() + (i + 1) * cellHeight,
                                 cellWidth,
                                 cellHeight);
    }
    
    // Position routing cells
    for (auto* cell : routingCells)
    {
        int windowIndex = -1;
        int deviceIndex = -1;
        
        // Find window index
        for (int i = 0; i < windowLabels.size(); ++i)
        {
            if (windowLabels[i]->getText() == cell->window)
            {
                windowIndex = i;
                break;
            }
        }
        
        // Find device index
        for (int i = 0; i < deviceLabels.size(); ++i)
        {
            if (deviceLabels[i]->getText() == cell->device)
            {
                deviceIndex = i;
                break;
            }
        }
        
        if (windowIndex >= 0 && deviceIndex >= 0)
        {
            cell->setBounds(bounds.getX() + (windowIndex + 1) * cellWidth,
                          bounds.getY() + (deviceIndex + 1) * cellHeight,
                          cellWidth,
                          cellHeight);
        }
    }
}

void WindowRoutingComponent::buttonClicked(juce::Button* button)
{
    if (button == &newWindowButton)
    {
        createNewWindow();
    }
    else if (auto* cell = dynamic_cast<RoutingCell*>(button))
    {
        if (cell->getToggleState())
            windowManager.routeDeviceToWindow(cell->device, cell->window);
        else
            windowManager.unrouteDeviceFromWindow(cell->device, cell->window);
    }
}

void WindowRoutingComponent::updateGrid()
{
    // Clear existing components
    deviceLabels.clear();
    windowLabels.clear();
    routingCells.clear();
    
    // Get current windows and devices
    auto windows = windowManager.getWindowNames();
    auto midiInputs = juce::MidiInput::getAvailableDevices();
    
    // Create window labels
    for (const auto& window : windows)
    {
        auto* label = new juce::Label();
        label->setText(window, juce::dontSendNotification);
        label->setJustificationType(juce::Justification::centred);
        addAndMakeVisible(label);
        windowLabels.add(label);
    }
    
    // Create device labels
    for (const auto& device : midiInputs)
    {
        auto* label = new juce::Label();
        label->setText(device.name, juce::dontSendNotification);
        label->setJustificationType(juce::Justification::centred);
        addAndMakeVisible(label);
        deviceLabels.add(label);
    }
    
    // Create routing cells
    for (const auto& device : midiInputs)
    {
        for (const auto& window : windows)
        {
            auto* cell = new RoutingCell(device.name, window);
            cell->setToggleState(windowManager.isDeviceRoutedToWindow(device.name, window),
                               juce::dontSendNotification);
            cell->addListener(this);
            addAndMakeVisible(cell);
            routingCells.add(cell);
        }
    }
    
    resized();
}

void WindowRoutingComponent::createNewWindow()
{
    // Generate next window name (A, B, C, etc.)
    auto windows = windowManager.getWindowNames();
    juce::String newName;
    
    for (char c = 'A'; c <= 'Z'; ++c)
    {
        newName = juce::String::charToString(c);
        if (!windows.contains(newName))
            break;
    }
    
    if (newName.isEmpty())
        return; // All letters used
        
    windowManager.createWindow(newName);
    updateGrid();
}

} // namespace MidiPortal 