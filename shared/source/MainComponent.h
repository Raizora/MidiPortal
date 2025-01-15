//
// Created by Jamie Benchia on 1/3/25.
//

#ifndef MAINCOMPONENT_H
#define MAINCOMPONENT_H

#include <../../libs/juce/modules/juce_gui_basics/juce_gui_basics.h>

namespace MidiPortal {
class MainComponent : public juce::Component {
public:
  MainComponent();
  ~MainComponent() override;

  void paint(juce::Graphics& g) override;
  void resized() override;

private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
}

#endif // MAINCOMPONENT_H