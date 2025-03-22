# **MidiPortal - Version 0.2.0**

**Peer through the MidiPortal to observe all traffic on your system.**

MidiPortal is a JUCE-based MIDI monitoring utility that allows you to visualize and analyze MIDI traffic in real-time. Designed as a lightweight tool, MidiPortal serves as the foundation for future enhancements, ultimately building toward the full iLumidi application.

## **Features**
- **Real-Time Monitoring**: Observe MIDI messages as they traverse your system.
- **Cross-Platform Support**: Built with JUCE for compatibility with macOS, Windows, and Linux.
- **Highly Customizable**: Modular design serves as a foundation for future functionality.
- **Multi-Window Device Selection**: Spawn additional windows for device-specific monitoring.
- **Comprehensive Settings**: Fully configurable colors for different MIDI message types.
- **Multiple Visualization Modes**: Switch between LIST/SCROLL and GRID views.
- **Device Routing System**: Route specific MIDI devices to dedicated windows.
- **Lightweight Design**: Optimized for performance with minimal system resource usage.
- **Machine Learning Integration**: AI-powered analysis of MIDI patterns and performance metrics.
- **Intelligent Insights**: Real-time feedback and suggestions based on MIDI input analysis.
- **Shared Memory Architecture**: Efficient data exchange between C++ and Rust components.

## **Roadmap**
- Visualize incoming and outgoing MIDI messages.
- Filter and categorize MIDI traffic by device, channel, or message type.
- **Planned Enhancements**:
  - Add educational and analytical tools.
  - Expand to include MIDI manipulation and visualization features.
  - Integrate with iLumidi for an all-encompassing audio-visual experience.
  - Automatic log management for MIDI traffic.
  - Advanced machine learning models for deeper musical analysis.
  - Pattern recognition for compositional assistance.

## **Version History**
### **0.2.0** (Machine Learning Integration):
- Added AI-powered MIDI analysis capabilities
- Implemented shared memory buffer for efficient data exchange between C++ and Rust
- Integrated machine learning context for pattern recognition
- Added AIInsightComponent for displaying real-time insights
- Enhanced MIDI routing system with device name propagation
- Improved window management for multi-window MIDI monitoring
- Upgraded Rust midi_engine to version 0.2.0 with ML capabilities
- Added comprehensive architecture documentation
- Fixed MIDI routing issues between windows

### **0.1.0** (First Stable Release):
- Fixed window background color synchronization issues
- Improved device-specific settings management
- Enhanced Log Display Settings UI with proper layout
- Fixed reset button functionality for device-specific settings
- Optimized device selector to show relevant MIDI devices
- Comprehensive documentation throughout the codebase
- Stable foundation for future feature development

### **0.0.9.9** (Pre-release):
- Implemented comprehensive Log Display Settings component with scrollable viewport
- Added color selectors for each MIDI message type with clearly labeled UI sections
- Introduced multi-window functionality for device-specific monitoring
- Refined and polished UI components and behaviors across the application
- Enhanced overall usability, responsiveness, and user experience
- Improved build system with robust Homebrew Clang 19+ detection and usage
- Final major milestone before the 0.1.0 release

### **0.0.8**:
- Added scrolling, fading MIDI log display for real-time visualization
- Implemented color-coding for different MIDI message types
- Added support for switching between different visualization modes
- Enhanced UI with proper layout and component management
- Added build script with enforced Ninja generator and Apple Clang compiler

### **0.0.7**:
- Implemented new JUCE-first architecture for MIDI device management
- Added MidiChannelSelector UI for per-device MIDI channel control
- Added MidiActivityIndicator for real-time MIDI activity visualization
- Enhanced Settings window with responsive layout and dynamic sizing
- Improved MIDI data flow: JUCE → Rust midi_engine (v0.1.2) → JUCE logging
- Reduced cross-language communication overhead

### **0.0.6**:
- Added macOS menu bar integration
- Added preferences window with device selection
- Implemented proper memory management
- Added foundation for MIDI filtering system

### **0.0.5**:
- Added Rust-powered MIDI processing engine
- Implemented comprehensive safety features
- Added FFI interface for C++/Rust integration
- Added note tracking and MPE support
- Established protected branch structure

### **0.0.2**:
- Implemented `MidiLogger` for real-time MIDI logging to file.
- Validated core functionality: MIDI monitoring and traffic logging.
- Established a functional prototype of MidiPortal.
- Integrated GitHub Actions for automated CI/CD.

### **0.0.1**:
- Initial setup with JUCE framework and basic MIDI message handling.

## **Requirements**
### **System Requirements**
- **C++**: Version 23 or higher.
- **JUCE Framework**: Version 8.0.6.
- **CMake**: Version 3.31.5 or higher.
- **Rust**: Stable version 1.84 or higher.
- **Cargo**: Rust's package manager and build system.
- **Homebrew**: For installing dependencies on macOS.
- **LLVM/Clang**: Version 19+ (installed via Homebrew).
- **Ninja**: Version 1.12.1 or higher (for fast builds).
- **CLion (Recommended)**: IDE for development.
- **Python 3.13.2 (Optional)**: For pre-commit hooks.

## **Getting Started**

### **1. Clone the Repository**
```bash
git clone https://github.com/Raizora/MidiPortal.git
cd MidiPortal
```

### **2. Install Dependencies**
On macOS:
```bash
# Install Homebrew if not already installed
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install required dependencies
brew install llvm ninja cmake rust
```

### **3. Build Using the Script (Recommended)**
The build script automatically detects and uses Homebrew Clang 19+ and handles all necessary steps:
```bash
./build.sh                  # Build standalone only (Debug)
./build.sh --release        # Build standalone only (Release)
./build.sh --plugin         # Build standalone and plugin (Debug)
./build.sh --plugin --release # Build standalone and plugin (Release)
```

### **4. Run the Application**
```bash
./build/standalone/MidiPortalStandalone
```

### **Alternative: Manual Build Process**
If you prefer to build manually:

#### **Build Rust Library**
Navigate to the `rust` directory and build the Rust library:
```bash
cd rust
cargo build --release
cd ..
```

#### **Configure CMake**
Use Ninja to configure the build system (the toolchain file will automatically detect Homebrew Clang):
```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
```

#### **Build the Project**
Compile the project:
```bash
cmake --build build
```

#### **Run the Targets**
- Run `MidiPortalStandalone`:
  ```bash
  ./build/standalone/MidiPortalStandalone
  ```
- Run the `AudioPlugin`:
  ```bash
  ./build/plugin/AudioPlugin_artefacts/Debug/Standalone/MidiPortalPlugin.app/Contents/MacOS/MidiPortalPlugin
  ```

## **Enforced Build System**
MidiPortal enforces the use of Ninja as the build generator and Homebrew Clang 19+ as the compiler on macOS. This ensures consistent builds across different environments and prevents issues with build system switching.

### **How It Works**
- **Dynamic Compiler Detection**: The build system automatically finds Homebrew Clang 19+ installations.
- **Toolchain File**: A custom toolchain file (`cmake/toolchains/macos-clang.cmake`) enforces compiler settings.
- **Build Script**: The `build.sh` script handles all build steps with proper configuration.
- **Fallback Mechanism**: If Homebrew Clang isn't found, it will be installed automatically.

### **Benefits**
- **Consistent Builds**: Prevents accidental switching to Unix Makefiles or other generators.
- **Faster Compilation**: Ninja provides faster builds than traditional Make.
- **Modern C++ Support**: Ensures the use of a compiler with full C++23 support.
- **Simplified Workflow**: Single command to handle the entire build process.
- **Robust Detection**: Works even if Homebrew Clang version changes.

## **Architecture**

MidiPortal uses a hybrid C++/Rust architecture:
- JUCE (C++) handles UI, audio I/O, and application framework
- Rust provides high-performance MIDI processing and analysis
- FFI (Foreign Function Interface) enables communication between C++ and Rust

For more details, see the [Architecture Guide](ARCHITECTURE.md).

## **Main Components**

### **Using CMake Presets**
```bash
cmake --preset default  # Uses the Ninja build system
cmake --build build
ctest --preset default
```
Presets available: `default`, `release`, and `Xcode`.

### **Rust Components**
- **midi_engine.rs**: Core MIDI processing engine
- **shared_buffer.rs**: Lock-free ring buffer for MIDI data
- **ml/**: Machine learning modules for pattern recognition

## **User Interface**

MidiPortal offers several key UI components:
- **Main Window**: Primary display for MIDI traffic
- **Settings Window**: Configure MIDI devices and audio settings
- **Log Display Settings**: Customize colors and appearance of MIDI messages
- **Window Routing**: Control which devices route to which windows

## **Machine Learning Features**

### **Pattern Recognition**
The machine learning integration enables sophisticated pattern recognition:
- Real-time analysis of MIDI input
- Identification of recurring patterns
- Musical context awareness

### **Performance Analysis**
MidiPortal analyzes your MIDI performance:
- Timing accuracy measurement
- Velocity and expression analysis
- Playing style recognition

### **AI Insights**
The AIInsightComponent provides intelligent feedback:
- Real-time suggestions
- Pattern identification
- Performance metrics

## **Common Issues & Fixes**

For common build issues and notable problems, please refer to files in the "Key Issues | Solutions" folder.

## **Development**

### **Adding New Menu Items**
To add new menu items that open additional windows:
1. Define a unique menu item ID
2. Add the menu item to the appropriate menu
3. Handle the menu item selection in `menuItemSelected`
4. Create a method to open your new window
5. Add the window as a member variable

### **Plugin Development**
When working on the plugin version:
- Ensure your plugin's CMake references the latest shared code
- Note that plugins typically don't manage audio devices directly
- Test thoroughly in a DAW to confirm functionality