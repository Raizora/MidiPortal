/**
 * @file Standalone.h
 * @brief Defines the standalone application classes for MidiPortal.
 * 
 * This file contains the declarations for the StandaloneWindow and StandaloneApp classes,
 * which together form the standalone application version of MidiPortal. The standalone
 * application provides the same functionality as the plugin version but runs as an
 * independent application rather than within a DAW.
 */



#ifndef STANDALONEAPP_H
#define STANDALONEAPP_H

#include "../../shared/source/MainComponent.h"

namespace MidiPortal {
namespace Standalone {

/**
 * @class StandaloneWindow
 * @brief The main window for the standalone application.
 * 
 * This class represents the main window of the standalone application.
 * It creates and manages the MainComponent that provides the core functionality
 * of the MidiPortal application.
 */
class StandaloneWindow : public juce::DocumentWindow {
public:
  /**
   * @brief Constructor that creates the main window.
   * 
   * Initializes the window with a title, background color, and standard window buttons.
   * Creates a MainComponent as the content of the window and makes the window visible.
   */
  StandaloneWindow();
  
  /**
   * @brief Handles the window close button being pressed.
   * 
   * Called when the user clicks the close button on the window.
   * Requests that the application quit by calling systemRequestedQuit().
   */
  void closeButtonPressed() override;
};

/**
 * @class StandaloneApp
 * @brief The main application class for the standalone application.
 * 
 * This class represents the standalone application itself. It handles application
 * lifecycle events such as initialization and shutdown, and creates the main window.
 */
class StandaloneApp : public juce::JUCEApplication {
public:
  /**
   * @brief Gets the application name.
   * @return The name of the application ("MidiPortal").
   * 
   * This name is used in various places, such as the application menu
   * on macOS and the window title.
   */
  const juce::String getApplicationName() override;
  
  /**
   * @brief Gets the application version.
   * @return The version of the application.
   * 
   * This version is used in various places, such as the about box
   * and for checking for updates.
   */
  const juce::String getApplicationVersion() override;
  
  /**
   * @brief Initializes the application.
   * @param commandLine The command line arguments passed to the application.
   * 
   * This method is called when the application starts. It creates the main
   * application window and performs any other necessary initialization.
   */
  void initialise(const juce::String& commandLine) override;
  
  /**
   * @brief Shuts down the application.
   * 
   * This method is called when the application is about to exit. It releases
   * the main window and performs any other necessary cleanup.
   */
  void shutdown() override;

private:
  /**
   * @brief Pointer to the main window of the application.
   * 
   * This unique_ptr manages the lifetime of the main window, ensuring
   * that it is properly destroyed when the application exits.
   */
  std::unique_ptr<StandaloneWindow> mainWindow;
};

} // namespace Standalone
} // namespace MidiPortal

#endif // STANDALONEAPP_H