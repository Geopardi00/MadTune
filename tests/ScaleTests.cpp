#include "dsp/Scale.h"

#include <cmath>
#include <cstdio>

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

static bool near (double a, double b) { return std::abs (a - b) < 1.0e-6; }

int main()
{
    check (near (frequencyToMidi (440.0), 69.0), "A4 is MIDI 69");
    check (near (midiToFrequency (60.0), 261.6255653), "C4 is 261.63 Hz");
    check (near (frequencyToMidi (midiToFrequency (57.3)), 57.3), "Hz/MIDI round trip");

    const auto cMajor = makeNoteMask (0, ScaleType::Major);
    check (cMajor[0] && ! cMajor[1] && cMajor[4] && cMajor[11], "C major note mask");

    const auto aMinor = makeNoteMask (9, ScaleType::NaturalMinor);
    for (int pc = 0; pc < 12; ++pc)
        check (aMinor[(size_t) pc] == cMajor[(size_t) pc], "A minor has the notes of C major");

    // C#4 (61) is not in C major: 61.2 is nearer D (62) than C (60).
    check (near (snapToScale (61.2, cMajor), 62.0), "61.2 snaps up to D in C major");
    check (near (snapToScale (60.9, cMajor), 60.0), "60.9 snaps down to C in C major");
    check (near (snapToScale (61.0, cMajor), 60.0), "exact tie snaps down");
    check (near (snapToScale (63.4, makeNoteMask (0, ScaleType::Chromatic)), 63.0), "chromatic rounds");
    check (near (snapToScale (66.0, makeNoteMask (2, ScaleType::Major)), 66.0), "F# stays in D major");

    // Minor pentatonic in A: A C D E G. 70 (A#) -> 69 (A).
    check (near (snapToScale (70.0, makeNoteMask (9, ScaleType::MinorPentatonic)), 69.0), "A# snaps to A in A minor pentatonic");

    NoteMask none {};
    check (near (snapToScale (61.3, none), 61.3), "empty mask leaves pitch alone");

    if (failures == 0)
        std::printf ("All scale tests passed\n");
    return failures == 0 ? 0 : 1;
}
