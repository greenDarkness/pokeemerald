# Individual Color Variation — Tuning Cheat Sheet

Per-species overrides live in **`src/pokemon_color_variation.c`** in the
`sColorVariationOverrides[]` array. The preview tool reads that array (and the
tuning `#define`s) directly every time it runs, so there's nothing to sync —
save the C file and re-run the preview.

To preview changes without rebuilding the ROM:

```
python tech/generate_color_variations.py <species_name> [more_names...]
python tech/generate_color_variations.py --overrides   # every overridden species
# or just double-click  tech/Color Variations.bat  (blank input = all overridden species)
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
    u8  tintHue;      // sine-table units. Center of the neutral-tint arc.
    s8  tintSpread;   // sine-table units per hue step (-4..+3).
    u8  tintStrength; // 5-bit color units added to grays. 0 = off.
    u8  tintDarken;   // 1/64ths toward black at hue step -4. 0 = off.
    u8  tintLighten;  // 1/64ths toward white at hue step +3. 0 = off.
    s8  grayFade;     // 1/64ths desaturation at the extreme step; sign picks the side. 0 = off.
    s8  whiteFade;    // 1/64ths toward white at the extreme step; sign picks the side. 0 = off.
    s8  brownFade;    // 1/64ths toward sepia/brown at the extreme step; sign picks the side. 0 = off.
};
```

Trailing fields can be left off an entry; C fills them with 0.

Defaults if no override:  `maxAngle = 14, angleBias = 0, satCap = 2048, satMul = 1024`, all tint fields `0`.

---

## What each field does

### `maxAngle` — how far the hue can drift
| Value | Approx. swing | Use when… |
|------:|:-------------:|:----------|
| `14` (default) | ±20° | Most Pokémon. Wide colorful spread. |
| `10` | ±14° | Sprites where ±20° pushes into a different "color identity." |
| `6`  | ±8.5° | Should clearly read as the same Pokémon. |
| `4`  | ±5.6° | Very tight — barely-perceptible hue shift. |
| `0`  | none  | No hue rotation, only mode-based saturation effects. |

> **Sine-table units:** the engine uses a 256-step sine table where 64 units = 90°,
> so 1 unit ≈ 1.4°. `maxAngle / 4` becomes the per-step magnitude.

### `angleBias` — push the whole range away from a "danger color"
Added to the final angle of every variation, in the same sine-table units.

Positive angles turn red → yellow → green → blue; negative angles go the other
way (yellow → orange → red, blue → green).

| Value | Effect on hue | Example |
|------:|:--------------|:--------|
| `0`   | symmetric around the base color (default) | Most species. |
| `+6` to `+10` | bias **away from red/orange** (yellow → green) | A yellow Pokémon whose shiny is orange. |
| `−6` to `−10` | bias **toward red/orange** (green → yellow) | A green Pokémon whose shiny is blue. |

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
| `750`  | clearly muted; bright yellows become tan/khaki |
| `600`  | strong brown/gray pull |
| `400`  | nearly grayscale |
| `0`    | full grayscale |

Because `satMul` is applied **after** the mode-based saturation work, the
8 modes still produce distinguishable cells — they're just all shifted
toward gray together.

### `tintHue` / `tintSpread` / `tintStrength` — color for gray Pokémon
Hue rotation spins colors around the gray axis, so **gray pixels never change**.
A mostly-gray species (e.g. Shuppet) only shows variation in its few colored
details. The neutral tint fixes this by adding a small amount of color to
unsaturated palette entries *before* the normal hue/saturation logic runs.

- Each individual's tint hue = `tintHue + hueStep * tintSpread` (hueStep is −4..+3).
- Hues in sine-table units: `0` red, `43` yellow, `85` green, `128` cyan,
  `171` blue, `213` magenta.
- Darker shades get proportionally less tint (outlines stay black) and
  near-white highlights fade back to white.
- Colors that are already saturated (eyes, markings) are left alone. "Gray"
  here means a channel spread under `TINT_MAX_CHROMA` (6), so slightly
  blue/purple grays like Banette's body are tinted too.

| `tintStrength` | Effect |
|---------------:|:-------|
| `0` | off (default) |
| `2`–`3` | subtle — still reads as gray, but each individual leans a color |
| `5` | clearly pastel |

Pick the arc (`tintHue ± 4 * tintSpread`) so it stays away from the shiny's hue.

### `tintDarken` / `tintLighten` — spread gray Pokémon toward black/white
Also only affects the gray colors. Values are 1/64ths of the way to black
(`tintDarken`) or white (`tintLighten`). Hue step −4 gets the full darken,
step +3 the full lighten, and the steps in between are spread evenly.
Set `tintLighten = 0` for an original → black range. The tint fades out
toward either extreme, so the ends read as black/white rather than dark/pale
tint. Dark outlines are protected when lightening, and near-white highlights
(eye whites, teeth) are protected when darkening.

| `tintDarken` | Darkest individual |
|-------------:|:-------------------|
| `0`  | off (default) |
| `24` | dark gray |
| `40` | near-black, shading still visible *(Shuppet line)* |
| `56` | almost solid black — loses detail |

`tintLighten` works the same way toward white (`40` ≈ near-white).

### `grayFade` / `whiteFade` / `brownFade` — swap a hue direction for a fade
Hue steps run −4..+3; negative steps rotate one way around the color wheel,
positive steps the other. These replace a side's hue rotation with a fade:

- `> 0`: steps +1..+3 don't rotate hue; they fade instead (full amount at +3).
- `< 0`: steps −1..−4 fade instead (full amount at −4).

- `grayFade` removes saturation.
- `whiteFade` lightens toward white (dark outlines are protected).
- `brownFade` blends toward a sepia/brown version of each color, keeping the
  shading (yellow → golden brown, darker shades → darker brown). Near-white
  highlights are protected. Sepia tone is set by `SEPIA_R/G/B`.

Magnitude is 1/64ths of the way at the extreme step. They can target opposite
sides (e.g. white one way, brown the other) or the same side. All work on
**every** color (not just grays) and are applied last, so vivid modes can't
undo them. Party/PC icons fade the same way.

| Magnitude | `grayFade` | `whiteFade` | `brownFade` |
|----------:|:-----------|:------------|:------------|
| `16`–`24` | subtly muted *(Zigzagoon: 24)* | slightly paler *(Zigzagoon: −20, Pikachu line: 24)* | subtle golden/tan deepening *(Pikachu line: −20)* |
| `40`–`48` | mostly gray | clearly pastel | clearly brown |
| `64` | fully gray | very washed out | full sepia |

---

## Recipe book

### "Shiny is a hue shift of the base — avoid it entirely"  *(Pikachu line)*
```c
{ SPECIES_FOO, 0, 0, 900, FP_SCALE, 0, 0, 0, 0, 0, 0, 24, -20 }
```
- No hue rotation at all (`maxAngle = 0`), so nothing drifts toward the shiny's hue
- Positive steps fade slightly toward white (cream), negative steps toward brown
- Cap vivid modes (`900`) so nothing competes with the shiny's saturation

### "Shiny is brighter / more saturated than the base"
```c
{ SPECIES_FOO, 6, 0, 900, 750 }
```
- Tight hue range (`6`) so it stays recognizable
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

### "Mostly-gray Pokémon: black → original gray"  *(Shuppet, Banette)*
```c
{ SPECIES_FOO, COLOR_VARIATION_MAX_ANGLE, 0, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 40, 0 }
```
No tint; individuals range from near-black up to the original sprite colors.
Colored details (eyes, zippers) still get the normal hue variation.

### "Mostly-gray Pokémon: give the grays a color"
```c
{ SPECIES_FOO, COLOR_VARIATION_MAX_ANGLE, 0, FP_SCALE * 2, FP_SCALE, 208, 9, 3 }
```
Slate-blue → violet → dusty-rose tint on the grays.

### "Gray Pokémon, but I want white / brown / black individuals"
```c
{ SPECIES_FOO, 8, 0, FP_SCALE * 2, FP_SCALE, 20, 1, 6, 40, 40 }
```
Brown tint on the grays, with darken/lighten spreading individuals from
near-black through brown to near-white. Smaller `maxAngle` keeps the browns brown.

### "Hue directions look bad — fade to gray / white instead"  *(Zigzagoon)*
```c
{ SPECIES_FOO, COLOR_VARIATION_MAX_ANGLE, 0, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 24, -20 }
```
Positive steps (Zigzagoon: yellow/olive) become subtly grayer; negative steps
(Zigzagoon: red) become slightly paler. No hue rotation on either side.

### "I want the variation away from a specific direction only"
```c
{ SPECIES_FOO, 10, +8, 2048, 1024 }   // push warm
{ SPECIES_FOO, 10, -8, 2048, 1024 }   // push cool
```

---

## Workflow

1. Edit `sColorVariationOverrides[]` in `src/pokemon_color_variation.c` and save.
2. Run the preview: `python tech/generate_color_variations.py <name>`
   (or double-click `tech/Color Variations.bat`). It reads the C file directly.
3. Open `tech/color_variation_previews/<name>_front_variations.png`.
   - Center cell (green border) is the original.
   - Bottom-right cell (red border) is the shiny.
   - Confirm: variations look like the species, none collide with the shiny.
4. Tweak and repeat. Only rebuild the ROM (`bash build.sh`) when you're happy.

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
