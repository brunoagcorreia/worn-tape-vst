#include "PluginEditor.h"

namespace
{
    const juce::Colour bgColour   { 0xff2b2622 };
    const juce::Colour textColour { 0xffe8d9b5 };
}

TapeWornAudioProcessorEditor::TapeWornAudioProcessorEditor (TapeWornAudioProcessor& p)
    : juce::AudioProcessorEditor (&p), audioProcessor (p)
{
    for (size_t i = 0; i < knobs.size(); ++i)
    {
        auto& k = knobs[i];
        const auto& info = kParams[i];

        k.label.setText (info.name, juce::dontSendNotification);
        k.label.setJustificationType (juce::Justification::centred);
        k.label.setColour (juce::Label::textColourId, textColour);
        addAndMakeVisible (k.label);

        k.slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 18);
        k.slider.setColour (juce::Slider::textBoxTextColourId, textColour);
        addAndMakeVisible (k.slider);

        k.attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
            audioProcessor.apvts, info.id, k.slider);
    }

    setSize (560, 220);
}

void TapeWornAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (bgColour);

    g.setColour (textColour);
    g.setFont (20.0f);
    g.drawText ("TapeWorn", getLocalBounds().removeFromTop (40).reduced (14, 0),
                juce::Justification::centredLeft);
}

void TapeWornAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced (12);
    area.removeFromTop (30);                       // room for the title

    const int colWidth = area.getWidth() / (int) knobs.size();

    for (auto& k : knobs)
    {
        auto col = area.removeFromLeft (colWidth);
        k.label.setBounds (col.removeFromTop (20));
        k.slider.setBounds (col);
    }
}
