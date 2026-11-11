#pragma once

#include <array>
#include <memory>
#include "PluginProcessor.h"

class TapeWornAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit TapeWornAudioProcessorEditor (TapeWornAudioProcessor&);
    ~TapeWornAudioProcessorEditor() override = default;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    struct Knob
    {
        juce::Slider slider { juce::Slider::RotaryHorizontalVerticalDrag,
                              juce::Slider::TextBoxBelow };
        juce::Label label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    };

    TapeWornAudioProcessor& audioProcessor;
    std::array<Knob, kNumParams> knobs;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TapeWornAudioProcessorEditor)
};
