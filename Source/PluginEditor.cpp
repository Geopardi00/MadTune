#include "PluginEditor.h"

MadTuneEditor::MadTuneEditor (MadTuneProcessor& p)
    : AudioProcessorEditor (p), processor (p), sliders (p)
{
    addAndMakeVisible (sliders);
    setSize (420, readoutHeight + juce::jmax (200, sliders.getHeight()));
    startTimerHz (30);
}

MadTuneEditor::~MadTuneEditor()
{
    stopTimer();
}

void MadTuneEditor::timerCallback()
{
    // Repaint every tick: the target also changes when Key or Scale move.
    shownHz = processor.detectedHz.load();
    shownConfidence = processor.detectedConfidence.load();
    repaint (0, 0, getWidth(), readoutHeight);
}

void MadTuneEditor::paint (juce::Graphics& g)
{
    auto area = getLocalBounds().removeFromTop (readoutHeight).reduced (12, 8);
    g.fillAll (juce::Colour (0xff1e1f24));

    if (shownHz <= 0.0f)
    {
        g.setColour (juce::Colours::grey);
        g.setFont (juce::FontOptions (22.0f));
        g.drawText ("No pitch", area, juce::Justification::centredLeft);
        return;
    }

    const double midi = madtune::frequencyToMidi (shownHz);
    const int nearest = (int) std::lround (midi);
    const double cents = (midi - nearest) * 100.0;
    const int target = (int) madtune::snapToScale (midi, processor.getNoteMask());
    const double centsToTarget = (target - midi) * 100.0;

    auto top = area.removeFromTop (area.getHeight() / 2);

    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (30.0f, juce::Font::bold));
    g.drawText (madtune::noteName (nearest), top.removeFromLeft (90), juce::Justification::centredLeft);

    g.setFont (juce::FontOptions (18.0f));
    g.setColour (std::abs (cents) < 10.0 ? juce::Colours::lightgreen : juce::Colours::orange);
    g.drawText (juce::String (cents >= 0 ? "+" : "") + juce::String (cents, 0) + " cents   "
                    + juce::String (shownHz, 1) + " Hz",
                top, juce::Justification::centredLeft);

    g.setColour (juce::Colours::lightgrey);
    g.setFont (juce::FontOptions (16.0f));
    g.drawText ("Target in scale: " + juce::String (madtune::noteName (target))
                    + "  (" + juce::String (centsToTarget >= 0 ? "+" : "") + juce::String (centsToTarget, 0)
                    + " cents)   confidence " + juce::String (juce::roundToInt (shownConfidence * 100.0f)) + "%",
                area, juce::Justification::centredLeft);
}

void MadTuneEditor::resized()
{
    sliders.setBounds (getLocalBounds().withTrimmedTop (readoutHeight));
}
