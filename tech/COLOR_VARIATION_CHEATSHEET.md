# Individual Color Variation — Tuning Cheat Sheet

Per-species overrides live in **`src/pokemon_color_variation.c`** in the
`sColorVariationOverrides[]` array. The preview tool reads that array (and the
tuning `#define`s) directly every time it runs, so there's nothing to sync —
save the C file and re-run the preview.

> **One thing does need to stay in sync: the engine itself.** The preview
> script (`tech/generate_color_variations.py`) re-implements the C engine's
> math, so it only knows about fields it has been taught. `bandDarken`,
> `bandGray` and `bandMaxUp` exist in **both** `src/pokemon_color_variation.c`
> and the script. If the preview ever disagrees with the game (for example a
> color is much grayer in-game than on the sheet), check first that both files
> have the same fields. A field the script doesn't know is silently ignored.

To preview changes without rebuilding the ROM:

```
python tech/generate_color_variations.py <species_name> [more_names...]
python tech/generate_color_variations.py 25 172 26-27  # National Pokédex numbers / ranges
python tech/generate_color_variations.py --overrides   # every overridden species
python tech/generate_color_variations.py --existing    # refresh every sheet already generated
# or just double-click  tech/Color Variations.bat  (enter 0 = refresh existing,
#   blank = all overridden species, or type names / Pokédex numbers)
```

Output PNGs land in `tech/color_variation_previews/`. The grid shows the
64 possible variations (bits 16-21 of the personality value), with the
**original sprite framed in green** in the center and the **shiny framed in red**
in the bottom-right corner so you can compare drift vs. shiny clash directly.

---

## How a variation is picked

Every individual gets two numbers from its personality value:

- **Hue step**: −4, −3, −2, −1, 0, +1, +2, +3. This is the main "direction"
  of the variation. Normally negative steps rotate the hue one way around the
  color wheel and positive steps the other; step 0 doesn't rotate.
- **Mode**: 0–7, a saturation flavor applied on top:

  | Mode | Effect |
  |-----:|:-------|
  | 0 | hue rotation only |
  | 1 | hue + muted (90% saturation) |
  | 2 | hue + vivid (120% saturation, limited by `satCap`) |
  | 3 | split hue: warm colors rotate one way, cool colors the other |
  | 4 | warm colors vivid, cool colors muted |
  | 5 | warm colors muted, cool colors vivid |
  | 6 | split hue + muted |
  | 7 | split hue + vivid |

8 steps × 8 modes = the 64 cells in a preview sheet. Most override settings
say what happens at a particular **step** (e.g. "full amount at step +3"),
so when you read "negative steps" below, think "half of all individuals".

---

## Writing an entry (syntax)

Each entry is one line inside `sColorVariationOverrides[]` in
`src/pokemon_color_variation.c`. Add one line per species (evolutions need
their own lines), and end every line with a comma:

```c
static const struct ColorVariationOverride sColorVariationOverrides[] =
{
    // Comments explaining why are encouraged.
    { SPECIES_ZIGZAGOON, COLOR_VARIATION_MAX_ANGLE, 0, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 24, -20 },
    { SPECIES_LINOONE,   COLOR_VARIATION_MAX_ANGLE, 0, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 24, -20 },
};
```

### Style 1: positional (values in field order)

Values are matched to fields **by position**, in this exact order:

| # | Field | Default (no override) | Section below |
|--:|:------|:----------------------|:--------------|
| 1 | `species` | n/a | the `SPECIES_` constant |
| 2 | `maxAngle` | `COLOR_VARIATION_MAX_ANGLE` (14) | [maxAngle](#maxangle--how-far-the-hue-can-drift) |
| 3 | `angleBias` | `0` | [angleBias](#anglebias--push-the-whole-range-away-from-a-danger-color) |
| 4 | `satCap` | `FP_SCALE * 2` (2048) | [satCap](#satcap--kill-or-limit-vivid-mode) |
| 5 | `satMul` | `FP_SCALE` (1024) | [satMul](#satmul--uniform-always-on-saturation-pull) |
| 6 | `tintHue` | `0` | [tint](#tinthue--tintspread--tintstrength--color-for-gray-pokémon) |
| 7 | `tintSpread` | `0` | ↑ |
| 8 | `tintStrength` | `0` | ↑ |
| 9 | `tintDarken` | `0` | [darken/lighten](#tintdarken--tintlighten--spread-gray-pokémon-toward-blackwhite) |
| 10 | `tintLighten` | `0` | ↑ |
| 11 | `grayFade` | `0` | [fades](#grayfade--whitefade--brownfade--swap-a-hue-direction-for-a-fade) |
| 12 | `whiteFade` | `0` | ↑ |
| 13 | `brownFade` | `0` | ↑ |
| 14 | `bandHue` | `0` | [hue band](#bandhue--bandwidth--bandshift--bandlighten--banddarken--bandgray--bandmaxup--vary-just-one-color) |
| 15 | `bandWidth` | `0` | ↑ |
| 16 | `bandShift` | `0` | ↑ |
| 17 | `bandLighten` | `0` | ↑ |
| 18 | `bandDarken` | `0` | ↑ |
| 19 | `bandGray` | `0` | ↑ |
| 20 | `bandMaxUp` | `0` | ↑ |

- **Start from the "do nothing" line** and change only what you need:
  ```c
  { SPECIES_FOO, COLOR_VARIATION_MAX_ANGLE, 0, FP_SCALE * 2, FP_SCALE },
  ```
  Fields 2–5 are the defaults; everything after them is `0` (off).
- **You can stop early.** Any fields left off the end are `0`. But to set a
  later field you must fill in **every field before it** (with `0` where
  unused). E.g. to set only `whiteFade` (#12) you still need values for
  #2–#11.
- **Count carefully.** An extra or missing value shifts every later value
  into the wrong field, with no error. For example, one extra value after
  `satCap` makes `FP_SCALE * 2` land in `satMul` (doubling saturation) and
  every fade/band value land one field late. If a species looks wildly off,
  count the values first.

### Style 2: named fields (safer for long entries)

Name each field with `.field = value`, in any order:

```c
{ .species = SPECIES_GEODUDE, .maxAngle = 0, .satCap = FP_SCALE, .satMul = 512,
  .grayFade = 16, .brownFade = -12, .bandHue = 47, .bandWidth = 10, .bandShift = 48 },
```

- Fields you leave out are `0`, **not** the defaults. Always write
  `.maxAngle`, `.satCap` and `.satMul` explicitly; leaving them out means no
  hue rotation (`maxAngle 0`), vivid modes that go fully gray (`satCap 0`)
  and a grayscale sprite (`satMul 0`).
- Each field can only appear **once** per entry (agbcc errors with
  "field already initialized").
- Don't mix named and positional values in one entry; pick one style per line.

Both styles compile and both are read by the preview tool.

### Value formats

- `FP_SCALE` = 1024 = 100%. `FP_SCALE * 2` = 200%, `900` ≈ 88%, `512` = 50%.
  Used by `satCap` and `satMul`.
- **Sine-table units** (hue angles): 256 units = 360°, so 1 unit ≈ 1.4°.
  Degrees × 256 / 360 = units. Used by `maxAngle`, `angleBias`, `tintHue`,
  `tintSpread`, `bandHue`, `bandWidth`, `bandShift`, `bandMaxUp`. Common hues: `0` red,
  `21` orange, `43` yellow, `85` green, `128` cyan, `171` blue, `213` magenta.
- **1/64ths**: `64` = all the way, `32` = halfway. Used by `tintDarken`,
  `tintLighten`, all three fades, `bandLighten`, `bandDarken` and `bandGray`.
- **Sign picks the side** for `grayFade`/`whiteFade`/`brownFade`: positive =
  steps +1..+3, negative = steps −1..−4. `bandDarken` uses the same sign rule
  but only reaches the two extreme steps of its side (see its section).

### Other rules

- One entry per species. If a species is listed twice, only the first entry is used.
- Species without an entry use the defaults.
- After editing, run the preview (`Color Variations.bat`, then `0` to refresh
  existing sheets) to check it before building.

---

## Which setting do I reach for?

| I want to… | Use |
|:-----------|:----|
| Make variations subtler overall | lower `maxAngle` |
| Stop hue rotation entirely | `maxAngle = 0` |
| Lean every variant in one hue direction | `angleBias` |
| Stop variants getting more saturated than the original | `satCap = 900` (or `FP_SCALE`) |
| Make the whole Pokémon grayer / earthier, or richer | `satMul` below / above `FP_SCALE` |
| Replace one hue direction with gray / white / brown | `grayFade` / `whiteFade` / `brownFade` |
| Vary a mostly-gray Pokémon | `tintHue`/`tintSpread`/`tintStrength`, or `tintDarken`/`tintLighten` |
| Change only one color (a body, flowers, a mushroom) | `bandHue`/`bandWidth`/`bandShift` (+ `bandLighten`) |
| Turn a saturated color pink/pastel | `bandShift` + `bandLighten` |
| Turn a saturated color (red markings, a body) toward black | `bandDarken` (`tintDarken` only reaches grays) |
| Gray out one color gradually across all steps, keeping eyes/tongue | `bandGray` (not `grayFade`) |
| Stop a color drifting toward the shiny's hue in *every* mode | `bandMaxUp` |
| Vary a saturated body by lightness when hue rotation can't | `bandLighten` / `bandDarken` / `bandGray` |

---

## The override struct (reference)

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
    u8  bandHue;      // hue band: center hue of the colors to shift (sine-table units)
    u8  bandWidth;    // hue band: half-width that gets the full shift. 0 = off.
    s8  bandShift;    // hue band: hue rotation at step +3 (+ = higher hue number, - = lower)
    u8  bandLighten;  // hue band: 1/64ths toward white at step +3. 0 = off.
    s8  bandDarken;   // hue band: 1/64ths toward black at the two extreme steps of one side
                      //   (negative: -3 half, -4 full; positive: +2 half, +3 full). 0 = off.
    u8  bandGray;     // hue band: 1/64ths toward gray at step +3, tapering across all steps. 0 = off.
    u8  bandMaxUp;    // hue band: caps positive hue rotation of band colors (sine-table units). 0 = off.
};
```

Trailing fields can be left off an entry; C fills them with 0.

Defaults if no override:  `maxAngle = 14, angleBias = 0, satCap = 2048, satMul = 1024`, all tint fields `0`.

---

## What each field does

### `maxAngle` — how far the hue can drift
How far the normal hue rotation goes at the extreme steps (−4 / +3).

**How to use:** start at the default `14`. If variants stop looking like the
species (e.g. a pink Pokémon turning orange), lower it. Set `0` when you want
to control color only through fades / bands (Pikachu line, Paras, Geodude line).

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

**How to use:** look at the preview. If many variants drift toward the shiny
(or an ugly color) on one side, set a small bias the other way and re-check.
Start at `±3`–`±6`; Mankey uses `-4` to keep yellows from turning olive, Wigglytuff
uses `-8` to pull peach toward pink.

Positive angles turn red → yellow → green → blue; negative angles go the other
way (yellow → orange → red, blue → green).

| Value | Effect on hue | Example |
|------:|:--------------|:--------|
| `0`   | symmetric around the base color (default) | Most species. |
| `+6` to `+10` | bias **away from red/orange** (yellow → green) | A yellow Pokémon whose shiny is orange. |
| `−6` to `−10` | bias **toward red/orange** (green → yellow) | A green Pokémon whose shiny is blue. |

Rule of thumb: pick `|angleBias| ≈ maxAngle` if you want all variations on
**one side** of the original. Pick smaller if you want to merely lean.

> **Split-hue modes (3, 6, 7) flip the bias too.** In those modes cool colors
> rotate the opposite way to warm ones, and the bias is part of that angle. A
> bias that pulls a warm color away from a danger hue pushes a cool color
> *toward* it in those modes. If the same bad color shows up at opposite steps
> in different modes, a bias won't fix it: use a fade on one side, a hue band,
> or `bandMaxUp` (see [Gotchas](#gotchas)).

### `satCap` — kill or limit "vivid" mode
The variation algorithm has 8 modes; some boost saturation up to ~120%.
`satCap` is the maximum fixed-point multiplier the boost can reach.

**How to use:** leave at `FP_SCALE * 2` unless vivid cells look too strong or
too close to a more saturated shiny; then use `FP_SCALE` (no boost) or `900`
(vivid cells come out slightly muted).

| Value | Meaning |
|------:|:--------|
| `2048` (default ≈ 200%) | full boost allowed (the vivid boost itself is ~120%, so anything ≥ `1229` behaves the same as the default) |
| `1024` | no boost — vivid modes act as no-op |
| `900`  | actively *desaturate* even the "vivid" variants — guarantees nothing reads as more saturated than the original. **Use this whenever the shiny is more saturated than the base sprite.** |
| `768`  | aggressive cap, very flat. |

### `satMul` — uniform "always-on" saturation pull
Applied to **every** pixel of **every** variation, after the mode logic.
This is the main lever for shifting the entire palette toward gray/brown
without flattening the per-mode variety.

**How to use:** below `FP_SCALE` for earthier / grayer Pokémon (Geodude line
`512`); slightly above `FP_SCALE` to keep every variant richer than a dull
shiny (Mankey `1229`, Primeape `1126`). Values far above ~1300 clip colors and
make variants look alike.

| Value  | Effect |
|-------:|:-------|
| `1024` | no change (default) |
| `1126`–`1229` | slightly richer (110–120%) |
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

**How to use:** set `tintHue` to the hue you want the middle individuals to
lean toward, `tintSpread` to how much the hue changes per step (`0` = all the
same hue, `9` ≈ ±50° across the range), and `tintStrength` `2`–`3`. Raise
`tintStrength` only if the grays still look plain.

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

**How to use:** pick how dark the darkest individual should be (`tintDarken`)
and how light the lightest should be (`tintLighten`); leave the other at `0`
for a one-sided range. If the base sprite is already dark, use a smaller
value (Duskull `28`, Dusclops `16`).

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

Things to know before reaching for a fade:

- **A fade switches hue rotation off on its side** (the bias still applies).
  Fades on both sides mean no rotation at all, the same as `maxAngle 0`.
- **A fade only exists on its own side and tapers only there.** A positive
  value ramps across +1, +2, +3 (⅓, ⅔, full) and is zero on every other step;
  a negative value ramps across −1..−4 (¼ … full). The other half of the sheet
  gets none, so the change can look like a plateau followed by a short ramp.
  For a smooth taper across all eight steps use the band effects
  (`bandShift`, `bandLighten`, `bandGray`) or the tint.
- **They act on every color, including eyes, tongues and other details.** A
  gray fade dulls a red mouth along with the body. To change the body only,
  use `bandGray` / `bandLighten` instead.

| Magnitude | `grayFade` | `whiteFade` | `brownFade` |
|----------:|:-----------|:------------|:------------|
| `16`–`24` | subtly muted *(Zigzagoon: 24)* | slightly paler *(Zigzagoon: −20, Pikachu line: 24)* | subtle golden/tan deepening *(Pikachu line: −20)* |
| `40`–`48` | mostly gray | clearly pastel | clearly brown |
| `64` | fully gray | very washed out | full sepia |

**How to use:** find which side looks wrong. Preview cells run in reading
order (skipping the green-framed original), cycling through steps −4, −3, −2,
−1, 0, +1, +2, +3 every 8 cells, so a problem color that shows up in every
8-cell group comes from one step. If you're not sure which side it is, try
`24` and look; if the bad colors are still there, flip the sign. Then adjust
in steps of ~4 (`16`–`28` is usually enough).

### `bandHue` / `bandWidth` / `bandShift` / `bandLighten` / `bandDarken` / `bandGray` / `bandMaxUp` — vary just one color
Affects **only** the palette colors whose hue is near `bandHue` (e.g. just a
blue body, or just red flowers). Everything else gets the normal variation.
Colors within `bandWidth` of `bandHue` get the full effect; the next
`BAND_FEATHER` (8) units fade out so nothing jumps at the edge. Grays are
never touched.

| Field | What it does to the band colors |
|:------|:--------------------------------|
| `bandShift` | Hue rotation. None at step −4 (original color), the full amount at +3, spread evenly. |
| `bandLighten` | 1/64ths toward white, scaled by brightness so shading is kept. None at −4, full at +3. |
| `bandGray` | 1/64ths toward gray (desaturation). None at −4, full at +3, spread evenly. |
| `bandDarken` | 1/64ths toward black, but only at the **two extreme steps of one side**. Negative value: step −3 gets half, step −4 the full amount. Positive value: step +2 gets half, step +3 the full amount. Every other step is untouched. |
| `bandMaxUp` | A ceiling on the *general* hue rotation: positive (hue-increasing) rotation of band colors is trimmed to this many units. Negative rotation is never limited. |

`bandShift`, `bandLighten`, `bandGray` and `bandDarken` run **before** the
normal variation, so modes and fades still apply on top. `bandMaxUp` limits the
normal rotation itself.

- Hues in sine-table units: `0` red, `21` orange, `43` yellow, `85` green,
  `128` cyan, `142` slate-blue (~200°), `171` blue, `213` magenta.
  Degrees × 256 / 360 = units.
- **`bandShift` direction:** `+` raises the hue number (red → yellow → green →
  cyan → blue → magenta → red), so what it does depends on the color. `+` turns
  a green toward blue, a red toward orange, and a purple toward magenta/pink;
  `−` goes the other way (a red toward crimson/pink, a purple toward blue).
  `48` ≈ 68°.
- **`bandLighten`** lightens toward white, scaled by brightness so shading is
  kept. A saturated red only rotates to crimson/magenta, so to get **pink**
  combine a small negative `bandShift` with `bandLighten`.
- **`bandDarken`** is the way to darken a *saturated* color (`tintDarken` only
  reaches grays). `-64` takes the band colors to black at step −4 (red markings
  → black); `-40` is about 60% of the way at −4 and 30% at −3 (deep wine);
  `-20` gives a gentle shadowed end. It darkens everything in the band,
  shading included, and any other color sharing that hue (a pink face next to
  red markings), so narrow `bandWidth` if that matters.
- **`bandGray` vs `grayFade`:** prefer `bandGray` when you want a smooth
  gradient across all eight steps, when only some colors should gray (eyes,
  tongue and horns stay), or when hue rotation should keep working. `grayFade`
  is for replacing one side's rotation with a fade. A fully gray band
  (`bandGray 64`) at +3 reads as a flat gray, so `32`–`48` is usually plenty.
- **`bandMaxUp`** exists for split-hue modes (3, 6, 7), where cool colors
  rotate the opposite way: a blue body can drift toward a purple shiny from
  *both* directions depending on the mode, and neither a bias nor a fade can
  stop both. Set a small positive cap (`4` ≈ 6°): the body can still drift
  freely toward cyan, but never more than a few units past its original hue
  toward purple. The band's hue range is tested against the color as it is
  when the rotation happens, widened by `|bandShift|`. Not mirrored in the
  party/menu icon hue shortcut, so an icon can drift slightly further than its
  sprite.

| Example | `bandHue, bandWidth, bandShift, bandLighten, bandDarken, bandGray, bandMaxUp` |
|:--------|:------------------------------------------------------------------------------|
| Blue body → violet *(Oddish line)* | `142, 12, 48, 0, 0, 0, 0` |
| Red flowers → rose pink *(Bellossom)* | `7, 12, -12, 36, 0, 0, 0` |
| Red mushrooms → pink/lilac *(Paras)* | `0, 6, -48, 24, 0, 0, 0` |
| Olive body → sage/teal *(Geodude line)* | `47, 10, 48, 0, 0, 0, 0` |
| Purple body, hue held, lighter/darker *(Starmie, Staryu)* | `210, 8, 0, 30, -20, 0, 0` |
| Red → magenta, paler and grayer *(Mr. Mime)* | `6, 8, -25, 14, 0, 32, 0` |
| Gold → bronze → gray *(Tauros)* | `31, 6, 0, 12, -28, 48, 0` |
| Orange-red → wine / coral *(Scizor)* | `13, 6, 0, 24, -40, 0, 0` |
| Red markings → black *(Goldeen, Seaking)* | `9, 6, 0, 0, -64, 0, 0` |
| Blue body held back from purple *(Lapras)* | `152, 6, 0, 0, 0, 0, 4` |

**How to use:**
1. Find the hue of the color you want to change. Either convert degrees from
   an image editor (× 256 / 360), or run this to print each palette color's
   hue in sine-table units (`-1` = gray):
   ```
   python -c "import sys; sys.path.insert(0,'tech'); import generate_color_variations as g; p=g.load_jasc_pal(g.GFX_ROOT/'paras'/'normal.pal'); print({i: g.color_hue256(*c) for i, c in enumerate(p)})"
   ```
2. Set `bandHue` to that value and `bandWidth` just wide enough to cover all
   shades of that color (`6`–`12` usually). If another part of the sprite with
   a similar hue changes too (e.g. Golem's face), narrow the band.
3. Set `bandShift` for how far the color should travel. Add `bandLighten`
   if it should also get paler, `bandGray` if it should get duller,
   `bandDarken` if one end should go dark.
4. Usually combine with `maxAngle = 0` (or a small value) so the rest of the
   sprite doesn't also rotate.

---

## Order of effects

What runs, in this order, on every color of every variation:

1. **Tint / darken / lighten** (grays only).
2. **Hue band**: `bandShift`, then `bandLighten`, then `bandDarken`, then `bandGray`.
3. **Hue rotation**: step × `maxAngle` / 4 plus `angleBias`. Split modes flip it
   for cool colors; a side with a fade doesn't rotate; band colors are capped by
   `bandMaxUp`.
4. **Mode saturation** (muted / vivid / split saturation), limited by `satCap`.
5. **`satMul`**.
6. **Fades** (`grayFade`, `whiteFade`, `brownFade`), last, so nothing undoes them.

A preview cell's position tells you its step and mode: counting the 64
variations in reading order (skipping the green-framed original), variation
`i` has **mode = `i // 8`** and **step = `i % 8 − 4`**. So every 8th cell is the
same step, and each group of 8 is one mode.

---

## Gotchas

- **Split-hue modes flip rotation *and* bias.** Cool colors rotate opposite to
  warm ones in modes 3, 6 and 7, so a color can reach the same bad hue from
  both ends of the sheet. Clues: the problem cells are at opposite steps in
  different mode groups. Fixes: a fade on one side, a hue band, or `bandMaxUp`.
- **Fades disable rotation on their side**, and fades on both sides mean no
  rotation at all. Fading both sides while also expecting rotation is a common
  surprise.
- **`tintDarken` / `tintLighten` / tint only reach colors with a channel spread
  under `TINT_MAX_CHROMA` (6).** Moderately saturated bodies (spread 7–9, such
  as Gengar, Haunter and Starmie) are skipped, and so are strongly saturated
  ones (reds). Use `bandDarken` / `bandLighten` / `bandGray` for those. Raising
  `TINT_MAX_CHROMA` is global: it changes every species that uses tint or
  darken (Seel, Dewgong, Shuppet, Duskull…), so refresh their sheets if you do.
- **Near-white highlights are protected from darkening.** A white body stays
  white while its gray shading goes black (Goldeen's "black fins, white body").
  To blacken the colored markings instead, use `bandDarken`.
- **Low-saturation colors barely respond to hue rotation.** Colors are stored
  in 5 bits per channel, so a purple with a channel spread of 7–8 changes by
  only a few units however far you rotate it (why Starmie's body looked
  unchanged). Vary lightness and saturation (`bandLighten`, `bandDarken`,
  `bandGray`) instead of hue.
- **`satCap` above ~1229 is the same as the default** (the vivid boost is
  ~120%). Values between `1024` and `1229` limit it partially.
- **A fade or `bandGray` also reaches the colors you didn't mean it to** unless
  you use the band: a gray fade dulls red eyes; a white fade lightens a tongue.
- **The preview must have the same fields as the game** (see the note at the
  top). If the sheet and an in-game screenshot disagree, diff the two files'
  band code before touching any numbers.

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

### "Earthy / stone colors, away from a warm shiny"  *(Geodude line)*
```c
{ SPECIES_FOO, 0, 0, FP_SCALE, 512, 0, 0, 0, 0, 0, 16, 0, -12, 47, 10, 48 }
```
- `satMul 512` pulls the base color halfway to gray (stone)
- Hue band on the body (`47` ≈ 66° olive) rotates it toward sage and a hint of teal
  (raise `48` toward `90` for slate blue)
- Negative steps fade toward muted brown/gray instead of rotating toward gold/red
- `satCap FP_SCALE` stops vivid modes from re-saturating it

### "Keep the body fixed, vary one detail"  *(Paras)*
```c
{ SPECIES_FOO, 0, 0, 900, FP_SCALE, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, -48, 24 }
```
- `maxAngle 0` + `satCap 900` keep the body's color (and away from the shiny)
- Hue band on the red mushrooms (`0`) turns them toward pink/lilac, lightening slightly

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
Check the sheet's split-hue groups too: the bias flips for cool colors there.

### "Red markings → black, gray fins stay light"  *(Goldeen, Seaking)*
```c
{ SPECIES_FOO, 5, -9, FP_SCALE * 2, 1126, 0, 0, 0, 0, 24, 0, 0, 0, 9, 6, 0, 0, -64 }
```
- `maxAngle 5` with `angleBias -9` keeps the reds in the red family (no orange)
- Hue band on the red markings with `bandDarken -64`: black at step −4, deep wine
  at −3, nothing at the other steps, so most individuals keep a vivid red
- `tintLighten 24` whitens the gray fins and tail toward +3
- `satMul 1126` keeps the reds punchy

### "One color fades gradually toward pink / purple, and duller"  *(Mr. Mime)*
```c
{ SPECIES_FOO, 6, 0, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 0, 0, 0, 6, 8, -25, 14, 0, 32 }
```
- Hue band on the reds and pink skin; `bandShift` rotates them toward magenta
- `bandLighten` + `bandGray` make the high steps paler and duller, tapering
  across all eight steps (a `grayFade` would have plateaued on one side)
- Small `maxAngle` keeps the rest of the rotation from reaching yellow

### "Keep a color from drifting toward the shiny's hue, in every mode"  *(Lapras)*
```c
{ SPECIES_FOO, COLOR_VARIATION_MAX_ANGLE, 0, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 0, 0, 0, 152, 6, 0, 0, 0, 0, 4 }
```
- Full default rotation for everything, so the sheet keeps its variety
- Band on the blue body with `bandMaxUp 4`: it can drift toward cyan freely but
  never more than ~6° toward the purple shiny, in normal *and* split modes

### "Saturated body: vary lightness instead of hue"  *(Starmie, Staryu, Tauros)*
```c
{ SPECIES_STARMIE, 10, 3, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 0, 0, 0, 210, 8, 0, 30, -20 }
{ SPECIES_TAUROS,  6, -4, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 0, 0, 0, 31, 6, 0, 12, -28, 48 }
```
- Starmie: the purple body runs from a shadowed violet to a pale lavender; hue
  stays inside a purple window
- Tauros: gold goes bronze at the dark end and gray khaki at the top, with the
  lavender horns outside the band and untouched

### "Keep an orange-red in the red family"  *(Scizor)*
```c
{ SPECIES_FOO, 8, -12, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 0, 0, 0, 13, 6, 0, 24, -40 }
```
`angleBias -12` moves the whole range from orange-red toward true red; the band
adds wine at the dark end (`bandDarken`) and coral at the light end (`bandLighten`).

---

## Workflow

1. Edit `sColorVariationOverrides[]` in `src/pokemon_color_variation.c` and save.
2. Run the preview: `python tech/generate_color_variations.py <name>`
   (or double-click `tech/Color Variations.bat`). It reads the C file directly.
3. Open `tech/color_variation_previews/<name>_front_variations.png`.
   - Center cell (green border) is the original.
   - Bottom-right cell (red border) is the shiny.
   - Confirm: variations look like the species, none collide with the shiny.
   - To find out which step and mode a cell is, use `mode = i // 8`,
     `step = i % 8 − 4` (reading order, skipping the original).
4. Tweak and repeat. Only rebuild the ROM (`bash build.sh`) when you're happy.
5. If a species looks different in-game than on the sheet, make sure
   `tech/generate_color_variations.py` and `src/pokemon_color_variation.c` have
   the same band fields (`bandDarken`, `bandGray`, `bandMaxUp`).

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
