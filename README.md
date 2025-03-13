# **MidiPortal - Version 0.2.0**

**Peer through the MidiPortal to observe all traffic on your system.**

MidiPortal is a JUCE-based MIDI monitoring utility that allows you to visualize and analyze MIDI traffic in real-time. Designed as a lightweight tool, MidiPortal serves as the foundation for future enhancements, ultimately building toward the full iLumidi application.

## **Features**
- **Real-Time Monitoring**: Observe MIDI messages as they traverse your system.
- **Cross-Platform Support**: Built with JUCE for compatibility with macOS, Windows, and Linux.
- **Highly Customizable**: Modular design serves as a foundation for future functionality.
- **Multi-Window Device Selection**: Spawn additional windows for device-specific monitoring.
- **Comprehensive Settings**: Fully configurable colors for different MIDI message types.
- **Lightweight Design**: Optimized for performance with minimal system resource usage.
- **Robust Tooling**: Effortless management of third-party dependencies using the CPM package manager.
- **Unit Testing**: Ready-to-go unit testing with GoogleTest.
- **Enforced Code Quality**: Highest warning levels with "treat warnings as errors" for maximum safety.
- **Consistent Build System**: Enforced Ninja generator and Homebrew Clang 19+ compiler for reliable builds.
- **Machine Learning Integration**: AI-powered analysis of MIDI patterns and performance metrics.
- **Intelligent Insights**: Real-time feedback and suggestions based on MIDI input analysis.

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

## **Continuous Integration (CI/CD)**
MidiPortal uses **GitHub Actions** for automated **building and testing**.  
Each push to `experimental`, `main`, or `development` triggers the CI pipeline.

### **How CI/CD Works**
1. **Installs dependencies** (`ninja-build`, `JUCE`, etc.).
2. **Configures and builds the project automatically**.
3. **Caches dependencies** to avoid redundant downloads.

### **GitHub Actions CI Pipeline**
You can check workflow runs in the **GitHub Actions** tab:  
📌 **GitHub → Your Repo → Actions → Build and Test**

## **Advanced Build Options**

### **Using CMake Presets**
```bash
cmake --preset default  # Uses the Ninja build system
cmake --build build
ctest --preset default
```
Presets available: `default`, `release`, and `Xcode`.

## **Code Quality & Formatting**

### **Enable Automatic Code Formatting**
Enable automatic code formatting on every commit:
```bash
pre-commit install
```
> **Note**: Requires `pre-commit`. Install it with:
> ```bash
> pip install pre-commit
> ```

### **Static Analysis & Warnings**
- The highest warning levels are enforced (`treat warnings as errors`).
- Use `clang-tidy` for linting:
  ```bash
  clang-tidy -checks='*' source/*.cpp
  ```

## **Customization**
- Modify the default plugin name (`YourPluginName`) throughout the project to match your requirements.
- Easily integrate third-party libraries alongside JUCE using the CPM package manager.
- Update configurations in the `CMakeLists.txt` file to tailor the project to your workflow.

## **Contribution and Acknowledgments**
Special thanks to **WolfSound** for providing the free, unrestricted, public quickstart JUCE/CMake template, which served as the foundation for this project.

## **Common Issues & Fixes**

This section documents some of the most common issues encountered when building and running **MidiPortal**, along with their fixes.

---

### **1️⃣ Missing Build Targets (`MidiPortalStandalone` or `MidiPortalShared`)**
**Issue:**
- Running `cmake --build out/build --target help` does not list `MidiPortalStandalone` or `MidiPortalShared`.
- The project builds but the executable is missing.

**Fix:**
1. **Invalidate CLion's Cache & Restart**
  - **File > Invalidate Caches & Restart**
  - Check **"Clear CMake Cache"** and restart CLion.

2. **Manually Clean CMake Cache & Rebuild**
   ```bash
   rm -rf out/build
   cmake -G Ninja -S . -B out/build \
       -DCMAKE_C_COMPILER=/opt/homebrew/opt/llvm/bin/clang \
       -DCMAKE_CXX_COMPILER=/opt/homebrew/opt/llvm/bin/clang++ \
       -DCMAKE_BUILD_TYPE=Debug
   cmake --build out/build --parallel
   ```

3. **Verify the Target Exists**
   ```bash
   cmake --build out/build --target help
   ```
   If `MidiPortalStandalone` is still missing, check if `add_subdirectory(standalone)` exists in `CMakeLists.txt`.

---

### **2️⃣ Duplicate Target Error (`add_library cannot create target "MidiPortalShared"`)**
**Issue:**
- CMake throws an error that `MidiPortalShared` already exists.
- Happens when `add_subdirectory(shared)` is uncommented in the root `CMakeLists.txt`.

**Fix:**
1. **Keep `add_subdirectory(shared)` Commented Out** (Only `standalone` and `plugin` should include `shared`).
   ```cmake
   # add_subdirectory(shared)  # Keep this commented!
   ```

2. **Modify `shared/CMakeLists.txt` to Prevent Duplicate Definitions**
   ```cmake
   if (NOT TARGET MidiPortalShared)
       add_library(MidiPortalShared
           source/MainComponent.cpp
           source/MainComponent.h
           source/MidiLogger.cpp
           source/MidiLogger.h
       )
   endif()
   ```

---

### **3️⃣ CLion CMake Cache Not Reloading**
**Issue:**
- Changes to `CMakeLists.txt` are not reflected in CLion.

**Fix:**
1. **Manually Reload CMake in CLion**
  - **File > Reload CMake Project** (`Cmd + Shift + S`)
  - If this fails, remove and re-add the CMake profile:
    - **Settings > Build, Execution, Deployment > CMake**
    - Delete the existing profile, then re-add it with:
      - **CMake Generator**: `Ninja`
      - **C++ Standard**: `C++23`
      - **C Compiler**: `/opt/homebrew/opt/llvm/bin/clang`
      - **C++ Compiler**: `/opt/homebrew/opt/llvm/bin/clang++`

2. **Manually Run CMake Again**
   ```bash
   rm -rf cmake-build-debug
   cmake -G Ninja -S . -B cmake-build-debug \
       -DCMAKE_C_COMPILER=/opt/homebrew/opt/llvm/bin/clang \
       -DCMAKE_CXX_COMPILER=/opt/homebrew/opt/llvm/bin/clang++ \
       -DCMAKE_BUILD_TYPE=Debug
   ```

---

### **4️⃣ CMake Fails to Detect Changes in `CMakeLists.txt`**
**Issue:**
- `cmake --build out/build --target help` still shows old configurations.

**Fix:**
1. **Remove the CMake Cache and Force a Full Rebuild**
   ```bash
   rm -rf CMakeCache.txt CMakeFiles/
   cmake -G Ninja -S . -B out/build \
       -DCMAKE_C_COMPILER=/opt/homebrew/opt/llvm/bin/clang \
       -DCMAKE_CXX_COMPILER=/opt/homebrew/opt/llvm/bin/clang++ \
       -DCMAKE_BUILD_TYPE=Debug
   ```
2. **If Using CLion, Restart and Reload the Project**
  - **File > Invalidate Caches & Restart**
  - **File > Reload CMake Project**

---

### **5️⃣ VST3 Plugin Build Fails with `juce_vst3_helper: command not found`**
**Issue:**
- When building the VST3 plugin target, the build fails with:
  ```
  /bin/sh: juce_vst3_helper: command not found
  ```
- This happens because the JUCE VST3 helper tool is not in the PATH or wasn't built correctly.

**Fix:**
1. **Use Audio Units (AU) Format on macOS (Recommended)**
   ```bash
   # The project now uses AU format on macOS instead of VST3
   ./build.sh --plugin
   ```
   Audio Units (AU) is the native plugin format for macOS and doesn't require the VST3 helper tool.

2. **Build Only the Standalone Target**
   ```bash
   cmake --build build --target MidiPortalStandalone
   ```
   This bypasses the plugin build entirely.

3. **For VST3 Development (Advanced)**
   If you specifically need VST3 format:
   - Edit plugin/CMakeLists.txt to change `FORMATS AU Standalone` back to `FORMATS VST3 Standalone`
   - Build the JUCE extras which include the VST3 helper:
     ```bash
     cd /path/to/JUCE
     cmake -B build -G Ninja
     cmake --build build --target juce_vst3_helper
     ```
   - Add the helper to your PATH:
     ```bash
     export PATH="/path/to/JUCE/build/extras/Build/juceaide/juce_vst3_helper:$PATH"
     ```

**Note**: The standalone application has all the functionality needed for MIDI monitoring. The plugin version is only necessary if you want to use MidiPortal within a DAW.

---

### **Final Notes**
If you continue to have issues, ensure you:
✅ Run `cmake --build out/build --target help` to verify targets.
✅ Clear CMake and CLion caches if targets go missing.
✅ Keep `add_subdirectory(shared)` **commented out** in the root `CMakeLists.txt`.
✅ Use the `build.sh` script for the most reliable build experience.

This should help prevent and quickly fix the most common build issues in **MidiPortal**! 🚀



### **Final Thoughts**
This README ensures that **anyone pulling the repository** has everything they need to **build, run, and understand** the purpose and future plans for **MidiPortal**.

# Information specific to MidiPortal.Cursor

## Project Structure

- `shared/`: Common code used by both standalone and plugin versions
  - MIDI processing logic
  - Visualization components
  - Rust FFI interface
- `standalone/`: Standalone application target
- `plugin/`: VST3/AU plugin target
- `rust/`: Rust components for MIDI visualization

## Building

Prerequisites:
- CMake 3.30.5 or higher
- JUCE
- Rust toolchain
- Ninja build system

## New Features
- Rust-powered MIDI processing engine with:
  - Comprehensive safety features and error handling
  - FFI interface for C++ integration
  - MIDI message validation and processing
  - Note tracking and expression monitoring
  - MPE (MIDI Polyphonic Expression) support

## Architecture
MidiPortal uses a JUCE-first architecture where:
1. JUCE handles all device and channel management through the UI
2. Filtered MIDI data is passed to the Rust midi_engine (v0.1.2) for processing
3. Processing results are returned to JUCE for logging and visualization

## Safety Features
- Input validation and bounds checking
- Memory safety with proper FFI
- Error handling and reporting
- Resource cleanup (Drop implementations)
- Safe state management (Clone support)

## Branch Structure
- `cursor-main`: Stable, protected branch with working features
- `cursor-development`: Active development branch

## Machine Learning Features

### **Pattern Recognition**
The new machine learning integration in MidiPortal 0.2.0 enables sophisticated pattern recognition capabilities:

- **Real-time Analysis**: Identifies patterns as you play without noticeable latency
- **Pattern Library**: Builds a database of recurring patterns for future reference
- **Variation Detection**: Recognizes variations on established patterns
- **Context-Aware Processing**: Understands musical context for more relevant insights

### **Performance Analysis**
MidiPortal now analyzes your MIDI performance in real-time:

- **Timing Analysis**: Measures timing accuracy and consistency
- **Velocity Patterns**: Evaluates dynamics and expression
- **Articulation Detection**: Identifies playing techniques
- **Style Recognition**: Learns your playing style over time

### **AI Insights**
The new AIInsightComponent provides intelligent feedback:

- **Real-time Suggestions**: Offers tips to improve your playing
- **Pattern Identification**: Highlights recurring motifs in your performance
- **Performance Metrics**: Shows quantitative measures of your playing
- **Learning Progression**: Tracks improvement over time

## Architecture Improvements

### **Shared Memory Buffer**
Version 0.2.0 introduces a shared memory buffer system for efficient data exchange:

- **Zero-Copy Design**: Eliminates redundant data copying between C++ and Rust
- **Lock-Free Operation**: Minimizes contention for high-performance operation
- **Circular Buffer**: Efficiently handles continuous MIDI data streams
- **Thread-Safe Implementation**: Ensures data integrity in multi-threaded contexts

### **Enhanced MIDI Routing**
The MIDI routing system has been significantly improved:

- **Device Name Propagation**: MIDI messages now carry device name information throughout the system
- **Multi-Window Routing**: Messages can be selectively routed to specific windows
- **Flexible Configuration**: Routing can be changed dynamically through the Window Routing panel
- **Consistent Behavior**: Ensures messages appear in all appropriate windows

[package]
name = "midi_engine"
version = "0.2.0"