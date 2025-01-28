# **MidiPortal - Version 0.0.2**
**Peer through the MidiPortal to observe all traffic on your system.**

MidiPortal is a JUCE-based MIDI monitoring utility that allows you to visualize and analyze MIDI traffic in real-time. Designed as a lightweight tool, MidiPortal serves as the foundation for future enhancements, ultimately building toward the full iLumidi application.

## **Features**
- **Real-Time Monitoring**: Observe MIDI messages as they traverse your system.
- **Cross-Platform Support**: Built with JUCE for compatibility with macOS, Windows, and Linux.
- **Highly Customizable**: Modular design serves as a foundation for future functionality.
- **Lightweight Design**: Optimized for performance with minimal system resource usage.
- **Robust Tooling**: Effortless management of third-party dependencies using the CPM package manager.
- **Unit Testing**: Ready-to-go unit testing with GoogleTest.
- **Enforced Code Quality**: Highest warning levels with “treat warnings as errors” for maximum safety.

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

## **Requirements**
### **System Requirements**
- **C++**: Version 23 or higher.
- **JUCE Framework**: Version 8.0.4.
- **CMake**: Version 3.26 or higher (minimum of 3.30.5 recommended).
- **Rust**: Stable version 1.84 or higher.
- **Cargo**: Rust's package manager and build system.
- **Ninja**: A fast build system for CMake.
- **CLion (Recommended)**: IDE for development.
- **Python (Optional)**: For pre-commit hooks.

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

### **Final Thoughts**
This README ensures that **anyone pulling the repository** has everything they need to **build, run, and understand** the purpose and future plans for **MidiPortal**.
