#include "dsp/PitchDetector.h"
#include "dsp/Scale.h"

#include <cmath>
#include <cstdio>
#include <random>
#include <vector>

using namespace madtune;

static int failures = 0;

static void check (bool ok, const char* what)
{
    if (! ok)
    {
        std::printf ("FAIL: %s\n", what);
        ++failures;
    }
}

static const double pi = 3.14159265358979323846;

// A voice-like test tone: fundamental plus decaying harmonics.
static std::vector<float> tone (double hz, double sr, int n, int harmonics = 1)
{
    std::vector<float> out ((size_t) n);
    for (int i = 0; i < n; ++i)
    {
        double v = 0.0;
        for (int h = 1; h <= harmonics; ++h)
            v += std::sin (2.0 * pi * hz * h * i / sr) / h;
        out[(size_t) i] = (float) (0.3 * v);
    }
    return out;
}

static float detect (const std::vector<float>& x, double sr, float* confidence = nullptr)
{
    PitchDetector d;
    d.prepare (sr);
    d.push (x.data(), (int) x.size());
    if (confidence != nullptr)
        *confidence = d.getLatest().confidence;
    return d.getLatest().frequency;
}

static bool within (float measured, double expected, double cents)
{
    if (measured <= 0.0f)
        return false;
    return std::abs (1200.0 * std::log2 (measured / expected)) <= cents;
}

int main()
{
    for (double sr : { 44100.0, 48000.0, 96000.0 })
    {
        for (double hz : { 82.41, 110.0, 196.0, 261.63, 440.0, 659.25, 987.77 })
        {
            const auto x = tone (hz, sr, (int) (sr * 0.2));
            char what[96];
            std::snprintf (what, sizeof what, "sine %.2f Hz at %.0f Hz within 5 cents", hz, sr);
            check (within (detect (x, sr), hz, 5.0), what);

            // Strong harmonics must not trick it into an octave error.
            const auto rich = tone (hz, sr, (int) (sr * 0.2), 8);
            std::snprintf (what, sizeof what, "harmonic tone %.2f Hz at %.0f Hz within 5 cents", hz, sr);
            check (within (detect (rich, sr), hz, 5.0), what);
        }
    }

    // Slightly flat A4, the case pitch correction exists for.
    {
        const double hz = 440.0 * std::pow (2.0, -30.0 / 1200.0);
        const float f = detect (tone (hz, 48000.0, 9600, 6), 48000.0);
        check (within (f, hz, 2.0), "A4 30 cents flat measured within 2 cents");
        check (std::lround (frequencyToMidi (f)) == 69, "flat A4 still rounds to MIDI 69");
    }

    // Silence and noise report no pitch.
    {
        std::vector<float> silence (9600, 0.0f);
        check (detect (silence, 48000.0) == 0.0f, "silence has no pitch");

        std::mt19937 rng (1);
        std::uniform_real_distribution<float> u (-0.3f, 0.3f);
        std::vector<float> noise (9600);
        for (auto& s : noise)
            s = u (rng);
        check (detect (noise, 48000.0) == 0.0f, "white noise has no pitch");
    }

    // Confidence is high for a clean tone.
    {
        float conf = 0.0f;
        detect (tone (220.0, 44100.0, 8820, 4), 44100.0, &conf);
        check (conf > 0.9f, "clean tone has confidence above 0.9");
    }

    check (noteName (60) == "C4" && noteName (69) == "A4" && noteName (61) == "C#4" && noteName (0) == "C-1",
           "note names");

    if (failures == 0)
        std::printf ("All pitch detector tests passed\n");
    return failures == 0 ? 0 : 1;
}
