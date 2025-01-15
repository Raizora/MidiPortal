MidiPortal

**Peer through the MidiPortal to observe all traffic on your system.**

MidiPortal is a JUCE-based MIDI monitoring utility that allows you to visualize and analyze MIDI traffic in real time. Designed as a lightweight tool, MidiPortal serves as the foundation for future enhancements, ultimately building toward the full iLumidi application.

Features:

    Real-Time Monitoring: Observe MIDI messages as they traverse your system. 
    Cross-Platform Support: Built with JUCE for compatibility with macOS, Windows, and Linux.
    Highly Customizable: Modular design serves as a foundation for future functionality. 
    Lightweight Design: Optimized for performance with minimal system resource usage. 
    Robust Tooling: Effortless management of third-party dependencies using the CPM package manager. 
    Ready-to-go unit testing with GoogleTest. 
    Highest warning levels enforced with “treat warnings as errors” for maximum safety.

Roadmap:

    Visualize incoming and outgoing MIDI messages.
    Filter and categorize MIDI traffic by device, channel, or message type.	
    Planned Enhancements:
    Add educational and analytical tools.
    Expand to include MIDI manipulation and visualization features.
    Integrate with iLumidi for an all-encompassing audio-visual experience.
    Automatic log management for MIDI traffic.

Requirements:

    C++: Version 23 or higher.
    JUCE Framework: Version 8.0.4.
    CMake: Version 3.30.5 or higher.
    CLion: Recommended IDE for development.
    Cursor: Optional AI-powered coding assistant.

Getting Started

1.	Clone the repository:

	git clone https://github.com/Raizora/MidiPortal.git
	cd MidiPortal

2.	Run CMake to configure and build the project:

	cmake -S . -B build
	cmake --build build

The first build will download dependencies like CPM, JUCE, and GoogleTest.


3.	Alternatively, use bundled CMake presets:

	cmake --preset default # Uses the Ninja build system
	cmake --build build
	ctest --preset default

Presets available: default, release, and Xcode.

4.	Enable automatic code formatting on every commit:

	pre-commit install

Note: This requires pre-commit. Install it with:

	pip install pre-commit

Usage

This repository provides a modular, ready-to-extend framework for monitoring MIDI traffic.

Initial Setup:

Modify the default plugin name YourPluginName throughout the project to match your requirements.

Customization:

Easily integrate third-party libraries alongside JUCE using the CPM package manager. 
Update configurations in the CMakeLists.txt file to tailor the project to your workflow.

Contribution and Acknowledgments:

    Thank you to WolfSound for providing the free, unrestricted, public quickstart JUCE/CMake template, which served as the foundation for this project.
