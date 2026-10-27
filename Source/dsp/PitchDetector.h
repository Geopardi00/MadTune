#pragma once

#include <algorithm>
#include <cmath>
#include <vector>

namespace madtune
{

// Monophonic pitch detector using the YIN algorithm
// (de Cheveigne & Kawahara, 2002). Plain C++, no allocation after prepare().
class PitchDetector
{
public:
    struct Result
    {
        float frequency = 0.0f;  // Hz, 0 when no clear pitch was found
        float confidence = 0.0f; // 0..1, 1 = perfectly periodic
    };

    // minHz sets the analysis window length (longest period), so the
    // detector reacts in roughly 2 / minHz seconds.
    void prepare (double newSampleRate, float minHz = 70.0f, float maxHz = 1200.0f, int newHopSize = 256)
    {
        // Cost grows with the square of the sample rate, so high rates are
        // averaged down to about 48 kHz first. Voice pitch sits far below that.
        decimation = std::max (1, (int) std::lround (newSampleRate / 48000.0));
        sampleRate = newSampleRate / decimation;
        tauMin = std::max (2, (int) std::floor (sampleRate / maxHz));
        tauMax = std::max (tauMin + 2, (int) std::ceil (sampleRate / minHz));
        windowSize = tauMax;
        frameSize = windowSize + tauMax + 1;
        hopSize = std::max (1, newHopSize);

        ring.assign ((size_t) frameSize, 0.0f);
        frame.assign ((size_t) frameSize, 0.0f);
        diff.assign ((size_t) tauMax + 2, 0.0f);
        reset();
    }

    void reset()
    {
        std::fill (ring.begin(), ring.end(), 0.0f);
        writePos = 0;
        samplesSinceAnalysis = 0;
        decimationCount = 0;
        decimationSum = 0.0f;
        latest = {};
    }

    // Feed mono samples. Returns true if at least one new estimate was made.
    bool push (const float* samples, int numSamples)
    {
        bool updated = false;
        for (int i = 0; i < numSamples; ++i)
        {
            decimationSum += samples[i];
            if (++decimationCount < decimation)
                continue;

            ring[(size_t) writePos] = decimationSum / (float) decimation;
            decimationSum = 0.0f;
            decimationCount = 0;
            writePos = (writePos + 1) % frameSize;

            if (++samplesSinceAnalysis >= hopSize)
            {
                samplesSinceAnalysis = 0;
                for (int k = 0; k < frameSize; ++k)
                    frame[(size_t) k] = ring[(size_t) ((writePos + k) % frameSize)];
                latest = analyse (frame.data(), frameSize);
                updated = true;
            }
        }
        return updated;
    }

    Result getLatest() const { return latest; }

    // Analyse one frame directly. Needs length >= getFrameSize().
    Result analyse (const float* x, int length)
    {
        if (length < frameSize)
            return {};

        // Ignore near silence: YIN happily finds "pitch" in noise floor.
        double energy = 0.0;
        for (int j = 0; j < windowSize; ++j)
            energy += (double) x[j] * x[j];
        if (energy / windowSize < silenceThreshold * silenceThreshold)
            return {};

        // Difference function and its cumulative mean normalisation.
        diff[0] = 1.0f;
        double runningSum = 0.0;
        for (int tau = 1; tau <= tauMax; ++tau)
        {
            double d = 0.0;
            for (int j = 0; j < windowSize; ++j)
            {
                const double delta = (double) x[j] - x[j + tau];
                d += delta * delta;
            }
            runningSum += d;
            diff[(size_t) tau] = runningSum > 0.0 ? (float) (d * tau / runningSum) : 1.0f;
        }

        // First dip under the threshold, followed down to its local minimum.
        int best = -1;
        for (int tau = tauMin; tau <= tauMax; ++tau)
        {
            if (diff[(size_t) tau] < threshold)
            {
                while (tau + 1 <= tauMax && diff[(size_t) tau + 1] < diff[(size_t) tau])
                    ++tau;
                best = tau;
                break;
            }
        }

        if (best < 0)
            return {};

        // Parabolic interpolation for sub-sample accuracy.
        double refined = best;
        if (best > tauMin && best < tauMax)
        {
            const double a = diff[(size_t) best - 1], b = diff[(size_t) best], c = diff[(size_t) best + 1];
            const double denom = a - 2.0 * b + c;
            if (std::abs (denom) > 1.0e-12)
                refined += 0.5 * (a - c) / denom;
        }

        Result r;
        r.frequency = (float) (sampleRate / refined);
        r.confidence = std::clamp (1.0f - diff[(size_t) best], 0.0f, 1.0f);
        return r;
    }

    // In samples at the analysis rate (after any decimation).
    int getFrameSize() const { return frameSize; }

    // Lower = stricter about what counts as a pitch.
    float threshold = 0.15f;
    // RMS below this is treated as silence.
    float silenceThreshold = 0.003f;

private:
    double sampleRate = 44100.0;
    int tauMin = 2, tauMax = 2, windowSize = 2, frameSize = 4, hopSize = 256;
    std::vector<float> ring, frame, diff;
    int writePos = 0, samplesSinceAnalysis = 0;
    int decimation = 1, decimationCount = 0;
    float decimationSum = 0.0f;
    Result latest;
};

} // namespace madtune
