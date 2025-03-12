/**
 * @file Standalone.cpp
 * @brief Implementation of the standalone application classes for MidiPortal.
 * 
 * This file contains the implementation of the StandaloneWindow and StandaloneApp classes,
 * which together form the standalone application version of MidiPortal. The standalone
 * application provides the same functionality as the plugin version but runs as an
 * independent application rather than within a DAW.
 */

#include "Standalone.h"

namespace MidiPortal {
namespace Standalone {

/**
 * @brief Constructor that creates the main window.
 * 
 * Initializes the window with a title ("MidiPortal Standalone App"), a dark grey
 * background color, and standard window buttons (minimize, maximize, and close).
 * Creates a MainComponent as the content of the window, makes the window resizable,
 * uses the native title bar of the operating system, and makes the window visible.
 */
StandaloneWindow::StandaloneWindow()
    : DocumentWindow("MidiPortal Standalone App",
                     juce::Colours::darkgrey,
                     DocumentWindow::allButtons) {
  setContentOwned(new MidiPortal::MainComponent(), true);
  setResizable(true, true);
  setUsingNativeTitleBar(true);
  setVisible(true);
}

/**
 * @brief Handles the window close button being pressed.
 * 
 * Called when the user clicks the close button on the window.
 * Requests that the application quit by calling systemRequestedQuit()
 * on the JUCEApplication instance.
 */
void StandaloneWindow::closeButtonPressed() {
  juce::JUCEApplication::getInstance()->systemRequestedQuit();
}

/**
 * @brief Gets the application name.
 * @return The name of the application ("MidiPortal").
 * 
 * This name is used in various places, such as the application menu
 * on macOS and the window title.
 */
const juce::String StandaloneApp::getApplicationName() {
  return "MidiPortal";
}

/**
 * @brief Gets the application version.
 * @return The version of the application ("0.0.8").
 * 
 * This version is used in various places, such as the about box
 * and for checking for updates.
 */
const juce::String StandaloneApp::getApplicationVersion() {
  return "0.0.8";
}

/**
 * @brief Initializes the application.
 * @param commandLine The command line arguments passed to the application (unused).
 * 
 * This method is called when the application starts. It creates the main
 * application window (StandaloneWindow) and performs any other necessary initialization.
 */
void StandaloneApp::initialise(const juce::String&) {
  mainWindow.reset(new StandaloneWindow());
}

/**
 * @brief Shuts down the application.
 * 
 * This method is called when the application is about to exit. It releases
 * the main window by resetting the unique_ptr, which triggers the window's
 * destructor and performs any other necessary cleanup.
 */
void StandaloneApp::shutdown() {
  mainWindow = nullptr;
}
}
}

/**
 * @brief Macro that generates the main() function for the application.
 * 
 * This macro creates the main() function that is the entry point for the
 * standalone application. It creates an instance of the StandaloneApp class
 * and starts the application. This is only used when building the standalone
 * application, not when building the plugin.
 */
#if JUCE_USE_CUSTOM_PLUGIN_STANDALONE_APP
START_JUCE_APPLICATION(MidiPortal::Standalone::StandaloneApp)
#endif