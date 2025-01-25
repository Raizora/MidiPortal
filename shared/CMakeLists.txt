# shared/CMakeLists.txt
set(JUCE_PATH "/Users/jbenchia/JUCE" CACHE PATH "Path to JUCE")

# Create a library for shared functionality
add_library(MidiPortalShared
        source/MainComponent.cpp
        source/MainComponent.h
        source/MidiLogger.cpp
        source/MidiLogger.h
)

target_include_directories(MidiPortalShared
        PUBLIC
        ${JUCE_PATH}/modules
        ${JUCE_PATH}/extras/Projucer
)

# Extract Rust target directory
execute_process(
        COMMAND cargo metadata --format-version 1 --no-deps
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../rust
        OUTPUT_VARIABLE CARGO_METADATA
        ERROR_QUIET
)

# Extract the target directory
string(REGEX MATCH "\"target_directory\":\"([^\"]+)\"" _ ${CARGO_METADATA})
string(REGEX REPLACE ".*\"target_directory\":\"([^\"]+)\".*" "\\1" RUST_LIBRARY_PATH ${CARGO_METADATA})

# Set CMAKE_PREFIX_PATH
set(CMAKE_PREFIX_PATH "${RUST_LIBRARY_PATH}/release" PARENT_SCOPE)

# Debug: Print the extracted target directory
message("Rust library target directory: ${RUST_LIBRARY_PATH}")

# Locate the Rust library
find_library(MIDI_VISUALS_LIB NAMES midi_visuals PATHS "${RUST_LIBRARY_PATH}/release" NO_DEFAULT_PATH)

# Check if the library was found
if (NOT MIDI_VISUALS_LIB)
    message(FATAL_ERROR "Rust library 'midi_visuals' not found at ${RUST_LIBRARY_PATH}/release")
else()
    message("Rust library 'midi_visuals' found at ${MIDI_VISUALS_LIB}")
endif()

# Link the Rust library to the shared target
target_link_libraries(MidiPortalShared PRIVATE ${MIDI_VISUALS_LIB})

# Link JUCE modules to the shared library
target_link_libraries(MidiPortalShared
        PUBLIC
        juce::juce_gui_basics
        juce::juce_audio_processors
        juce::juce_audio_devices
        juce::juce_analytics
        juce::juce_audio_utils
)