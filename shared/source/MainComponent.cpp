//
// Created by Jamie Benchia on 1/3/25.
//

#include "MainComponent.h"

namespace MidiPortal {
MainComponent::MainComponent() {
  // Initialize UI components here
}

MainComponent::~MainComponent() {}

void MainComponent::paint(juce::Graphics& g) {
  g.fillAll(juce::Colours::black);
  g.setColour(juce::Colours::white);
  g.setFont(20.0f);
  g.drawText("MidiPortal", getLocalBounds(), juce::Justification::centred, true);
}

void MainComponent::resized() {
  // Resize child components if needed
}
}