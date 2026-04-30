#include "global.h"
#include "pokemon_color_variation.h"
#include "trig.h"
#include "constants/species.h"

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
    s16 satCap;       // fixed-point. Caps maximum saturation boost (1024 = 100%, < 1024 disables vivid modes).
    s16 satMul;       // fixed-point. Always-applied saturation multiplier (1024 = none). Pulls everything toward gray/brown.
};

static const struct ColorVariationOverride sColorVariationOverrides[] =
{
    // Pikachu line: shiny is bright orange. Bias hue toward green-yellow,
    // hard-cap saturation, AND apply a uniform 70% saturation pull so every
    // variation reads as a tan/brown shade — but the 8 modes still differ
    // among themselves (some more muted, some hue-rotated, etc).
    { SPECIES_PICHU,   6, -6, 900, 750 },
    { SPECIES_PIKACHU, 6, -6, 900, 750 },
    { SPECIES_RAICHU,  6, -6, 900, 750 },
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

// Fixed-point scale (10 bits = multiply by 1024)
#define FP_SHIFT 10
#define FP_SCALE (1 << FP_SHIFT)

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
    struct HueMatrix matPos, matNeg;
    bool8 hasHue, hasSplit, hasMuted, hasVivid, hasSplitSat, warmIsVivid;
    s32 vividFactor = COLOR_VARIATION_SAT_VIVID;
    u32 i;

    // Map 3-bit hue step (0-7) to signed angle
    // step 0..3 = negative hue, step 4 = zero, step 5..7 = positive hue
    signedStep = hueStep - 4;
    angleMag = (signedStep < 0 ? -signedStep : signedStep) * maxAngle / 4;

    if (angleMag == 0 && mode == 0 && angleBias == 0)
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

    hasHue = (angleIndex != 0);

    // Precompute hue matrices
    if (hasHue)
    {
        ComputeHueMatrix(angleIndex, &matPos);
        if (hasSplit)
        {
            s32 negAngle = 256 - angleIndex;
            ComputeHueMatrix(negAngle, &matNeg);
        }
    }

    for (i = 1; i < 16; i++)
    {
        s32 origR = (palette[i] >>  0) & 0x1F;
        s32 origG = (palette[i] >>  5) & 0x1F;
        s32 origB = (palette[i] >> 10) & 0x1F;
        bool8 isSaturated = IsColorSaturated(origR, origG, origB);
        u8 bucket = GetHueBucket(origR, origG, origB);

        // Apply hue rotation
        if (hasHue)
        {
            if (hasSplit && isSaturated)
            {
                // Split: warm rotates positive, cool rotates negative
                if (bucket == 0)
                    RotateColor(&palette[i], &matPos);
                else
                    RotateColor(&palette[i], &matNeg);
            }
            else if (!hasSplit)
            {
                RotateColor(&palette[i], &matPos);
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
    }
    }
}
