#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <array>
#include <atomic>
#include <vector>
#include "SmartInflator.h"

class SmartInflatorAudioProcessor : public juce::AudioProcessor
{
public:
    static constexpr int scopeSize = 2048;

    SmartInflatorAudioProcessor();
    ~SmartInflatorAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Smart Inflator"; }
    bool acceptsMidi() const override { return false; }
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

    // UI side
    bool fetchScopeBlocks (std::array<float, scopeSize>& destIn, std::array<float, scopeSize>& destOut);
    float takeInPeak()  { return inPeak.exchange (0.f); }
    float takeOutPeak() { return outPeak.exchange (0.f); }

    juce::AudioProcessorValueTreeState apvts;

private:
    void applyParameters();
    static void accumulatePeak (std::atomic<float>& a, float v)
    {
        float cur = a.load (std::memory_order_relaxed);
        while (v > cur && ! a.compare_exchange_weak (cur, v, std::memory_order_relaxed)) {}
    }

    SmartInflator inflator;

    std::vector<float> inScratch;
    std::array<float, scopeSize> fifoIn {}, fifoOut {}, snapIn {}, snapOut {};
    int fifoIndex = 0;
    std::atomic<bool> snapshotReady { false };

    std::atomic<float> inPeak { 0.f }, outPeak { 0.f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SmartInflatorAudioProcessor)
};
