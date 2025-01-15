# Specify the minimum version of CMake required
cmake_minimum_required(VERSION 3.30.5)

# Set the project name and version
project(MidiPortalStandalone VERSION 0.1.0)

# Use C++ 23 standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Create an executable for the standalone app
add_executable(MidiPortalStandalone
        source/Main.cpp
)

# Add shared directory
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../shared shared)

# Set include directories for the standalone target
target_include_directories(MidiPortalStandalone
        PRIVATE
        ${JUCE_SOURCE_DIR}/modules
        ${CMAKE_CURRENT_SOURCE_DIR}/../shared/source
)

# Add the JUCE_USE_CUSTOM_PLUGIN_STANDALONE_APP definition
target_compile_definitions(MidiPortalStandalone
        PRIVATE
        JUCE_USE_CUSTOM_PLUGIN_STANDALONE_APP=1 # X- Enables custom standalone app support in JUCE
)

# Link the shared library and JUCE modules to the standalone app
target_link_libraries(MidiPortalStandalone
        PRIVATE
        MidiPortalShared
        juce::juce_analytics
        juce::juce_audio_basics
        juce::juce_audio_devices
        #juce::juce_audio_formats
        #juce::juce_audio_plugin_client
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_box2d
        juce::juce_core
        #juce::juce_cryptography
        juce::juce_data_structures
        #juce::juce_dsp
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
        juce::juce_javascript
        juce::juce_midi_ci
        juce::juce_opengl
        juce::juce_osc
)