/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "ChordComponent.h"

//==============================================================================
AutomaticChordGeneratorAudioProcessorEditor::AutomaticChordGeneratorAudioProcessorEditor (AutomaticChordGeneratorAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    addAndMakeVisible(chordComponent);
    audioProcessor.chordGenerated = chordComponent.getNoteNameAddress();
    audioProcessor.chordGenerated->setText("C", juce::NotificationType::sendNotification);
    setSize (400, 300);
}

AutomaticChordGeneratorAudioProcessorEditor::~AutomaticChordGeneratorAudioProcessorEditor()
{
}

//==============================================================================
void AutomaticChordGeneratorAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    // g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    // g.setColour (juce::Colours::white);
    // g.setFont (15.0f);
    // g.drawFittedText ("Chord Generated: " + audioProcessor.chordGenerated.getText(), getLocalBounds(), juce::Justification::centred, 1);
}

void AutomaticChordGeneratorAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    chordComponent.setBounds(0, 0, getWidth(), getHeight());
}

void AutomaticChordGeneratorAudioProcessorEditor::labelTextChanged(juce::Label* text)
{
    // Update the GUI
    repaint();
}
