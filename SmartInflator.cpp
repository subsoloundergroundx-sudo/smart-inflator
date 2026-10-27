#include "SmartInflator.h"

SmartInflator::SmartInflator()
{
    updateSmoothing();
    reset();
}

void SmartInflator::setSampleRate (double sampleRate)
{
    sampleRate_ = sampleRate > 0.0 ? (float) sampleRate : 48000.0f;
    updateSmoothing();
}

void SmartInflator::updateSmoothing()
{
    // ~20 ms parameter smoothing
    smoothCoeff_ = 1.0f - std::exp (-1.0f / (0.020f * sampleRate_));
}

void SmartInflator::reset()
{
    drive_ = driveT_;
    power_ = powerT_;
    mix_ = mixT_;
}

void SmartInflator::setDrive (float percent)   { driveT_ = std::clamp (percent, 0.0f, 100.0f) / 100.0f; }
void SmartInflator::setCurve (float curve)     { powerT_ = curvePower (std::clamp (curve, -50.0f, 50.0f)); }
void SmartInflator::setAutoGain (bool enabled) { autoGain_ = enabled; }
void SmartInflator::setMix (float percent)     { mixT_ = std::clamp (percent, 0.0f, 100.0f) / 100.0f; }
void SmartInflator::setBypass (bool bypass)    { bypass_ = bypass; }

void SmartInflator::process (float* left, float* right, int numFrames)
{
    if (bypass_ || numFrames <= 0)
        return;

    const bool ag = autoGain_;
    const float k = smoothCoeff_;

    auto smooth = [k] (float& s, float t)
    {
        s += (t - s) * k;
        if (std::abs (t - s) < 1.0e-6f)
            s = t;
    };

    for (int i = 0; i < numFrames; ++i)
    {
        smooth (drive_, driveT_);
        smooth (power_, powerT_);
        smooth (mix_, mixT_);

        const float comp = ag ? autoGainFor (drive_) : 1.0f;

        const float inL = left[i];
        const float inR = right[i];

        left[i]  = shape (inL, drive_, power_, comp) * mix_ + inL * (1.0f - mix_);
        right[i] = shape (inR, drive_, power_, comp) * mix_ + inR * (1.0f - mix_);
    }
}
