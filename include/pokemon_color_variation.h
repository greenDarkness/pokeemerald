#ifndef GUARD_POKEMON_COLOR_VARIATION_H
#define GUARD_POKEMON_COLOR_VARIATION_H

// Individual color variation: applies a hue shift and optional saturation
// modifier to a Pokémon's palette based on its personality value, so each
// individual has a unique color tint. Inspired by Pokémon Stadium's
// nickname-based color variation and Polished Crystal's IV-based coloring.
//
// Uses bits 16-21 of the personality (6 bits → 64 variants):
//   bits 0-2 (3 bits): hue step (8 steps, ±20°)
//   bits 3-5 (3 bits): mode (8 modes)
//
// Modes:
//   0: Standard hue rotation
//   1: Hue + muted (90% saturation)
//   2: Hue + vivid (120% saturation)
//   3: Split hue (warm colors rotate positive, cool negative)
//   4: Hue + warm vivid / cool muted
//   5: Hue + warm muted / cool vivid
//   6: Split hue + muted
//   7: Split hue + vivid

// The hue shift range in sine-table units (256 = full circle).
// 14 ≈ 20 degrees, giving a ±20° range.
#define COLOR_VARIATION_MAX_ANGLE 14

// Saturation multipliers (fixed-point, 1024 = 100%)
#define COLOR_VARIATION_SAT_MUTED  922  // 90%
#define COLOR_VARIATION_SAT_VIVID 1229  // 120%

// Minimum channel spread to consider a color "saturated" (chromatic)
#define COLOR_VARIATION_CHROMA_THRESHOLD 4

// Applies a personality-based color variation to a 16-color palette in place.
// Color index 0 (transparent) is not modified.
void ApplyIndividualColorVariation(u16 *palette, u32 personality);

#endif // GUARD_POKEMON_COLOR_VARIATION_H
