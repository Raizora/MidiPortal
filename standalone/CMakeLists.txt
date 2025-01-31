# Specify the minimum version of CMake required
cmake_minimum_required(VERSION 3.30.5)

# Set the project name and version
project(MidiPortalStandalone VERSION 0.1.0)

# Use C++ 23 standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Ensure GTK3 is found before building
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

# Include GTK3 headers and link its libraries
include_directories(${GTK3_INCLUDE_DIRS})
link_directories(${GTK3_LIBRARY_DIRS})
add_definitions(${GTK3_CFLAGS_OTHER})

# Create an executable for the standalone app
add_executable(MidiPortalStandalone
        source/Main.cpp
)

# Add shared directory
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../shared shared)

# Set include directories for the standalone target
target_include_directories(MidiPortalStandalone
        PRIVATE
        ${JUCE_PATH}/modules
        ${CMAKE_CURRENT_SOURCE_DIR}/../shared/source
        ${GTK3_INCLUDE_DIRS}  # Ensure GTK3 include paths are set
)

# Find and link the Rust library (midi_visuals)
find_library(MIDI_VISUALS_LIB NAMES midi_visuals)
if(NOT MIDI_VISUALS_LIB)
    # Use CMAKE_PREFIX_PATH so find_library can locate the Rust target dir
    execute_process(COMMAND cargo metadata --format-version 1 --no-deps
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE CARGO_METADATA)
    string(JSON target_directory CARGO_TARGET_DIRECTORY ${CARGO_METADATA})
    set(CMAKE_PREFIX_PATH ${CARGO_TARGET_DIRECTORY}/release ${CMAKE_PREFIX_PATH}) # or /debug
    find_library(MIDI_VISUALS_LIB NAMES midi_visuals)  # Try again after adding the path
    if(NOT MIDI_VISUALS_LIB)
        message(FATAL_ERROR "Rust library midi_visuals still not found even with CMAKE_PREFIX_PATH")
    endif()
endif()

# Add the JUCE_USE_CUSTOM_PLUGIN_STANDALONE_APP definition
target_compile_definitions(MidiPortalStandalone
        PRIVATE
        JUCE_USE_CUSTOM_PLUGIN_STANDALONE_APP=1  # Enables custom standalone app support in JUCE
)

# Link the shared library, JUCE modules, and GTK3 to the standalone app
target_link_libraries(MidiPortalStandalone
        PRIVATE
        MidiPortalShared
        juce::juce_analytics
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        # juce::juce_audio_plugin_client
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_box2d
        juce::juce_core
        # juce::juce_cryptography
        juce::juce_data_structures
        # juce::juce_dsp
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
        juce::juce_javascript
        juce::juce_midi_ci
        juce::juce_opengl
        juce::juce_osc
        ${GTK3_LIBRARIES}  # ✅ Ensures GTK3 is linked
)