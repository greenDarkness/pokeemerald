#ifndef GUARD_POKEMON_COLOR_VARIATION_H
#define GUARD_POKEMON_COLOR_VARIATION_H

// Individual color variation: applies a slight hue shift to a Pokémon's
// palette based on its personality value, so each individual has a
// unique color tint. Inspired by the Pokémon Stadium nickname-based
// color variation and Polished Crystal's IV-based coloring.

// The hue shift range in sine-table units (256 = full circle).
// 14 ≈ 20 degrees, giving a ±20° range.
#define COLOR_VARIATION_MAX_ANGLE 14

// Applies a personality-based hue shift to a 16-color palette in place.
// Uses bits 16-21 of the personality value (6 bits → 64 steps, ±20°).
// Color index 0 (transparent) is not modified.
void ApplyIndividualColorVariation(u16 *palette, u32 personality);

#endif // GUARD_POKEMON_COLOR_VARIATION_H
