#include "PluginEditor.h"
#include <cmath>

//==============================================================================
// Look and feel
//==============================================================================
IosLookAndFeel::IosLookAndFeel()
{
    setColour (juce::Slider::textBoxTextColourId, juce::Colour (0xffe9edf2));
    setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    setColour (juce::Label::textColourId, juce::Colour (0xffe9edf2));
}

void IosLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                       float pos, float startAngle, float endAngle, juce::Slider& slider)
{
    auto bounds = juce::Rectangle<float> ((float) x, (float) y, (float) width, (float) height).reduced (3.f);
    const float d = std::min (bounds.getWidth(), bounds.getHeight());
    auto area = juce::Rectangle<float> (d, d).withCentre (bounds.getCentre());
    const auto centre = area.getCentre();
    const float radius = d * 0.5f;
    const auto accent = slider.findColour (juce::Slider::rotarySliderFillColourId);
    const float angle = startAngle + pos * (endAngle - startAngle);

    // drop shadow
    g.setColour (juce::Colours::black.withAlpha (0.55f));
    g.fillEllipse (area.translated (0.f, 2.f).expanded (1.f));

    // groove + value arc
    const float arcR = radius - 2.f;
    juce::Path track;
    track.addCentredArc (centre.x, centre.y, arcR, arcR, 0.f, startAngle, endAngle, true);
    g.setColour (juce::Colour (0xff07090d));
    g.strokePath (track, juce::PathStrokeType (4.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    juce::Path val;
    val.addCentredArc (centre.x, centre.y, arcR, arcR, 0.f, startAngle, angle, true);
    g.setColour (accent.withAlpha (0.28f));
    g.strokePath (val, juce::PathStrokeType (8.f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.setColour (accent);
    g.strokePath (val, juce::PathStrokeType (3.f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // metal body
    const float bodyR = radius - 8.f;
    auto body = juce::Rectangle<float> (bodyR * 2.f, bodyR * 2.f).withCentre (centre);

    g.setGradientFill (juce::ColourGradient (juce::Colour (0xfff4f6f9), centre.x, body.getY(),
                                             juce::Colour (0xff3d424a), centre.x, body.getBottom(), false));
    g.fillEllipse (body.expanded (1.5f));

    auto face = body.reduced (2.5f);
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xffe1e5ea), face.getX(), face.getY(),
                                             juce::Colour (0xff5d636d), face.getRight(), face.getBottom(), false));
    g.fillEllipse (face);

    // brushed rings
    g.setColour (juce::Colours::white.withAlpha (0.05f));
    for (float rr = face.getWidth() * 0.5f - 2.f; rr > 4.f; rr -= 3.f)
        g.drawEllipse (centre.x - rr, centre.y - rr, rr * 2.f, rr * 2.f, 0.6f);

    // gloss
    juce::Path gloss;
    gloss.addEllipse (face.getX() + face.getWidth() * 0.12f, face.getY() + face.getHeight() * 0.04f,
                      face.getWidth() * 0.76f, face.getHeight() * 0.46f);
    g.setGradientFill (juce::ColourGradient (juce::Colours::white.withAlpha (0.6f), 0.f, face.getY(),
                                             juce::Colours::white.withAlpha (0.0f), 0.f, face.getY() + face.getHeight() * 0.5f, false));
    g.fillPath (gloss);

    // pointer
    juce::Path pointer;
    pointer.addRoundedRectangle (-1.6f, -bodyR + 5.f, 3.2f, bodyR * 0.45f, 1.5f);
    g.setColour (juce::Colour (0xff12151a));
    g.fillPath (pointer, juce::AffineTransform::rotation (angle).translated (centre.x, centre.y));
}

void IosLookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& b, bool, bool)
{
    auto r = b.getLocalBounds().toFloat();
    const float h = 22.f, w = 42.f;
    auto sw = juce::Rectangle<float> (r.getX() + 2.f, r.getCentreY() - h * 0.5f, w, h);
    const bool on = b.getToggleState();

    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff04060a), 0.f, sw.getY(),
                                             juce::Colour (0xff2c313b), 0.f, sw.getBottom(), false));
    g.fillRoundedRectangle (sw.expanded (1.f), h * 0.5f + 1.f);

    if (on)
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xff74e48f), 0.f, sw.getY(),
                                                 juce::Colour (0xff22a042), 0.f, sw.getBottom(), false));
    else
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xff6d737e), 0.f, sw.getY(),
                                                 juce::Colour (0xff383c45), 0.f, sw.getBottom(), false));
    g.fillRoundedRectangle (sw, h * 0.5f);

    auto glossR = sw.reduced (2.f, 1.5f).withHeight (h * 0.42f);
    g.setGradientFill (juce::ColourGradient (juce::Colours::white.withAlpha (0.32f), 0.f, glossR.getY(),
                                             juce::Colours::white.withAlpha (0.02f), 0.f, glossR.getBottom(), false));
    g.fillRoundedRectangle (glossR, 6.f);

    const float ts = h - 4.f;
    const float tx = on ? sw.getRight() - ts - 2.f : sw.getX() + 2.f;
    auto thumb = juce::Rectangle<float> (tx, sw.getY() + 2.f, ts, ts);
    g.setColour (juce::Colours::black.withAlpha (0.45f));
    g.fillEllipse (thumb.translated (0.f, 1.2f));
    g.setGradientFill (juce::ColourGradient (juce::Colours::white, 0.f, thumb.getY(),
                                             juce::Colour (0xffb4bac4), 0.f, thumb.getBottom(), false));
    g.fillEllipse (thumb);

    g.setColour (juce::Colours::white.withAlpha (on ? 0.95f : 0.6f));
    g.setFont (ios::font (12.f, true));
    g.drawText (b.getButtonText(), r.withTrimmedLeft (w + 10.f), juce::Justification::centredLeft);
}

//==============================================================================
// Spectrum view
//==============================================================================
SpectrumView::SpectrumView (SmartInflatorAudioProcessor& p) : proc (p)
{
    smIn.assign ((size_t) numCols, 0.f);
    smOut.assign ((size_t) numCols, 0.f);
}

void SpectrumView::analyse (const std::array<float, N>& src, std::vector<float>& dest, float sr)
{
    std::copy (src.begin(), src.end(), fftData.begin());
    std::fill (fftData.begin() + N, fftData.end(), 0.f);
    window.multiplyWithWindowingTable (fftData.data(), (size_t) N);
    fft.performFrequencyOnlyForwardTransform (fftData.data());

    for (int i = 0; i < numCols; ++i)
    {
        const float k = (float) (numCols - 1);
        const float f0 = 20.f * std::pow (1000.f, ((float) i - 0.5f) / k);
        const float f1 = 20.f * std::pow (1000.f, ((float) i + 0.5f) / k);
        const float fc = 20.f * std::pow (1000.f, (float) i / k);

        int b0 = std::max (1, (int) std::floor (f0 * (float) N / sr));
        int b1 = std::min (N / 2 - 1, (int) std::ceil (f1 * (float) N / sr));
        b0 = std::min (b0, N / 2 - 1);
        if (b1 < b0) b1 = b0;

        float m = 0.f;
        for (int b = b0; b <= b1; ++b)
            m = std::max (m, fftData[(size_t) b]);

        const float level = m / ((float) N * 0.25f);
        const float db = juce::Decibels::gainToDecibels (level, -120.f) + 3.f * std::log2 (fc / 1000.f);
        const float norm = juce::jlimit (0.f, 1.f, (db + 84.f) / 84.f);

        float& s = dest[(size_t) i];
        s += (norm - s) * (norm > s ? 0.65f : 0.18f);
    }
}

void SpectrumView::update()
{
    if (proc.fetchScopeBlocks (waveIn, waveOut))
    {
        idleTicks = 0;
        const float sr = (float) proc.getSampleRate();
        if (sr > 0.f)
        {
            analyse (waveIn, smIn, sr);
            analyse (waveOut, smOut, sr);
        }
    }
    else if (++idleTicks > 8)
    {
        for (auto& s : smIn)  s *= 0.92f;
        for (auto& s : smOut) s *= 0.92f;
    }

    repaint();
}

void SpectrumView::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff0a0c10), 0.f, bounds.getY(),
                                             juce::Colour (0xff3a404b), 0.f, bounds.getBottom(), false));
    g.fillRoundedRectangle (bounds, 11.f);

    auto screen = bounds.reduced (3.f);
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff101a27), 0.f, screen.getY(),
                                             juce::Colour (0xff070b11), 0.f, screen.getBottom(), false));
    g.fillRoundedRectangle (screen, 8.f);

    g.saveState();
    juce::Path clip;
    clip.addRoundedRectangle (screen, 8.f);
    g.reduceClipRegion (clip);

    auto xOf = [&screen] (float f)
    {
        const float t = std::log (juce::jlimit (20.f, 20000.f, f) / 20.f) / std::log (1000.f);
        return screen.getX() + t * screen.getWidth();
    };

    // grid
    g.setColour (juce::Colours::white.withAlpha (0.05f));
    const float gridF[] = { 30.f, 50.f, 100.f, 200.f, 300.f, 500.f, 1000.f, 2000.f, 3000.f, 5000.f, 10000.f };
    for (float f : gridF)
        g.drawVerticalLine ((int) xOf (f), screen.getY(), screen.getBottom());
    for (int i = 1; i < 4; ++i)
        g.drawHorizontalLine ((int) (screen.getY() + screen.getHeight() * (float) i * 0.25f), screen.getX(), screen.getRight());

    g.setColour (juce::Colours::white.withAlpha (0.28f));
    g.setFont (ios::font (9.f));
    const std::pair<float, const char*> gridLabels[] = { { 100.f, "100" }, { 1000.f, "1k" }, { 10000.f, "10k" } };
    for (auto& gl : gridLabels)
        g.drawText (gl.second, juce::Rectangle<float> (xOf (gl.first) + 2.f, screen.getBottom() - 13.f, 28.f, 11.f),
                    juce::Justification::centredLeft);

    // faint waveforms (input steel, output orange)
    auto drawWave = [&] (const std::array<float, N>& w, juce::Colour c, float alpha)
    {
        juce::Path p;
        const int W = std::max (2, (int) screen.getWidth());
        const float midY = screen.getCentreY();
        for (int px = 0; px < W; px += 2)
        {
            const int idx = juce::jlimit (0, N - 1, px * N / W);
            const float yy = midY - juce::jlimit (-1.f, 1.f, w[(size_t) idx]) * screen.getHeight() * 0.45f;
            if (px == 0) p.startNewSubPath (screen.getX(), yy);
            else         p.lineTo (screen.getX() + (float) px, yy);
        }
        g.setColour (c.withAlpha (alpha));
        g.strokePath (p, juce::PathStrokeType (1.f));
    };

    // spectrum paths
    auto buildLine = [&] (const std::vector<float>& v)
    {
        juce::Path p;
        for (int i = 0; i < numCols; ++i)
        {
            const float x = screen.getX() + screen.getWidth() * (float) i / (float) (numCols - 1);
            const float y = screen.getBottom() - 2.f - v[(size_t) i] * (screen.getHeight() - 8.f);
            if (i == 0) p.startNewSubPath (x, y);
            else        p.lineTo (x, y);
        }
        return p;
    };

    juce::Path inLine = buildLine (smIn);
    juce::Path outLine = buildLine (smOut);

    juce::Path outFill (outLine);
    outFill.lineTo (screen.getRight(), screen.getBottom());
    outFill.lineTo (screen.getX(), screen.getBottom());
    outFill.closeSubPath();

    // output: filled orange glow
    g.setGradientFill (juce::ColourGradient (ios::orangeCol.withAlpha (0.85f), 0.f, screen.getY(),
                                             ios::orangeCol.withAlpha (0.04f), 0.f, screen.getBottom(), false));
    g.fillPath (outFill);
    g.setColour (ios::orangeCol.brighter (0.5f));
    g.strokePath (outLine, juce::PathStrokeType (1.7f));

    // input: steel line on top
    g.setColour (ios::steelCol.withAlpha (0.75f));
    g.strokePath (inLine, juce::PathStrokeType (1.2f));

    drawWave (waveIn, ios::steelCol, 0.14f);
    drawWave (waveOut, ios::orangeCol, 0.28f);

    // legend
    g.setFont (ios::font (10.f, true));
    g.setColour (ios::orangeCol.brighter (0.3f));
    g.drawText ("OUT", juce::Rectangle<float> (screen.getRight() - 78.f, screen.getY() + 4.f, 34.f, 12.f), juce::Justification::centredRight);
    g.setColour (ios::steelCol);
    g.drawText ("IN", juce::Rectangle<float> (screen.getRight() - 40.f, screen.getY() + 4.f, 34.f, 12.f), juce::Justification::centredRight);

    // glass reflection
    auto top = screen.withHeight (screen.getHeight() * 0.45f);
    g.setGradientFill (juce::ColourGradient (juce::Colours::white.withAlpha (0.08f), 0.f, top.getY(),
                                             juce::Colours::white.withAlpha (0.0f), 0.f, top.getBottom(), false));
    g.fillRect (top);

    g.restoreState();

    g.setColour (juce::Colours::white.withAlpha (0.09f));
    g.drawRoundedRectangle (screen, 8.f, 1.f);
}

//==============================================================================
// Transfer curve
//==============================================================================
void CurveView::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff0a0c10), 0.f, bounds.getY(),
                                             juce::Colour (0xff3a404b), 0.f, bounds.getBottom(), false));
    g.fillRoundedRectangle (bounds, 11.f);

    auto screen = bounds.reduced (3.f);
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff101a27), 0.f, screen.getY(),
                                             juce::Colour (0xff070b11), 0.f, screen.getBottom(), false));
    g.fillRoundedRectangle (screen, 8.f);

    g.saveState();
    juce::Path clip;
    clip.addRoundedRectangle (screen, 8.f);
    g.reduceClipRegion (clip);

    auto plot = screen.reduced (14.f);
    const float yRange = 1.5f;

    auto X = [&plot] (float x) { return plot.getCentreX() + x * plot.getWidth() * 0.5f; };
    auto Y = [&plot, yRange] (float y) { return plot.getCentreY() - (y / yRange) * plot.getHeight() * 0.5f; };

    // unity box + axes
    g.setColour (juce::Colours::white.withAlpha (0.06f));
    g.drawRect (juce::Rectangle<float> (X (-1.f), Y (1.f), X (1.f) - X (-1.f), Y (-1.f) - Y (1.f)), 1.f);
    g.setColour (juce::Colours::white.withAlpha (0.16f));
    g.drawLine (X (-1.f), Y (0.f), X (1.f), Y (0.f), 1.f);
    g.drawLine (X (0.f), Y (-yRange), X (0.f), Y (yRange), 1.f);

    // reference diagonal (no processing)
    const float dashes[] = { 3.f, 3.f };
    g.setColour (juce::Colours::white.withAlpha (0.4f));
    g.drawDashedLine (juce::Line<float> (X (-1.f), Y (-1.f), X (1.f), Y (1.f)), dashes, 2, 1.f);

    // current parameters
    const float drive = proc.apvts.getRawParameterValue ("drive")->load() / 100.f;
    const float curveVal = proc.apvts.getRawParameterValue ("curve")->load();
    const bool autoGain = proc.apvts.getRawParameterValue ("autoGain")->load() > 0.5f;
    const float p = SmartInflator::curvePower (curveVal);
    const float comp = autoGain ? SmartInflator::autoGainFor (drive) : 1.f;

    juce::Path curvePath;
    const int steps = 160;
    for (int i = 0; i <= steps; ++i)
    {
        const float x = -1.f + 2.f * (float) i / (float) steps;
        const float y = SmartInflator::shape (x, drive, p, comp);
        if (i == 0) curvePath.startNewSubPath (X (x), Y (y));
        else        curvePath.lineTo (X (x), Y (y));
    }

    g.setColour (ios::orangeCol.withAlpha (0.25f));
    g.strokePath (curvePath, juce::PathStrokeType (7.f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.setColour (ios::orangeCol.brighter (0.4f));
    g.strokePath (curvePath, juce::PathStrokeType (2.2f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // live level dot (mirrored on both polarities)
    const float lv = juce::jlimit (0.f, 1.f, level);
    for (float sgn : { 1.f, -1.f })
    {
        const float x = sgn * lv;
        const float y = SmartInflator::shape (x, drive, p, comp);
        auto dot = juce::Rectangle<float> (8.f, 8.f).withCentre (juce::Point<float> (X (x), Y (y)));
        g.setColour (juce::Colours::white.withAlpha (0.25f));
        g.fillEllipse (dot.expanded (4.f));
        g.setColour (juce::Colours::white);
        g.fillEllipse (dot);
    }

    g.setFont (ios::font (10.f, true));
    g.setColour (juce::Colours::white.withAlpha (0.55f));
    g.drawText ("TRANSFER", juce::Rectangle<float> (screen.getX() + 8.f, screen.getY() + 5.f, 80.f, 12.f), juce::Justification::centredLeft);

    auto top = screen.withHeight (screen.getHeight() * 0.45f);
    g.setGradientFill (juce::ColourGradient (juce::Colours::white.withAlpha (0.08f), 0.f, top.getY(),
                                             juce::Colours::white.withAlpha (0.0f), 0.f, top.getBottom(), false));
    g.fillRect (top);

    g.restoreState();

    g.setColour (juce::Colours::white.withAlpha (0.09f));
    g.drawRoundedRectangle (screen, 8.f, 1.f);
}

//==============================================================================
// Level meters
//==============================================================================
void LevelMeterView::paint (juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    auto labelArea = b.removeFromBottom (14.f);

    const float levels[2] = { inLevel, outLevel };
    const char* names[2] = { "IN", "OUT" };
    const float colW = b.getWidth() / 2.f;
    constexpr int segs = 14;

    for (int c = 0; c < 2; ++c)
    {
        juce::Rectangle<float> col (b.getX() + colW * (float) c + colW * 0.25f, b.getY(), colW * 0.5f, b.getHeight());
        const float db = juce::Decibels::gainToDecibels (levels[c], -60.f);
        const float segH = col.getHeight() / (float) segs;

        for (int s = 0; s < segs; ++s)
        {
            auto seg = juce::Rectangle<float> (col.getX(), col.getBottom() - (float) (s + 1) * segH, col.getWidth(), segH).reduced (0.f, 1.f);
            const float thr = -48.f + (float) s * (48.f / (float) (segs - 1));
            const bool lit = db >= thr;

            if (lit)
            {
                const juce::Colour base = s < 9 ? juce::Colour (0xff34c759)
                                        : s < 12 ? juce::Colour (0xffffcc00)
                                                 : juce::Colour (0xffff453a);
                g.setGradientFill (juce::ColourGradient (base.brighter (0.3f), 0.f, seg.getY(),
                                                         base.darker (0.2f), 0.f, seg.getBottom(), false));
            }
            else
            {
                g.setColour (juce::Colour (0xff0d1117));
            }
            g.fillRoundedRectangle (seg, 1.5f);

            if (! lit)
            {
                g.setColour (juce::Colours::white.withAlpha (0.04f));
                g.drawRoundedRectangle (seg, 1.5f, 0.6f);
            }
        }

        g.setColour (juce::Colours::white.withAlpha (0.6f));
        g.setFont (ios::font (9.f, true));
        g.drawText (names[c], juce::Rectangle<float> (b.getX() + colW * (float) c, labelArea.getY(), colW, labelArea.getHeight()),
                    juce::Justification::centred);
    }
}

//==============================================================================
// Editor
//==============================================================================
namespace
{
    constexpr int kWidth = 560;
    constexpr int kHeight = 380;
}

SmartInflatorAudioProcessorEditor::SmartInflatorAudioProcessorEditor (SmartInflatorAudioProcessor& p)
    : AudioProcessorEditor (&p), proc (p), spectrum (p), curve (p)
{
    setLookAndFeel (&laf);

    setupKnob (knobs[0], "drive", "DRIVE", " %", 1, ios::orangeCol);
    setupKnob (knobs[1], "curve", "CURVE", "",   1, ios::blueCol);
    setupKnob (knobs[2], "mix",   "MIX",   " %", 0, ios::purpleCol);

    setupToggle (bypassToggle, "bypass", "Bypass");
    setupToggle (autoGainToggle, "autoGain", "Auto");

    addAndMakeVisible (spectrum);
    addAndMakeVisible (curve);
    addAndMakeVisible (meters);

    setSize (kWidth, kHeight);
    startTimerHz (30);
}

SmartInflatorAudioProcessorEditor::~SmartInflatorAudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

void SmartInflatorAudioProcessorEditor::setupKnob (Knob& k, const juce::String& id, const juce::String& name,
                                                   const juce::String& suffix, int decimals, juce::Colour colour)
{
    k.slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    k.slider.setRotaryParameters (juce::MathConstants<float>::pi * 1.25f, juce::MathConstants<float>::pi * 2.75f, true);
    k.slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 80, 18);
    k.slider.setTextValueSuffix (suffix);
    k.slider.setNumDecimalPlacesToDisplay (decimals);
    k.slider.setColour (juce::Slider::rotarySliderFillColourId, colour);

    k.label.setText (name, juce::dontSendNotification);
    k.label.setJustificationType (juce::Justification::centred);
    k.label.setFont (ios::font (11.f, true));
    k.label.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.7f));

    addAndMakeVisible (k.slider);
    addAndMakeVisible (k.label);

    k.attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, id, k.slider);
}

void SmartInflatorAudioProcessorEditor::setupToggle (Toggle& t, const juce::String& id, const juce::String& text)
{
    t.button.setButtonText (text);
    addAndMakeVisible (t.button);
    t.attachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, id, t.button);
}

void SmartInflatorAudioProcessorEditor::timerCallback()
{
    const float pi = proc.takeInPeak();
    const float po = proc.takeOutPeak();
    inDisp  = std::max (pi, inDisp * 0.84f);
    outDisp = std::max (po, outDisp * 0.84f);

    meters.setLevels (inDisp, outDisp);
    curve.setInputLevel (inDisp);

    spectrum.update();
    curve.repaint();
    meters.repaint();
}

void SmartInflatorAudioProcessorEditor::renderBackground()
{
    const int w = getWidth();
    const int h = getHeight();
    if (w <= 0 || h <= 0)
        return;

    background = juce::Image (juce::Image::ARGB, w, h, true);
    juce::Graphics g (background);

    // base + linen texture
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff3b4351), 0.f, 0.f,
                                             juce::Colour (0xff151922), 0.f, (float) h, false));
    g.fillAll();
    g.setColour (juce::Colours::white.withAlpha (0.025f));
    for (int y = 0; y < h; y += 2)
        g.drawHorizontalLine (y, 0.f, (float) w);
    g.setColour (juce::Colours::black.withAlpha (0.05f));
    for (int x = 0; x < w; x += 2)
        g.drawVerticalLine (x, 0.f, (float) h);

    // glossy header bar
    juce::Rectangle<float> header (0.f, 0.f, (float) w, 40.f);
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff4a5263), 0.f, 0.f,
                                             juce::Colour (0xff1c212b), 0.f, header.getBottom(), false));
    g.fillRect (header);
    g.setColour (juce::Colours::white.withAlpha (0.18f));
    g.drawHorizontalLine (0, 0.f, (float) w);
    g.setColour (juce::Colours::black.withAlpha (0.7f));
    g.drawHorizontalLine (39, 0.f, (float) w);

    // embossed title
    g.setFont (ios::font (20.f, true));
    g.setColour (juce::Colours::black.withAlpha (0.8f));
    g.drawText ("Smart Inflator", juce::Rectangle<float> (18.f, 3.f, 260.f, 34.f), juce::Justification::centredLeft);
    g.setColour (juce::Colour (0xfff0f3f7));
    g.drawText ("Smart Inflator", juce::Rectangle<float> (18.f, 2.f, 260.f, 34.f), juce::Justification::centredLeft);

    // plate
    auto plate = [&g] (juce::Rectangle<float> r, const juce::String& title)
    {
        g.setColour (juce::Colours::black.withAlpha (0.5f));
        g.fillRoundedRectangle (r.translated (0.f, 2.f), 12.f);

        g.setGradientFill (juce::ColourGradient (juce::Colour (0xff2e3544), 0.f, r.getY(),
                                                 juce::Colour (0xff1a1f29), 0.f, r.getBottom(), false));
        g.fillRoundedRectangle (r, 12.f);

        g.setColour (juce::Colours::white.withAlpha (0.12f));
        g.drawRoundedRectangle (r.reduced (0.5f), 12.f, 1.f);

        auto gloss = r.reduced (2.f, 2.f).withHeight (r.getHeight() * 0.28f);
        g.setGradientFill (juce::ColourGradient (juce::Colours::white.withAlpha (0.07f), 0.f, gloss.getY(),
                                                 juce::Colours::white.withAlpha (0.0f), 0.f, gloss.getBottom(), false));
        g.fillRoundedRectangle (gloss, 10.f);

        g.setFont (ios::font (9.5f, true));
        g.setColour (juce::Colours::black.withAlpha (0.8f));
        g.drawText (title, juce::Rectangle<float> (r.getX() + 12.f, r.getY() + 5.f, r.getWidth() - 20.f, 12.f), juce::Justification::centredLeft);
        g.setColour (juce::Colours::white.withAlpha (0.5f));
        g.drawText (title, juce::Rectangle<float> (r.getX() + 12.f, r.getY() + 4.f, r.getWidth() - 20.f, 12.f), juce::Justification::centredLeft);
    };

    plate ({ 14.f, 226.f, 532.f, 140.f }, "INFLATOR");
}

void SmartInflatorAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.drawImageAt (background, 0, 0);
}

void SmartInflatorAudioProcessorEditor::resized()
{
    renderBackground();

    spectrum.setBounds (14, 46, 352, 170);
    curve.setBounds (376, 46, 170, 170);

    for (size_t i = 0; i < knobs.size(); ++i)
    {
        juce::Rectangle<int> cell (24 + (int) i * 112, 226 + 18, 108, 116);
        knobs[i].label.setBounds (cell.removeFromTop (14));
        knobs[i].slider.setBounds (cell);
    }

    meters.setBounds (366, 248, 84, 108);
    autoGainToggle.button.setBounds (458, 290, 84, 24);
    bypassToggle.button.setBounds (kWidth - 116, 8, 104, 24);
}
