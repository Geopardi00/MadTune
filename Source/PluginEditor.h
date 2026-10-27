#pragma once

#include "PluginProcessor.h"

// Pitch readout on top, the generic parameter sliders below.
class MadTuneEditor : public juce::AudioProcessorEditor,
                      private juce::Timer
{
public:
    explicit MadTuneEditor (MadTuneProcessor&);
    ~MadTuneEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    MadTuneProcessor& processor;
    juce::GenericAudioProcessorEditor sliders;

    float shownHz = 0.0f;
    float shownConfidence = 0.0f;

    static constexpr int readoutHeight = 90;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MadTuneEditor)
};
