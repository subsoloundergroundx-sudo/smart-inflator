#pragma once

#include <algorithm>
#include <cmath>

// Port of the Android SmartInflator to plain C++.
// Pure per-sample waveshaper: NO oversampling, NO look-ahead (zero latency).
//
//   shaped = x * (1 + drive * (1 - |x|^power)) * comp
//   out    = shaped * mix + dry * (1 - mix)
class SmartInflator
{
public:
    SmartInflator();

    void setSampleRate (double sampleRate);
    void reset();                              // snap smoothers to their targets
    void process (float* left, float* right, int numFrames);

    void setDrive (float percent);             // 0 .. 100
    void setCurve (float curve);               // -50 .. +50  (power = 10^(curve/50))
    void setAutoGain (bool enabled);
    void setMix (float percent);               // 0 .. 100
    void setBypass (bool bypass);

    // Exposed so the UI can draw the exact transfer curve the DSP uses
    static inline float shape (float x, float drive, float power, float comp)
    {
        const float absX = std::min (std::abs (x), 1.0f);
        const float pw = (power == 1.0f) ? absX : std::pow (absX, power);
        return x * (1.0f + drive * (1.0f - pw)) * comp;
    }

    static inline float curvePower (float curve) { return std::pow (10.0f, curve / 50.0f); }
    static inline float autoGainFor (float drive) { return 1.0f - drive * 0.25f; }

private:
    void updateSmoothing();

    float sampleRate_ = 48000.0f;
    float smoothCoeff_ = 0.0f;

    // targets
    float driveT_ = 0.0f;
    float powerT_ = 1.0f;
    float mixT_ = 1.0f;
    bool autoGain_ = true;
    bool bypass_ = false;

    // smoothed (per-sample) values - avoid zipper noise when turning knobs
    float drive_ = 0.0f;
    float power_ = 1.0f;
    float mix_ = 1.0f;
};
