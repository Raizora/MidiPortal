# MidiPortal Architecture Guide

## Overview

MidiPortal is a JUCE-based application for real-time MIDI monitoring and analysis. It provides a flexible system for visualizing MIDI data across multiple windows with customizable display settings and intelligent analysis capabilities.

## Core Components

### 1. MIDI Processing Pipeline

The MIDI processing pipeline follows this flow:

1. **MIDI Input**: JUCE's `MidiInputCallback` receives messages from enabled MIDI devices
2. **Channel Filtering**: Messages are filtered based on enabled MIDI channels (1-16)
3. **Rust Engine Processing**: Messages are sent to the Rust `midi_engine` for analysis
4. **Routing**: Messages are routed to appropriate display windows
5. **Visualization**: Messages are displayed with color-coding based on message type
6. **AI Analysis**: Messages are analyzed for patterns and insights

### 2. Window System

MidiPortal uses a multi-window architecture:

- **MAIN Window**: Always present, shows all enabled MIDI devices
- **Additional Windows (A, B, C, etc.)**: Created via the Window Routing panel
- **Settings Windows**: Configuration dialogs for various aspects of the application

Each window can be customized with its own background color and can display MIDI messages from specific devices.

### 3. Routing System

The routing system determines which MIDI messages appear in which windows:

- **Default Routing**: All enabled MIDI devices automatically send messages to the MAIN window
- **Custom Routing**: Through the Window Routing panel, users can route specific devices to specific windows
- **Multiple Destinations**: A device can send to MAIN (always) plus any combination of additional windows

The routing configuration is stored in the `WindowManager` class in a map of device names to window names.

### 4. Display Settings

Display settings control the visual appearance of MIDI messages:

- **Global Settings**: Applied to all windows unless overridden
- **Device-Specific Settings**: Applied to messages from specific devices
- **Window-Specific Settings**: Applied to specific windows (background color)

Settings include:
- Font size
- Background color
- Colors for different MIDI message types (Note On, Note Off, Controller, etc.)

### 5. Machine Learning Integration

The ML system analyzes MIDI data for patterns and insights:

- **Rust-Based Engine**: Core analysis happens in the Rust `midi_engine`
- **Pattern Recognition**: Identifies recurring patterns in MIDI input
- **Performance Analysis**: Analyzes timing, velocity, and other performance metrics
- **AI Insights**: Generates insights about the musical content

## Key Classes

### MainComponent

The central component that coordinates all aspects of the application:

- Manages MIDI input and routing
- Creates and manages windows
- Handles menu bar and user interface
- Coordinates between C++ and Rust components

### WindowManager

Manages the creation, destruction, and routing of display windows:

- Creates new windows (A, B, C, etc.)
- Maintains routing information (which devices go to which windows)
- Routes MIDI messages to appropriate windows

### DisplaySettingsManager

Manages display settings for different devices and windows:

- Stores color schemes for different message types
- Manages font sizes and other display properties
- Provides default settings when device-specific settings aren't available

### MidiLogDisplay

Displays MIDI messages with appropriate formatting and colors:

- Renders messages with color-coding based on message type
- Handles scrolling and fading effects
- Applies appropriate display settings based on device and window

### WindowRoutingComponent

Provides the user interface for configuring window routing:

- Shows a grid of devices and windows
- Allows toggling routing with checkboxes
- Provides controls for setting window background colors

### MidiAIManager

Manages the AI analysis of MIDI data:

- Interfaces with the Rust ML engine
- Processes MIDI messages for pattern recognition
- Generates insights based on analysis results

## Data Flow

1. MIDI messages are received by `MidiInputCallback`
2. Messages are passed to `MainComponent::addMidiMessage` with the device name
3. Messages are processed by the Rust engine via `process_midi_message`
4. Messages are stored in `midiMessages` for history
5. Messages are logged via `midiLogger`
6. Messages are routed via `routeMidiMessage` to:
   - The main display (`midiLogDisplay`)
   - Additional windows via `windowManager.routeMidiMessage`
   - The AI manager via `impl->processMidiMessage`
7. Each display renders the messages with appropriate colors and formatting

## Configuration

### MIDI Device Configuration

- Enabled via the Settings window
- Channel filtering available for each device
- Activity indicators show real-time MIDI activity

### Window Configuration

- Created via the Window Routing panel
- Background color set via RGB sliders
- Device routing configured via checkboxes

### Display Settings Configuration

- Accessed via the Log Display Settings window
- Color settings for different message types
- Font size and other display properties

## Machine Learning Features

### Pattern Recognition

- Identifies recurring patterns in MIDI input
- Builds a library of patterns for future reference
- Detects variations on known patterns

### Performance Analysis

- Analyzes timing accuracy and consistency
- Evaluates dynamics (velocity) patterns
- Measures aspects of musical expression

### AI Insights

- Generates insights about musical content
- Provides suggestions for improvement
- Identifies interesting musical features

## Implementation Notes

- JUCE framework provides cross-platform compatibility
- Rust engine provides high-performance, memory-safe processing
- C++/Rust FFI (Foreign Function Interface) bridges the two languages
- MIDI messages are passed as raw byte arrays for efficiency
- Display settings use JUCE's color system for consistent rendering

## Common Workflows

### Adding a New Window

1. Open Window Routing from the View menu
2. Click "New Window" to create a window (A, B, C, etc.)
3. Set the background color using RGB sliders
4. Check boxes to route devices to the new window

### Customizing Display Settings

1. Open Log Display Settings from the File menu
2. Select a device from the dropdown
3. Adjust font size and colors for different message types
4. Click Apply to save changes

### Monitoring Specific Devices

1. Enable devices in the Settings window
2. Create additional windows as needed
3. Configure routing in the Window Routing panel
4. Observe MIDI activity in the appropriate windows

## Troubleshooting

### Messages Not Appearing in Windows

- Verify device is enabled in Settings
- Check routing in Window Routing panel
- Ensure device name matches exactly between Settings and Window Routing
- Verify MIDI channels are enabled for the device

### Display Settings Not Applied

- Make sure settings are applied with the Apply button
- Check that the correct device is selected in the dropdown
- Verify that the window name matches the intended target

### Performance Issues

- Reduce the number of windows for better performance
- Lower the maximum number of displayed messages
- Close unused windows to free up resources 