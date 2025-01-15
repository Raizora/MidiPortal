set(JUCE_PATH "/Users/jbenchia/JUCE Projects/JUCE")  # Update this path accordingly

# Create a library for shared functionality
add_library(MidiPortalShared
        source/MainComponent.cpp
        source/MainComponent.h
)

# Set include directories for the shared library
target_include_directories(MidiPortalShared
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${JUCE_PATH}/modules
        ${JUCE_PATH}/extras/Projucer
        /Users/jbenchia/JUCE/Projects/JUCE/modules
)

# Link JUCE modules to the shared library
target_link_libraries(MidiPortalShared
        PUBLIC
        juce::juce_gui_basics
        juce::juce_audio_processors
        juce::juce_audio_devices
)