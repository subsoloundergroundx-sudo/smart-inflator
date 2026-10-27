#pragma once
#include "PluginProcessor.h"
#include <juce_dsp/juce_dsp.h>
#include <array>
#include <memory>
#include <vector>

namespace ios
{
    const juce::Colour orangeCol { 0xffff9500 };
    const juce::Colour steelCol  { 0xffa9b4c6 };
    const juce::Colour blueCol   { 0xff0a84ff };
    const juce::Colour purpleCol { 0xffbf5af2 };

    inline juce::Font font (float size, bool bold = false)
    {
        auto o = juce::FontOptions (size);
        if (bold)
            o = o.withStyle ("Bold");
        return juce::Font (o);
    }
}

//==============================================================================
// Old-school (iOS 6 era) skeuomorphic look: glossy metal knobs, glass, LED segments.
class IosLookAndFeel : public juce::LookAndFeel_V4
{
public:
    IosLookAndFeel();

    void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height,
                           float sliderPosProportional, float rotaryStartAngle,
                           float rotaryEndAngle, juce::Slider&) override;

    void drawToggleButton (juce::Graphics&, juce::ToggleButton&,
                           bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
};

//==============================================================================
// Real-time spectrum: output (filled, orange) over input (steel line) + faint waveforms.
// The difference between the two curves is the harmonic content the inflator adds.
class SpectrumView : public juce::Component
{
public:
    explicit SpectrumView (SmartInflatorAudioProcessor&);
    void update();
    void paint (juce::Graphics&) override;

private:
    static constexpr int N = SmartInflatorAudioProcessor::scopeSize;
    static constexpr int fftOrder = 11;
    static constexpr int numCols = 200;
    static_assert ((1 << fftOrder) == N, "scope size must match FFT size");

    void analyse (const std::array<float, N>& src, std::vector<float>& dest, float sampleRate);

    SmartInflatorAudioProcessor& proc;

    juce::dsp::FFT fft { fftOrder };
    juce::dsp::WindowingFunction<float> window { (size_t) N, juce::dsp::WindowingFunction<float>::hann };

    std::array<float, N * 2> fftData {};
    std::array<float, N> waveIn {}, waveOut {};
    std::vector<float> smIn, smOut;
    int idleTicks = 0;
};

//==============================================================================
// Live transfer curve (exactly the function the DSP uses) with a dot at the current level
class CurveView : public juce::Component
{
public:
    explicit CurveView (SmartInflatorAudioProcessor& p) : proc (p) {}
    void setInputLevel (float v) { level = v; }
    void paint (juce::Graphics&) override;

private:
    SmartInflatorAudioProcessor& proc;
    float level = 0.f;
};

//==============================================================================
class LevelMeterView : public juce::Component
{
public:
    void setLevels (float in, float out) { inLevel = in; outLevel = out; }
    void paint (juce::Graphics&) override;

private:
    float inLevel = 0.f, outLevel = 0.f;
};

//==============================================================================
class SmartInflatorAudioProcessorEditor : public juce::AudioProcessorEditor,
                                          private juce::Timer
{
public:
    explicit SmartInflatorAudioProcessorEditor (SmartInflatorAudioProcessor&);
    ~SmartInflatorAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    struct Knob
    {
        juce::Slider slider;
        juce::Label label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    };

    struct Toggle
    {
        juce::ToggleButton button;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> attachment;
    };

    void setupKnob (Knob&, const juce::String& id, const juce::String& name,
                    const juce::String& suffix, int decimals, juce::Colour colour);
    void setupToggle (Toggle&, const juce::String& id, const juce::String& text);
    void renderBackground();
    void timerCallback() override;

    IosLookAndFeel laf;
    SmartInflatorAudioProcessor& proc;

    SpectrumView spectrum;
    CurveView curve;
    LevelMeterView meters;

    std::array<Knob, 3> knobs;
    Toggle bypassToggle, autoGainToggle;

    float inDisp = 0.f, outDisp = 0.f;
    juce::Image background;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SmartInflatorAudioProcessorEditor)
};
