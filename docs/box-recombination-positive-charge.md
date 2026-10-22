# Positive ionization charge from the Box recombination model

## Symptom

With the segment sampler in `recombination` mode
(`wcph_tracksegmentset_to_deposet`, `wct_tla: { ionization: 'recombination' }`),
a few sampled depos have **positive** charge. WCT's convention is that
ionization electrons are negative, so these depos are unphysical.

Example: a 100 MeV electron in `pdhd_like.gdml`, fired from (-1000, 3071, 1150) mm
along +z, produced 675 sampled depos totalling -2.93e6 e. Six of them were
positive, totalling +538.9 e (max +189.9 e).

## Cause

`tracksegmentset-sampler.jsonnet` configures `Gen::TrackSegmentSampler` with a
`BoxRecombination` (A = 0.930, B = 0.212, rho = 1.396 g/cm^3, Wi = 23.6 eV,
E = 500 V/cm by default). In `recombination` mode the sampler evaluates the
model once per edep-sim segment, using dE/dx = energy / track_length, and then
splits that segment's charge across its depos (1 mm steps).

The modified Box model is

    xi = B * (dE/dx) / (rho * E)
    R  = ln(A + xi) / xi
    dQ = -(R / Wi) * dE

When `A + xi < 1`, the log is negative, so R < 0 and the charge comes out
positive. At 500 V/cm that happens below

    dE/dx < (1 - A) * rho * E / B = 0.07 / 0.3037 cm/MeV ~= 0.23 MeV/cm

This is the Box model being used outside its valid range, not a bug in the
sampler or the converters. As dE/dx -> 0 the positive charge does not vanish.
It tends to `+ln(1/A) * L / (B' * Wi)`, about +10,000 e per cm of segment
length, regardless of the energy deposited.

## Verification

The six positive depos come from exactly the six segments below 0.23 MeV/cm.
They are short (about 0.5 mm), low-energy (7-12 keV) electron steps, with
dE/dx of 0.14-0.23 MeV/cm. Evaluating the formula above on every segment
reproduces the sampled charge exactly: -2.933e6 e in total, +538.9 e from the
positive segments.

## Impact

In this event the effect is negligible: 539 e out of 2.93e6 e (0.02%).
It could matter for many long, low-dE/dx segments, because the spurious
charge scales with length rather than energy. Examples are coarse stepping, or
segments whose energy was mostly sent elsewhere.

## Options

- **Leave it:** fine for events like the one above.
- **Floor dE/dx** before evaluating the Box model, so that R stays positive.
- **Use a model that cannot go negative,** e.g. `BirksRecombination`
  (R = a / (1 + b dE/dx)). `tracksegmentset-sampler.jsonnet` hard-codes
  `BoxRecombination`, so this would need its own `wct_config`.
- **Use `quanta` mode,** which takes edep-sim's `n_electrons` directly. Note
  that edep-sim currently recombines almost all of the charge (53k e for a
  100 MeV deposit). That looks like it is not applying the GDML's 500 V/cm
  `EField`; this has not been confirmed.
