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
//
// The `species` argument allows per-species overrides (e.g. constraining the
// hue range for Pokémon whose default variations would land too close to the
// shiny palette, or tinting mostly-gray Pokémon whose palettes hue rotation
// can't affect). Pass SPECIES_NONE to disable overrides. `isShiny` says which
// palette is being varied, since an override can apply to only the regular or
// only the shiny palette (see appliesTo in pokemon_color_variation.c).
// The species' base entry (appliesTo 3 / 4), if any, recolors the palette first.
void ApplyIndividualColorVariation(u16 *palette, u32 personality, u16 species, bool8 isShiny);

// Applies only the hue bands of the species' base entry (appliesTo 3 = regular,
// 4 = shiny): the recolor of the original palette that every variation starts
// from. Use it where the unvaried sprite is shown. Does nothing without one.
void ApplyColorVariationBase(u16 *palette, u16 species, bool8 isShiny);

// Forms with their own palette are tuned separately: each has its own override
// key, and the form's palette is varied with that key passed as `species`.
// Form 0 is the species itself, so SPECIES_CASTFORM is the normal form's entry.
// A form with no entry gets the default variation (it doesn't fall back to the
// normal form's entry). Species values must stay below 1 << the shift.
#define COLOR_VARIATION_FORM_SHIFT 11
#define COLOR_VARIATION_FORM_KEY(species, form) ((species) | ((form) << COLOR_VARIATION_FORM_SHIFT))
#define SPECIES_CASTFORM_SUNNY COLOR_VARIATION_FORM_KEY(SPECIES_CASTFORM, CASTFORM_FIRE)
#define SPECIES_CASTFORM_RAINY COLOR_VARIATION_FORM_KEY(SPECIES_CASTFORM, CASTFORM_WATER)
#define SPECIES_CASTFORM_SNOWY COLOR_VARIATION_FORM_KEY(SPECIES_CASTFORM, CASTFORM_ICE)

// Mini icons (party menu / PC) are varied from the icon palette (remapped to
// the shiny colors for shinies) with the same function as the sprite. The
// party menu gives every icon its own exactly varied palette. The PC can't
// spare an OBJ palette slot per icon, so each species' icons share
// COLOR_VARIATION_ICON_BUCKETS palettes, varied at representative hue steps
// (mode 0), and every icon uses the bucket closest to its exact variation.
// Species without an override or base entry share one set of buckets per
// icon palette, whose middle bucket is the unchanged icon palette.
#define COLOR_VARIATION_ICON_BUCKETS        3
#define COLOR_VARIATION_ICON_NEUTRAL_BUCKET 1

bool8 HasColorVariationOverride(u16 species, bool8 isShiny);
bool8 HasColorVariationBase(u16 species, bool8 isShiny);

// Varies `palette` as bucket `bucket` (0..COLOR_VARIATION_ICON_BUCKETS-1).
void ApplyColorVariationIconBucket(u16 *palette, u8 bucket, u16 species, bool8 isShiny);

// Returns the bucket whose palette is closest to `exact` on the colors the
// icon uses. `buckets` holds the COLOR_VARIATION_ICON_BUCKETS palettes back to
// back and `colorCounts` how many icon pixels use each palette index.
u8 GetClosestColorVariationIconBucket(const u16 *exact, const u16 *buckets, const u16 *colorCounts);

#endif // GUARD_POKEMON_COLOR_VARIATION_H
