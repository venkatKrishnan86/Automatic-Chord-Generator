/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "HelperFunctions.h"

//==============================================================================
/**
*/
class SineOscillator
{
public:
    SineOscillator() {}
    
    void setFrequency (float frequency, float sampleRate)
    {
        auto cyclesPerSample = frequency / sampleRate;
        angleDelta = cyclesPerSample * juce::MathConstants<float>::twoPi;
    }

    forcedinline float getNextSample() noexcept
    {
        auto currentSample = std::sin (currentAngle);
        updateAngle();
        return currentSample;
    }

    forcedinline void updateAngle() noexcept
    {
        currentAngle += angleDelta;
        if (currentAngle >= juce::MathConstants<float>::twoPi)
            currentAngle -= juce::MathConstants<float>::twoPi;
    }

    static float midiToFrequency(float midiNote) {
        return 440.0 * pow (2.0, (midiNote - 69.0) / 12.0);
    }
 
private:
    float currentAngle = 0.0f, angleDelta = 0.0f;
};


class AutomaticChordGeneratorAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    AutomaticChordGeneratorAudioProcessor();
    ~AutomaticChordGeneratorAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    static constexpr auto fftOrder = 12;                // [1]
    static constexpr auto fftSize  = 1 << fftOrder;     // [2] (N = 4096)

    juce::Label* chordGenerated;

private:
    // void pushNextSampleIntoFifo(float) noexcept;
    
    juce::OwnedArray<SineOscillator> oscillators;
    std::string prevChordGenerated = "C";
    torch::jit::script::Module module;
    torch::Tensor chromaVector;
    
    juce::dsp::FFT forwardFFT;
    std::array<float, fftSize> fftData;
    int fftIndex;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AutomaticChordGeneratorAudioProcessor)
};
