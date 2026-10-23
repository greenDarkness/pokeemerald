#include "global.h"
#include "pokemon_color_variation.h"
#include "trig.h"
#include "constants/species.h"

// Fixed-point scale (10 bits = multiply by 1024)
#define FP_SHIFT 10
#define FP_SCALE (1 << FP_SHIFT)

// Per-species override: tweaks the standard color variation to keep the same
// rich distribution but constrains the hue range and biases it away from a
// danger zone (e.g. Pikachu's shiny is bright orange — biasing the hue range
// toward green-yellow keeps variations distinct from the shiny while still
// preserving the natural mode-based variety).
struct ColorVariationOverride
{
    u16 species;
    s8  maxAngle;     // sine-table units. Replaces COLOR_VARIATION_MAX_ANGLE for this species.
    s8  angleBias;    // sine-table units. Added to the final hue angle (negative = away from orange/red).
    // Split-hue modes (3, 6, 7) rotate cool colors the opposite way, bias
    // included, so a bias that turns a green body bluer turns it yellower in
    // those modes. 1 = flip only the per-step rotation there and keep angleBias
    // pointing the same way for every color. 0 = off (the bias flips too).
    u8  holdBias;
    s16 satCap;       // fixed-point. Caps maximum saturation boost (1024 = 100%, < 1024 disables vivid modes).
    s16 satMul;       // fixed-point. Always-applied saturation multiplier (1024 = none). Pulls everything toward gray/brown.
    // Neutral tint: hue rotation can't affect (near-)gray colors, so species
    // whose body is mostly gray barely change. These fields inject a small
    // amount of chroma into unsaturated colors before the normal variation
    // runs. The tint hue is picked per individual from the same hue step.
    u8  tintHue;      // sine-table units (0 = red, 85 = green, 171 = blue). Center of the tint arc.
    s8  tintSpread;   // sine-table units per hue step (steps range -4..+3).
    u8  tintStrength; // max chroma added, in 5-bit color units (0 = no tint).
    // Lightness spread for the same gray colors, in 1/64ths of the way to
    // black/white. Hue step -4 gets the full tintDarken, step +3 the full
    // tintLighten, and the steps in between are spread evenly.
    u8  tintDarken;
    u8  tintLighten;
    // tintRamp 1 = the tint ramps like the hue band, none at step -4 and the
    // full tintStrength at +3, so individuals range from the original grays
    // to the tint color. 0 = every individual gets the same strength.
    u8  tintRamp;
    // tintWhites 1 = near-white colors are tinted too (normally they're left
    // white so eye shine stays clean), for mostly white Pokemon. They're also
    // darkened by tintDarken like any other gray (normally whites are protected).
    u8  tintWhites;
    // Side fades: one side of the hue steps fades toward gray/white instead of
    // rotating hue. Positive = steps +1..+3 (full at +3); negative = steps
    // -1..-4 (full at -4). Magnitude is 1/64ths of the way at the extreme
    // step. 0 = off. Both can target the same or opposite sides.
    s8  grayFade;     // removes saturation
    s8  whiteFade;    // lightens toward white (dark outlines are protected)
    s8  brownFade;    // shifts toward a sepia/brown version of the color (shading is kept)
    // Hue band: rotates only the colors whose hue lies near bandHue (e.g. just
    // a blue body or just red flowers), leaving the rest of the palette to
    // the normal variation. The rotation is spread across hue steps: step -4
    // gets none, step +3 the full bandShift. Applied before the normal
    // variation, so modes/fades still act on the shifted colors.
    u8  bandHue;      // sine-table units (0 = red, 43 = yellow, 85 = green, 128 = cyan, 171 = blue, 213 = magenta)
    u8  bandWidth;    // sine-table units either side of bandHue that get the full shift (0 = off)
    s8  bandShift;    // sine-table units of hue rotation at step +3 (+ = toward blue/purple, - = toward red/pink)
    // 1/64ths toward white at step +3 for the band colors (e.g. red -> pink).
    // Negative = toward black instead, tapering the same way (e.g. a light
    // purple -> deep violet). 0 = off.
    s8  bandLighten;
    // Darkens only the band colors toward black (e.g. red markings -> black while
    // gray fins stay light). Only the two extreme steps on one side are touched,
    // so most individuals keep their normal color. Sign picks the side: negative
    // = step -3 gets half, step -4 the full amount; positive = step +2 gets half,
    // step +3 the full amount. 1/64ths of the way to black. 0 = off.
    s8  bandDarken;
    // Desaturates only the band colors toward gray, tapering across the whole
    // range like bandLighten: none at step -4, full at step +3. 1/64ths of the
    // way to gray at +3. Unlike grayFade it doesn't touch other colors or turn
    // off hue rotation. Negative = more saturated instead (-64 = double the
    // chroma, -128 = triple). 0 = off.
    s8  bandGray;
    // Caps how far the general hue rotation can push band colors in the positive
    // (hue-increasing) direction, in sine-table units (about 1.4 degrees each).
    // Rotation the other way is never limited, and colors outside the band are
    // untouched. Split-hue modes flip the rotation for cool colors, so this holds
    // a color back from a hue it must not reach (e.g. a blue body drifting toward
    // a purple shiny) without turning rotation off. A negative cap also pushes
    // band colors that would rotate less than that the other way, so they
    // always end up at least that far below their hue. 0 = off.
    s8  bandMaxUp;
    // Chroma filter for the band: hue alone can't tell a pale part from a
    // saturated one of the same hue (cream hair vs yellow flames). Positive =
    // only colors whose chroma (largest minus smallest 5-bit channel) is at most
    // this many units are in the band; negative = only colors with at least
    // that much chroma. Decided from the color before either band moves it, and
    // the bandMaxUp cap follows it too. 0 = off (any chroma).
    s8  bandChroma;
    // 1 = the band's colors skip the general hue rotation (maxAngle /
    // angleBias), so the band alone sets their hue: step -4 keeps the original
    // color and step +3 gets the full bandShift. Colors in the band's fade zone
    // skip it in proportion. Saturation modes, fades and satMul still apply;
    // bandMaxUp has nothing left to cap on fully held colors. 0 = off.
    u8  bandHold;
    // Second hue band: the same fields as the first band (its chroma filter
    // and hold, band2Chroma / band2Hold, are the last fields), for another
    // color part (e.g. blue plates -> indigo while a red belly -> orange).
    // Membership is decided from the color before either band moves it, so the
    // two bands never chain. A color in reach of both bands' caps follows the
    // first band's bandMaxUp.
    u8  band2Hue;
    u8  band2Width;   // 0 = off
    s8  band2Shift;
    s8  band2Lighten;
    s8  band2Darken;
    s8  band2Gray;
    s8  band2MaxUp;
    s8  band2Chroma;  // like bandChroma, for the second band
    u8  band2Hold;    // like bandHold, for the second band
};

static const struct ColorVariationOverride sColorVariationOverrides[] =
{
    //{ species, maxAngle, angleBias,
    // holdBias, satCap, satMul,
    // tintHue, tintSpread, tintStrength,
    // tintDarken, tintLighten, tintRamp,
    // tintWhites, grayFade, whiteFade,
    // brownFade, bandHue, bandWidth,
    // bandShift, bandLighten, bandDarken,
    // bandGray, bandMaxUp, bandChroma,
    // bandHold, band2Hue, band2Width,
    // band2Shift, band2Lighten, band2Darken,
    // band2Gray, band2MaxUp, band2Chroma,
    // band2Hold },

    {
        .species = SPECIES_BULBASAUR,
        .maxAngle = 15,                         // +                        hue rotation range (14 = default)
        .angleBias = 6,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = 900,                          // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = 1228,                         // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 116,                         // 0-255 needs bandWidth    band center hue
        .bandWidth = 15,                        // +                        band half-width (0 = band off)
        .bandShift = 10,                        // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_IVYSAUR,
        .maxAngle = 15,                         // +                        hue rotation range (14 = default)
        .angleBias = 6,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = 900,                          // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = 1228,                         // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 116,                         // 0-255 needs bandWidth    band center hue
        .bandWidth = 15,                        // +                        band half-width (0 = band off)
        .bandShift = 10,                        // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_VENUSAUR,
        .maxAngle = 15,                         // +                        hue rotation range (14 = default)
        .angleBias = 6,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = 900,                          // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = 1228,                         // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 116,                         // 0-255 needs bandWidth    band center hue
        .bandWidth = 15,                        // +                        band half-width (0 = band off)
        .bandShift = 10,                        // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    {
        .species = SPECIES_CHARMANDER,
        .maxAngle = 8,                          // +                        hue rotation range (14 = default)
        .angleBias = -12,                       // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 13,                          // 0-255 needs bandWidth    band center hue
        .bandWidth = 6,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 10,                      // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = -10,                      // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_CHARMELEON,
        .maxAngle = 8,                          // +                        hue rotation range (14 = default)
        .angleBias = -12,                       // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 13,                          // 0-255 needs bandWidth    band center hue
        .bandWidth = 6,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 10,                      // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = -10,                      // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_CHARIZARD,
        .maxAngle = 8,                          // +                        hue rotation range (14 = default)
        .angleBias = -12,                       // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 13,                          // 0-255 needs bandWidth    band center hue
        .bandWidth = 6,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 10,                      // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = -10,                      // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    {
        .species = SPECIES_SQUIRTLE,
        .maxAngle = 12,                         // +                        hue rotation range (14 = default)
        .angleBias = -6,                        // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = 1228,                         // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 181,                         // 0-255 needs bandWidth    band center hue
        .bandWidth = 8,                         // +                        band half-width (0 = band off)
        .bandShift = -10,                       // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 4,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 8,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_WARTORTLE,
        .maxAngle = 12,                         // +                        hue rotation range (14 = default)
        .angleBias = -6,                        // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = 1228,                         // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 181,                         // 0-255 needs bandWidth    band center hue
        .bandWidth = 8,                         // +                        band half-width (0 = band off)
        .bandShift = -10,                       // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 4,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 8,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = -12,                       // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_BLASTOISE,
        .maxAngle = 12,                         // +                        hue rotation range (14 = default)
        .angleBias = -6,                        // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = 1228,                         // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 181,                         // 0-255 needs bandWidth    band center hue
        .bandWidth = 8,                         // +                        band half-width (0 = band off)
        .bandShift = -10,                       // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 4,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 8,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = -12,                       // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    {
        .species = SPECIES_CATERPIE,
        .maxAngle = 6,                          // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 48,                          // 0-255 needs bandWidth    band center hue
        .bandWidth = 16,                        // +                        band half-width (0 = band off)
        .bandShift = 35,                        // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_METAPOD,
        .maxAngle = 6,                          // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 48,                          // 0-255 needs bandWidth    band center hue
        .bandWidth = 16,                        // +                        band half-width (0 = band off)
        .bandShift = 35,                        // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_BUTTERFREE,
        .maxAngle = 6,                          // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 108,                         // 0-255 needs bandWidth    band center hue
        .bandWidth = 16,                        // +                        band half-width (0 = band off)
        .bandShift = 35,                        // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    {
        .species = SPECIES_WEEDLE,
        .maxAngle = 20,                         // +                        hue rotation range (14 = default)
        .angleBias = 6,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = 2056,                         // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 24,                        // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 0,                           // 0-255 needs bandWidth    band center hue
        .bandWidth = 0,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_KAKUNA,
        .maxAngle = 20,                         // +                        hue rotation range (14 = default)
        .angleBias = 10,                        // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = 2056,                         // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 24,                        // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 0,                           // 0-255 needs bandWidth    band center hue
        .bandWidth = 0,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_BEEDRILL,
        .maxAngle = 20,                         // +                        hue rotation range (14 = default)
        .angleBias = 10,                        // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = 2056,                         // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 24,                        // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 0,                           // 0-255 needs bandWidth    band center hue
        .bandWidth = 0,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    
    {
        .species = SPECIES_PIDGEY,
        .maxAngle = COLOR_VARIATION_MAX_ANGLE,  // +                        hue rotation range (14 = default)
        .angleBias = 3,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 24,                         // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = -20,                       // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 0,                           // 0-255 needs bandWidth    band center hue
        .bandWidth = 0,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_PIDGEOTTO,
        .maxAngle = COLOR_VARIATION_MAX_ANGLE,  // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 24,                         // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = -20,                       // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 0,                           // 0-255 needs bandWidth    band center hue
        .bandWidth = 0,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_PIDGEOT,
        .maxAngle = COLOR_VARIATION_MAX_ANGLE,  // +                        hue rotation range (14 = default)
        .angleBias = -5,                        // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 24,                         // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = -20,                       // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 0,                           // 0-255 needs bandWidth    band center hue
        .bandWidth = 0,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    {
        .species = SPECIES_RATTATA,
        .maxAngle = COLOR_VARIATION_MAX_ANGLE,  // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 24,                        // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = -20,                       // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 31,                          // 0-255 needs bandWidth    band center hue
        .bandWidth = 16,                        // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 24,                         // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_RATICATE,
        .maxAngle = COLOR_VARIATION_MAX_ANGLE,  // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 24,                        // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = -20,                       // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 31,                          // 0-255 needs bandWidth    band center hue
        .bandWidth = 16,                        // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 24,                         // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    {
        .species = SPECIES_SPEAROW,
        .maxAngle = COLOR_VARIATION_MAX_ANGLE,  // +                        hue rotation range (14 = default)
        .angleBias = 3,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 24,                         // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = -20,                       // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 0,                           // 0-255 needs bandWidth    band center hue
        .bandWidth = 0,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_FEAROW,
        .maxAngle = COLOR_VARIATION_MAX_ANGLE,  // +                        hue rotation range (14 = default)
        .angleBias = -5,                        // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 24,                         // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = -20,                       // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 0,                           // 0-255 needs bandWidth    band center hue
        .bandWidth = 0,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    {
        .species = SPECIES_EKANS,
        .maxAngle = COLOR_VARIATION_MAX_ANGLE,  // +                        hue rotation range (14 = default)
        .angleBias = -8,                        // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 0,                           // 0-255 needs bandWidth    band center hue
        .bandWidth = 0,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_ARBOK,
        .maxAngle = COLOR_VARIATION_MAX_ANGLE,  // +                        hue rotation range (14 = default)
        .angleBias = -8,                        // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 0,                           // 0-255 needs bandWidth    band center hue
        .bandWidth = 0,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    // Pikachu line: rotating hue turns them orange/red (toward the shinies)
    // or green. Instead, positive steps fade toward white and negative steps
    // toward brown. satCap stops vivid modes from approaching the more
    // saturated shinies.
    {
        .species = SPECIES_PICHU,
        .maxAngle = 24,                         // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = 1228,                         // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 22,                        // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = -14,                       // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 0,                           // 0-255 needs bandWidth    band center hue
        .bandWidth = 0,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_PIKACHU,
        .maxAngle = 24,                         // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = 1228,                         // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 22,                        // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = -14,                       // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 0,                           // 0-255 needs bandWidth    band center hue
        .bandWidth = 0,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_RAICHU,
        .maxAngle = 24,                         // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = 1228,                         // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 22,                        // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = -14,                       // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 0,                           // 0-255 needs bandWidth    band center hue
        .bandWidth = 0,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    {
        .species = SPECIES_SANDSHREW,
        .maxAngle = 4,                          // +                        hue rotation range (14 = default)
        .angleBias = -4,                        // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = 1953,                         // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = 880,                          // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = -36,                       // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 0,                           // 0-255 needs bandWidth    band center hue
        .bandWidth = 0,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_SANDSLASH,
        .maxAngle = 4,                          // +                        hue rotation range (14 = default)
        .angleBias = -4,                        // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = 1953,                         // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = 880,                          // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = -36,                       // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 0,                           // 0-255 needs bandWidth    band center hue
        .bandWidth = 0,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    // Clefairy line: positive hue steps turn the pink peach/yellow. Fade
    // those toward white instead; negative steps keep the normal rotation.
    {
        .species = SPECIES_CLEFFA,
        .maxAngle = COLOR_VARIATION_MAX_ANGLE,  // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 28,                        // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 0,                           // 0-255 needs bandWidth    band center hue
        .bandWidth = 0,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_CLEFAIRY,
        .maxAngle = COLOR_VARIATION_MAX_ANGLE,  // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 28,                        // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 0,                           // 0-255 needs bandWidth    band center hue
        .bandWidth = 0,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_CLEFABLE,
        .maxAngle = COLOR_VARIATION_MAX_ANGLE,  // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 28,                        // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 0,                           // 0-255 needs bandWidth    band center hue
        .bandWidth = 0,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    {
        .species = SPECIES_VULPIX,
        .maxAngle = 8,                          // +                        hue rotation range (14 = default)
        .angleBias = -12,                       // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 13,                          // 0-255 needs bandWidth    band center hue
        .bandWidth = 6,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 10,                      // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = -10,                      // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_NINETALES,
        .maxAngle = 8,                          // +                        hue rotation range (14 = default)
        .angleBias = 4,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 13,                          // 0-255 needs bandWidth    band center hue
        .bandWidth = 6,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 10,                      // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = -10,                      // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    // Jigglypuff line: positive hue steps turn the pink peach/yellow. Fade
    // those toward white instead; negative steps keep the normal rotation.
    // Igglybuff and Wigglytuff's base palettes are more peach than
    // Jigglypuff's, so angleBias rotates their whole range toward pink to
    // match. (Kept at -6: Igglybuff's shiny is pink, so more bias would push
    // its most-rotated variants into the shiny's hue range.)
    {
        .species = SPECIES_IGGLYBUFF,
        .maxAngle = COLOR_VARIATION_MAX_ANGLE,  // +                        hue rotation range (14 = default)
        .angleBias = -6,                        // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 36,                        // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 0,                           // 0-255 needs bandWidth    band center hue
        .bandWidth = 0,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_JIGGLYPUFF,
        .maxAngle = COLOR_VARIATION_MAX_ANGLE,  // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 36,                        // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 0,                           // 0-255 needs bandWidth    band center hue
        .bandWidth = 0,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_WIGGLYTUFF,
        .maxAngle = COLOR_VARIATION_MAX_ANGLE,  // +                        hue rotation range (14 = default)
        .angleBias = -8,                        // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 36,                        // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 0,                           // 0-255 needs bandWidth    band center hue
        .bandWidth = 0,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    {
        .species = SPECIES_ZUBAT,
        .maxAngle = COLOR_VARIATION_MAX_ANGLE,  // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = 720,                          // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 175,                         // 0-255 needs bandWidth    band center hue
        .bandWidth = 8,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 20,                         // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_GOLBAT,
        .maxAngle = COLOR_VARIATION_MAX_ANGLE,  // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = 720,                          // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 224,                         // 0-255 needs bandWidth    band center hue
        .bandWidth = 8,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 20,                         // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_CROBAT,
        .maxAngle = COLOR_VARIATION_MAX_ANGLE,  // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = 720,                          // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 206,                         // 0-255 needs bandWidth    band center hue
        .bandWidth = 8,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 20,                         // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    // Oddish line: hue band rotates only the slate-blue body (~200 deg) toward
    // purple, from the original blue (step -4) to violet (step +3).
    {
        .species = SPECIES_ODDISH,
        .maxAngle = COLOR_VARIATION_MAX_ANGLE,  // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 20,                        // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 142,                         // 0-255 needs bandWidth    band center hue
        .bandWidth = 12,                        // +                        band half-width (0 = band off)
        .bandShift = 48,                        // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_GLOOM,
        .maxAngle = COLOR_VARIATION_MAX_ANGLE,  // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 20,                        // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 142,                         // 0-255 needs bandWidth    band center hue
        .bandWidth = 12,                        // +                        band half-width (0 = band off)
        .bandShift = 48,                        // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_VILEPLUME,
        .maxAngle = COLOR_VARIATION_MAX_ANGLE,  // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 20,                        // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 142,                         // 0-255 needs bandWidth    band center hue
        .bandWidth = 12,                        // +                        band half-width (0 = band off)
        .bandShift = 48,                        // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    // Bellossom: hue band turns only the red flowers (~10 deg) toward a
    // lighter rose pink.
    {
        .species = SPECIES_BELLOSSOM,
        .maxAngle = COLOR_VARIATION_MAX_ANGLE,  // +                        hue rotation range (14 = default)
        .angleBias = -6,                        // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 10,                        // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 7,                           // 0-255 needs bandWidth    band center hue
        .bandWidth = 12,                        // +                        band half-width (0 = band off)
        .bandShift = -12,                       // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 36,                      // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    // Paras line. Parasect fades toward white on the side that heads for its
    // (yellow) shiny and keeps normal hue rotation on the side that heads
    // away from it.
    // Paras: the shiny is a red-orange body, so the body's hue is locked
    // (maxAngle 0) and capped (satCap 900) to keep it tan-orange. Variation
    // comes from the mushrooms instead: a hue band turns the red caps toward
    // pink/lilac (away from the shiny's red), lightening slightly.
    {
        .species = SPECIES_PARAS,
        .maxAngle = 0,                          // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = 900,                          // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE * 2,                 // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 0,                           // 0-255 needs bandWidth    band center hue
        .bandWidth = 6,                         // +                        band half-width (0 = band off)
        .bandShift = -48,                       // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 24,                      // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_PARASECT,
        .maxAngle = COLOR_VARIATION_MAX_ANGLE,  // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 28,                        // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 0,                           // 0-255 needs bandWidth    band center hue
        .bandWidth = 0,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    {
        .species = SPECIES_VENONAT,
        .maxAngle = 12,                         // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE,                     // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = 940,                          // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 204,                         // 0-255 needs bandWidth    band center hue
        .bandWidth = 1,                         // +                        band half-width (0 = band off)
        .bandShift = 20,                        // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 30,                       // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_VENOMOTH,
        .maxAngle = 18,                         // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE,                     // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = 940,                          // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 197,                         // 0-255 needs bandWidth    band center hue
        .bandWidth = 15,                        // +                        band half-width (0 = band off)
        .bandShift = 20,                        // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 15,                       // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 10,                         // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    {
        .species = SPECIES_MEOWTH,
        .maxAngle = 0,                          // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = 1126,                         // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 22,                        // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = -14,                       // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 20,                          // 0-255 needs bandWidth    band center hue
        .bandWidth = 6,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 6,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_PERSIAN,
        .maxAngle = 0,                          // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = 1126,                         // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 32,                        // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 34,                          // 0-255 needs bandWidth    band center hue
        .bandWidth = 11,                        // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 6,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    // Mankey line: the shinies are duller/darker fur with olive-green
    // accents, so muted modes and positive hue steps drift toward them.
    // A permanent saturation boost (satMul) keeps every variant richer than
    // the shiny while leaving the modes free to vary; negative steps rotate
    // toward orange/coral, positive steps fade toward cream instead of
    // rotating toward green. angleBias (~4-6 degrees toward amber) keeps the
    // dark yellows from reading as olive and the vivid creams from reading
    // as lemon-green, the shinies' direction.
    {
        .species = SPECIES_MANKEY,
        .maxAngle = COLOR_VARIATION_MAX_ANGLE,  // +                        hue rotation range (14 = default)
        .angleBias = -4,                        // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = 1229,                         // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 28,                        // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 0,                           // 0-255 needs bandWidth    band center hue
        .bandWidth = 0,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_PRIMEAPE,
        .maxAngle = COLOR_VARIATION_MAX_ANGLE,  // +                        hue rotation range (14 = default)
        .angleBias = -3,                        // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = 1126,                         // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 28,                        // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 0,                           // 0-255 needs bandWidth    band center hue
        .bandWidth = 0,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    {
        .species = SPECIES_GROWLITHE,
        .maxAngle = 8,                          // +                        hue rotation range (14 = default)
        .angleBias = -12,                       // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 13,                          // 0-255 needs bandWidth    band center hue
        .bandWidth = 6,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 10,                      // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = -10,                      // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_ARCANINE,
        .maxAngle = 8,                          // +                        hue rotation range (14 = default)
        .angleBias = -12,                       // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 13,                          // 0-255 needs bandWidth    band center hue
        .bandWidth = 6,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 10,                      // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = -10,                      // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    {
        .species = SPECIES_MACHOP,
        .maxAngle = COLOR_VARIATION_MAX_ANGLE,  // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = 880,                          // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 94,                          // 0-255 needs bandWidth    band center hue
        .bandWidth = 8,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 40,                         // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_MACHOKE,
        .maxAngle = COLOR_VARIATION_MAX_ANGLE,  // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = 880,                          // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 186,                         // 0-255 needs bandWidth    band center hue
        .bandWidth = 8,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 40,                         // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_MACHAMP,
        .maxAngle = COLOR_VARIATION_MAX_ANGLE,  // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = 880,                          // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 92,                          // 0-255 needs bandWidth    band center hue
        .bandWidth = 14,                        // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 40,                         // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    {
        .species = SPECIES_BELLSPROUT,
        .maxAngle = 12,                         // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 60,                          // 0-255 needs bandWidth    band center hue
        .bandWidth = 10,                        // +                        band half-width (0 = band off)
        .bandShift = 25,                        // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_WEEPINBELL,
        .maxAngle = 12,                         // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 60,                          // 0-255 needs bandWidth    band center hue
        .bandWidth = 10,                        // +                        band half-width (0 = band off)
        .bandShift = 25,                        // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_VICTREEBEL,
        .maxAngle = 12,                         // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 60,                          // 0-255 needs bandWidth    band center hue
        .bandWidth = 10,                        // +                        band half-width (0 = band off)
        .bandShift = 25,                        // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    // Geodude line: stone colors. The olive body is pulled toward gray
    // (satMul 512) and a hue band turns it from olive toward sage and a hint
    // of teal (positive steps). Negative steps fade toward a muted brown or
    // gray instead of rotating toward gold/red (the shinies). Golem's band is
    // narrower so its tan face isn't turned green along with the shell.
    {
        .species = SPECIES_GEODUDE,
        .maxAngle = 0,                          // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE,                     // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = 1125,                         // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = -6,                        // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 47,                          // 0-255 needs bandWidth    band center hue
        .bandWidth = 10,                        // +                        band half-width (0 = band off)
        .bandShift = 30,                        // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 50,                         // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_GRAVELER,
        .maxAngle = 0,                          // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE,                     // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = 1125,                         // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = -6,                        // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 47,                          // 0-255 needs bandWidth    band center hue
        .bandWidth = 10,                        // +                        band half-width (0 = band off)
        .bandShift = 30,                        // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 50,                         // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_GOLEM,
        .maxAngle = 0,                          // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE,                     // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = 1125,                         // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = -6,                        // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 47,                          // 0-255 needs bandWidth    band center hue
        .bandWidth = 10,                        // +                        band half-width (0 = band off)
        .bandShift = 30,                        // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 50,                         // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    {
        .species = SPECIES_SLOWPOKE,
        .maxAngle = COLOR_VARIATION_MAX_ANGLE,  // +                        hue rotation range (14 = default)
        .angleBias = -6,                        // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 36,                        // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 0,                           // 0-255 needs bandWidth    band center hue
        .bandWidth = 0,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_SLOWBRO,
        .maxAngle = COLOR_VARIATION_MAX_ANGLE,  // +                        hue rotation range (14 = default)
        .angleBias = -6,                        // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 36,                        // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 0,                           // 0-255 needs bandWidth    band center hue
        .bandWidth = 0,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_SLOWKING,
        .maxAngle = COLOR_VARIATION_MAX_ANGLE,  // +                        hue rotation range (14 = default)
        .angleBias = -6,                        // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 36,                        // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 0,                           // 0-255 needs bandWidth    band center hue
        .bandWidth = 0,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    // Magnemite/Magneton: pale green-gray body. A hue band carries it toward blue
    // (positive steps), while tintDarken/tintLighten spread individuals from a dark
    // steel gray (step -4) to a pale icy blue (step +3). maxAngle 0 keeps the red
    // and blue magnets fixed; satMul adds a little richness so the blue reads.
    {
        .species = SPECIES_MAGNEMITE,
        .maxAngle = 0,                          // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE,                     // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = 1126,                         // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 18,                       // +                        grays toward black at step -4
        .tintLighten = 20,                      // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 95,                          // 0-255 needs bandWidth    band center hue
        .bandWidth = 12,                        // +                        band half-width (0 = band off)
        .bandShift = 72,                        // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_MAGNETON,
        .maxAngle = 0,                          // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE,                     // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = 1126,                         // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 18,                       // +                        grays toward black at step -4
        .tintLighten = 20,                      // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 95,                          // 0-255 needs bandWidth    band center hue
        .bandWidth = 12,                        // +                        band half-width (0 = band off)
        .bandShift = 72,                        // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    {
        .species = SPECIES_FARFETCHD,
        .maxAngle = COLOR_VARIATION_MAX_ANGLE,  // +                        hue rotation range (14 = default)
        .angleBias = -4,                        // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 24,                         // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = -20,                       // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 0,                           // 0-255 needs bandWidth    band center hue
        .bandWidth = 0,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    {
        .species = SPECIES_DODUO,
        .maxAngle = COLOR_VARIATION_MAX_ANGLE,  // +                        hue rotation range (14 = default)
        .angleBias = 3,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 24,                         // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = -20,                       // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 0,                           // 0-255 needs bandWidth    band center hue
        .bandWidth = 0,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_DODRIO,
        .maxAngle = COLOR_VARIATION_MAX_ANGLE,  // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 24,                         // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = -20,                       // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 0,                           // 0-255 needs bandWidth    band center hue
        .bandWidth = 0,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    {
        .species = SPECIES_SEEL,
        .maxAngle = 6,                          // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 144,                         // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 3,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 3,                      // +                        gray tint amount (0 = off)
        .tintDarken = 4,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 0,                           // 0-255 needs bandWidth    band center hue
        .bandWidth = 0,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_DEWGONG,
        .maxAngle = 6,                          // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 144,                         // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 3,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 3,                      // +                        gray tint amount (0 = off)
        .tintDarken = 4,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 0,                           // 0-255 needs bandWidth    band center hue
        .bandWidth = 0,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    {
        .species = SPECIES_SHELLDER,
        .maxAngle = COLOR_VARIATION_MAX_ANGLE,  // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 28,                         // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 0,                           // 0-255 needs bandWidth    band center hue
        .bandWidth = 0,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_CLOYSTER,
        .maxAngle = COLOR_VARIATION_MAX_ANGLE,  // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 28,                         // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 0,                           // 0-255 needs bandWidth    band center hue
        .bandWidth = 0,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    // Gastly line: ranges from the Emerald palette (step -4) to a GSC-style
    // look (step +3). The purple band gets more saturated (negative bandGray)
    // and, for Haunter/Gengar, darker (negative bandLighten), and Gengar's
    // mauve is rotated toward GSC's deep indigo.
    {
        .species = SPECIES_GASTLY,
        .maxAngle = 8,                          // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = 1400,                         // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 215,                         // 0-255 needs bandWidth    band center hue
        .bandWidth = 14,                        // +                        band half-width (0 = band off)
        .bandShift = -10,                       // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = -40,                     // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = -80,                        // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_HAUNTER,
        .maxAngle = 6,                          // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 205,                         // 0-255 needs bandWidth    band center hue
        .bandWidth = 12,                        // +                        band half-width (0 = band off)
        .bandShift = -6,                        // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = -50,                     // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = -80,                        // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_GENGAR,
        .maxAngle = 6,                          // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 215,                         // 0-255 needs bandWidth    band center hue
        .bandWidth = 14,                        // +                        band half-width (0 = band off)
        .bandShift = -28,                       // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = -50,                     // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = -80,                        // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    {
        .species = SPECIES_ONIX,
        .maxAngle = COLOR_VARIATION_MAX_ANGLE,  // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 148,                         // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 3,                      // +                        gray tint amount (0 = off)
        .tintDarken = 8,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 56,                         // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 0,                           // 0-255 needs bandWidth    band center hue
        .bandWidth = 0,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_STEELIX,
        .maxAngle = COLOR_VARIATION_MAX_ANGLE,  // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 148,                         // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 8,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 56,                         // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 0,                           // 0-255 needs bandWidth    band center hue
        .bandWidth = 0,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    {
        .species = SPECIES_DROWZEE,
        .maxAngle = COLOR_VARIATION_MAX_ANGLE,  // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = 940,                          // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = -32,                       // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 30,                          // 0-255 needs bandWidth    band center hue
        .bandWidth = 13,                        // +                        band half-width (0 = band off)
        .bandShift = -14,                       // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_HYPNO,
        .maxAngle = 4,                          // +                        hue rotation range (14 = default)
        .angleBias = 6,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = 940,                          // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = -32,                       // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 30,                          // 0-255 needs bandWidth    band center hue
        .bandWidth = 13,                        // +                        band half-width (0 = band off)
        .bandShift = -14,                       // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    {
        .species = SPECIES_KRABBY,
        .maxAngle = 4,                          // +                        hue rotation range (14 = default)
        .angleBias = -6,                        // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 22,                          // 0-255 needs bandWidth    band center hue
        .bandWidth = 12,                        // +                        band half-width (0 = band off)
        .bandShift = -20,                       // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_KINGLER,
        .maxAngle = 4,                          // +                        hue rotation range (14 = default)
        .angleBias = -6,                        // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 22,                          // 0-255 needs bandWidth    band center hue
        .bandWidth = 12,                        // +                        band half-width (0 = band off)
        .bandShift = -20,                       // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    {
        .species = SPECIES_EXEGGCUTE,
        .maxAngle = COLOR_VARIATION_MAX_ANGLE,  // +                        hue rotation range (14 = default)
        .angleBias = -3,                        // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = 900,                          // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 28,                        // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 0,                           // 0-255 needs bandWidth    band center hue
        .bandWidth = 0,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_EXEGGUTOR,
        .maxAngle = COLOR_VARIATION_MAX_ANGLE,  // +                        hue rotation range (14 = default)
        .angleBias = 8,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = 1200,                         // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 0,                           // 0-255 needs bandWidth    band center hue
        .bandWidth = 0,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    {
        .species = SPECIES_CUBONE,
        .maxAngle = 18,                         // +                        hue rotation range (14 = default)
        .angleBias = -6,                        // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 26,                          // 0-255 needs bandWidth    band center hue
        .bandWidth = 16,                        // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_MAROWAK,
        .maxAngle = 18,                         // +                        hue rotation range (14 = default)
        .angleBias = -6,                        // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 26,                          // 0-255 needs bandWidth    band center hue
        .bandWidth = 16,                        // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    {
        .species = SPECIES_TYROGUE,
        .maxAngle = COLOR_VARIATION_MAX_ANGLE,  // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = -36,                       // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 218,                         // 0-255 needs bandWidth    band center hue
        .bandWidth = 16,                        // +                        band half-width (0 = band off)
        .bandShift = 18,                        // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_HITMONCHAN,
        .maxAngle = COLOR_VARIATION_MAX_ANGLE,  // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 28,                         // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 0,                           // 0-255 needs bandWidth    band center hue
        .bandWidth = 0,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_HITMONLEE,
        .maxAngle = COLOR_VARIATION_MAX_ANGLE,  // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 28,                        // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 0,                           // 0-255 needs bandWidth    band center hue
        .bandWidth = 0,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_HITMONTOP,
        .maxAngle = 10,                         // +                        hue rotation range (14 = default)
        .angleBias = -3,                        // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = 1229,                         // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 16,                        // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 0,                           // 0-255 needs bandWidth    band center hue
        .bandWidth = 0,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    {
        .species = SPECIES_LICKITUNG,
        .maxAngle = 12,                         // +                        hue rotation range (14 = default)
        .angleBias = -16,                       // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 9,                           // 0-255 needs bandWidth    band center hue
        .bandWidth = 8,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 24,                      // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    {
        .species = SPECIES_KOFFING,
        .maxAngle = 24,                         // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 0,                           // 0-255 needs bandWidth    band center hue
        .bandWidth = 0,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_WEEZING,
        .maxAngle = 24,                         // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 0,                           // 0-255 needs bandWidth    band center hue
        .bandWidth = 0,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    {
        .species = SPECIES_RHYHORN,
        .maxAngle = COLOR_VARIATION_MAX_ANGLE,  // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 148,                         // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 8,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 56,                         // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 0,                           // 0-255 needs bandWidth    band center hue
        .bandWidth = 0,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_RHYDON,
        .maxAngle = COLOR_VARIATION_MAX_ANGLE,  // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 148,                         // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 4,                      // +                        gray tint amount (0 = off)
        .tintDarken = 8,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 56,                         // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 0,                           // 0-255 needs bandWidth    band center hue
        .bandWidth = 0,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    {
        .species = SPECIES_TANGELA,
        .maxAngle = 8,                          // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = -12,                       // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 141,                         // 0-255 needs bandWidth    band center hue
        .bandWidth = 8,                         // +                        band half-width (0 = band off)
        .bandShift = 32,                        // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 14,                      // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    {
        .species = SPECIES_GOLDEEN,
        .maxAngle = 5,                          // +                        hue rotation range (14 = default)
        .angleBias = -9,                        // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = 1229,                         // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 24,                      // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 9,                           // 0-255 needs bandWidth    band center hue
        .bandWidth = 6,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = -64,                      // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_SEAKING,
        .maxAngle = 5,                          // +                        hue rotation range (14 = default)
        .angleBias = -9,                        // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = 1229,                         // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 24,                      // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 9,                           // 0-255 needs bandWidth    band center hue
        .bandWidth = 6,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = -64,                      // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    
    {
        .species = SPECIES_STARYU,
        .maxAngle = 10,                         // +                        hue rotation range (14 = default)
        .angleBias = 3,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 210,                         // 0-255 needs bandWidth    band center hue
        .bandWidth = 8,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 30,                      // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = -20,                      // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_STARMIE,
        .maxAngle = 10,                         // +                        hue rotation range (14 = default)
        .angleBias = 3,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 210,                         // 0-255 needs bandWidth    band center hue
        .bandWidth = 8,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 30,                      // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = -20,                      // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    // Mr. Mime: reds and pink skin drift from red toward magenta, getting paler and
    // grayer (bandGray) as the steps rise. Rotation stays small so nothing heads
    // toward the green shiny, and no hue goes toward yellow.
    {
        .species = SPECIES_MR_MIME,
        .maxAngle = 6,                          // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 6,                           // 0-255 needs bandWidth    band center hue
        .bandWidth = 8,                         // +                        band half-width (0 = band off)
        .bandShift = -25,                       // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 14,                      // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 32,                         // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    {
        .species = SPECIES_SCYTHER,
        .maxAngle = 6,                          // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 63,                          // 0-255 needs bandWidth    band center hue
        .bandWidth = 8,                         // +                        band half-width (0 = band off)
        .bandShift = 16,                        // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 10,                       // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_SCIZOR,
        .maxAngle = 8,                          // +                        hue rotation range (14 = default)
        .angleBias = -12,                       // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 13,                          // 0-255 needs bandWidth    band center hue
        .bandWidth = 6,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 10,                      // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = -10,                      // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    {
        .species = SPECIES_ELEKID,
        .maxAngle = 0,                          // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = 900,                          // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 24,                        // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = -20,                       // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 0,                           // 0-255 needs bandWidth    band center hue
        .bandWidth = 0,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_ELECTABUZZ,
        .maxAngle = COLOR_VARIATION_MAX_ANGLE,  // +                        hue rotation range (14 = default)
        .angleBias = 2,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = 1126,                         // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = -20,                       // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 0,                           // 0-255 needs bandWidth    band center hue
        .bandWidth = 0,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    {
        .species = SPECIES_MAGBY,
        .maxAngle = 8,                          // +                        hue rotation range (14 = default)
        .angleBias = -12,                       // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 13,                          // 0-255 needs bandWidth    band center hue
        .bandWidth = 6,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 10,                      // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = -10,                      // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_MAGMAR,
        .maxAngle = 8,                          // +                        hue rotation range (14 = default)
        .angleBias = -12,                       // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 13,                          // 0-255 needs bandWidth    band center hue
        .bandWidth = 6,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 10,                      // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = -10,                      // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    {
        .species = SPECIES_TAUROS,
        .maxAngle = 6,                          // +                        hue rotation range (14 = default)
        .angleBias = -4,                        // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 31,                          // 0-255 needs bandWidth    band center hue
        .bandWidth = 6,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 12,                      // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = -16,                      // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    // Lapras: the blue body keeps its full hue rotation (cyan and light blue),
    // but bandMaxUp stops it from drifting toward the purple shiny in either
    // the normal or split-hue modes.
    {
        .species = SPECIES_LAPRAS,
        .maxAngle = COLOR_VARIATION_MAX_ANGLE,  // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 152,                         // 0-255 needs bandWidth    band center hue
        .bandWidth = 6,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 4,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    {
        .species = SPECIES_EEVEE,
        .maxAngle = 0,                          // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = 1126,                         // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = -12,                        // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 32,                        // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 28,                          // 0-255 needs bandWidth    band center hue
        .bandWidth = 11,                        // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = -4,                       // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_VAPOREON,
        .maxAngle = 6,                          // +                        hue rotation range (14 = default)
        .angleBias = 6,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 118,                         // 0-255 needs bandWidth    band center hue
        .bandWidth = 6,                         // +                        band half-width (0 = band off)
        .bandShift = 30,                        // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_JOLTEON,
        .maxAngle = 0,                          // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = 900,                          // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 24,                        // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = -20,                       // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 0,                           // 0-255 needs bandWidth    band center hue
        .bandWidth = 0,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_FLAREON,
        .maxAngle = 8,                          // +                        hue rotation range (14 = default)
        .angleBias = -12,                       // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 13,                          // 0-255 needs bandWidth    band center hue
        .bandWidth = 6,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 10,                      // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = -10,                      // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_ESPEON,
        .maxAngle = 12,                         // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 0,                           // 0-255 needs bandWidth    band center hue
        .bandWidth = 0,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_UMBREON,
        .maxAngle = 8,                          // +                        hue rotation range (14 = default)
        .angleBias = 6,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 40,                       // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 0,                           // 0-255 needs bandWidth    band center hue
        .bandWidth = 0,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    {
        .species = SPECIES_PORYGON,
        .maxAngle = 17,                         // +                        hue rotation range (14 = default)
        .angleBias = -6,                        // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = 1024,                         // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 145,                         // 0-255 needs bandWidth    band center hue
        .bandWidth = 15,                        // +                        band half-width (0 = band off)
        .bandShift = 20,                        // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 24,                         // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_PORYGON2,
        .maxAngle = 17,                         // +                        hue rotation range (14 = default)
        .angleBias = -6,                        // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = 1024,                         // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 145,                         // 0-255 needs bandWidth    band center hue
        .bandWidth = 15,                        // +                        band half-width (0 = band off)
        .bandShift = 20,                        // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 24,                         // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    {
        .species = SPECIES_OMANYTE,
        .maxAngle = COLOR_VARIATION_MAX_ANGLE,  // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 10,                        // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 123,                         // 0-255 needs bandWidth    band center hue
        .bandWidth = 12,                        // +                        band half-width (0 = band off)
        .bandShift = 30,                        // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = -4,                        // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_OMASTAR,
        .maxAngle = COLOR_VARIATION_MAX_ANGLE,  // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 10,                        // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 123,                         // 0-255 needs bandWidth    band center hue
        .bandWidth = 12,                        // +                        band half-width (0 = band off)
        .bandShift = 30,                        // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = -4,                        // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    {
        .species = SPECIES_KABUTO,
        .maxAngle = 20,                         // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = 850,                          // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = -30,                      // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 24,                        // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 0,                           // 0-255 needs bandWidth    band center hue
        .bandWidth = 0,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_KABUTOPS,
        .maxAngle = 18,                         // +                        hue rotation range (14 = default)
        .angleBias = -16,                       // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = 880,                          // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 30,                       // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = -32,                       // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 0,                           // 0-255 needs bandWidth    band center hue
        .bandWidth = 0,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    {
        .species = SPECIES_AERODACTYL,
        .maxAngle = COLOR_VARIATION_MAX_ANGLE,  // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 148,                         // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 4,                      // +                        gray tint amount (0 = off)
        .tintDarken = 6,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 10,                         // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 10,                        // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 196,                         // 0-255 needs bandWidth    band center hue
        .bandWidth = 15,                        // +                        band half-width (0 = band off)
        .bandShift = -10,                       // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    }, 

    {
        .species = SPECIES_SNORLAX,
        .maxAngle = 10,                         // +                        hue rotation range (14 = default)
        .angleBias = -6,                        // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 80,                          // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 4,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 6,                      // +                        gray tint amount (0 = off)
        .tintDarken = 30,                       // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 0,                           // 0-255 needs bandWidth    band center hue
        .bandWidth = 0,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    {
        .species = SPECIES_ARTICUNO,
        .maxAngle = 6,                          // +                        hue rotation range (14 = default)
        .angleBias = 6,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 118,                         // 0-255 needs bandWidth    band center hue
        .bandWidth = 6,                         // +                        band half-width (0 = band off)
        .bandShift = 30,                        // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_ZAPDOS,
        .maxAngle = 0,                          // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = 900,                          // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 24,                        // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = -15,                       // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 0,                           // 0-255 needs bandWidth    band center hue
        .bandWidth = 0,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_MOLTRES,
        .maxAngle = 8,                          // +                        hue rotation range (14 = default)
        .angleBias = -12,                       // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 13,                          // 0-255 needs bandWidth    band center hue
        .bandWidth = 6,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 10,                      // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = -10,                      // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    {
        .species = SPECIES_DRATINI,
        .maxAngle = 6,                          // +                        hue rotation range (14 = default)
        .angleBias = -6,                        // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 192,                         // 0-255 needs bandWidth    band center hue
        .bandWidth = 6,                         // +                        band half-width (0 = band off)
        .bandShift = -30,                       // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    //Dragonair was fine
    {
        .species = SPECIES_DRAGONITE,
        .maxAngle = COLOR_VARIATION_MAX_ANGLE,  // +                        hue rotation range (14 = default)
        .angleBias = -16,                       // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 0,                           // 0-255 needs bandWidth    band center hue
        .bandWidth = 0,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    {
        .species = SPECIES_MEWTWO,
        .maxAngle = 12,                         // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 197,                         // 0-255 needs bandWidth    band center hue
        .bandWidth = 6,                         // +                        band half-width (0 = band off)
        .bandShift = -20,                       // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_MEW,
        .maxAngle = COLOR_VARIATION_MAX_ANGLE,  // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 36,                        // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 0,                           // 0-255 needs bandWidth    band center hue
        .bandWidth = 0,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    {
        .species = SPECIES_CHIKORITA,
        .maxAngle = 6,                          // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE,                     // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 20,                      // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 52,                          // 0-255 needs bandWidth    band center hue
        .bandWidth = 8,                         // +                        band half-width (0 = band off)
        .bandShift = 33,                        // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_BAYLEEF,
        .maxAngle = 6,                          // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE,                     // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 52,                          // 0-255 needs bandWidth    band center hue
        .bandWidth = 8,                         // +                        band half-width (0 = band off)
        .bandShift = 33,                        // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_MEGANIUM,
        .maxAngle = 6,                          // +                        hue rotation range (14 = default)
        .angleBias = 6,                         // +/-                      hue offset for every individual
        .holdBias = 1,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = 1228,                         // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 251,                         // 0-255 needs bandWidth    band center hue
        .bandWidth = 14,                        // +                        band half-width (0 = band off)
        .bandShift = -20,                       // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 28,                      // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = -24,                       // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    {
        .species = SPECIES_CYNDAQUIL,
        .maxAngle = 8,                          // +                        hue rotation range (14 = default)
        .angleBias = -12,                       // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 13,                          // 0-255 needs bandWidth    band center hue
        .bandWidth = 6,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 10,                      // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = -10,                      // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_QUILAVA,
        .maxAngle = 8,                          // +                        hue rotation range (14 = default)
        .angleBias = -12,                       // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 13,                          // 0-255 needs bandWidth    band center hue
        .bandWidth = 6,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 10,                      // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = -10,                      // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_TYPHLOSION,
        .maxAngle = 8,                          // +                        hue rotation range (14 = default)
        .angleBias = -12,                       // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 13,                          // 0-255 needs bandWidth    band center hue
        .bandWidth = 6,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 10,                      // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = -10,                      // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    {
        .species = SPECIES_SENTRET,
        .maxAngle = 0,                          // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = 1126,                         // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = -12,                        // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 28,                        // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 16,                          // 0-255 needs bandWidth    band center hue
        .bandWidth = 11,                        // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = -6,                       // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_FURRET,
        .maxAngle = 0,                          // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = 1126,                         // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = -12,                        // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 28,                        // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 35,                          // 0-255 needs bandWidth    band center hue
        .bandWidth = 11,                        // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = -6,                       // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    {
        .species = SPECIES_HOOTHOOT,
        .maxAngle = 0,                          // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = 1126,                         // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 6,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 25,                        // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 36,                          // 0-255 needs bandWidth    band center hue
        .bandWidth = 16,                        // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 10,                       // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_NOCTOWL,
        .maxAngle = 0,                          // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = 1126,                         // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 18,                         // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 25,                        // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 36,                          // 0-255 needs bandWidth    band center hue
        .bandWidth = 16,                        // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 10,                       // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    {
        .species = SPECIES_LEDYBA,
        .maxAngle = 18,                         // +                        hue rotation range (14 = default)
        .angleBias = 12,                        // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 23,                          // 0-255 needs bandWidth    band center hue
        .bandWidth = 16,                        // +                        band half-width (0 = band off)
        .bandShift = -40,                       // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 4,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_LEDIAN,
        .maxAngle = COLOR_VARIATION_MAX_ANGLE,  // +                        hue rotation range (14 = default)
        .angleBias = -18,                       // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 0,                           // 0-255 needs bandWidth    band center hue
        .bandWidth = 0,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    {
        .species = SPECIES_TOGEPI,
        .maxAngle = 0,                          // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE,                     // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 134,                         // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 4,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 6,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 1,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 1,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 8,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 0,                           // 0-255 needs bandWidth    band center hue
        .bandWidth = 0,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_TOGETIC,
        .maxAngle = 0,                          // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE,                     // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 134,                         // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 4,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 3,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 1,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 1,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 8,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 0,                           // 0-255 needs bandWidth    band center hue
        .bandWidth = 0,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    {
        .species = SPECIES_NATU,
        .maxAngle = 4,                          // +                        hue rotation range (14 = default)
        .angleBias = -6,                        // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = 1228,                         // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = 880,                          // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 81,                          // 0-255 needs bandWidth    band center hue
        .bandWidth = 8,                         // +                        band half-width (0 = band off)
        .bandShift = 5,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 16,                      // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = -24,                        // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_XATU,
        .maxAngle = 4,                          // +                        hue rotation range (14 = default)
        .angleBias = -6,                        // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = 1228,                         // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = 880,                          // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 81,                          // 0-255 needs bandWidth    band center hue
        .bandWidth = 8,                         // +                        band half-width (0 = band off)
        .bandShift = 5,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 16,                      // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = -24,                        // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    {
        .species = SPECIES_SUDOWOODO,
        .maxAngle = 8,                          // +                        hue rotation range (14 = default)
        .angleBias = -16,                       // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 0,                           // 0-255 needs bandWidth    band center hue
        .bandWidth = 0,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    {
        .species = SPECIES_POLITOED,
        .maxAngle = 12,                         // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 61,                          // 0-255 needs bandWidth    band center hue
        .bandWidth = 11,                        // +                        band half-width (0 = band off)
        .bandShift = -20,                       // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    {
        .species = SPECIES_HOPPIP,
        .maxAngle = COLOR_VARIATION_MAX_ANGLE,  // +                        hue rotation range (14 = default)
        .angleBias = -2,                        // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 20,                        // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 0,                           // 0-255 needs bandWidth    band center hue
        .bandWidth = 0,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    
    {
        .species = SPECIES_SUNKERN,
        .maxAngle = 8,                          // +                        hue rotation range (14 = default)
        .angleBias = 12,                        // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 70,                          // 0-255 needs bandWidth    band center hue
        .bandWidth = 13,                        // +                        band half-width (0 = band off)
        .bandShift = -45,                       // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 20,                       // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 4,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_SUNFLORA,
        .maxAngle = 6,                          // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 27,                          // 0-255 needs bandWidth    band center hue
        .bandWidth = 15,                        // +                        band half-width (0 = band off)
        .bandShift = -15,                       // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 8,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    {
        .species = SPECIES_MURKROW,
        .maxAngle = 6,                          // +                        hue rotation range (14 = default)
        .angleBias = -8,                        // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 40,                       // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 0,                           // 0-255 needs bandWidth    band center hue
        .bandWidth = 4,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = -32,                     // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    {
        .species = SPECIES_MISDREAVUS,
        .maxAngle = 6,                          // +                        hue rotation range (14 = default)
        .angleBias = -8,                        // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 6,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 119,                         // 0-255 needs bandWidth    band center hue
        .bandWidth = 8,                         // +                        band half-width (0 = band off)
        .bandShift = 30,                        // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = -20,                     // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = -80,                        // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    {
        .species = SPECIES_GIRAFARIG,
        .maxAngle = 6,                          // +                        hue rotation range (14 = default)
        .angleBias = -2,                        // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 6,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 31,                          // 0-255 needs bandWidth    band center hue
        .bandWidth = 10,                        // +                        band half-width (0 = band off)
        .bandShift = -10,                       // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 12,                      // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = -16,                      // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 4,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    {
        .species = SPECIES_PINECO,
        .maxAngle = 0,                          // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE,                     // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 142,                         // 0-255 needs bandWidth    band center hue
        .bandWidth = 16,                        // +                        band half-width (0 = band off)
        .bandShift = -30,                       // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_FORRETRESS,
        .maxAngle = 6,                          // +                        hue rotation range (14 = default)
        .angleBias = -6,                        // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = 880,                          // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 221,                         // 0-255 needs bandWidth    band center hue
        .bandWidth = 11,                        // +                        band half-width (0 = band off)
        .bandShift = -21,                       // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 56,                         // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    {
        .species = SPECIES_GLIGAR,
        .maxAngle = 6,                          // +                        hue rotation range (14 = default)
        .angleBias = -2,                        // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = 1536,                         // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 20,                         // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 20,                        // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 0,                           // 0-255 needs bandWidth    band center hue
        .bandWidth = 0,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = -40,                        // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    {
        .species = SPECIES_SNUBBULL,
        .maxAngle = COLOR_VARIATION_MAX_ANGLE,  // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 20,                        // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 0,                           // 0-255 needs bandWidth    band center hue
        .bandWidth = 0,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    {
        .species = SPECIES_SHUCKLE,
        .maxAngle = 20,                         // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE,                     // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 40,                        // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 0,                           // 0-255 needs bandWidth    band center hue
        .bandWidth = 0,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    {
        .species = SPECIES_HERACROSS,
        .maxAngle = 16,                         // +                        hue rotation range (14 = default)
        .angleBias = -12,                       // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE,                     // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 164,                         // 0-255 needs bandWidth    band center hue
        .bandWidth = 8,                         // +                        band half-width (0 = band off)
        .bandShift = -18,                       // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 6,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = -8,                        // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    {
        .species = SPECIES_SNEASEL,
        .maxAngle = 12,                         // +                        hue rotation range (14 = default)
        .angleBias = -8,                        // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 40,                       // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 6,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 0,                           // 0-255 needs bandWidth    band center hue
        .bandWidth = 4,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = -32,                     // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = -30,                      // +/-   needs bandWidth    band to black at the end steps
        .bandGray = -80,                        // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    {
        .species = SPECIES_TEDDIURSA,
        .maxAngle = 0,                          // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE,                     // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 20,                        // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = -48,                       // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 31,                          // 0-255 needs bandWidth    band center hue
        .bandWidth = 10,                        // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = -6,                       // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_URSARING,
        .maxAngle = 0,                          // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE,                     // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 20,                        // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = -48,                       // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 30,                          // 0-255 needs bandWidth    band center hue
        .bandWidth = 11,                        // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = -6,                       // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    {
        .species = SPECIES_SWINUB,
        .maxAngle = 0,                          // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE,                     // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 10,                        // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = -48,                       // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 30,                          // 0-255 needs bandWidth    band center hue
        .bandWidth = 16,                        // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = -6,                       // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_PILOSWINE,
        .maxAngle = 0,                          // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE,                     // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 10,                        // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = -48,                       // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 30,                          // 0-255 needs bandWidth    band center hue
        .bandWidth = 16,                        // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = -6,                       // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    {
        .species = SPECIES_REMORAID,
        .maxAngle = 30,                         // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = 820,                          // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 160,                         // 0-255 needs bandWidth    band center hue
        .bandWidth = 8,                         // +                        band half-width (0 = band off)
        .bandShift = -20,                       // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 8,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_OCTILLERY,
        .maxAngle = 8,                          // +                        hue rotation range (14 = default)
        .angleBias = -12,                       // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 13,                          // 0-255 needs bandWidth    band center hue
        .bandWidth = 6,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 10,                      // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = -10,                      // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    {
        .species = SPECIES_DELIBIRD,
        .maxAngle = 6,                          // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 76,                          // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 6,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 8,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 1,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 1,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 0,                           // 0-255 needs bandWidth    band center hue
        .bandWidth = 0,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    {
        .species = SPECIES_MANTINE,
        .maxAngle = 20,                         // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = 760,                          // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 24,                        // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 30,                        // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 155,                         // 0-255 needs bandWidth    band center hue
        .bandWidth = 10,                        // +                        band half-width (0 = band off)
        .bandShift = 12,                        // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    {
        .species = SPECIES_SKARMORY,
        .maxAngle = 12,                         // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 142,                         // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 1,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 4,                      // +                        gray tint amount (0 = off)
        .tintDarken = 4,                        // +                        grays toward black at step -4
        .tintLighten = 4,                       // +                        grays toward white at step +3
        .tintRamp = 1,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 0,                           // 0-255 needs bandWidth    band center hue
        .bandWidth = 0,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    {
        .species = SPECIES_HOUNDOUR,
        .maxAngle = 20,                         // +                        hue rotation range (14 = default)
        .angleBias = -12,                       // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 40,                       // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 6,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 13,                          // 0-255 needs bandWidth    band center hue
        .bandWidth = 6,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 10,                      // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = -10,                      // +/-   needs bandWidth    band to black at the end steps
        .bandGray = -80,                        // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_HOUNDOOM,
        .maxAngle = 20,                         // +                        hue rotation range (14 = default)
        .angleBias = -12,                       // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 40,                       // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 6,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 13,                          // 0-255 needs bandWidth    band center hue
        .bandWidth = 6,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 10,                      // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = -10,                      // +/-   needs bandWidth    band to black at the end steps
        .bandGray = -80,                        // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    {
        .species = SPECIES_STANTLER,
        .maxAngle = 0,                          // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = 1126,                         // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 32,                         // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = -12,                       // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 28,                          // 0-255 needs bandWidth    band center hue
        .bandWidth = 16,                        // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 6,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    {
        .species = SPECIES_SMEARGLE,
        .maxAngle = 0,                          // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = 1126,                         // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = -20,                        // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 20,                        // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 78,                          // 0-255 needs bandWidth    band center hue
        .bandWidth = 4,                         // +                        band half-width (0 = band off)
        .bandShift = 125,                       // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = -6,                       // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    {
        .species = SPECIES_MILTANK,
        .maxAngle = 12,                         // +                        hue rotation range (14 = default)
        .angleBias = -4,                        // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 31,                          // 0-255 needs bandWidth    band center hue
        .bandWidth = 6,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 12,                      // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = -8,                       // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    {
        .species = SPECIES_RAIKOU,
        .maxAngle = 6,                          // +                        hue rotation range (14 = default)
        .angleBias = 6,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = 925,                          // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 190,                         // 0-255 needs bandWidth    band center hue
        .bandWidth = 9,                         // +                        band half-width (0 = band off)
        .bandShift = -10,                       // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 4,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    {
        .species = SPECIES_ENTEI,
        .maxAngle = 8,                          // +                        hue rotation range (14 = default)
        .angleBias = -12,                       // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 2,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 4,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 1,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 1,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 13,                          // 0-255 needs bandWidth    band center hue
        .bandWidth = 6,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 10,                      // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = -10,                      // +/-   needs bandWidth    band to black at the end steps
        .bandGray = -30,                        // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    {
        .species = SPECIES_LARVITAR,
        .maxAngle = 18,                         // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 30,                         // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 15,                        // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 85,                          // 0-255 needs bandWidth    band center hue
        .bandWidth = 8,                         // +                        band half-width (0 = band off)
        .bandShift = 48,                        // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = -30,                        // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 6,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_PUPITAR,
        .maxAngle = COLOR_VARIATION_MAX_ANGLE,  // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 30,                         // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = -12,                       // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 147,                         // 0-255 needs bandWidth    band center hue
        .bandWidth = 8,                         // +                        band half-width (0 = band off)
        .bandShift = 12,                        // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 24,                         // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 6,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_TYRANITAR,
        .maxAngle = 18,                         // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 30,                         // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 15,                        // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 85,                          // 0-255 needs bandWidth    band center hue
        .bandWidth = 8,                         // +                        band half-width (0 = band off)
        .bandShift = 48,                        // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = -30,                        // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 6,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    {
        .species = SPECIES_LUGIA,
        .maxAngle = 20,                         // +                        hue rotation range (14 = default)
        .angleBias = 12,                        // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 156,                         // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 4,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 3,                      // +                        gray tint amount (0 = off)
        .tintDarken = 5,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 1,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 1,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 15,                         // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 155,                         // 0-255 needs bandWidth    band center hue
        .bandWidth = 12,                        // +                        band half-width (0 = band off)
        .bandShift = 20,                        // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    
    {
        .species = SPECIES_HO_OH,
        .maxAngle = 12,                         // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 25,                        // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 89,                          // 0-255 needs bandWidth    band center hue
        .bandWidth = 8,                         // +                        band half-width (0 = band off)
        .bandShift = 20,                        // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 20,                         // 0-255 needs band2Width   second band center hue
        .band2Width = 12,                       // +                        second band half-width (0 = off)
        .band2Shift = -18,                      // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    {
        .species = SPECIES_CELEBI,
        .maxAngle = 40,                         // +                        hue rotation range (14 = default)
        .angleBias = 8,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 40,                        // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 20,                        // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 0,                           // 0-255 needs bandWidth    band center hue
        .bandWidth = 0,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = -15,                        // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    {
        .species = SPECIES_TREECKO,
        .maxAngle = 12,                         // +                        hue rotation range (14 = default)
        .angleBias = 6,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 15,                        // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 59,                          // 0-255 needs bandWidth    band center hue
        .bandWidth = 9,                         // +                        band half-width (0 = band off)
        .bandShift = 33,                        // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 6,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 4,                        // +                        second band half-width (0 = off)
        .band2Shift = -39,                      // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_GROVYLE,
        .maxAngle = 12,                         // +                        hue rotation range (14 = default)
        .angleBias = 6,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 15,                        // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 59,                          // 0-255 needs bandWidth    band center hue
        .bandWidth = 9,                         // +                        band half-width (0 = band off)
        .bandShift = 33,                        // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 6,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 4,                        // +                        second band half-width (0 = off)
        .band2Shift = -39,                      // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_SCEPTILE,
        .maxAngle = 12,                         // +                        hue rotation range (14 = default)
        .angleBias = 6,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 15,                        // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 59,                          // 0-255 needs bandWidth    band center hue
        .bandWidth = 9,                         // +                        band half-width (0 = band off)
        .bandShift = 33,                        // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 6,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 4,                        // +                        second band half-width (0 = off)
        .band2Shift = -39,                      // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    {
        .species = SPECIES_BLAZIKEN,
        .maxAngle = COLOR_VARIATION_MAX_ANGLE,  // +                        hue rotation range (14 = default)
        .angleBias = -6,                        // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 26,                          // 0-255 needs bandWidth    band center hue
        .bandWidth = 3,                         // +                        band half-width (0 = band off)
        .bandShift = -30,                       // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = -10,                        // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 14,                       // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 1,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 26,                         // 0-255 needs band2Width   second band center hue
        .band2Width = 1,                        // +                        second band half-width (0 = off)
        .band2Shift = -20,                      // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = -15,                     // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
   
    {
        .species = SPECIES_POOCHYENA,
        .maxAngle = COLOR_VARIATION_MAX_ANGLE,  // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE,                     // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 6,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = -48,                        // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 20,                        // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 23,                          // 0-255 needs bandWidth    band center hue
        .bandWidth = 8,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = -6,                       // +/-   needs bandWidth    band to black at the end steps
        .bandGray = -10,                        // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_MIGHTYENA,
        .maxAngle = COLOR_VARIATION_MAX_ANGLE,  // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE,                     // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 6,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = -48,                        // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 30,                        // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 160,                         // 0-255 needs bandWidth    band center hue
        .bandWidth = 8,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = -6,                       // +/-   needs bandWidth    band to black at the end steps
        .bandGray = -10,                        // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    // Zigzagoon: hue rotation would turn it yellow/olive (positive steps) or
    // red (negative steps). Instead, positive steps fade subtly toward gray
    // and negative steps lighten slightly toward white.
    {
        .species = SPECIES_ZIGZAGOON,
        .maxAngle = COLOR_VARIATION_MAX_ANGLE,  // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 24,                         // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = -20,                       // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 21,                           // 0-255 needs bandWidth    band center hue
        .bandWidth = 9,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 10,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = -10,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_LINOONE,
        .maxAngle = COLOR_VARIATION_MAX_ANGLE,  // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 4,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 3,                      // +                        gray tint amount (0 = off)
        .tintDarken = 0,                        // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 1,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 1,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 24,                         // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = -20,                       // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 25,                           // 0-255 needs bandWidth    band center hue
        .bandWidth = 9,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 8,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    
    //quick
    //{ species, maxAngle, angleBias,
    // holdBias, satCap, satMul,
    // tintHue, tintSpread, tintStrength,
    // tintDarken, tintLighten, tintRamp,
    // tintWhites, grayFade, whiteFade,
    // brownFade, bandHue, bandWidth,
    // bandShift, bandLighten, bandDarken,
    // bandGray, bandMaxUp, bandChroma,
    // bandHold, band2Hue, band2Width,
    // band2Shift, band2Lighten, band2Darken,
    // band2Gray, band2MaxUp, band2Chroma,
    // band2Hold },

    

    

    // Shuppet line: body is almost pure gray, so only the eyes/details react
    // to the hue rotation. Spread individuals from the original gray (hue
    // step +3) down to near-black (step -4); no tint.
    {
        .species = SPECIES_SHUPPET,
        .maxAngle = COLOR_VARIATION_MAX_ANGLE,  // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 40,                       // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 0,                           // 0-255 needs bandWidth    band center hue
        .bandWidth = 0,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_BANETTE,
        .maxAngle = COLOR_VARIATION_MAX_ANGLE,  // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 40,                       // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 0,                         // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 0,                           // 0-255 needs bandWidth    band center hue
        .bandWidth = 0,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },

    // Duskull line: middle ground between the Shuppet and Zigzagoon setups.
    // Negative steps darken the grays (stopping at dark gray so Dusclops
    // doesn't become a silhouette); positive steps fade slightly toward white
    // instead of rotating hue (which would turn Duskull's skull green).
    {
        .species = SPECIES_DUSKULL,
        .maxAngle = COLOR_VARIATION_MAX_ANGLE,  // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 28,                       // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 0,                          // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 20,                        // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 0,                           // 0-255 needs bandWidth    band center hue
        .bandWidth = 0,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
    {
        .species = SPECIES_DUSCLOPS,
        .maxAngle = COLOR_VARIATION_MAX_ANGLE,  // +                        hue rotation range (14 = default)
        .angleBias = 0,                         // +/-                      hue offset for every individual
        .holdBias = 0,                          // 0/1   needs angleBias    split modes keep the bias direction
        .satCap = FP_SCALE * 2,                 // +                        vivid-mode cap (FP_SCALE = no boost)
        .satMul = FP_SCALE,                     // +                        saturation x (FP_SCALE = none)
        .tintHue = 0,                           // 0-255 needs tintStrength gray tint center hue
        .tintSpread = 0,                        // +/-   needs tintStrength tint hue change per step
        .tintStrength = 0,                      // +                        gray tint amount (0 = off)
        .tintDarken = 16,                       // +                        grays toward black at step -4
        .tintLighten = 0,                       // +                        grays toward white at step +3
        .tintRamp = 0,                          // 0/1   needs tintStrength tint ramps from step -4 to +3
        .tintWhites = 0,                        // 0/1   needs tintStrength near-whites take the tint / tintDarken too
        .grayFade = 16,                         // +/-                      fade to gray; + = +steps, - = -steps
        .whiteFade = 14,                        // +/-                      fade to white; + = +steps, - = -steps
        .brownFade = 0,                         // +/-                      fade to sepia; + = +steps, - = -steps
        .bandHue = 0,                           // 0-255 needs bandWidth    band center hue
        .bandWidth = 0,                         // +                        band half-width (0 = band off)
        .bandShift = 0,                         // +/-   needs bandWidth    band hue shift by step +3
        .bandLighten = 0,                       // +/-   needs bandWidth    band to white by +3 (- = black)
        .bandDarken = 0,                        // +/-   needs bandWidth    band to black at the end steps
        .bandGray = 0,                          // +/-   needs bandWidth    band desaturate (- = more vivid)
        .bandMaxUp = 0,                         // +/-   needs bandWidth    cap on upward band rotation
        .bandChroma = 0,                        // +/-   needs bandWidth    band chroma filter (+ = pale only, - = vivid only)
        .bandHold = 0,                          // 0/1   needs bandWidth    band colors skip the general hue rotation
        .band2Hue = 0,                          // 0-255 needs band2Width   second band center hue
        .band2Width = 0,                        // +                        second band half-width (0 = off)
        .band2Shift = 0,                        // +/-   needs band2Width   second band hue shift by step +3
        .band2Lighten = 0,                      // +/-   needs band2Width   second band to white by +3 (- = black)
        .band2Darken = 0,                       // +/-   needs band2Width   second band to black at the end steps
        .band2Gray = 0,                         // +/-   needs band2Width   second band desaturate (- = more vivid)
        .band2MaxUp = 0,                        // +/-   needs band2Width   cap on upward second-band rotation
        .band2Chroma = 0,                       // +/-   needs band2Width   second band chroma filter (+ pale, - vivid)
        .band2Hold = 0,                         // 0/1   needs band2Width   second band skips the general hue rotation
    },
};

static const struct ColorVariationOverride *FindColorVariationOverride(u16 species)
{
    u32 i;
    for (i = 0; i < ARRAY_COUNT(sColorVariationOverrides); i++)
    {
        if (sColorVariationOverrides[i].species == species)
            return &sColorVariationOverrides[i];
    }
    return NULL;
}

// sqrt(1/3) in fixed point: round(0.57735 * 1024) = 591
#define SQRT_ONE_THIRD_FP 591

// Hue rotation matrix coefficients (circulant matrix around (1,1,1) axis)
struct HueMatrix {
    s32 mA, mB, mC;
};

static void ComputeHueMatrix(s32 angleIndex, struct HueMatrix *mat)
{
    s32 cosA = Cos(angleIndex, FP_SCALE);
    s32 sinA = Sin(angleIndex, FP_SCALE);
    s32 oneMinusCosDiv3 = (FP_SCALE - cosA) / 3;
    s32 sqrtSinTerm = (SQRT_ONE_THIRD_FP * sinA) >> FP_SHIFT;

    mat->mA = cosA + oneMinusCosDiv3;
    mat->mB = oneMinusCosDiv3 + sqrtSinTerm;
    mat->mC = oneMinusCosDiv3 - sqrtSinTerm;
}

static void RotateColor(u16 *color, const struct HueMatrix *mat)
{
    s32 r = (*color >>  0) & 0x1F;
    s32 g = (*color >>  5) & 0x1F;
    s32 b = (*color >> 10) & 0x1F;

    s32 newR = (mat->mA * r + mat->mC * g + mat->mB * b) >> FP_SHIFT;
    s32 newG = (mat->mB * r + mat->mA * g + mat->mC * b) >> FP_SHIFT;
    s32 newB = (mat->mC * r + mat->mB * g + mat->mA * b) >> FP_SHIFT;

    if (newR < 0) newR = 0; else if (newR > 31) newR = 31;
    if (newG < 0) newG = 0; else if (newG > 31) newG = 31;
    if (newB < 0) newB = 0; else if (newB > 31) newB = 31;

    *color = (u16)(newR | (newG << 5) | (newB << 10));
}

static void AdjustSaturation(u16 *color, s32 factor)
{
    s32 r = (*color >>  0) & 0x1F;
    s32 g = (*color >>  5) & 0x1F;
    s32 b = (*color >> 10) & 0x1F;
    s32 gray = (r + g + b) / 3;

    r = gray + ((r - gray) * factor) / FP_SCALE;
    g = gray + ((g - gray) * factor) / FP_SCALE;
    b = gray + ((b - gray) * factor) / FP_SCALE;

    if (r < 0) r = 0; else if (r > 31) r = 31;
    if (g < 0) g = 0; else if (g > 31) g = 31;
    if (b < 0) b = 0; else if (b > 31) b = 31;

    *color = (u16)(r | (g << 5) | (b << 10));
}

// Returns TRUE if the color has enough chroma to classify as warm/cool
static bool8 IsColorSaturated(s32 r, s32 g, s32 b)
{
    s32 max = r;
    s32 min = r;
    if (g > max) max = g;
    if (b > max) max = b;
    if (g < min) min = g;
    if (b < min) min = b;
    return (max - min) >= COLOR_VARIATION_CHROMA_THRESHOLD;
}

// Returns 0 for warm (red-dominant), 1 for cool (green/blue-dominant)
static u8 GetHueBucket(s32 r, s32 g, s32 b)
{
    if (r >= g && r >= b)
        return 0; // warm
    return 1; // cool
}

// Gray level at which the neutral tint reaches full strength. Darker shades
// get proportionally less (keeps outlines black), and near-white highlights
// fade back out so they stay white.
#define TINT_FULL_GRAY  24
#define TINT_FADE_START 27

// Colors with a channel spread below this count as "gray" for tinting. It's
// looser than COLOR_VARIATION_CHROMA_THRESHOLD so slightly blue-ish grays
// (e.g. Banette's body) are tinted together with the pure grays.
#define TINT_MAX_CHROMA 6

// tintDarken/tintLighten are in 1/64ths of the way to black/white.
#define TINT_LIGHT_ONE 64

static s32 GetColorChroma(s32 r, s32 g, s32 b)
{
    s32 max = r;
    s32 min = r;
    if (g > max) max = g;
    if (b > max) max = b;
    if (g < min) min = g;
    if (b < min) min = b;
    return max - min;
}

// Pushes unsaturated palette colors toward a hue picked from the override's
// tint arc, and optionally lightens/darkens them per hue step. The tint
// offset vector sums to zero, so the tint itself preserves brightness.
static void ApplyNeutralTint(u16 *palette, const struct ColorVariationOverride *override, s32 signedStep)
{
    s32 angle = (override->tintHue + signedStep * override->tintSpread) & 0xFF;
    s32 dirR = Cos(angle, FP_SCALE);
    s32 dirG = Cos((angle - 85) & 0xFF, FP_SCALE);
    s32 dirB = Cos((angle + 85) & 0xFF, FP_SCALE);
    s32 lightMag;
    s32 tintScale;
    u32 i;

    // Linear from -tintDarken at step -4 to +tintLighten at step +3.
    // Positive = toward white, negative = toward black.
    s32 light = -override->tintDarken
              + (override->tintDarken + override->tintLighten) * (signedStep + 4) / 7;
    lightMag = (light < 0) ? -light : light;
    if (lightMag > TINT_LIGHT_ONE)
        lightMag = TINT_LIGHT_ONE;
    // The further an individual is pushed toward white/black, the less tint
    // it gets, so the extremes read as white/black instead of pale/dark tint.
    tintScale = TINT_LIGHT_ONE - lightMag;

    for (i = 1; i < 16; i++)
    {
        s32 r = (palette[i] >>  0) & 0x1F;
        s32 g = (palette[i] >>  5) & 0x1F;
        s32 b = (palette[i] >> 10) & 0x1F;
        s32 gray, weight, amount;

        if (GetColorChroma(r, g, b) >= TINT_MAX_CHROMA)
            continue;

        gray = (r + g + b) / 3;
        weight = (gray < TINT_FULL_GRAY) ? gray : TINT_FULL_GRAY;

        if (light > 0)
        {
            // Lighten toward white, scaled by the shade's brightness so dark
            // outlines stay dark.
            s32 lift = lightMag * weight;
            r += ((31 - r) * lift) / (TINT_LIGHT_ONE * TINT_FULL_GRAY);
            g += ((31 - g) * lift) / (TINT_LIGHT_ONE * TINT_FULL_GRAY);
            b += ((31 - b) * lift) / (TINT_LIGHT_ONE * TINT_FULL_GRAY);
        }
        else if (light < 0)
        {
            // Darken toward black. Near-white highlights (eye whites, teeth)
            // fade out of the darkening so they stay bright.
            s32 drop = lightMag;
            if (gray > TINT_FADE_START && !override->tintWhites)
                drop = drop * (31 - gray) / (31 - TINT_FADE_START);
            r -= (r * drop) / TINT_LIGHT_ONE;
            g -= (g * drop) / TINT_LIGHT_ONE;
            b -= (b * drop) / TINT_LIGHT_ONE;
        }

        gray = (r + g + b) / 3;
        weight = (gray < TINT_FULL_GRAY) ? gray : TINT_FULL_GRAY;
        if (gray > TINT_FADE_START && !override->tintWhites)
            weight = weight * (31 - gray) / (31 - TINT_FADE_START);
        amount = override->tintStrength * weight * tintScale / TINT_LIGHT_ONE;
        if (override->tintRamp)
            amount = amount * (signedStep + 4) / 7;

        r += (dirR * amount) / (FP_SCALE * TINT_FULL_GRAY);
        g += (dirG * amount) / (FP_SCALE * TINT_FULL_GRAY);
        b += (dirB * amount) / (FP_SCALE * TINT_FULL_GRAY);

        // A white can't get any brighter, so clipping would keep only the
        // channels the tint lowers (sky would turn cyan). Lower all three by
        // the overflow instead, which keeps the tint's hue.
        if (override->tintWhites)
        {
            s32 over = r;
            if (g > over) over = g;
            if (b > over) over = b;
            over -= 31;
            if (over > 0)
            {
                r -= over;
                g -= over;
                b -= over;
            }
        }

        if (r < 0) r = 0; else if (r > 31) r = 31;
        if (g < 0) g = 0; else if (g > 31) g = 31;
        if (b < 0) b = 0; else if (b > 31) b = 31;

        palette[i] = (u16)(r | (g << 5) | (b << 10));
    }
}

// Colors this far (sine-table units) outside the band fade from full to no
// shift, so a color sitting right at the band's edge doesn't jump.
#define BAND_FEATHER 8

// Returns the color's hue in sine-table units (0-255), or -1 if it's too
// close to gray for its hue to be meaningful.
static s32 GetColorHue256(s32 r, s32 g, s32 b)
{
    s32 max = r, min = r, delta, hue;

    if (g > max) max = g;
    if (b > max) max = b;
    if (g < min) min = g;
    if (b < min) min = b;
    delta = max - min;
    if (delta < COLOR_VARIATION_CHROMA_THRESHOLD)
        return -1;

    if (max == r)
        hue = (g - b) * 256 / (6 * delta);
    else if (max == g)
        hue = (2 * delta + b - r) * 256 / (6 * delta);
    else
        hue = (4 * delta + r - g) * 256 / (6 * delta);
    return hue & 0xFF;
}

// bandDarken only reaches the two extreme steps on its side (see the struct).
static s32 GetBandDarkenAmount(s8 darken, s32 signedStep)
{
    s32 amount = 0;

    if (darken > 0 && signedStep >= 2)
        amount = darken * (signedStep - 1) / 2;
    else if (darken < 0 && signedStep <= -3)
        amount = darken * (signedStep + 2) / 2;
    if (amount > TINT_LIGHT_ONE)
        amount = TINT_LIGHT_ONE;
    return amount;
}

// One hue band's effects for the current hue step.
struct BandStep
{
    s32 hue;
    s32 width;
    s32 shift;
    s32 lighten;
    s32 darken;
    s32 gray;
};

static bool8 GetBandStep(struct BandStep *band, u8 hue, u8 width, s8 shift, s8 lighten, s8 darken, s8 gray, s32 signedStep)
{
    band->hue = hue;
    band->width = width;
    band->shift = shift * (signedStep + 4) / 7;
    band->lighten = lighten * (signedStep + 4) / 7;
    band->darken = GetBandDarkenAmount(darken, signedStep);
    band->gray = gray * (signedStep + 4) / 7;
    return width != 0 && (band->shift != 0 || band->lighten != 0 || band->darken != 0 || band->gray != 0);
}

// hue is the color's hue before any band touched it.
static void ApplyBandToColor(u16 *color, s32 hue, const struct BandStep *band)
{
    s32 r, g, b, dist, angle, lighten, darken, desat, max;
    struct HueMatrix mat;

    dist = (hue - band->hue) & 0xFF;
    if (dist > 128)
        dist = 256 - dist;
    if (dist > band->width + BAND_FEATHER)
        return;

    angle = band->shift;
    lighten = band->lighten;
    darken = band->darken;
    desat = band->gray;
    if (dist > band->width)
    {
        angle = angle * (band->width + BAND_FEATHER - dist) / BAND_FEATHER;
        lighten = lighten * (band->width + BAND_FEATHER - dist) / BAND_FEATHER;
        darken = darken * (band->width + BAND_FEATHER - dist) / BAND_FEATHER;
        desat = desat * (band->width + BAND_FEATHER - dist) / BAND_FEATHER;
    }

    if (angle != 0)
    {
        ComputeHueMatrix(angle & 0xFF, &mat);
        RotateColor(color, &mat);
    }

    if (lighten > 0)
    {
        r = (*color >>  0) & 0x1F;
        g = (*color >>  5) & 0x1F;
        b = (*color >> 10) & 0x1F;
        // Scale by brightness so dark shading stays darker than the highlights.
        max = r;
        if (g > max) max = g;
        if (b > max) max = b;
        r += (31 - r) * lighten * max / (TINT_LIGHT_ONE * 31);
        g += (31 - g) * lighten * max / (TINT_LIGHT_ONE * 31);
        b += (31 - b) * lighten * max / (TINT_LIGHT_ONE * 31);
        *color = (u16)(r | (g << 5) | (b << 10));
    }
    else if (lighten < 0)
    {
        // Negative bandLighten: straight blend toward black.
        r = (*color >>  0) & 0x1F;
        g = (*color >>  5) & 0x1F;
        b = (*color >> 10) & 0x1F;
        if (lighten < -TINT_LIGHT_ONE)
            lighten = -TINT_LIGHT_ONE;
        r += (r * lighten) / TINT_LIGHT_ONE;
        g += (g * lighten) / TINT_LIGHT_ONE;
        b += (b * lighten) / TINT_LIGHT_ONE;
        *color = (u16)(r | (g << 5) | (b << 10));
    }

    if (darken != 0)
    {
        // Straight blend toward black: only band colors reach this, so near-white
        // highlights and neutral grays are never affected.
        r = (*color >>  0) & 0x1F;
        g = (*color >>  5) & 0x1F;
        b = (*color >> 10) & 0x1F;
        r -= (r * darken) / TINT_LIGHT_ONE;
        g -= (g * darken) / TINT_LIGHT_ONE;
        b -= (b * darken) / TINT_LIGHT_ONE;
        *color = (u16)(r | (g << 5) | (b << 10));
    }

    if (desat != 0)
        AdjustSaturation(color, FP_SCALE * (TINT_LIGHT_ONE - desat) / TINT_LIGHT_ONE);
}

// Bits of a band skip mask: bit i = palette color i is outside the first band's
// chroma filter, bit 16 + i = outside the second band's.
#define BAND1_SKIP(i) (1u << (i))
#define BAND2_SKIP(i) (1u << ((i) + 16))

// TRUE if the color's chroma is outside a bandChroma / band2Chroma filter.
static bool8 IsOutsideBandChroma(u16 color, s8 limit)
{
    s32 r = (color >>  0) & 0x1F;
    s32 g = (color >>  5) & 0x1F;
    s32 b = (color >> 10) & 0x1F;
    s32 max = r, min = r;

    if (limit == 0)
        return FALSE;
    if (g > max) max = g;
    if (b > max) max = b;
    if (g < min) min = g;
    if (b < min) min = b;
    if (limit > 0)
        return (max - min) > limit;
    return (max - min) < -limit;
}

// Which palette colors each band's chroma filter leaves out, from the colors
// as they are before either band moves them.
static u32 GetBandSkipMask(const u16 *palette, const struct ColorVariationOverride *override)
{
    u32 skip = 0;
    u32 i;

    if (override->bandChroma == 0 && override->band2Chroma == 0)
        return 0;
    for (i = 1; i < 16; i++)
    {
        if (IsOutsideBandChroma(palette[i], override->bandChroma))
            skip |= BAND1_SKIP(i);
        if (IsOutsideBandChroma(palette[i], override->band2Chroma))
            skip |= BAND2_SKIP(i);
    }
    return skip;
}

// How much of a band (0 = none .. BAND_FEATHER = all of it) a color of this
// hue gets, like ApplyBandToColor's fade.
static s32 GetBandShare(s32 hue, u8 bandHue, u8 width)
{
    s32 dist;

    if (width == 0)
        return 0;
    dist = (hue - bandHue) & 0xFF;
    if (dist > 128)
        dist = 256 - dist;
    if (dist <= width)
        return BAND_FEATHER;
    if (dist > width + BAND_FEATHER)
        return 0;
    return width + BAND_FEATHER - dist;
}

// Applies both hue bands. Returns the skip mask of the chroma filters, so the
// bandMaxUp caps can leave the same colors alone. If hold isn't NULL it gets,
// per color, how much of the general hue rotation bandHold / band2Hold take
// away (0 = none .. BAND_FEATHER = all).
static u32 ApplyHueBand(u16 *palette, const struct ColorVariationOverride *override, s32 signedStep, u8 *hold)
{
    struct BandStep band1, band2;
    bool8 hasBand1 = GetBandStep(&band1, override->bandHue, override->bandWidth, override->bandShift,
                                 override->bandLighten, override->bandDarken, override->bandGray, signedStep);
    bool8 hasBand2 = GetBandStep(&band2, override->band2Hue, override->band2Width, override->band2Shift,
                                 override->band2Lighten, override->band2Darken, override->band2Gray, signedStep);
    bool8 hasHold = (hold != NULL && ((override->bandHold && override->bandWidth != 0)
                                   || (override->band2Hold && override->band2Width != 0)));
    u32 skip = GetBandSkipMask(palette, override);
    u32 i;

    if (hold != NULL)
    {
        for (i = 0; i < 16; i++)
            hold[i] = 0;
    }
    if (!hasBand1 && !hasBand2 && !hasHold)
        return skip;

    for (i = 1; i < 16; i++)
    {
        s32 hue = GetColorHue256((palette[i] >> 0) & 0x1F, (palette[i] >> 5) & 0x1F, (palette[i] >> 10) & 0x1F);

        if (hue < 0)
            continue;
        if (hasHold)
        {
            s32 share1 = 0, share2 = 0;

            if (override->bandHold && !(skip & BAND1_SKIP(i)))
                share1 = GetBandShare(hue, override->bandHue, override->bandWidth);
            if (override->band2Hold && !(skip & BAND2_SKIP(i)))
                share2 = GetBandShare(hue, override->band2Hue, override->band2Width);
            hold[i] = (share1 > share2) ? share1 : share2;
        }
        if (hasBand1 && !(skip & BAND1_SKIP(i)))
            ApplyBandToColor(&palette[i], hue, &band1);
        if (hasBand2 && !(skip & BAND2_SKIP(i)))
            ApplyBandToColor(&palette[i], hue, &band2);
    }
    return skip;
}

// Returns how far (in 1/64ths) this hue step should fade for a grayFade /
// whiteFade value, or 0 if the step isn't on that fade's side.
static s32 GetSideFadeAmount(s8 fade, s32 signedStep)
{
    s32 amount = 0;

    if (fade > 0 && signedStep > 0)
        amount = fade * signedStep / 3;
    else if (fade < 0 && signedStep < 0)
        amount = fade * signedStep / 4;
    if (amount > TINT_LIGHT_ONE)
        amount = TINT_LIGHT_ONE;
    return amount;
}

struct SideFades
{
    s32 gray;
    s32 white;
    s32 brown;
};

static bool8 GetSideFades(const struct ColorVariationOverride *override, s32 signedStep, struct SideFades *fades)
{
    fades->gray = fades->white = fades->brown = 0;
    if (override != NULL)
    {
        fades->gray = GetSideFadeAmount(override->grayFade, signedStep);
        fades->white = GetSideFadeAmount(override->whiteFade, signedStep);
        fades->brown = GetSideFadeAmount(override->brownFade, signedStep);
    }
    return (fades->gray != 0 || fades->white != 0 || fades->brown != 0);
}

// Sepia tone multipliers (1/64ths of the color's gray level), used as the
// brownFade target. Keeps the original shading, just recolored brown.
#define SEPIA_R 70
#define SEPIA_G 48
#define SEPIA_B 28

static void ApplySideFades(u16 *color, const struct SideFades *fades)
{
    if (fades->gray != 0)
        AdjustSaturation(color, FP_SCALE * (TINT_LIGHT_ONE - fades->gray) / TINT_LIGHT_ONE);

    if (fades->brown != 0)
    {
        s32 r = (*color >>  0) & 0x1F;
        s32 g = (*color >>  5) & 0x1F;
        s32 b = (*color >> 10) & 0x1F;
        s32 gray = (r + g + b) / 3;
        s32 amount = fades->brown;
        s32 sr = gray * SEPIA_R / TINT_LIGHT_ONE;
        s32 sg = gray * SEPIA_G / TINT_LIGHT_ONE;
        s32 sb = gray * SEPIA_B / TINT_LIGHT_ONE;

        if (sr > 31) sr = 31;
        // Near-white highlights (eye shine) stay white.
        if (gray > TINT_FADE_START)
            amount = amount * (31 - gray) / (31 - TINT_FADE_START);
        r += ((sr - r) * amount) / TINT_LIGHT_ONE;
        g += ((sg - g) * amount) / TINT_LIGHT_ONE;
        b += ((sb - b) * amount) / TINT_LIGHT_ONE;
        *color = (u16)(r | (g << 5) | (b << 10));
    }

    if (fades->white != 0)
    {
        s32 r = (*color >>  0) & 0x1F;
        s32 g = (*color >>  5) & 0x1F;
        s32 b = (*color >> 10) & 0x1F;
        s32 gray = (r + g + b) / 3;
        // Scale by brightness so dark outlines stay dark.
        s32 lift = fades->white * ((gray < TINT_FULL_GRAY) ? gray : TINT_FULL_GRAY);

        r += ((31 - r) * lift) / (TINT_LIGHT_ONE * TINT_FULL_GRAY);
        g += ((31 - g) * lift) / (TINT_LIGHT_ONE * TINT_FULL_GRAY);
        b += ((31 - b) * lift) / (TINT_LIGHT_ONE * TINT_FULL_GRAY);
        *color = (u16)(r | (g << 5) | (b << 10));
    }
}

// Rotates one color by angleIndex (0-255, where values above 128 are negative).
// If the species sets bandMaxUp (or band2MaxUp), positive rotation beyond that
// limit is trimmed for colors inside that hue band (and its chroma filter). A
// color in reach of both capped bands follows the first. Everything else uses
// the precomputed matrix.
static bool8 IsInBandReach(s32 hue, u8 bandHue, u8 width, s8 shift, s8 maxUp)
{
    s32 dist;

    if (width == 0 || maxUp == 0)
        return FALSE;
    dist = (hue - bandHue) & 0xFF;
    if (dist > 128)
        dist = 256 - dist;
    // The band may already have shifted this color, so allow for that.
    return dist <= width + BAND_FEATHER + (shift < 0 ? -shift : shift);
}

static void RotateColorBandCapped(u16 *color, s32 angleIndex, const struct HueMatrix *mat, const struct ColorVariationOverride *override, u32 skip, u32 index)
{
    s32 signedAngle = angleIndex & 0xFF;
    s32 hue, cap;
    struct HueMatrix capped;

    if (signedAngle > 128)
        signedAngle -= 256;

    if (override == NULL
     || ((override->bandMaxUp == 0 || override->bandWidth == 0 || signedAngle <= override->bandMaxUp)
      && (override->band2MaxUp == 0 || override->band2Width == 0 || signedAngle <= override->band2MaxUp)))
    {
        RotateColor(color, mat);
        return;
    }

    hue = GetColorHue256((*color >> 0) & 0x1F, (*color >> 5) & 0x1F, (*color >> 10) & 0x1F);
    if (hue >= 0)
    {
        if (!(skip & BAND1_SKIP(index))
         && IsInBandReach(hue, override->bandHue, override->bandWidth, override->bandShift, override->bandMaxUp))
            cap = override->bandMaxUp;
        else if (!(skip & BAND2_SKIP(index))
              && IsInBandReach(hue, override->band2Hue, override->band2Width, override->band2Shift, override->band2MaxUp))
            cap = override->band2MaxUp;
        else
            cap = 0;
        if (cap != 0 && signedAngle > cap)
        {
            ComputeHueMatrix(cap & 0xFF, &capped);
            RotateColor(color, &capped);
            return;
        }
    }
    RotateColor(color, mat);
}

// RotateColorBandCapped minus the share of the rotation bandHold / band2Hold
// take away (hold: 0 = none .. BAND_FEATHER = all of it).
static void RotateColorHeld(u16 *color, s32 angleIndex, const struct HueMatrix *mat, const struct ColorVariationOverride *override, u32 skip, u32 index, s32 hold)
{
    struct HueMatrix scaled;
    s32 signedAngle;

    if (hold <= 0)
    {
        RotateColorBandCapped(color, angleIndex, mat, override, skip, index);
        return;
    }
    if (hold >= BAND_FEATHER)
        return;
    signedAngle = angleIndex & 0xFF;
    if (signedAngle > 128)
        signedAngle -= 256;
    signedAngle = signedAngle * (BAND_FEATHER - hold) / BAND_FEATHER;
    ComputeHueMatrix(signedAngle & 0xFF, &scaled);
    RotateColorBandCapped(color, signedAngle & 0xFF, &scaled, override, skip, index);
}

void ApplyIndividualColorVariation(u16 *palette, u32 personality, u16 species)
{
    u8 shift = (personality >> 16) & 0x3F;
    const struct ColorVariationOverride *override = FindColorVariationOverride(species);
    s32 maxAngle = (override != NULL) ? override->maxAngle : COLOR_VARIATION_MAX_ANGLE;
    s32 angleBias = (override != NULL) ? override->angleBias : 0;
    s32 satCap = (override != NULL) ? override->satCap : FP_SCALE * 2; // effectively no cap by default
    s32 satMul = (override != NULL) ? override->satMul : FP_SCALE;     // 1.0 = no extra desaturation
    s32 angleIndex;

    {
    u8 hueStep = shift & 0x7;        // 3 bits: 0-7
    u8 mode = (shift >> 3) & 0x7;    // 3 bits: 0-7
    s32 signedStep, angleMag;
    struct SideFades fades;
    bool8 hasFade;
    struct HueMatrix matPos, matNeg;
    bool8 hasHue, hasSplit, hasMuted, hasVivid, hasSplitSat, warmIsVivid;
    // A negative bandMaxUp / band2MaxUp pushes band colors down on every step,
    // including steps that otherwise wouldn't rotate at all (step 0 with no angleBias).
    bool8 forcedCap = (override != NULL && ((override->bandWidth != 0 && override->bandMaxUp < 0)
                                         || (override->band2Width != 0 && override->band2MaxUp < 0)));
    s32 vividFactor = COLOR_VARIATION_SAT_VIVID;
    s32 coolIndex;
    u32 bandSkip = 0;
    u8 bandHold[16];
    bool8 hasBandHold = FALSE;
    u32 i;

    // Map 3-bit hue step (0-7) to signed angle
    // step 0..3 = negative hue, step 4 = zero, step 5..7 = positive hue
    signedStep = hueStep - 4;
    angleMag = (signedStep < 0 ? -signedStep : signedStep) * maxAngle / 4;

    // Steps on a fade side fade toward gray/white/brown instead of rotating hue.
    hasFade = GetSideFades(override, signedStep, &fades);
    if (hasFade)
        angleMag = 0;

    // Tint grays first so the hue rotation / saturation modes below vary
    // the injected color just like any naturally chromatic one.
    if (override != NULL && (override->tintStrength != 0 || override->tintDarken != 0 || override->tintLighten != 0))
        ApplyNeutralTint(palette, override, signedStep);

    if (override != NULL && (override->bandWidth != 0 || override->band2Width != 0))
    {
        bandSkip = ApplyHueBand(palette, override, signedStep, bandHold);
        hasBandHold = TRUE;
    }

    if (angleMag == 0 && mode == 0 && angleBias == 0 && !hasFade && satMul == FP_SCALE && !forcedCap)
        return; // No change at all

    // Compute angle index for sine table
    if (angleMag == 0)
        angleIndex = 0;
    else if (signedStep >= 0)
        angleIndex = angleMag;
    else
        angleIndex = 256 - angleMag;

    // Apply per-species angle bias (e.g. shift entire range away from a
    // problematic hue like the Pikachu line's orange shiny).
    if (angleBias != 0)
    {
        s32 biased = angleIndex + angleBias;
        while (biased < 0)
            biased += 256;
        angleIndex = biased & 0xFF;
    }

    // Cap the vivid saturation factor (overrides may forbid making the mon
    // brighter than the base palette by setting satCap = FP_SCALE).
    if (vividFactor > satCap)
        vividFactor = satCap;

    // Decode mode flags
    hasSplit = (mode == 3    || mode == 6 || mode == 7);
    hasMuted = (mode == 1    || mode == 6);
    hasVivid = (mode == 2    || mode == 7);
    hasSplitSat = (mode == 4 || mode == 5);
    warmIsVivid = (mode == 4); // mode 4: warm vivid/cool muted; mode 5: opposite

    hasHue = (angleIndex != 0 || forcedCap);

    // Cool colors in split modes rotate the other way: normally the whole
    // angle flips, with holdBias only the per-step part does.
    if (override != NULL && override->holdBias)
        coolIndex = (angleBias - (angleMag == 0 ? 0 : (signedStep >= 0 ? angleMag : -angleMag))) & 0xFF;
    else
        coolIndex = (256 - angleIndex) & 0xFF;
    if (hasSplit && coolIndex != 0)
        hasHue = TRUE;

    // Precompute hue matrices
    if (hasHue)
    {
        ComputeHueMatrix(angleIndex, &matPos);
        if (hasSplit)
            ComputeHueMatrix(coolIndex, &matNeg);
    }

    for (i = 1; i < 16; i++)
    {
        s32 origR = (palette[i] >>  0) & 0x1F;
        s32 origG = (palette[i] >>  5) & 0x1F;
        s32 origB = (palette[i] >> 10) & 0x1F;
        bool8 isSaturated = IsColorSaturated(origR, origG, origB);
        u8 bucket = GetHueBucket(origR, origG, origB);
        s32 hold = hasBandHold ? bandHold[i] : 0;

        // Apply hue rotation
        if (hasHue)
        {
            if (hasSplit && isSaturated)
            {
                // Split: warm rotates positive, cool rotates negative
                if (bucket == 0)
                    RotateColorHeld(&palette[i], angleIndex, &matPos, override, bandSkip, i, hold);
                else
                    RotateColorHeld(&palette[i], coolIndex, &matNeg, override, bandSkip, i, hold);
            }
            else if (!hasSplit)
            {
                RotateColorHeld(&palette[i], angleIndex, &matPos, override, bandSkip, i, hold);
            }
            // else: split mode but neutral color — leave unchanged
        }

        // Apply saturation adjustment
        if (hasMuted)
            AdjustSaturation(&palette[i], COLOR_VARIATION_SAT_MUTED);
        else if (hasVivid)
            AdjustSaturation(&palette[i], vividFactor);
        else if (hasSplitSat && isSaturated)
        {
            if ((bucket == 0) == warmIsVivid)
                AdjustSaturation(&palette[i], vividFactor);
            else
                AdjustSaturation(&palette[i], COLOR_VARIATION_SAT_MUTED);
        }

        // Always-applied per-species saturation pull (e.g. brown bias).
        if (satMul != FP_SCALE)
            AdjustSaturation(&palette[i], satMul);

        // Applied last so vivid modes can't re-saturate the faded colors.
        if (hasFade)
            ApplySideFades(&palette[i], &fades);
    }
    }
}

bool8 HasColorVariationOverride(u16 species)
{
    return FindColorVariationOverride(species) != NULL;
}

// Mirrors the hue math in ApplyIndividualColorVariation and resolves it to the
// single direction an icon should shift. Split modes rotate warm and cool colors
// in opposite directions, so the direction is taken from whichever temperature
// covers more of the icon's visible pixels.
s8 GetColorVariationIconHue(u32 personality, u16 species, const u16 *palette, const u8 *iconPixels, u32 iconPixelBytes)
{
    u8 shift = (personality >> 16) & 0x3F;
    u8 mode = (shift >> 3) & 0x7;
    const struct ColorVariationOverride *override = FindColorVariationOverride(species);
    s32 maxAngle = (override != NULL) ? override->maxAngle : COLOR_VARIATION_MAX_ANGLE;
    s32 angleBias = (override != NULL) ? override->angleBias : 0;
    s32 signedStep = (shift & 0x7) - 4;
    s32 angleMag = (signedStep < 0 ? -signedStep : signedStep) * maxAngle / 4;
    s32 angle = ((signedStep < 0) ? -angleMag : angleMag) + angleBias;
    struct SideFades fades;

    // Fade steps don't rotate hue at all, so their direction is just the
    // step's sign (ApplyColorVariationIconHue fades instead of rotating).
    if (GetSideFades(override, signedStep, &fades))
        return (signedStep > 0) ? 1 : -1;

    if (mode == 3 || mode == 6 || mode == 7)
    {
        s8 temperature[16];
        s32 balance = 0;
        u32 i;

        for (i = 0; i < 16; i++)
        {
            s32 r = (palette[i] >>  0) & 0x1F;
            s32 g = (palette[i] >>  5) & 0x1F;
            s32 b = (palette[i] >> 10) & 0x1F;

            if (i == 0 || !IsColorSaturated(r, g, b))
                temperature[i] = 0;
            else
                temperature[i] = (GetHueBucket(r, g, b) == 0) ? 1 : -1;
        }

        if (iconPixels != NULL)
        {
            for (i = 0; i < iconPixelBytes; i++)
            {
                balance += temperature[iconPixels[i] & 0xF];
                balance += temperature[iconPixels[i] >> 4];
            }
        }
        else
        {
            for (i = 1; i < 16; i++)
                balance += temperature[i];
        }

        if (balance < 0)
            angle = (override != NULL && override->holdBias) ? angleBias - (angle - angleBias) : -angle;
    }

    if (angle >= COLOR_VARIATION_ICON_NEUTRAL_ANGLE)
        return 1;
    if (angle <= -COLOR_VARIATION_ICON_NEUTRAL_ANGLE)
        return -1;
    return 0;
}

void ApplyColorVariationIconHue(u16 *palette, s8 hue, u16 species)
{
    const struct ColorVariationOverride *override = FindColorVariationOverride(species);
    struct HueMatrix mat, held;
    struct SideFades fades;
    bool8 hasFade = GetSideFades(override, hue * 3, &fades);
    u8 bandHold[16];
    bool8 hasBandHold = FALSE;
    u32 i;

    // Icons only know the hue direction; ±3 steps approximates the middle of
    // the main sprite's positive (+2..+3) and negative (-2..-4) step ranges.
    if (override != NULL && (override->tintStrength != 0 || override->tintDarken != 0 || override->tintLighten != 0))
        ApplyNeutralTint(palette, override, hue * 3);

    if (override != NULL && (override->bandWidth != 0 || override->band2Width != 0))
    {
        ApplyHueBand(palette, override, hue * 3, bandHold);
        hasBandHold = TRUE;
    }

    if (hasFade)
    {
        for (i = 1; i < 16; i++)
            ApplySideFades(&palette[i], &fades);
    }
    else if (hue != 0)
    {
        s32 angle = (hue > 0) ? COLOR_VARIATION_ICON_ANGLE : -COLOR_VARIATION_ICON_ANGLE;

        ComputeHueMatrix(angle & 0xFF, &mat);
        for (i = 1; i < 16; i++)
        {
            s32 hold = hasBandHold ? bandHold[i] : 0;

            if (hold >= BAND_FEATHER)
                continue;
            if (hold > 0)
            {
                ComputeHueMatrix((angle * (BAND_FEATHER - hold) / BAND_FEATHER) & 0xFF, &held);
                RotateColor(&palette[i], &held);
            }
            else
            {
                RotateColor(&palette[i], &mat);
            }
        }
    }

    if (override != NULL && override->satMul != FP_SCALE)
    {
        for (i = 1; i < 16; i++)
            AdjustSaturation(&palette[i], override->satMul);
    }
}
