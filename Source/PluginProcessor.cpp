#include "PluginProcessor.h"
#include "dsp/Scale.h"

namespace ParamIDs
{
    static const juce::String key         = "key";
    static const juce::String scale       = "scale";
    static const juce::String retuneSpeed = "retuneSpeed";
    static const juce::String humanize    = "humanize";
    static const juce::String mix         = "mix";
}

MadTuneProcessor::MadTuneProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      params (*this, nullptr, "MadTune", createParameterLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout MadTuneProcessor::createParameterLayout()
{
    using namespace juce;

    StringArray scales;
    for (int i = 0; i < (int) madtune::ScaleType::count; ++i)
        scales.add (madtune::scaleName ((madtune::ScaleType) i));

    AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add (std::make_unique<AudioParameterChoice> (
        ParameterID { ParamIDs::key, 1 }, "Key",
        StringArray { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" }, 0));

    layout.add (std::make_unique<AudioParameterChoice> (
        ParameterID { ParamIDs::scale, 1 }, "Scale", scales, (int) madtune::ScaleType::Chromatic));

    // 0 ms = instant, robotic snap. Higher = slower, more natural glide.
    layout.add (std::make_unique<AudioParameterFloat> (
        ParameterID { ParamIDs::retuneSpeed, 1 }, "Retune speed",
        NormalisableRange<float> (0.0f, 400.0f, 1.0f, 0.5f), 50.0f,
        AudioParameterFloatAttributes().withLabel ("ms")));

    layout.add (std::make_unique<AudioParameterFloat> (
        ParameterID { ParamIDs::humanize, 1 }, "Humanize",
        NormalisableRange<float> (0.0f, 100.0f, 1.0f), 0.0f,
        AudioParameterFloatAttributes().withLabel ("%")));

    layout.add (std::make_unique<AudioParameterFloat> (
        ParameterID { ParamIDs::mix, 1 }, "Mix",
        NormalisableRange<float> (0.0f, 100.0f, 1.0f), 100.0f,
        AudioParameterFloatAttributes().withLabel ("%")));

    return layout;
}

bool MadTuneProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto out = layouts.getMainOutputChannelSet();
    if (out != juce::AudioChannelSet::mono() && out != juce::AudioChannelSet::stereo())
        return false;
    return out == layouts.getMainInputChannelSet();
}

void MadTuneProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) samplesPerBlock,
                                  (juce::uint32) getTotalNumOutputChannels() };
    mixer.prepare (spec);
    mixer.reset();
    setLatencySamples (0);
}

void MadTuneProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    for (auto ch = getTotalNumInputChannels(); ch < getTotalNumOutputChannels(); ++ch)
        buffer.clear (ch, 0, buffer.getNumSamples());

    juce::dsp::AudioBlock<float> block (buffer);
    mixer.setWetMixProportion (params.getRawParameterValue (ParamIDs::mix)->load() / 100.0f);
    mixer.pushDrySamples (block);

    // Milestone 1: the wet signal is the input untouched. Pitch detection and
    // shifting replace this in the next milestones.

    mixer.mixWetSamples (block);
}

juce::AudioProcessorEditor* MadTuneProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor (*this);
}

void MadTuneProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = params.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void MadTuneProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (params.state.getType()))
            params.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MadTuneProcessor();
}
