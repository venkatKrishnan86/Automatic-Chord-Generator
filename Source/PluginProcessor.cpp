/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "HelperFunctions.h"
#include <torch/script.h>
#include <torch/torch.h>

//==============================================================================
AutomaticChordGeneratorAudioProcessor::AutomaticChordGeneratorAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
        forwardFFT (fftOrder)
#endif
{
    // chordGenerated.addListener(this);
    // prevChordGenerated = chordGenerated.getText();
    try {
        // Deserialize the ScriptModule from a file using torch::jit::load().
        module = torch::jit::load("/Users/venkatakrishnanvk/Desktop/Music Technology/Audio Software Engineering/Plugins/AutomaticChordGenerator/assets/models/ChordPredictor.ts");
        std::cout << "Successfully loaded the model\n";
    }
    catch (const c10::Error& e) {
        std::cerr << "error loading the model\n";
    }
}

AutomaticChordGeneratorAudioProcessor::~AutomaticChordGeneratorAudioProcessor()
{
    // chordGenerated.removeListener(this);
}

//==============================================================================
const juce::String AutomaticChordGeneratorAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AutomaticChordGeneratorAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool AutomaticChordGeneratorAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool AutomaticChordGeneratorAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double AutomaticChordGeneratorAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AutomaticChordGeneratorAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int AutomaticChordGeneratorAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AutomaticChordGeneratorAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String AutomaticChordGeneratorAudioProcessor::getProgramName (int index)
{
    return {};
}

void AutomaticChordGeneratorAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void AutomaticChordGeneratorAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    std::fill(fftData.begin(), fftData.end(), 0.0f);
    std::string currLabel = "C";
    std::vector<int> midiNotesOfChord = returnMidiNotesOfChord(currLabel, CHORD_TEMPLATE);
    for(int note: midiNotesOfChord) {
        auto* oscillator = new SineOscillator();
        auto frequency = SineOscillator::midiToFrequency(note);
        oscillator->setFrequency((float) frequency, (float) getSampleRate());
        oscillators.add(oscillator);
    }
}

void AutomaticChordGeneratorAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool AutomaticChordGeneratorAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void AutomaticChordGeneratorAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    // for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
    //     buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    // for (int channel = 0; channel < totalNumInputChannels; ++channel)
    // {
    //     auto* channelData = bufferToFill.buffer->getReadPointer (0, bufferToFill.startSample);

    //     for (auto i = 0; i < bufferToFill.numSamples; ++i)
    //         pushNextSampleIntoFifo (channelData[i]);
    // }
    auto* channelReadData = buffer.getReadPointer(0);
    for (auto i = 0; i < buffer.getNumSamples(); ++i) {
        if (fftIndex==fftSize-1)
            break;
        fftData[fftIndex++] = channelReadData[i];
    }

    if(fftIndex==fftSize-1){
        chromaVector = torch::zeros({6, 12});
        forwardFFT.performFrequencyOnlyForwardTransform (fftData.data());
        std::vector<float> inputFftData(fftData.begin(), fftData.end());
        auto FftTensor = torch::tensor(inputFftData, {torch::kFloat});
        auto chromaResult = calculate_chroma_spectrum(FftTensor, getSampleRate());
        for(auto i = 0; i < 6; i++) {
            chromaVector[i] = chromaResult;
        }
        chromaVector = torch::stack({at::unsqueeze(chromaVector.transpose(0, 1), 0), at::unsqueeze(chromaVector.transpose(0, 1), 0)});
        std::vector<torch::jit::IValue> inputs {chromaVector};
        std::string chordPred = predictChord(module.forward(inputs).toTensor()[0], CHORD_TEMPLATE);
        // std::string chordPred = predictChord(chromaResult, CHORD_TEMPLATE);
        chordGenerated->setText(chordPred, juce::NotificationType::sendNotification);
        std::fill(fftData.begin(), fftData.end(), 0.0f);
        fftIndex = 0;

        if(prevChordGenerated != chordPred){
            oscillators.clear();
            std::vector<int> midiNotesOfChord = returnMidiNotesOfChord(chordPred, CHORD_TEMPLATE);
            for(int note: midiNotesOfChord) {
                auto* oscillator = new SineOscillator();
                auto frequency = SineOscillator::midiToFrequency(note);
                oscillator->setFrequency((float) frequency, (float) getSampleRate());
                oscillators.add(oscillator);
            }
            prevChordGenerated = chordPred;
        }
    }

    auto* leftChannelData = buffer.getWritePointer(0);
    auto* rightChannelData = buffer.getWritePointer(1);
    for (auto i = 0; i < buffer.getNumSamples(); ++i) {
        float sampleTotal = 0.0;
        for(auto& oscillator: oscillators) {
            sampleTotal += oscillator->getNextSample();
        }
        leftChannelData[i] = sampleTotal/3.0;
        rightChannelData[i] = sampleTotal/3.0;
    }
}

// void AutomaticChordGeneratorAudioProcessor::pushNextSampleIntoFifo (float sample) noexcept
// {
//     // if the fifo contains enough data, set a flag to say
//     // that the next line should now be rendered..
//     if (fifoIndex == fftSize)       // [8]
//     {
//         if (!nextFFTBlockReady)    // [9]
//         {
//             std::fill(fftData.begin(), fftData.end(), 0.0f);
//             std::copy(fifo.begin(), fifo.end(), fftData.begin());
//             nextFFTBlockReady = true;
//         }

//         fifoIndex = 0;
//     }

//     fifo[(size_t) fifoIndex++] = sample; // [9]
// }

//==============================================================================
bool AutomaticChordGeneratorAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* AutomaticChordGeneratorAudioProcessor::createEditor()
{
    return new AutomaticChordGeneratorAudioProcessorEditor (*this);
}

//==============================================================================
void AutomaticChordGeneratorAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void AutomaticChordGeneratorAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AutomaticChordGeneratorAudioProcessor();
}
