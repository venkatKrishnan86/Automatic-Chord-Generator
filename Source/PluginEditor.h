/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "ChordComponent.h"

//==============================================================================
/**
*/
class AutomaticChordGeneratorAudioProcessorEditor  
  : public juce::AudioProcessorEditor,
  private juce::Label::Listener
{
public:
    AutomaticChordGeneratorAudioProcessorEditor (AutomaticChordGeneratorAudioProcessor&);
    ~AutomaticChordGeneratorAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void labelTextChanged(juce::Label*) override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    AutomaticChordGeneratorAudioProcessor& audioProcessor;
    ChordComponent chordComponent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AutomaticChordGeneratorAudioProcessorEditor)
};
