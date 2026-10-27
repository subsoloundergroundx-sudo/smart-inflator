#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
    juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
    {
        using F = juce::AudioParameterFloat;
        using B = juce::AudioParameterBool;
        using R = juce::NormalisableRange<float>;

        juce::AudioProcessorValueTreeState::ParameterLayout layout;
        layout.add (std::make_unique<F> (juce::ParameterID { "drive", 1 }, "Drive", R (0.f, 100.f, 0.1f),   50.f));
        layout.add (std::make_unique<F> (juce::ParameterID { "curve", 1 }, "Curve", R (-50.f, 50.f, 0.1f),  0.f));
        layout.add (std::make_unique<F> (juce::ParameterID { "mix", 1 },   "Mix",   R (0.f, 100.f, 0.1f),   100.f));
        layout.add (std::make_unique<B> (juce::ParameterID { "autoGain", 1 }, "Auto Gain", true));
        layout.add (std::make_unique<B> (juce::ParameterID { "bypass", 1 },   "Bypass", false));
        return layout;
    }
}

SmartInflatorAudioProcessor::SmartInflatorAudioProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMS", createLayout())
{
}

bool SmartInflatorAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainInputChannelSet()  == juce::AudioChannelSet::stereo()
        && layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void SmartInflatorAudioProcessor::applyParameters()
{
    auto get = [this] (const char* id) { return apvts.getRawParameterValue (id)->load(); };

    inflator.setDrive (get ("drive"));
    inflator.setCurve (get ("curve"));
    inflator.setMix (get ("mix"));
    inflator.setAutoGain (get ("autoGain") > 0.5f);
    inflator.setBypass (get ("bypass") > 0.5f);
}

void SmartInflatorAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    inflator.setSampleRate (sampleRate);
    applyParameters();
    inflator.reset();

    inScratch.assign ((size_t) std::max (samplesPerBlock * 2, 8192), 0.f);
    fifoIn.fill (0.f);
    fifoOut.fill (0.f);
    fifoIndex = 0;
    snapshotReady.store (false);
    inPeak.store (0.f);
    outPeak.store (0.f);
}

void SmartInflatorAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    if (buffer.getNumChannels() < 2)
        return;

    const int n = buffer.getNumSamples();
    float* L = buffer.getWritePointer (0);
    float* R = buffer.getWritePointer (1);

    const bool canScope = (int) inScratch.size() >= n;

    // input metering + scope capture (before processing)
    float pIn = 0.f;
    for (int i = 0; i < n; ++i)
    {
        pIn = std::max (pIn, std::max (std::abs (L[i]), std::abs (R[i])));
        if (canScope)
            inScratch[(size_t) i] = 0.5f * (L[i] + R[i]);
    }
    accumulatePeak (inPeak, pIn);

    applyParameters();
    inflator.process (L, R, n);

    // output metering + scope capture
    float pOut = 0.f;
    for (int i = 0; i < n; ++i)
    {
        pOut = std::max (pOut, std::max (std::abs (L[i]), std::abs (R[i])));

        if (canScope)
        {
            fifoIn[(size_t) fifoIndex]  = inScratch[(size_t) i];
            fifoOut[(size_t) fifoIndex] = 0.5f * (L[i] + R[i]);
            ++fifoIndex;

            if (fifoIndex >= scopeSize)
            {
                fifoIndex = 0;
                if (! snapshotReady.load (std::memory_order_acquire))
                {
                    snapIn = fifoIn;
                    snapOut = fifoOut;
                    snapshotReady.store (true, std::memory_order_release);
                }
            }
        }
    }
    accumulatePeak (outPeak, pOut);
}

bool SmartInflatorAudioProcessor::fetchScopeBlocks (std::array<float, scopeSize>& destIn, std::array<float, scopeSize>& destOut)
{
    if (! snapshotReady.load (std::memory_order_acquire))
        return false;

    destIn = snapIn;
    destOut = snapOut;
    snapshotReady.store (false, std::memory_order_release);
    return true;
}

juce::AudioProcessorEditor* SmartInflatorAudioProcessor::createEditor()
{
    return new SmartInflatorAudioProcessorEditor (*this);
}

void SmartInflatorAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void SmartInflatorAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, sizeInBytes));
    if (xml != nullptr && xml->hasTagName (apvts.state.getType()))
        apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SmartInflatorAudioProcessor();
}
