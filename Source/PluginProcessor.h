#pragma once

#include <array>
#include <atomic>
#include <juce_audio_processors/juce_audio_processors.h>
#include "TapeDSP.h"

// Single source of truth for the parameters. The editor builds its knobs from
// this table, the processor builds its parameter layout from it.
struct ParamInfo
{
    const char* id;     // stable ID, never rename once released (breaks saved projects)
    const char* name;   // display name
    float       def;    // default, 0..1
};

inline constexpr int kNumParams = 7;
inline constexpr ParamInfo kParams[kNumParams] = {
    { "wow",       "Wow",       0.3f },
    { "flutter",   "Flutter",   0.2f },
    { "bandwidth", "Bandwidth", 0.5f },
    { "drive",     "Drive",     0.3f },
    { "hiss",      "Hiss",      0.1f },
    { "dropouts",  "Dropouts",  0.2f },
    { "pops",      "Pops",      0.1f },
};

class TapeWornAudioProcessor : public juce::AudioProcessor
{
public:
    TapeWornAudioProcessor();
    ~TapeWornAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override  { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

    TapeDSP dsp;
    int preparedChannels = 2;

    // Cached raw pointers: reading them on the audio thread is lock-free.
    std::atomic<float>* wowP       = nullptr;
    std::atomic<float>* flutterP   = nullptr;
    std::atomic<float>* bandwidthP = nullptr;
    std::atomic<float>* driveP     = nullptr;
    std::atomic<float>* hissP      = nullptr;
    std::atomic<float>* dropoutsP  = nullptr;
    std::atomic<float>* popsP      = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TapeWornAudioProcessor)
};
