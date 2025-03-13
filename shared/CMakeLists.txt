# shared/CMakeLists.txt
set(JUCE_PATH "/Users/jbenchia/JUCE" CACHE PATH "Path to JUCE")

# Create a library for shared functionality
if(NOT TARGET MidiPortalShared)
    add_library(MidiPortalShared STATIC
            source/MainComponent.cpp
            source/MainComponent.h
            source/MidiLogger.cpp
            source/MidiLogger.h
            source/SettingsComponent.cpp
            source/SettingsComponent.h
            source/SettingsWindow.h
            source/RustMidiEngine.h
            source/MidiActivityIndicator.cpp
            source/MidiActivityIndicator.h
            source/MidiChannelSelector.cpp
            source/MidiChannelSelector.h
            source/MidiLogDisplay.cpp
            source/MidiLogDisplay.h
            source/LogDisplaySettingsComponent.cpp
            source/LogDisplaySettingsComponent.h
            source/LogDisplaySettingsWindow.h
            source/WindowManager.cpp
            source/WindowManager.h
            source/DisplaySettingsManager.cpp
            source/DisplaySettingsManager.h
            source/WindowRoutingComponent.cpp
            source/WindowRoutingComponent.h
            source/SharedMidiBuffer.cpp
            source/SharedMidiBuffer.h
            source/SettingsManager.h
            source/MidiAIManager.cpp
            source/MidiAIManager.h
            source/AIInsightComponent.cpp
            source/AIInsightComponent.h
    )

    target_include_directories(MidiPortalShared
            PUBLIC
            ${JUCE_PATH}/modules
            ${CMAKE_BINARY_DIR}/JUCE/modules
            ${CMAKE_BINARY_DIR}/JuceLibraryCode
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

    # Use the path provided by the build script if available
    if(DEFINED RUST_LIBRARY_PATH)
        set(RUST_LIB_SEARCH_PATH "${RUST_LIBRARY_PATH}")
    else()
        set(RUST_LIB_SEARCH_PATH "${CMAKE_SOURCE_DIR}/rust/target")
    endif()

    # Look for the Rust library
    message(STATUS "Looking for Rust library in: ${RUST_LIB_SEARCH_PATH}/release/deps and ${RUST_LIB_SEARCH_PATH}/debug/deps")
    find_library(MIDI_ENGINE_LIB 
        NAMES midi_engine
        PATHS 
            "${RUST_LIB_SEARCH_PATH}/release/deps"
            "${RUST_LIB_SEARCH_PATH}/debug/deps"
        NO_DEFAULT_PATH
        REQUIRED
    )
    message(STATUS "MIDI_ENGINE_LIB = ${MIDI_ENGINE_LIB}")

    # Link libraries in correct order
    target_link_libraries(MidiPortalShared
        PRIVATE
        ${MIDI_ENGINE_LIB}  # Must be first
        PUBLIC
        juce::juce_core
        juce::juce_gui_basics
        juce::juce_audio_processors
        juce::juce_audio_devices
        juce::juce_analytics
        juce::juce_audio_utils
    )

    target_compile_definitions(MidiPortalShared
        PUBLIC
        JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1
        JUCE_MODULE_AVAILABLE_juce_core=1
        JUCE_MODULE_AVAILABLE_juce_data_structures=1
        JUCE_MODULE_AVAILABLE_juce_events=1
        JUCE_MODULE_AVAILABLE_juce_graphics=1
        JUCE_MODULE_AVAILABLE_juce_gui_basics=1
        JUCE_MODULE_AVAILABLE_juce_audio_basics=1
        JUCE_MODULE_AVAILABLE_juce_audio_devices=1
        JUCE_MODULE_AVAILABLE_juce_audio_processors=1
        JUCE_MODULE_AVAILABLE_juce_audio_utils=1
        JUCE_MODULE_AVAILABLE_juce_analytics=1
    )

    # Make sure RustMidiEngine.h is included in the build
    target_sources(MidiPortalShared 
        PRIVATE
        source/RustMidiEngine.h
    )

    # Add the Rust library directory to runtime path
    set_target_properties(MidiPortalShared PROPERTIES
        BUILD_WITH_INSTALL_RPATH TRUE
        INSTALL_RPATH "${RUST_LIB_SEARCH_PATH}/release/deps"
    )
endif()