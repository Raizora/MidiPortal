# **MidiPortal - Version 0.0.7**

**Peer through the MidiPortal to observe all traffic on your system.**

MidiPortal is a JUCE-based MIDI monitoring utility that allows you to visualize and analyze MIDI traffic in real-time. Designed as a lightweight tool, MidiPortal serves as the foundation for future enhancements, ultimately building toward the full iLumidi application.

## **Features**
- **Real-Time Monitoring**: Observe MIDI messages as they traverse your system.
- **Cross-Platform Support**: Built with JUCE for compatibility with macOS, Windows, and Linux.
- **Highly Customizable**: Modular design serves as a foundation for future functionality.
- **Lightweight Design**: Optimized for performance with minimal system resource usage.
- **Robust Tooling**: Effortless management of third-party dependencies using the CPM package manager.
- **Unit Testing**: Ready-to-go unit testing with GoogleTest.
- **Enforced Code Quality**: Highest warning levels with "treat warnings as errors" for maximum safety.

## **Roadmap**
- Visualize incoming and outgoing MIDI messages.
- Filter and categorize MIDI traffic by device, channel, or message type.
- **Planned Enhancements**:
  - Add educational and analytical tools.
  - Expand to include MIDI manipulation and visualization features.
  - Integrate with iLumidi for an all-encompassing audio-visual experience.
  - Automatic log management for MIDI traffic.

## **Version History**
### **0.0.2**:
- Implemented `MidiLogger` for real-time MIDI logging to file.
- Validated core functionality: MIDI monitoring and traffic logging.
- Established a functional prototype of MidiPortal.
- Integrated GitHub Actions for automated CI/CD.
- Added dependency caching to speed up builds.

### **0.0.1**:
- Initial setup with JUCE framework and basic MIDI message handling.

### **0.0.5**:
- Added Rust-powered MIDI processing engine
- Implemented comprehensive safety features
- Added FFI interface for C++/Rust integration
- Added note tracking and MPE support
- Established protected branch structure

### **0.0.6**:
- Added macOS menu bar integration
- Added preferences window with device selection
- Implemented proper memory management
- Added foundation for MIDI filtering system

### **0.0.7**:
- Implemented new JUCE-first architecture for MIDI device management
- Added MidiChannelSelector UI for per-device MIDI channel control
- Added MidiActivityIndicator for real-time MIDI activity visualization
- Enhanced Settings window with responsive layout and dynamic sizing
- Improved MIDI data flow: JUCE → Rust midi_engine (v0.1.2) → JUCE logging
- Reduced cross-language communication overhead

## **Requirements**
### **System Requirements**
- **C++**: Version 23 or higher.
- **JUCE Framework**: Version 8.0.6.
- **CMake**: Version 3.31.5 or higher.
- **Rust**: Stable version 1.84 or higher.
- **Cargo**: Rust's package manager and build system.
- **Ninja**: Version 1.12.1 or higher (for fast builds).
- **CLion (Recommended)**: IDE for development.
- **Python 3.13.2 (Optional)**: For pre-commit hooks.
- **SQLite**: Version 3.49.0 (for data storage)
- **OpenSSL**: Version 3.4.1
- **Node.js**: Version 23.7.0 (for build scripts)
- **XZ Utils**: Version 5.6.4
- **LibX11**: Version 1.8.11

## **Getting Started**

### **1. Clone the Repository**
```bash
git clone https://github.com/Raizora/MidiPortal.git
cd MidiPortal
```
### **2. Build Rust Library**
Navigate to the `rust` directory and build the Rust library:
```bash
cd rust
cargo build --release
cd ..
```
### **3. Configure CMake**
Use Ninja to configure the build system:
```bash
cmake -G Ninja -S . -B cmake-build-ninja
```
### **4. Build the Project**
Compile the project:
```bash
cmake --build cmake-build-ninja
```
### **5. Run the Targets**
- Run `MidiPortalStandalone`:
  ```bash
  ./cmake-build-ninja/standalone/MidiPortalStandalone
  ```
- Run the `AudioPlugin`:
  ```bash
  ./cmake-build-ninja/plugin/AudioPlugin_Standalone
  ```

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

### **Final Notes**
If you continue to have issues, ensure you:
✅ Run `cmake --build out/build --target help` to verify targets.
✅ Clear CMake and CLion caches if targets go missing.
✅ Keep `add_subdirectory(shared)` **commented out** in the root `CMakeLists.txt`.

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

[package]
name = "midi_engine"
version = "0.1.2"