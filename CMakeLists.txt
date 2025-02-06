include(cmake/cpm.cmake)
# Mandatory line, sets the minimum version of CMake that should be used with this repository.
# $ cmake --version
cmake_minimum_required(VERSION 3.30.5)

# Sets a few variables, like PROJECT_NAME
project(MidiPortal)

# Always use the newest C++ standard on green-field projects if possible.
set(CMAKE_CXX_STANDARD 23)

# Specify the path to JUCE
if(DEFINED ENV{GITHUB_ACTIONS})
    set(JUCE_PATH "${CMAKE_SOURCE_DIR}/juce" CACHE PATH "Path to JUCE")
else()
    set(JUCE_PATH "/Users/jbenchia/JUCE" CACHE PATH "Path to JUCE")
endif()

# Add JUCE as a subdirectory
add_subdirectory("${JUCE_PATH}" "${CMAKE_BINARY_DIR}/JUCE")

# Set Googletest path or fetch dynamically
set(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs)

# Add Googletest
CPMAddPackage(
        NAME GOOGLETEST
        GITHUB_REPOSITORY google/googletest
        GIT_TAG v1.15.2
        VERSION 1.15.2
        SOURCE_DIR ${LIB_DIR}/googletest
        OPTIONS
        "INSTALL_GTEST OFF"
        "gtest_force_shared_crt ON"
)

# This command allows running tests from the "build" folder (the one where CMake generates the project to).
enable_testing()

# Add shared target (shared logic used by both the plugin and standalone app)
add_subdirectory(shared)

# Add standalone target (for building the standalone application)
add_subdirectory(standalone)

# Adds all the targets configured in the "plugin" folder.
add_subdirectory(plugin)

# Adds all the targets configured in the "test" folder.
#add_subdirectory(test)

# Near the top of the root CMakeLists.txt, add:
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

