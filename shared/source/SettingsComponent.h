#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <map>

class SettingsComponent : public juce::Component,
                         private juce::Timer,
                         private juce::Button::Listener
{
public:
    struct MidiFilterSettings {
        bool logNotes = true;
        bool logCC = true;
        bool logClock = true;
        bool logSysEx = true;
        // etc...
    };

    struct DeviceFilterSettings {
        std::map<juce::String, bool> deviceEnabled;  // device name -> enabled
        std::map<juce::String, bool> deviceLogging;  // device name -> logging enabled
    };

    SettingsComponent()
    {
        // Add control buttons
        addAndMakeVisible(rescanButton);
        addAndMakeVisible(applyButton);
        addAndMakeVisible(autoScanToggle);
        
        rescanButton.setColour(juce::TextButton::textColourOffId, juce::Colours::blueviolet);
        applyButton.setColour(juce::TextButton::textColourOffId, juce::Colours::blueviolet);
        autoScanToggle.setColour(juce::ToggleButton::textColourId, juce::Colours::blueviolet);
        
        rescanButton.addListener(this);
        applyButton.addListener(this);
        autoScanToggle.addListener(this);
        
        autoScanToggle.setToggleState(true, juce::dontSendNotification);
        
        // Set up viewport for scrolling
        addAndMakeVisible(viewport);
        viewport.setViewedComponent(new ContentComponent(), true);
        
        // Start checking for device changes
        startTimer(1000);  // Check every second
        
        // Get content size and constrain to reasonable limits
        auto* content = dynamic_cast<ContentComponent*>(viewport.getViewedComponent());
        int preferredHeight = juce::jmin(content->getHeight(), 800);  // Max 800px tall
        setSize(600, preferredHeight);
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced(10);
        
        // Layout control buttons at bottom
        auto buttonArea = area.removeFromBottom(40);
        auto toggleArea = buttonArea.removeFromRight(200);
        
        rescanButton.setBounds(buttonArea.removeFromLeft(150).reduced(5));
        applyButton.setBounds(buttonArea.removeFromLeft(150).reduced(5));
        autoScanToggle.setBounds(toggleArea.reduced(5));
        
        viewport.setBounds(area);
    }

    void timerCallback() override
    {
        auto* content = dynamic_cast<ContentComponent*>(viewport.getViewedComponent());
        if (content != nullptr)
        {
            content->checkDeviceListChanges();
        }
    }

    void buttonClicked(juce::Button* button) override
    {
        if (button == &rescanButton)
        {
            auto* content = dynamic_cast<ContentComponent*>(viewport.getViewedComponent());
            if (content)
            {
                // Schedule the device check after a short delay
                juce::Timer::callAfterDelay(100, [this, content]()
                {
                    content->checkDeviceListChanges();
                    showDeviceStatus();
                });
            }
        }
        else if (button == &applyButton)
        {
            applySettings();
        }
        else if (button == &autoScanToggle)
        {
            if (autoScanToggle.getToggleState())
                startTimer(1000);
            else
                stopTimer();
        }
    }

    void showDeviceStatus()
    {
        juce::String message;
        auto devices = juce::MidiInput::getAvailableDevices();
        
        if (devices.isEmpty())
            message = "No MIDI devices found";
        else
        {
            message = "Connected MIDI devices:\n\n";
            for (auto& device : devices)
                message += "- " + device.name + "\n";
        }
        
        juce::AlertWindow::showMessageBoxAsync(
            juce::MessageBoxIconType::InfoIcon,
            "MIDI Device Status",
            message
        );
    }

    void applySettings()
    {
        // TODO: Apply settings to MainComponent
        juce::AlertWindow::showMessageBoxAsync(
            juce::MessageBoxIconType::InfoIcon,
            "Settings Applied",
            "MIDI device and filter settings have been updated."
        );
    }

private:
    // Inner content component that holds all the controls
    class ContentComponent : public juce::Component
    {
    public:
        ContentComponent()
        {
            // Input Devices Section
            addAndMakeVisible(inputDevicesGroup);
            inputDevicesGroup.setText("MIDI Input Devices");
            inputDevicesGroup.setColour(juce::GroupComponent::textColourId, juce::Colours::black);
            
            // Logging Devices Section
            addAndMakeVisible(loggingDevicesGroup);
            loggingDevicesGroup.setText("Logging Devices");
            loggingDevicesGroup.setColour(juce::GroupComponent::textColourId, juce::Colours::black);
            
            // Input Filters Section
            addAndMakeVisible(inputFiltersGroup);
            inputFiltersGroup.setText("Input Filters");
            inputFiltersGroup.setColour(juce::GroupComponent::textColourId, juce::Colours::black);
            
            // Logging Filters Section
            addAndMakeVisible(loggingFiltersGroup);
            loggingFiltersGroup.setText("Logging Filters");
            loggingFiltersGroup.setColour(juce::GroupComponent::textColourId, juce::Colours::black);
            
            // Get available MIDI devices and create checkboxes
            auto devices = juce::MidiInput::getAvailableDevices();
            for (const auto& device : devices)
            {
                // Device listening checkbox
                auto* listenBox = deviceListenBoxes.add(new juce::ToggleButton("* " + device.name));
                listenBox->setToggleState(true, juce::dontSendNotification);
                listenBox->setColour(juce::ToggleButton::textColourId, juce::Colours::black);
                addAndMakeVisible(listenBox);
                
                // Device logging checkbox
                auto* logBox = deviceLogBoxes.add(new juce::ToggleButton("* " + device.name));
                logBox->setToggleState(true, juce::dontSendNotification);
                logBox->setColour(juce::ToggleButton::textColourId, juce::Colours::black);
                addAndMakeVisible(logBox);
            }
            
            // Add filter checkboxes for both input and logging
            for (const auto& type : messageTypes)
            {
                // Input filter checkbox
                auto* inputBox = inputFilterBoxes.add(new juce::ToggleButton(type));
                inputBox->setToggleState(true, juce::dontSendNotification);
                addAndMakeVisible(inputBox);
                
                // Logging filter checkbox
                auto* logBox = loggingFilterBoxes.add(new juce::ToggleButton(type));
                logBox->setToggleState(true, juce::dontSendNotification);
                addAndMakeVisible(logBox);
            }
            
            // Set colors for device checkboxes
            for (auto* box : deviceListenBoxes)
                box->setColour(juce::ToggleButton::textColourId, juce::Colours::black);
            for (auto* box : deviceLogBoxes)
                box->setColour(juce::ToggleButton::textColourId, juce::Colours::black);
            
            // Set colors for filter checkboxes
            for (auto* box : inputFilterBoxes)
                box->setColour(juce::ToggleButton::textColourId, juce::Colours::black);
            for (auto* box : loggingFilterBoxes)
                box->setColour(juce::ToggleButton::textColourId, juce::Colours::black);
            
            // Calculate total height needed
            int totalHeight = 40;  // Initial padding
            totalHeight += static_cast<int>(deviceListenBoxes.size()) * 24 + 60;
            totalHeight += static_cast<int>(deviceLogBoxes.size()) * 24 + 60;
            totalHeight += static_cast<int>((messageTypes.size() + 2) / 3) * 24 + 60;
            totalHeight += static_cast<int>((messageTypes.size() + 2) / 3) * 24 + 60;  // Logging filters section
            
            setSize(580, totalHeight);  // Set content size
        }

        void resized() override
        {
            auto area = getLocalBounds().reduced(10);
            
            // Calculate device section heights based on content
            int deviceRowHeight = 24;
            int devicePadding = 60;
            int deviceSectionHeight = (deviceListenBoxes.size() * deviceRowHeight) + devicePadding;
            
            // Calculate filter section height
            int filterRowHeight = 24;
            int filterPadding = 60;
            int filterRows = static_cast<int>((messageTypes.size() + 2) / 3);  // +2 for rounding up
            int filterSectionHeight = (filterRows * filterRowHeight) + filterPadding;
            
            auto inputDeviceArea = area.removeFromTop(deviceSectionHeight);
            inputDevicesGroup.setBounds(inputDeviceArea);
            
            auto loggingDeviceArea = area.removeFromTop(deviceSectionHeight);
            loggingDevicesGroup.setBounds(loggingDeviceArea);
            
            // Layout filter sections
            auto inputFilterArea = area.removeFromTop(filterSectionHeight);
            inputFiltersGroup.setBounds(inputFilterArea);
            
            auto loggingFilterArea = area.removeFromTop(filterSectionHeight);
            loggingFiltersGroup.setBounds(loggingFilterArea);
            
            // Layout device checkboxes
            layoutDeviceBoxes(deviceListenBoxes, inputDeviceArea.reduced(10));
            layoutDeviceBoxes(deviceLogBoxes, loggingDeviceArea.reduced(10));
            
            // Layout filter checkboxes
            layoutFilterBoxes(inputFilterBoxes, inputFilterArea.reduced(10));
            layoutFilterBoxes(loggingFilterBoxes, loggingFilterArea.reduced(10));
        }

        void checkDeviceListChanges()
        {
            // Ensure we're not in the middle of a device update
            static bool isChecking = false;
            if (isChecking)
                return;
            
            isChecking = true;
            juce::ScopedValueSetter<bool> scope(isChecking, false);  // Auto-reset when function exits
            
            if (!juce::MessageManager::getInstance()->isThisTheMessageThread())
            {
                juce::MessageManager::callAsync([this]() { checkDeviceListChanges(); });
                return;
            }
            
            auto currentDevices = juce::MidiInput::getAvailableDevices();
            
            // Clear and rebuild device boxes
            deviceListenBoxes.clear(true);
            deviceLogBoxes.clear(true);
            
            for (const auto& device : currentDevices)
            {
                // Create device listening checkbox
                auto* listenBox = deviceListenBoxes.add(new juce::ToggleButton("* " + device.name));
                listenBox->setToggleState(true, juce::dontSendNotification);
                listenBox->setColour(juce::ToggleButton::textColourId, juce::Colours::black);
                addAndMakeVisible(listenBox);
                
                // Create device logging checkbox
                auto* logBox = deviceLogBoxes.add(new juce::ToggleButton("* " + device.name));
                logBox->setToggleState(true, juce::dontSendNotification);
                logBox->setColour(juce::ToggleButton::textColourId, juce::Colours::black);
                addAndMakeVisible(logBox);
            }
            
            resized();  // Relayout with new devices
        }

    private:
        // Helper to layout device checkboxes vertically
        void layoutDeviceBoxes(juce::OwnedArray<juce::ToggleButton>& boxes, juce::Rectangle<int> area)
        {
            area.removeFromTop(10);  // Add padding at top
            for (int i = 0; i < boxes.size(); ++i)
            {
                boxes[i]->setBounds(area.removeFromTop(24));
            }
        }
        
        // Helper to layout filter checkboxes in a grid
        void layoutFilterBoxes(juce::OwnedArray<juce::ToggleButton>& boxes, juce::Rectangle<int> area)
        {
            area.removeFromTop(10);  // Add padding at top
            int boxesPerRow = 3;
            int boxWidth = area.getWidth() / boxesPerRow;
            
            for (int i = 0; i < boxes.size(); ++i)
            {
                int row = i / boxesPerRow;
                int col = i % boxesPerRow;
                boxes[i]->setBounds(area.getX() + col * boxWidth,
                                  area.getY() + row * 24,
                                  boxWidth - 10,
                                  20);
            }
        }
        
        // Groups
        juce::GroupComponent inputDevicesGroup;
        juce::GroupComponent loggingDevicesGroup;
        juce::GroupComponent inputFiltersGroup;
        juce::GroupComponent loggingFiltersGroup;
        
        // Device checkboxes
        juce::OwnedArray<juce::ToggleButton> deviceListenBoxes;
        juce::OwnedArray<juce::ToggleButton> deviceLogBoxes;
        
        // Filter checkboxes
        juce::OwnedArray<juce::ToggleButton> inputFilterBoxes;
        juce::OwnedArray<juce::ToggleButton> loggingFilterBoxes;
        
        const std::vector<juce::String> messageTypes = {
            "Notes", "Control Change", "Program Change",
            "Pitch Bend", "Aftertouch", "Clock",
            "System Common", "System Exclusive"
        };
    };

    juce::Viewport viewport;
    juce::TextButton rescanButton {"Rescan MIDI Devices"};
    juce::TextButton applyButton {"Apply Changes"};
    juce::ToggleButton autoScanToggle {"Auto-scan for devices"};
    //juce::Label statusLabel;  // For showing status messages
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SettingsComponent)
}; 