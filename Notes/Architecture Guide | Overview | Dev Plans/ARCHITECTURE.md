# MidiPortal Architecture Guide

## 1. Overview

MidiPortal is a comprehensive MIDI monitoring and visualization application built using the JUCE framework with Rust integration for high-performance MIDI processing. The application provides real-time visualization of MIDI data with customizable display settings, multiple window support, and device routing capabilities.

This architecture guide outlines the core components, design patterns, and data flow within the MidiPortal application.

## 2. Core Architecture

MidiPortal uses a hybrid architecture combining:

- **C++/JUCE**: For UI, audio device management, and application framework
- **Rust**: For high-performance MIDI processing and analysis
- **FFI (Foreign Function Interface)**: For communication between C++ and Rust

The application follows a component-based design with specialized managers for different concerns, creating a modular and maintainable codebase.

### 2.1 High-Level Components

```
┌─────────────────────────────────────┐
│             MidiPortal              │
├─────────────┬───────────────────────┤
│   C++/JUCE  │         Rust          │
├─────────────┼───────────────────────┤
│  UI/Display │   MIDI Processing     │
│  Audio I/O  │   Pattern Analysis    │
│  Settings   │   Data Structures     │
└─────────────┴───────────────────────┘
```

## 3. Key Components

### 3.1 C++/JUCE Components

#### MainComponent
The central UI component that coordinates all visualization and MIDI handling. It contains:
- MIDI input/output handling
- View mode management (LIST/SCROLL, GRID)
- Menu handling
- Device connection management

#### Settings Management
- **SettingsManager**: Central manager for application settings
- **DisplaySettingsManager**: Manages visual settings for MIDI message display
- **SettingsComponent/Window**: UI for configuring audio and MIDI devices

#### Window Management
- **WindowManager**: Creates and manages multiple display windows
- **WindowRoutingComponent**: UI for configuring MIDI routing to windows
- **LogDisplayWindow**: Window for displaying MIDI messages

#### MIDI Processing and Display
- **MidiLogger**: Logs MIDI messages to files and provides timing analysis
- **MidiLogDisplay**: Component for displaying MIDI messages with customizable styles
- **RustMidiEngine**: C++ interface to the Rust-based MIDI processing engine
- **SharedMidiBuffer**: Shared memory buffer for MIDI data between C++ and Rust

#### AI/ML Components
- **MidiAIManager**: Manages ML models for MIDI analysis
- **AIInsightComponent**: Displays insights generated from MIDI data

### 3.2 Rust Components

#### MIDI Engine
- **midi_engine.rs**: Core MIDI processing functionality
- **shared_buffer.rs**: Lock-free ring buffer for sharing MIDI data with C++

#### Machine Learning
- **ml/context.rs**: Models the musical context for pattern recognition
- **ml/pattern.rs**: Pattern recognition algorithms for MIDI data

## 4. Data Flow

```
┌───────────────┐    ┌────────────────┐    ┌─────────────────┐
│ MIDI Hardware ├───►│  JUCE Audio    ├───►│  MainComponent  │
└───────────────┘    │  Device Mgr    │    └────────┬────────┘
                     └────────────────┘             │
                                                    ▼
┌───────────────┐    ┌────────────────┐    ┌─────────────────┐
│ Window        │◄───┤ MIDI Message   │◄───┤  Device Routing │
│ Display       │    │ Processing     │    │  & Filtering    │
└─────┬─────────┘    └────────────────┘    └─────────────────┘
      │                      ▲
      │                      │
      │              ┌───────┴────────┐    ┌─────────────────┐
      │              │ Rust MIDI      │◄───┤  Shared MIDI    │
      │              │ Engine         │    │  Buffer         │
      │              └────────────────┘    └─────────────────┘
      ▼
┌─────────────────┐    ┌────────────────┐
│ Visual          │    │ AI/ML Insights │
│ Rendering       │◄───┤ Generation     │
└─────────────────┘    └────────────────┘
```

## 5. Message Routing System

MidiPortal's message routing system allows MIDI messages to be selectively displayed in different windows:

```
┌─────────────┐     ┌───────────────┐     ┌────────────────┐
│ MIDI Device │────►│ WindowManager │────►│ Main Window    │
│  (Input A)  │     │ Routing Table │     └────────────────┘
└─────────────┘     │               │     ┌────────────────┐
                    │               │────►│ Custom Window 1 │
┌─────────────┐     │               │     └────────────────┘
│ MIDI Device │────►│               │     ┌────────────────┐
│  (Input B)  │     │               │────►│ Custom Window 2 │
└─────────────┘     └───────────────┘     └────────────────┘
```

## 6. Settings Management

The settings system is hierarchical with specialized managers:

```
┌─────────────────┐
│ SettingsManager │
└────────┬────────┘
         │
         ├─────────────────────────────────┐
         │                                 │
┌────────▼────────┐              ┌─────────▼───────┐
│ AudioDeviceManager│              │DisplaySettingsManager│
└─────────────────┘              └───────────┬─────┘
                                             │
                      ┌────────────┬─────────┴────────┬─────────────┐
                      │            │                  │             │
               ┌──────▼─────┐┌─────▼─────┐    ┌───────▼─────┐┌──────▼─────┐
               │ ALL Device ││Device A   │    │Device B     ││Window X    │
               │ Settings   ││Settings   │    │Settings     ││Settings    │
               └────────────┘└───────────┘    └─────────────┘└────────────┘
```

## 7. Display Mode Architecture

MidiPortal supports multiple visualization modes for MIDI data:

### 7.1 LIST/SCROLL Mode
- Text-based display of MIDI messages
- Customizable colors per message type
- Scrolling log format with fading effect

### 7.2 GRID Mode
- Visual grid representation of MIDI notes
- Note position mapped to grid coordinates
- Visual properties (color, opacity) mapped to MIDI parameters

### 7.3 Future Extensions
The architecture allows for additional visualization modes to be added with minimal changes to the core framework.

## 8. Build System

MidiPortal uses CMake with the following structure:

```
┌────────────────┐
│ Root CMakeLists│
└────────┬───────┘
         │
         ├─────────────┬────────────┬───────────┐
         │             │            │           │
┌────────▼─────┐┌──────▼─────┐┌─────▼────┐┌─────▼─────┐
│ shared       ││ standalone ││ plugin   ││ test      │
│ CMakeLists   ││ CMakeLists ││CMakeLists││CMakeLists │
└──────────────┘└────────────┘└──────────┘└───────────┘
```

The Rust components are built using Cargo and integrated through CMake.

## 9. Cross-Platform Support

MidiPortal is designed to be cross-platform with special considerations:

- JUCE provides cross-platform UI, audio and MIDI I/O
- Platform-specific path handling for configuration files
- Toolchain files for different compiler environments 
- Consistent UI scaling across platforms

## 10. FFI (Foreign Function Interface) Design

Communication between C++ and Rust is handled through a carefully designed FFI:

```
┌────────────────────┐                 ┌────────────────────┐
│  C++ Application   │                 │    Rust Library    │
│                    │                 │                    │
│ ┌────────────────┐ │◄──FFI Calls────►│ ┌────────────────┐ │
│ │ RustMidiEngine │ │                 │ │ FFI Functions  │ │
│ └────────────────┘ │                 │ └────────────────┘ │
│                    │                 │                    │
│ ┌────────────────┐ │                 │ ┌────────────────┐ │
│ │SharedMidiBuffer│ │◄─Shared Memory─►│ │SharedMidiBuffer│ │
│ └────────────────┘ │                 │ └────────────────┘ │
└────────────────────┘                 └────────────────────┘
```

## 11. Directory Structure

```
MidiPortal/
├── cmake/           # CMake modules and toolchains
├── libs/            # External libraries
├── rust/            # Rust code
│   ├── src/         # Rust source files
│   │   ├── midi_engine.rs
│   │   ├── shared_buffer.rs
│   │   └── ml/      # Machine learning modules
├── shared/          # Shared C++ code
│   ├── include/     # Public headers
│   └── source/      # Implementation files
├── standalone/      # Standalone application
│   └── source/      # Implementation files
└── plugin/          # Plugin version
```

## 12. Future Architecture Extensions

The modular design of MidiPortal allows for several natural extension points:

1. **Additional Visualization Modes**: The ViewMode enum and handler methods can be extended.
2. **ML Model Integration**: The MidiAIManager can support additional model types.
3. **Plugin Formats**: The plugin architecture can be extended to support additional formats.
4. **Remote Control**: The architecture could be extended to support remote control via OSC or MIDI. 