#include "global.h"
#include "pokemon_color_variation.h"
#include "trig.h"

// Fixed-point scale (10 bits = multiply by 1024)
#define FP_SHIFT 10
#define FP_SCALE (1 << FP_SHIFT)

// sqrt(1/3) in fixed point: round(0.57735 * 1024) = 591
#define SQRT_ONE_THIRD_FP 591

void ApplyIndividualColorVariation(u16 *palette, u32 personality)
{
    u8 shift = (personality >> 16) & 0x3F;
    s32 angleIndex;
    s32 cosA, sinA;
    s32 oneMinusCosDiv3, sqrtSinTerm;
    s32 mA, mB, mC;
    u32 i;

    // Values 0 and 0x20 both mean no shift
    if (shift == 0 || shift == 0x20)
        return;

    // Convert 6-bit shift to sine-table angle magnitude
    if (shift <= 0x1F)
        angleIndex = (shift * COLOR_VARIATION_MAX_ANGLE) / 0x1F;
    else
        angleIndex = ((shift - 0x20) * COLOR_VARIATION_MAX_ANGLE) / 0x1F;

    if (angleIndex == 0)
        return;

    // Negative direction: wrap to the other side of the sine table
    if (shift > 0x1F)
        angleIndex = 256 - angleIndex;

    // Compute hue rotation matrix coefficients in fixed point.
    // The rotation matrix around the (1,1,1) axis is a circulant:
    //   | a  c  b |
    //   | b  a  c |
    //   | c  b  a |
    // where:
    //   a = cos + (1-cos)/3
    //   b = (1-cos)/3 + sqrt(1/3)*sin
    //   c = (1-cos)/3 - sqrt(1/3)*sin
    cosA = Cos(angleIndex, FP_SCALE);
    sinA = Sin(angleIndex, FP_SCALE);

    oneMinusCosDiv3 = (FP_SCALE - cosA) / 3;
    sqrtSinTerm = (SQRT_ONE_THIRD_FP * sinA) >> FP_SHIFT;

    mA = cosA + oneMinusCosDiv3;
    mB = oneMinusCosDiv3 + sqrtSinTerm;
    mC = oneMinusCosDiv3 - sqrtSinTerm;

    // Apply to each color (skip index 0 = transparent)
    for (i = 1; i < 16; i++)
    {
        u16 color = palette[i];
        s32 r = (color >>  0) & 0x1F;
        s32 g = (color >>  5) & 0x1F;
        s32 b = (color >> 10) & 0x1F;

        s32 newR = (mA * r + mC * g + mB * b) >> FP_SHIFT;
        s32 newG = (mB * r + mA * g + mC * b) >> FP_SHIFT;
        s32 newB = (mC * r + mB * g + mA * b) >> FP_SHIFT;

        if (newR < 0) newR = 0; else if (newR > 31) newR = 31;
        if (newG < 0) newG = 0; else if (newG > 31) newG = 31;
        if (newB < 0) newB = 0; else if (newB > 31) newB = 31;

        palette[i] = (u16)(newR | (newG << 5) | (newB << 10));
    }
}
