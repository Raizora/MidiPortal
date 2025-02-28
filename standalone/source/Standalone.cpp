//
// Created by Jamie Benchia on 1/3/25.
//

#include "Standalone.h"

namespace MidiPortal {
namespace Standalone {
StandaloneWindow::StandaloneWindow()
    : DocumentWindow("MidiPortal Standalone App",
                     juce::Colours::darkgrey,
                     DocumentWindow::allButtons) {
  setContentOwned(new MidiPortal::MainComponent(), true);
  setResizable(true, true);
  setUsingNativeTitleBar(true);
  setVisible(true);
}

void StandaloneWindow::closeButtonPressed() {
  juce::JUCEApplication::getInstance()->systemRequestedQuit();
}

const juce::String StandaloneApp::getApplicationName() {
  return "MidiPortal";
}

const juce::String StandaloneApp::getApplicationVersion() {
  return "0.0.8";
}

void StandaloneApp::initialise(const juce::String&) {
  mainWindow.reset(new StandaloneWindow());
}

void StandaloneApp::shutdown() {
  mainWindow = nullptr;
}
}
}
#if JUCE_USE_CUSTOM_PLUGIN_STANDALONE_APP
START_JUCE_APPLICATION(MidiPortal::Standalone::StandaloneApp)
#endif