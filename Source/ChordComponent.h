/*
  ==============================================================================

    ChordComponent.h
    Created: 22 May 2024 9:17:19am
    Author:  Venkatakrishnan V K

  ==============================================================================
*/

#include <JuceHeader.h>

#pragma once

struct ChordComponent : juce::Component, juce::Label::Listener
{
    ChordComponent() {
        midiNoteName.setText("C", juce::NotificationType::dontSendNotification);
        midiNoteName.addListener(this);
    }
    
    ~ChordComponent() {
        midiNoteName.removeListener(this);
    }
    
    void paint (juce::Graphics& g) override
    {
        g.fillAll (juce::Colour(132, 193, 249));
        
        g.setColour(juce::Colours::green);
        juce::Rectangle<int> box (getWidth()/4, getHeight()/4, getWidth()/2, getHeight()/2);
        juce::Path roundedRectangle;
        roundedRectangle.addRoundedRectangle(box, 20.f, 20.f); // Check later
        g.fillPath(roundedRectangle);
        
        g.setColour(juce::Colours::white);
        g.drawText("Chord Generated: "+juce::String(midiNoteName.getText()), 0, (getHeight()/2)-10, getWidth(), 20, juce::Justification::centred, true);
    }
    
    void resized() override
    {
        
    }
    
    void labelTextChanged(juce::Label* text) override
    {
        repaint();
    }
    
    juce::Label* getNoteNameAddress() {
        return &midiNoteName;
    }
    
private:
    juce::Label midiNoteName;
};
