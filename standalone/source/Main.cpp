/**
 * @file Main.cpp
 * @brief Main entry point for the MidiPortal standalone application.
 * 
 * This file contains the MidiPortalApplication class, which is the main application
 * class for the MidiPortal standalone application. It handles application lifecycle
 * events such as initialization, shutdown, and system requests to quit.
 * 
 * The file also contains the MainWindow class, which is the main window of the
 * application and contains the MainComponent that provides the core functionality.
 */

//
// Created by Jamie Benchia on 1/3/25.
//
#include "../../shared/source/MainComponent.h"
#include <juce_analytics/juce_analytics.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_devices/juce_audio_devices.h>
//#include <juce_audio_formats/juce_audio_formats.h>
//#include <juce_audio_plugin_client/juce_audio_plugin_client.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_box2d/juce_box2d.h>
#include <juce_core/juce_core.h>
//#include <juce_cryptography/juce_cryptography.h>
#include <juce_data_structures/juce_data_structures.h>
//#include <juce_dsp/juce_dsp.h>
#include <juce_events/juce_events.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_javascript/juce_javascript.h>
#include <juce_midi_ci/juce_midi_ci.h>
#include <juce_opengl/juce_opengl.h>
#include <juce_osc/juce_osc.h>
//#include <juce_product_unlocking/juce_product_unlocking.h>
//#include <juce_video/juce_video.h>


/**
 * @class MidiPortalApplication
 * @brief Main application class for the MidiPortal standalone application.
 * 
 * This class handles the application lifecycle, including initialization,
 * shutdown, and system requests to quit. It creates and manages the main
 * application window.
 * 
 * The class extends juce::JUCEApplication, which provides the core
 * functionality for a JUCE application.
 */
class MidiPortalApplication : public juce::JUCEApplication
{
public:
    //==============================================================================
    /**
     * @brief Default constructor.
     * 
     * Initializes the application instance.
     */
    MidiPortalApplication() {}

    /**
     * @brief Gets the application name.
     * @return The name of the application ("MidiPortal").
     * 
     * This name is used in various places, such as the application menu
     * on macOS and the window title.
     */
    const juce::String getApplicationName() override       { return "MidiPortal"; }
    
    /**
     * @brief Gets the application version.
     * @return The version of the application.
     * 
     * This version is used in various places, such as the about box
     * and for checking for updates.
     */
    const juce::String getApplicationVersion() override    { return "0.1.0"; }
    
    /**
     * @brief Determines if multiple instances of the application are allowed.
     * @return true if multiple instances are allowed, false otherwise.
     * 
     * MidiPortal allows multiple instances to run simultaneously, which
     * can be useful for monitoring multiple sets of MIDI devices.
     */
    bool moreThanOneInstanceAllowed() override             { return true; }

    //==============================================================================
    /**
     * @brief Initializes the application.
     * @param commandLine The command line arguments passed to the application.
     * 
     * This method is called when the application starts. It creates the main
     * application window and performs any other necessary initialization.
     */
    void initialise (const juce::String& commandLine) override
    {
        juce::ignoreUnused(commandLine);
        
        mainWindow.reset (new MainWindow (getApplicationName()));
    }

    /**
     * @brief Shuts down the application.
     * 
     * This method is called when the application is about to exit. It releases
     * the main window and performs any other necessary cleanup.
     */
    void shutdown() override
    {
        mainWindow = nullptr;
    }

    //==============================================================================
    /**
     * @brief Handles a system request to quit the application.
     * 
     * This method is called when the system requests that the application quit,
     * such as when the user clicks the close button on the main window or
     * selects Quit from the application menu.
     */
    void systemRequestedQuit() override
    {
        quit();
    }

    /**
     * @brief Handles another instance of the application being started.
     * @param commandLine The command line arguments passed to the new instance.
     * 
     * This method is called when another instance of the application is started
     * while this instance is already running. It allows the application to
     * respond to the new instance, such as by bringing the existing window
     * to the front or processing command line arguments.
     */
    void anotherInstanceStarted (const juce::String& commandLine) override
    {
        juce::ignoreUnused(commandLine);
        
        // Handle when another instance of the app is started
    }

    //==============================================================================
    /**
     * @class MainWindow
     * @brief Main window of the MidiPortal application.
     * 
     * This class represents the main window of the application, which contains
     * the MainComponent that provides the core functionality.
     * 
     * The class extends juce::DocumentWindow, which provides the basic window
     * functionality such as title bar, minimize/maximize/close buttons, and
     * resizing.
     */
    class MainWindow : public juce::DocumentWindow
    {
    public:
        /**
         * @brief Constructor that creates the main window.
         * @param name The name of the window, which will appear in the title bar.
         * 
         * Creates the main window with the specified name and sets up the
         * MainComponent as its content.
         */
        MainWindow (juce::String name)
            : DocumentWindow (name,
                              juce::Desktop::getInstance().getDefaultLookAndFeel()
                                                      .findColour (juce::ResizableWindow::backgroundColourId),
                              DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar (true);
            setContentOwned (new MidiPortal::MainComponent(), true);

            //  changes to implement the DPI‐aware scaling for your main window and to add maximize (and minimize) buttons to additional windows
            #if JUCE_IOS || JUCE_ANDROID
                setFullScreen (true);
            #else
                setResizable (true, true);
                // X- Retrieve the current main display's user area (DPI-scaled)
                auto mainDisplay = juce::Desktop::getInstance().getDisplays().getMainDisplay();
                auto userArea = mainDisplay.userArea;
                // X- Set the resize limits to use the display's full resolution
                setResizeLimits(600, 400, userArea.getWidth(), userArea.getHeight());
                // X- Center the window using the full display resolution
                centreWithSize(userArea.getWidth(), userArea.getHeight());
            #endif

            setVisible (true);
        }

        /**
         * @brief Handles the window close button being pressed.
         * 
         * This method is called when the user clicks the close button on the
         * window. It requests that the application quit.
         */
        void closeButtonPressed() override
        {
            JUCEApplication::getInstance()->systemRequestedQuit();
        }

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    };

private:
    /**
     * @brief Pointer to the main window of the application.
     * 
     * This unique_ptr manages the lifetime of the main window, ensuring
     * that it is properly destroyed when the application exits.
     */
    std::unique_ptr<MainWindow> mainWindow;
};

//==============================================================================
/**
 * @brief Macro that generates the main() function for the application.
 * 
 * This macro creates the main() function that is the entry point for the
 * application. It creates an instance of the MidiPortalApplication class
 * and starts the application.
 */
START_JUCE_APPLICATION (MidiPortalApplication)