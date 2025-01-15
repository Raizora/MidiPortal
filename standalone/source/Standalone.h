//
// Created by Jamie Benchia on 1/3/25.
//

#ifndef STANDALONEAPP_H
#define STANDALONEAPP_H

#include "../../shared/source/MainComponent.h"

namespace MidiPortal {
namespace Standalone {
class StandaloneWindow : public juce::DocumentWindow {
public:
  StandaloneWindow();
  void closeButtonPressed() override;
};

class StandaloneApp : public juce::JUCEApplication {
public:
  const juce::String getApplicationName() override;
  const juce::String getApplicationVersion() override;
  void initialise(const juce::String&) override;
  void shutdown() override;

private:
  std::unique_ptr<StandaloneWindow> mainWindow;
};
}
}

#endif // STANDALONEAPP_H