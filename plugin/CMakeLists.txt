cmake_minimum_required(VERSION 3.30.5)

# Version is needed by JUCE.
project(AudioPlugin VERSION 0.1.0)

# Adding a directory with the library/application name as a subfolder of the
# include folder is a good practice. It helps avoid name clashes later on.
set(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../shared/")

# Adds a plugin target (that's basically what the Projucer does).
juce_add_plugin(${PROJECT_NAME}
        COMPANY_NAME "Entropy.EXE" # Update as needed
        IS_SYNTH FALSE # May change this
        NEEDS_MIDI_INPUT TRUE # Updated to reflect MIDI usage
        NEEDS_MIDI_OUTPUT FALSE # May change this
        PLUGIN_MANUFACTURER_CODE MCMP # Update as needed
        PLUGIN_CODE EXPL # Update as needed
        FORMATS AU Standalone # macOS native format
        PRODUCT_NAME "MidiPortalPlugin" # Update to your product name
)

# Sets the source files of the plugin project.
target_sources(${PROJECT_NAME}
        PRIVATE
        source/PluginEditor.cpp
        source/PluginProcessor.cpp
        #${INCLUDE_DIR}/MainComponent.h
        #../../shared/source/MainComponent.h # Added
        ${CMAKE_CURRENT_SOURCE_DIR}/../shared/source/MainComponent.cpp # Added
)

# Sets the include directories of the plugin project.
target_include_directories(${PROJECT_NAME}
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../shared/
)

# Links to all necessary dependencies. The present ones are recommended by JUCE.
# If you use one of the additional modules, like the DSP module, you need to specify it here.
target_link_libraries(${PROJECT_NAME}
        PRIVATE
        MidiPortalShared
        juce::juce_analytics
        juce::juce_audio_basics
        juce::juce_audio_devices
        #juce::juce_audio_formats
        juce::juce_audio_plugin_client
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
        #juce::juce_product_unlocking
        #juce::juce_video
        PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# These definitions are recommended by JUCE.
target_compile_definitions(${PROJECT_NAME}
        PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

# Enables all warnings and treats warnings as errors.
# This needs to be set up only for your projects, not 3rd party
if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

# In Visual Studio this command provides a nice grouping of source files in "filters".
#source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/..)

# Ensure JUCE_SOURCE_DIR points to the manually downloaded JUCE location
set(JUCE_SOURCE_DIR "${JUCE_PATH}")

# Update the source_group command
source_group(TREE "${JUCE_SOURCE_DIR}" FILES "${JUCE_SOURCE_DIR}/modules/juce_audio_plugin_client/VST3/juce_VST3ManifestHelper.mm")
message("JUCE_SOURCE_DIR: ${JUCE_SOURCE_DIR}")
message("CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")

# X- Removed VST3 helper custom command since we're using AU format instead