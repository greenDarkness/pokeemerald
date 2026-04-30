# Individual Color Variation — Tuning Cheat Sheet

Per-species overrides live in **`src/pokemon_color_variation.c`** in the
`sColorVariationOverrides[]` array, mirrored in **`tech/generate_color_variations.py`**
in the `OVERRIDES` dict. Both must be kept in sync.

To preview changes without rebuilding the ROM:

```
python tech/generate_color_variations.py <species_name> [more_names...]
# or just double-click  tech/Color Variations.bat
```

Output PNGs land in `tech/color_variation_previews/`. The grid shows the
64 possible variations (bits 16-21 of the personality value), with the
**original sprite framed in green** in the center and the **shiny framed in red**
in the bottom-right corner so you can compare drift vs. shiny clash directly.

---

## The override struct

```c
struct ColorVariationOverride {
    u16 species;
    s8  maxAngle;     // sine-table units (0–32 = 0–45°). Default 14.
    s8  angleBias;    // sine-table units. Added to every variation's hue.
    s16 satCap;       // fixed-point /1024. Caps the "vivid" boost.
    s16 satMul;       // fixed-point /1024. Always-applied saturation pull.
};
```

Defaults if no override:  `maxAngle = 14, angleBias = 0, satCap = 2048, satMul = 1024`.

---

## What each field does

### `maxAngle` — how far the hue can drift
| Value | Approx. swing | Use when… |
|------:|:-------------:|:----------|
| `14` (default) | ±20° | Most Pokémon. Wide colorful spread. |
| `10` | ±14° | Sprites where ±20° pushes into a different "color identity." |
| `6`  | ±8.5° | Should clearly read as the same Pokémon. *(Pikachu line)* |
| `4`  | ±5.6° | Very tight — barely-perceptible hue shift. |
| `0`  | none  | No hue rotation, only mode-based saturation effects. |

> **Sine-table units:** the engine uses a 256-step sine table where 64 units = 90°,
> so 1 unit ≈ 1.4°. `maxAngle / 4` becomes the per-step magnitude.

### `angleBias` — push the whole range away from a "danger color"
Added to the final angle of every variation, in the same sine-table units.

| Value | Effect on hue | Example |
|------:|:--------------|:--------|
| `0`   | symmetric around the base color (default) | Most species. |
| `−6` to `−10` | bias **away from red/orange** (toward green/yellow) | Pikachu line — shiny is bright orange. |
| `+6` to `+10` | bias **toward red/orange** (away from green) | A green Pokémon whose shiny is yellow. |

Rule of thumb: pick `|angleBias| ≈ maxAngle` if you want all variations on
**one side** of the original. Pick smaller if you want to merely lean.

### `satCap` — kill or limit "vivid" mode
The variation algorithm has 8 modes; some boost saturation up to ~120%.
`satCap` is the maximum fixed-point multiplier the boost can reach.

| Value | Meaning |
|------:|:--------|
| `2048` (default ≈ 200%) | full boost allowed |
| `1024` | no boost — vivid modes act as no-op |
| `900`  | actively *desaturate* even the "vivid" variants — guarantees nothing reads as more saturated than the original. **Use this whenever the shiny is more saturated than the base sprite.** |
| `768`  | aggressive cap, very flat. |

### `satMul` — uniform "always-on" saturation pull
Applied to **every** pixel of **every** variation, after the mode logic.
This is the main lever for shifting the entire palette toward gray/brown
without flattening the per-mode variety.

| Value  | Effect |
|-------:|:-------|
| `1024` | no change (default) |
| `850`  | gentle desaturation — colors look slightly washed |
| `750`  | clearly muted; bright yellows become tan/khaki *(Pikachu line)* |
| `600`  | strong brown/gray pull |
| `400`  | nearly grayscale |
| `0`    | full grayscale |

Because `satMul` is applied **after** the mode-based saturation work, the
8 modes still produce distinguishable cells — they're just all shifted
toward gray together.

---

## Recipe book

### "Shiny is brighter / more saturated than the base"  *(e.g. Pikachu)*
```c
{ SPECIES_FOO, 6, -6, 900, 750 }
```
- Tight hue range (`6`) so it stays recognizable
- Bias away from the shiny's hue
- Cap vivid modes (`900`) so nothing competes with the shiny's saturation
- Desaturate (`750`) so the whole spread reads as a different color family

### "Shiny is a totally different color"  *(common case)*
Usually no override needed — defaults give a nice spread that won't reach the shiny.

### "I just want subtler variations"
```c
{ SPECIES_FOO, 8, 0, 1024, 1024 }
```
Half the hue range, no bias, no vivid boost, no desaturation.

### "I want a sepia/old-photo look"
```c
{ SPECIES_FOO, 4, 0, 700, 500 }
```
Tiny hue jitter + heavy desaturation across the board.

### "I want the variation away from a specific direction only"
```c
{ SPECIES_FOO, 10, +8, 2048, 1024 }   // push warm
{ SPECIES_FOO, 10, -8, 2048, 1024 }   // push cool
```

---

## Workflow

1. Edit `sColorVariationOverrides[]` in `src/pokemon_color_variation.c`.
2. Mirror the same numbers in `OVERRIDES` in `tech/generate_color_variations.py`
   (divide `satCap` and `satMul` by `1024.0`).
3. Run the preview: `python tech/generate_color_variations.py <name>`
4. Open `tech/color_variation_previews/<name>_front_variations.png`.
   - Center cell (green border) is the original.
   - Bottom-right cell (red border) is the shiny.
   - Confirm: variations look like the species, none collide with the shiny.
5. Tweak and repeat. Only rebuild the ROM (`bash build.sh`) when you're happy.

---

## Quick reference — sine-table units → degrees

| units | degrees |
|------:|--------:|
| 1     | 1.4°    |
| 4     | 5.6°    |
| 6     | 8.4°    |
| 8     | 11.25°  |
| 10    | 14°     |
| 14    | 19.7°   |
| 16    | 22.5°   |
| 32    | 45°     |
| 64    | 90°     |
