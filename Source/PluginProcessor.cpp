#include "PluginProcessor.h"
#include "PluginEditor.h"

TapeWornAudioProcessor::TapeWornAudioProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", createLayout())
{
    wowP       = apvts.getRawParameterValue ("wow");
    flutterP   = apvts.getRawParameterValue ("flutter");
    bandwidthP = apvts.getRawParameterValue ("bandwidth");
    driveP     = apvts.getRawParameterValue ("drive");
    hissP      = apvts.getRawParameterValue ("hiss");
    dropoutsP  = apvts.getRawParameterValue ("dropouts");
    popsP      = apvts.getRawParameterValue ("pops");
}

juce::AudioProcessorValueTreeState::ParameterLayout TapeWornAudioProcessor::createLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    for (const auto& p : kParams)
        layout.add (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { p.id, 1 },          // version hint, needed for AU
            p.name,
            juce::NormalisableRange<float> (0.0f, 1.0f),
            p.def));

    return layout;
}

void TapeWornAudioProcessor::prepareToPlay (double sampleRate, int /*samplesPerBlock*/)
{
    preparedChannels = juce::jmax (1, getTotalNumOutputChannels());
    dsp.prepare (sampleRate, preparedChannels);
}

bool TapeWornAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& out = layouts.getMainOutputChannelSet();

    if (out != juce::AudioChannelSet::mono() && out != juce::AudioChannelSet::stereo())
        return false;

    return out == layouts.getMainInputChannelSet();
}

void TapeWornAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    // Clear any output channels that have no matching input
    for (int ch = getTotalNumInputChannels(); ch < getTotalNumOutputChannels(); ++ch)
        buffer.clear (ch, 0, buffer.getNumSamples());

    // Copy the current parameter values into the DSP core
    auto& p = dsp.p;
    p.wow       = wowP->load();
    p.flutter   = flutterP->load();
    p.bandwidth = bandwidthP->load();
    p.drive     = driveP->load();
    p.hiss      = hissP->load();
    p.dropouts  = dropoutsP->load();
    p.pops      = popsP->load();

    const int numCh = juce::jmin (buffer.getNumChannels(), preparedChannels);
    dsp.process (buffer.getArrayOfWritePointers(), numCh, buffer.getNumSamples());
}

juce::AudioProcessorEditor* TapeWornAudioProcessor::createEditor()
{
    return new TapeWornAudioProcessorEditor (*this);
}

void TapeWornAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void TapeWornAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

// This creates new instances of the plugin; required by JUCE.
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TapeWornAudioProcessor();
}
