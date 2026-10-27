#pragma once

#include <array>
#include <cmath>

namespace madtune
{

// Which of the 12 pitch classes the singer is allowed to land on.
enum class ScaleType
{
    Chromatic = 0,
    Major,
    NaturalMinor,
    HarmonicMinor,
    MajorPentatonic,
    MinorPentatonic,
    Dorian,
    count
};

inline const char* scaleName (ScaleType s)
{
    static const char* names[] = { "Chromatic", "Major", "Minor", "Harmonic minor",
                                   "Major pentatonic", "Minor pentatonic", "Dorian" };
    const int i = (int) s;
    return i >= 0 && i < (int) ScaleType::count ? names[i] : "?";
}

using NoteMask = std::array<bool, 12>;

// Allowed pitch classes for a key (0 = C .. 11 = B) and scale.
inline NoteMask makeNoteMask (int key, ScaleType scale)
{
    static const int chromatic[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, -1 };
    static const int major[]     = { 0, 2, 4, 5, 7, 9, 11, -1 };
    static const int minor[]     = { 0, 2, 3, 5, 7, 8, 10, -1 };
    static const int harmonic[]  = { 0, 2, 3, 5, 7, 8, 11, -1 };
    static const int majPent[]   = { 0, 2, 4, 7, 9, -1 };
    static const int minPent[]   = { 0, 3, 5, 7, 10, -1 };
    static const int dorian[]    = { 0, 2, 3, 5, 7, 9, 10, -1 };

    static const int* const table[] = { chromatic, major, minor, harmonic, majPent, minPent, dorian };
    const int s = (int) scale;
    const int* steps = s >= 0 && s < (int) ScaleType::count ? table[s] : chromatic;

    NoteMask mask {};
    for (int i = 0; steps[i] >= 0; ++i)
        mask[(size_t) (((key + steps[i]) % 12 + 12) % 12)] = true;
    return mask;
}

inline double frequencyToMidi (double hz)  { return 69.0 + 12.0 * std::log2 (hz / 440.0); }
inline double midiToFrequency (double midi) { return 440.0 * std::pow (2.0, (midi - 69.0) / 12.0); }

// Nearest allowed MIDI note to a (fractional) MIDI pitch. Ties go downwards.
// Returns the input unchanged if the mask allows nothing.
inline double snapToScale (double midi, const NoteMask& mask)
{
    const int centre = (int) std::lround (midi);
    double best = midi;
    double bestDist = 1.0e9;

    for (int n = centre - 6; n <= centre + 6; ++n)
    {
        if (! mask[(size_t) ((n % 12 + 12) % 12)])
            continue;

        const double d = std::abs ((double) n - midi);
        if (d < bestDist - 1.0e-9)
        {
            bestDist = d;
            best = (double) n;
        }
    }
    return best;
}

} // namespace madtune
