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
    // Split-hue modes (3, 6, 7) rotate cool colors the opposite way, bias
    // included, so a bias that turns a green body bluer turns it yellower in
    // those modes. 1 = flip only the per-step rotation there and keep angleBias
    // pointing the same way for every color. 0 = off (the bias flips too).
    u8  holdBias;
    // Neutral tint options (tintHue / tintSpread / tintStrength):
    // tintRamp 1 = the tint ramps like the hue band, none at step -4 and the
    // full tintStrength at +3, so individuals range from the original grays
    // to the tint color. 0 = every individual gets the same strength.
    u8  tintRamp;
    // tintWhites 1 = near-white colors are tinted too (normally they're left
    // white so eye shine stays clean), for mostly white Pokemon.
    u8  tintWhites;
};

static const struct ColorVariationOverride sColorVariationOverrides[] =
{
    //{ species, maxAngle, angleBias,
    // satCap, satMul, tintHue, 
    //tintSpread, tintStrength, tintDarken,
    // tintLighten, grayFade, whiteFade,
    // brownFade, bandHue, bandWidth,
    // bandShift, bandLighten, bandDarken,
    // bandGray, bandMaxUp, holdBias,
    // tintRamp, tintWhites },

    { SPECIES_BULBASAUR, 15, 6, 900, 1228, 0, 0, 0, 0, 0, 0, 0, 0, 116, 15, 10 },
    { SPECIES_IVYSAUR, 15, 6, 900, 1228, 0, 0, 0, 0, 0, 0, 0, 0, 116, 15, 10 },
    { SPECIES_VENUSAUR, 15, 6, 900, 1228, 0, 0, 0, 0, 0, 0, 0, 0, 116, 15, 10 },

    { SPECIES_CHARMANDER, 8, -12, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 0, 0, 0, 13, 6, 0, 10, -10 },
    { SPECIES_CHARMELEON, 8, -12, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 0, 0, 0, 13, 6, 0, 10, -10 },
    { SPECIES_CHARIZARD, 8, -12, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 0, 0, 0, 13, 6, 0, 10, -10 },

    { SPECIES_SQUIRTLE, 12, -6, 1228, FP_SCALE, 0, 0, 0, 0, 0, 0, 0, 0, 181, 8, -10, 0, 4, 8, 0 },
    { SPECIES_WARTORTLE, 12, -6, 1228, FP_SCALE, 0, 0, 0, 0, 0, 0, 0, 0, 181, 8, -10, 0, 4, 8, -12 },
    { SPECIES_BLASTOISE, 12, -6, 1228, FP_SCALE, 0, 0, 0, 0, 0, 0, 0, 0, 181, 8, -10, 0, 4, 8, -12 },

    { SPECIES_CATERPIE, 6, 0, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 0, 0, 0, 48, 16, 35 },
    { SPECIES_METAPOD, 6, 0, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 0, 0, 0, 48, 16, 35 },
    { SPECIES_BUTTERFREE, 6, 0, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 0, 0, 0, 108, 16, 35 },

    { SPECIES_WEEDLE, 20, 6, 2056, FP_SCALE, 0, 0, 0, 0, 0, 0, 0, 24, 0, 0, 0 },
    { SPECIES_KAKUNA, 20, 10, 2056, FP_SCALE, 0, 0, 0, 0, 0, 0, 0, 24, 0, 0, 0 },
    { SPECIES_BEEDRILL, 20, 10, 2056, FP_SCALE, 0, 0, 0, 0, 0, 0, 0, 24, 0, 0, 0 },
    
    { SPECIES_PIDGEY, COLOR_VARIATION_MAX_ANGLE, 3, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 24, -20 },
    { SPECIES_PIDGEOTTO, COLOR_VARIATION_MAX_ANGLE, 0, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 24, -20 },
    { SPECIES_PIDGEOT, COLOR_VARIATION_MAX_ANGLE, -5, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 24, -20 },

    { SPECIES_RATTATA, COLOR_VARIATION_MAX_ANGLE, 0, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 0, 24, -20, 31, 16, 0, 0, 0, 24 },
    { SPECIES_RATICATE, COLOR_VARIATION_MAX_ANGLE, 0, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 0, 24, -20, 31, 16, 0, 0, 0, 24 },

    { SPECIES_SPEAROW, COLOR_VARIATION_MAX_ANGLE, 3, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 24, -20 },
    { SPECIES_FEAROW, COLOR_VARIATION_MAX_ANGLE, -5, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 24, -20 },

    { SPECIES_EKANS, COLOR_VARIATION_MAX_ANGLE, -8, FP_SCALE * 2, FP_SCALE },
    { SPECIES_ARBOK, COLOR_VARIATION_MAX_ANGLE, -8, FP_SCALE * 2, FP_SCALE },

    // Pikachu line: rotating hue turns them orange/red (toward the shinies)
    // or green. Instead, positive steps fade toward white and negative steps
    // toward brown. satCap stops vivid modes from approaching the more
    // saturated shinies.
    { SPECIES_PICHU,   24, 0, 1228, FP_SCALE, 0, 0, 0, 0, 0, 0, 22, -14 },
    { SPECIES_PIKACHU, 24, 0, 1228, FP_SCALE, 0, 0, 0, 0, 0, 0, 22, -14 },
    { SPECIES_RAICHU,  24, 0, 1228, FP_SCALE, 0, 0, 0, 0, 0, 0, 22, -14 },

    { SPECIES_SANDSHREW, 4, -4, 1953, 880, 0, 0, 0, 0, 0, 0, 0, -36 },
    { SPECIES_SANDSLASH, 4, -4, 1953, 880, 0, 0, 0, 0, 0, 0, 0, -36 },

    // Clefairy line: positive hue steps turn the pink peach/yellow. Fade
    // those toward white instead; negative steps keep the normal rotation.
    { SPECIES_CLEFFA,   COLOR_VARIATION_MAX_ANGLE, 0, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 0, 28 },
    { SPECIES_CLEFAIRY, COLOR_VARIATION_MAX_ANGLE, 0, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 0, 28 },
    { SPECIES_CLEFABLE, COLOR_VARIATION_MAX_ANGLE, 0, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 0, 28 },

    { SPECIES_VULPIX, 8, -12, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 0, 0, 0, 13, 6, 0, 10, -10 },
    { SPECIES_NINETALES, 8, 4, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 0, 0, 0, 13, 6, 0, 10, -10 },

    // Jigglypuff line: positive hue steps turn the pink peach/yellow. Fade
    // those toward white instead; negative steps keep the normal rotation.
    // Igglybuff and Wigglytuff's base palettes are more peach than
    // Jigglypuff's, so angleBias rotates their whole range toward pink to
    // match. (Kept at -6: Igglybuff's shiny is pink, so more bias would push
    // its most-rotated variants into the shiny's hue range.)
    { SPECIES_IGGLYBUFF,  COLOR_VARIATION_MAX_ANGLE, -6, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 0, 36 },
    { SPECIES_JIGGLYPUFF, COLOR_VARIATION_MAX_ANGLE,  0, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 0, 36 },
    { SPECIES_WIGGLYTUFF, COLOR_VARIATION_MAX_ANGLE, -8, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 0, 36 },

    { SPECIES_ZUBAT, COLOR_VARIATION_MAX_ANGLE, 0, FP_SCALE * 2, 720, 0, 0, 0, 0, 0, 0, 0, 0, 175, 8, 0, 0, 0, 20 },
    { SPECIES_GOLBAT, COLOR_VARIATION_MAX_ANGLE, 0, FP_SCALE * 2, 720, 0, 0, 0, 0, 0, 0, 0, 0, 224, 8, 0, 0, 0, 20 },
    { SPECIES_CROBAT, COLOR_VARIATION_MAX_ANGLE, 0, FP_SCALE * 2, 720, 0, 0, 0, 0, 0, 0, 0, 0, 206, 8, 0, 0, 0, 20 },

    // Oddish line: hue band rotates only the slate-blue body (~200 deg) toward
    // purple, from the original blue (step -4) to violet (step +3).
    { SPECIES_ODDISH,    COLOR_VARIATION_MAX_ANGLE, 0, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 0, 20, 0, 142, 12, 48 },
    { SPECIES_GLOOM,     COLOR_VARIATION_MAX_ANGLE, 0, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 0, 20, 0, 142, 12, 48 },
    { SPECIES_VILEPLUME, COLOR_VARIATION_MAX_ANGLE, 0, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 0, 20, 0, 142, 12, 48 },
    // Bellossom: hue band turns only the red flowers (~10 deg) toward a
    // lighter rose pink.
    { SPECIES_BELLOSSOM, COLOR_VARIATION_MAX_ANGLE, -6, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 0, 10, 0, 7, 12, -12, 36 },

    // Paras line. Parasect fades toward white on the side that heads for its
    // (yellow) shiny and keeps normal hue rotation on the side that heads
    // away from it.
    // Paras: the shiny is a red-orange body, so the body's hue is locked
    // (maxAngle 0) and capped (satCap 900) to keep it tan-orange. Variation
    // comes from the mushrooms instead: a hue band turns the red caps toward
    // pink/lilac (away from the shiny's red), lightening slightly.
    { SPECIES_PARAS,    0, 0, 900, FP_SCALE * 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, -48, 24 },
    { SPECIES_PARASECT, COLOR_VARIATION_MAX_ANGLE, 0, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 0, 28 },

    { SPECIES_VENONAT, 12, 0, FP_SCALE, 940, 0, 0, 0, 0, 0, 0, 0, 0, 204, 1, 20, 0, 30, 0, 0 },
    { SPECIES_VENOMOTH, 18, 0, FP_SCALE, 940, 0, 0, 0, 0, 0, 0, 0, 0, 197, 15, 20, 0, 15, 10, 0 },

    { SPECIES_MEOWTH,   0, 0, 1126, FP_SCALE, 0, 0, 0, 0, 0, 0, 22, -14, 20, 6, 0, 6 },
    { SPECIES_PERSIAN, 0, 0, 1126, FP_SCALE, 0, 0, 0, 0, 0, 0, 0, 32, 34, 11, 0, 0, 6 },

    // Mankey line: the shinies are duller/darker fur with olive-green
    // accents, so muted modes and positive hue steps drift toward them.
    // A permanent saturation boost (satMul) keeps every variant richer than
    // the shiny while leaving the modes free to vary; negative steps rotate
    // toward orange/coral, positive steps fade toward cream instead of
    // rotating toward green. angleBias (~4-6 degrees toward amber) keeps the
    // dark yellows from reading as olive and the vivid creams from reading
    // as lemon-green, the shinies' direction.
    { SPECIES_MANKEY,   COLOR_VARIATION_MAX_ANGLE, -4, FP_SCALE * 2, 1229, 0, 0, 0, 0, 0, 0, 28 },
    { SPECIES_PRIMEAPE, COLOR_VARIATION_MAX_ANGLE, -3, FP_SCALE * 2, 1126, 0, 0, 0, 0, 0, 0, 28 },

    { SPECIES_GROWLITHE, 8, -12, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 0, 0, 0, 13, 6, 0, 10, -10 },
    { SPECIES_ARCANINE, 8, -12, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 0, 0, 0, 13, 6, 0, 10, -10 },

    { SPECIES_MACHOP, COLOR_VARIATION_MAX_ANGLE, 0, FP_SCALE * 2, 880, 0, 0, 0, 0, 0, 0, 0, 0, 94, 8, 0, 0, 0, 40 },
    { SPECIES_MACHOKE, COLOR_VARIATION_MAX_ANGLE, 0, FP_SCALE * 2, 880, 0, 0, 0, 0, 0, 0, 0, 0, 186, 8, 0, 0, 0, 40 },
    { SPECIES_MACHAMP, COLOR_VARIATION_MAX_ANGLE, 0, FP_SCALE * 2, 880, 0, 0, 0, 0, 0, 0, 0, 0, 92, 14, 0, 0, 0, 40 },

    { SPECIES_BELLSPROUT, 12, 0, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 0, 0, 0, 60, 10, 25 },
    { SPECIES_WEEPINBELL, 12, 0, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 0, 0, 0, 60, 10, 25 },
    { SPECIES_VICTREEBEL, 12, 0, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 0, 0, 0, 60, 10, 25 },

    // Geodude line: stone colors. The olive body is pulled toward gray
    // (satMul 512) and a hue band turns it from olive toward sage and a hint
    // of teal (positive steps). Negative steps fade toward a muted brown or
    // gray instead of rotating toward gold/red (the shinies). Golem's band is
    // narrower so its tan face isn't turned green along with the shell.
    { SPECIES_GEODUDE,  0, 0, FP_SCALE, 1125, 0, 0, 0, 0, 0, 0, 0, -6, 47, 10, 30, 0, 0, 50 },
    { SPECIES_GRAVELER, 0, 0, FP_SCALE, 1125, 0, 0, 0, 0, 0, 0, 0, -6, 47, 10, 30, 0, 0, 50 },
    { SPECIES_GOLEM,    0, 0, FP_SCALE, 1125, 0, 0, 0, 0, 0, 0, 0, -6, 47, 10, 30, 0, 0, 50 },

    { SPECIES_SLOWPOKE, COLOR_VARIATION_MAX_ANGLE,  -6, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 0, 36 },
    { SPECIES_SLOWBRO, COLOR_VARIATION_MAX_ANGLE,  -6, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 0, 36 },
    { SPECIES_SLOWKING, COLOR_VARIATION_MAX_ANGLE,  -6, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 0, 36 },

    // Magnemite/Magneton: pale green-gray body. A hue band carries it toward blue
    // (positive steps), while tintDarken/tintLighten spread individuals from a dark
    // steel gray (step -4) to a pale icy blue (step +3). maxAngle 0 keeps the red
    // and blue magnets fixed; satMul adds a little richness so the blue reads.
    { SPECIES_MAGNEMITE, 0, 0, FP_SCALE, 1126, 0, 0, 0, 18, 20, 0, 0, 0, 95, 12, 72, 0 },
    { SPECIES_MAGNETON,  0, 0, FP_SCALE, 1126, 0, 0, 0, 18, 20, 0, 0, 0, 95, 12, 72, 0 },

    { SPECIES_FARFETCHD, COLOR_VARIATION_MAX_ANGLE, -4, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 24, -20 },

    { SPECIES_DODUO, COLOR_VARIATION_MAX_ANGLE, 3, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 24, -20 },
    { SPECIES_DODRIO, COLOR_VARIATION_MAX_ANGLE, 0, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 24, -20 },

    { SPECIES_SEEL, 6, 0, FP_SCALE * 2, FP_SCALE, 144, 3, 3, 4, 0 },
    { SPECIES_DEWGONG, 6, 0, FP_SCALE * 2, FP_SCALE, 144, 3, 3, 4, 0 },

    { SPECIES_SHELLDER, COLOR_VARIATION_MAX_ANGLE, 0, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 28 },
    { SPECIES_CLOYSTER, COLOR_VARIATION_MAX_ANGLE, 0, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 28 },

    // Gastly line: ranges from the Emerald palette (step -4) to a GSC-style
    // look (step +3). The purple band gets more saturated (negative bandGray)
    // and, for Haunter/Gengar, darker (negative bandLighten), and Gengar's
    // mauve is rotated toward GSC's deep indigo.
    { SPECIES_GASTLY,  8, 0, FP_SCALE * 2, 1400, 0, 0, 0, 0, 0, 0, 0, 0, 215, 14, -10, -40, 0, -80 },
    { SPECIES_HAUNTER, 6, 0, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 0, 0, 0, 205, 12, -6, -50, 0, -80 },
    { SPECIES_GENGAR,  6, 0, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 0, 0, 0, 215, 14, -28, -50, 0, -80 },

    { SPECIES_ONIX, COLOR_VARIATION_MAX_ANGLE, 0, FP_SCALE * 2, FP_SCALE, 148, 0, 3, 8, 0, 56 },
    { SPECIES_STEELIX, COLOR_VARIATION_MAX_ANGLE, 0, FP_SCALE * 2, FP_SCALE, 148, 0, 0, 8, 0, 56 },

    { SPECIES_DROWZEE, COLOR_VARIATION_MAX_ANGLE, 0, FP_SCALE * 2, 940, 0, 0, 0, 0, 0, 0, 0, -32, 30, 13, -14 },
    { SPECIES_HYPNO, 4, 6, FP_SCALE * 2, 940, 0, 0, 0, 0, 0, 0, 0, -32, 30, 13, -14 },

    { SPECIES_KRABBY, 4, -6, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 0, 0, 0, 22, 12, -20 },
    { SPECIES_KINGLER, 4, -6, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 0, 0, 0, 22, 12, -20 },

    { SPECIES_EXEGGCUTE, COLOR_VARIATION_MAX_ANGLE, -3, 900, FP_SCALE, 0, 0, 0, 0, 0, 0, 28 },
    { SPECIES_EXEGGUTOR, COLOR_VARIATION_MAX_ANGLE, 8, 1200, FP_SCALE },

    { SPECIES_CUBONE, 18, -6, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 0, 0, 0, 26, 16 },
    { SPECIES_MAROWAK, 18, -6, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 0, 0, 0, 26, 16 },

    { SPECIES_TYROGUE, COLOR_VARIATION_MAX_ANGLE, 0, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 0, 0, -36, 218, 16, 18, 0 },
    { SPECIES_HITMONCHAN, COLOR_VARIATION_MAX_ANGLE, 0, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 28 },
    { SPECIES_HITMONLEE,  COLOR_VARIATION_MAX_ANGLE, 0, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 0, 28 },
    { SPECIES_HITMONTOP,  10, -3, FP_SCALE * 2, 1229, 0, 0, 0, 0, 0, 0, 16 },

    { SPECIES_LICKITUNG, 12, -16, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 0, 0, 0, 9, 8, 0, 24 },

    { SPECIES_KOFFING,  24, 0, FP_SCALE * 2, FP_SCALE },
    { SPECIES_WEEZING,  24, 0, FP_SCALE * 2, FP_SCALE },

    { SPECIES_RHYHORN, COLOR_VARIATION_MAX_ANGLE, 0, FP_SCALE * 2, FP_SCALE, 148, 0, 0, 8, 0, 56 },
    { SPECIES_RHYDON, COLOR_VARIATION_MAX_ANGLE, 0, FP_SCALE * 2, FP_SCALE, 148, 0, 4, 8, 0, 56 },

    { SPECIES_TANGELA, 8, 0, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 0, -12, 0, 141, 8, 32, 14 },

    { SPECIES_GOLDEEN, 5, -9, FP_SCALE * 2, 1229, 0, 0, 0, 0, 24, 0, 0, 0, 9, 6, 0, 0, -64 },
    { SPECIES_SEAKING, 5, -9, FP_SCALE * 2, 1229, 0, 0, 0, 0, 24, 0, 0, 0, 9, 6, 0, 0, -64 },
    
    { SPECIES_STARYU, 10, 3, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 0, 0, 0, 210, 8, 0, 30, -20 },
    { SPECIES_STARMIE, 10, 3, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 0, 0, 0, 210, 8, 0, 30, -20 },

    // Mr. Mime: reds and pink skin drift from red toward magenta, getting paler and
    // grayer (bandGray) as the steps rise. Rotation stays small so nothing heads
    // toward the green shiny, and no hue goes toward yellow.
    { SPECIES_MR_MIME, 6, 0, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 0, 0, 0, 6, 8, -25, 14, 0, 32 },

    { SPECIES_SCYTHER, 6, 0, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 0, 0, 0, 63, 8, 16, 0, 10 },
    { SPECIES_SCIZOR, 8, -12, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 0, 0, 0, 13, 6, 0, 10, -10 },

    { SPECIES_ELEKID,   0, 0, 900, FP_SCALE, 0, 0, 0, 0, 0, 0, 24, -20 },
    { SPECIES_ELECTABUZZ, COLOR_VARIATION_MAX_ANGLE, 2, FP_SCALE * 2, 1126, 0, 0, 0, 0, 0, 0, 0, -20 },

    { SPECIES_MAGBY, 8, -12, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 0, 0, 0, 13, 6, 0, 10, -10 },
    { SPECIES_MAGMAR, 8, -12, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 0, 0, 0, 13, 6, 0, 10, -10 },

    { SPECIES_TAUROS, 6, -4, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 0, 0, 0, 31, 6, 0, 12, -16 },

    // Lapras: the blue body keeps its full hue rotation (cyan and light blue),
    // but bandMaxUp stops it from drifting toward the purple shiny in either
    // the normal or split-hue modes.
    { SPECIES_LAPRAS, COLOR_VARIATION_MAX_ANGLE, 0, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 0, 0, 0, 152, 6, 0, 0, 0, 0, 4 },

    { SPECIES_EEVEE, 0, 0, 1126, FP_SCALE, 0, 0, 0, 0, 0, -12, 32, 0, 28, 11, 0, 0, -4 },
    { SPECIES_VAPOREON, 6, 6, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 0, 0, 0, 118, 6, 30 },
    { SPECIES_JOLTEON,   0, 0, 900, FP_SCALE, 0, 0, 0, 0, 0, 0, 24, -20 },
    { SPECIES_FLAREON, 8, -12, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 0, 0, 0, 13, 6, 0, 10, -10 },
    { SPECIES_ESPEON, 12, 0, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { SPECIES_UMBREON, 8, 6, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 40, 0 },

    { SPECIES_PORYGON, 17, -6, 1024, FP_SCALE, 0, 0, 0, 0, 0, 0, 0, 0, 145, 15, 20, 0, 0, 24 },
    { SPECIES_PORYGON2, 17, -6, 1024, FP_SCALE, 0, 0, 0, 0, 0, 0, 0, 0, 145, 15, 20, 0, 0, 24 },

    { SPECIES_OMANYTE, COLOR_VARIATION_MAX_ANGLE, 0, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 0, 0, 10, 123, 12, 30, 0, 0, 0, -4 },
    { SPECIES_OMASTAR, COLOR_VARIATION_MAX_ANGLE, 0, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 0, 0, 10, 123, 12, 30, 0, 0, 0, -4 },

    { SPECIES_KABUTO, 20, 0, FP_SCALE * 2, 850, 0, -30, 0, 0, 0, 0, 24 },
    { SPECIES_KABUTOPS, 18, -16, FP_SCALE * 2, 880, 0, 30, 0, 0, 0, 0, 0, -32 },

    { SPECIES_AERODACTYL, COLOR_VARIATION_MAX_ANGLE, 0, FP_SCALE * 2, FP_SCALE, 148, 0, 4, 6, 0, 10, 0, 10, 196, 15, -10 }, 

    { SPECIES_SNORLAX, 10, -6, FP_SCALE * 2, FP_SCALE, 80, 4, 6, 30, 0 },

    { SPECIES_ARTICUNO, 6, 6, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 0, 0, 0, 118, 6, 30 },
    { SPECIES_ZAPDOS,   0, 0, 900, FP_SCALE, 0, 0, 0, 0, 0, 0, 24, -15 },
    { SPECIES_MOLTRES, 8, -12, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 0, 0, 0, 13, 6, 0, 10, -10 },

    { SPECIES_DRATINI, 6, -6, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 0, 0, 0, 192, 6, -30 },
    //Dragonair was fine
    { SPECIES_DRAGONITE, COLOR_VARIATION_MAX_ANGLE, -16, FP_SCALE * 2, FP_SCALE },

    { SPECIES_MEWTWO, 12, 0, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 0, 0, 0, 197, 6, -20 },
    { SPECIES_MEW, COLOR_VARIATION_MAX_ANGLE,  0, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 0, 36 },

    { SPECIES_CHIKORITA, 6, 0, FP_SCALE, FP_SCALE, 0, 0, 0, 0, 20, 0, 0, 0, 52, 8, 33 },
    { SPECIES_BAYLEEF, 6, 0, FP_SCALE, FP_SCALE, 0, 0, 0, 0, 0, 0, 0, 0, 52, 8, 33 },
    { SPECIES_MEGANIUM, 6, 6, 1228, FP_SCALE, 0, 0, 0, 0, 0, 0, 0, 0, 251, 14, -20, 28, 0, 0, -24, 1 },

    { SPECIES_CYNDAQUIL, 8, -12, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 0, 0, 0, 13, 6, 0, 10, -10 },
    { SPECIES_QUILAVA, 8, -12, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 0, 0, 0, 13, 6, 0, 10, -10 },
    { SPECIES_TYPHLOSION, 8, -12, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 0, 0, 0, 13, 6, 0, 10, -10 },

    { SPECIES_SENTRET, 0, 0, 1126, FP_SCALE, 0, 0, 0, 0, 0, -12, 28, 0, 16, 11, 0, 0, -6 },
    { SPECIES_FURRET, 0, 0, 1126, FP_SCALE, 0, 0, 0, 0, 0, -12, 28, 0, 35, 11, 0, 0, -6 },

    { SPECIES_HOOTHOOT, 0, 0, 1126, FP_SCALE, 0, 0, 0, 0, 0, 6, 25, 0, 36, 16, 0, 0, 10 },
    { SPECIES_NOCTOWL, 0, 0, 1126, FP_SCALE, 0, 0, 0, 0, 0, 18, 25, 0, 36, 16, 0, 0, 10 },

    { SPECIES_LEDYBA, 18, 12, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 0, 0, 0, 23, 16, -40, 0, 0, 0, 4 },
    { SPECIES_LEDIAN, COLOR_VARIATION_MAX_ANGLE, -18, FP_SCALE * 2, FP_SCALE },

    { SPECIES_TOGEPI, 0, 0, FP_SCALE, FP_SCALE, 134, 4, 6, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1 },
    { SPECIES_TOGETIC, 0, 0, FP_SCALE, FP_SCALE, 134, 4, 3, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1 },

    { SPECIES_NATU, 4, -6, 1228, 880, 0, 0, 0, 0, 0, 0, 0, 0, 81, 8, 5, 16, 0, -24 },
    { SPECIES_XATU, 4, -6, 1228, 880, 0, 0, 0, 0, 0, 0, 0, 0, 81, 8, 5, 16, 0, -24 },

    { SPECIES_SUDOWOODO, 8, -16, FP_SCALE * 2, FP_SCALE },

    //{ species, maxAngle, angleBias,
    // satCap, satMul, tintHue, 
    //tintSpread, tintStrength, tintDarken,
    // tintLighten, grayFade, whiteFade,
    // brownFade, bandHue, bandWidth,
    // bandShift, bandLighten, bandDarken,
    // bandGray, bandMaxUp, holdBias,
    // tintRamp, tintWhites },

    
    
    


    // Zigzagoon: hue rotation would turn it yellow/olive (positive steps) or
    // red (negative steps). Instead, positive steps fade subtly toward gray
    // and negative steps lighten slightly toward white.
    { SPECIES_ZIGZAGOON, COLOR_VARIATION_MAX_ANGLE, 0, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 24, -20 },
    { SPECIES_LINOONE, COLOR_VARIATION_MAX_ANGLE, 0, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 0, 0, 24, -20 },

    // Shuppet line: body is almost pure gray, so only the eyes/details react
    // to the hue rotation. Spread individuals from the original gray (hue
    // step +3) down to near-black (step -4); no tint.
    { SPECIES_SHUPPET, COLOR_VARIATION_MAX_ANGLE, 0, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 40, 0 },
    { SPECIES_BANETTE, COLOR_VARIATION_MAX_ANGLE, 0, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 40, 0 },

    // Duskull line: middle ground between the Shuppet and Zigzagoon setups.
    // Negative steps darken the grays (stopping at dark gray so Dusclops
    // doesn't become a silhouette); positive steps fade slightly toward white
    // instead of rotating hue (which would turn Duskull's skull green).
    { SPECIES_DUSKULL,  COLOR_VARIATION_MAX_ANGLE, 0, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 28, 0, 0, 20 },
    { SPECIES_DUSCLOPS, COLOR_VARIATION_MAX_ANGLE, 0, FP_SCALE * 2, FP_SCALE, 0, 0, 0, 16, 0, 16, 14 },
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
            if (gray > TINT_FADE_START)
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

static void ApplyHueBand(u16 *palette, const struct ColorVariationOverride *override, s32 signedStep)
{
    s32 stepShift = override->bandShift * (signedStep + 4) / 7;
    s32 stepLighten = override->bandLighten * (signedStep + 4) / 7;
    s32 stepDarken = GetBandDarkenAmount(override->bandDarken, signedStep);
    s32 stepGray = override->bandGray * (signedStep + 4) / 7;
    u32 i;

    if (stepShift == 0 && stepLighten == 0 && stepDarken == 0 && stepGray == 0)
        return;

    for (i = 1; i < 16; i++)
    {
        s32 r = (palette[i] >>  0) & 0x1F;
        s32 g = (palette[i] >>  5) & 0x1F;
        s32 b = (palette[i] >> 10) & 0x1F;
        s32 hue = GetColorHue256(r, g, b);
        s32 dist, angle, lighten, darken, desat, max;
        struct HueMatrix mat;

        if (hue < 0)
            continue;

        dist = (hue - override->bandHue) & 0xFF;
        if (dist > 128)
            dist = 256 - dist;
        if (dist > override->bandWidth + BAND_FEATHER)
            continue;

        angle = stepShift;
        lighten = stepLighten;
        darken = stepDarken;
        desat = stepGray;
        if (dist > override->bandWidth)
        {
            angle = angle * (override->bandWidth + BAND_FEATHER - dist) / BAND_FEATHER;
            lighten = lighten * (override->bandWidth + BAND_FEATHER - dist) / BAND_FEATHER;
            darken = darken * (override->bandWidth + BAND_FEATHER - dist) / BAND_FEATHER;
            desat = desat * (override->bandWidth + BAND_FEATHER - dist) / BAND_FEATHER;
        }

        if (angle != 0)
        {
            ComputeHueMatrix(angle & 0xFF, &mat);
            RotateColor(&palette[i], &mat);
        }

        if (lighten > 0)
        {
            r = (palette[i] >>  0) & 0x1F;
            g = (palette[i] >>  5) & 0x1F;
            b = (palette[i] >> 10) & 0x1F;
            // Scale by brightness so dark shading stays darker than the highlights.
            max = r;
            if (g > max) max = g;
            if (b > max) max = b;
            r += (31 - r) * lighten * max / (TINT_LIGHT_ONE * 31);
            g += (31 - g) * lighten * max / (TINT_LIGHT_ONE * 31);
            b += (31 - b) * lighten * max / (TINT_LIGHT_ONE * 31);
            palette[i] = (u16)(r | (g << 5) | (b << 10));
        }
        else if (lighten < 0)
        {
            // Negative bandLighten: straight blend toward black.
            r = (palette[i] >>  0) & 0x1F;
            g = (palette[i] >>  5) & 0x1F;
            b = (palette[i] >> 10) & 0x1F;
            if (lighten < -TINT_LIGHT_ONE)
                lighten = -TINT_LIGHT_ONE;
            r += (r * lighten) / TINT_LIGHT_ONE;
            g += (g * lighten) / TINT_LIGHT_ONE;
            b += (b * lighten) / TINT_LIGHT_ONE;
            palette[i] = (u16)(r | (g << 5) | (b << 10));
        }

        if (darken != 0)
        {
            // Straight blend toward black: only band colors reach this, so near-white
            // highlights and neutral grays are never affected.
            r = (palette[i] >>  0) & 0x1F;
            g = (palette[i] >>  5) & 0x1F;
            b = (palette[i] >> 10) & 0x1F;
            r -= (r * darken) / TINT_LIGHT_ONE;
            g -= (g * darken) / TINT_LIGHT_ONE;
            b -= (b * darken) / TINT_LIGHT_ONE;
            palette[i] = (u16)(r | (g << 5) | (b << 10));
        }

        if (desat != 0)
            AdjustSaturation(&palette[i], FP_SCALE * (TINT_LIGHT_ONE - desat) / TINT_LIGHT_ONE);
    }
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
// If the species sets bandMaxUp, positive rotation beyond that limit is trimmed
// for colors inside the hue band. Everything else uses the precomputed matrix.
static void RotateColorBandCapped(u16 *color, s32 angleIndex, const struct HueMatrix *mat, const struct ColorVariationOverride *override)
{
    s32 signedAngle = angleIndex & 0xFF;
    s32 r, g, b, hue, dist, reach;
    struct HueMatrix capped;

    if (signedAngle > 128)
        signedAngle -= 256;

    if (override == NULL || override->bandMaxUp == 0 || override->bandWidth == 0 || signedAngle <= override->bandMaxUp)
    {
        RotateColor(color, mat);
        return;
    }

    r = (*color >>  0) & 0x1F;
    g = (*color >>  5) & 0x1F;
    b = (*color >> 10) & 0x1F;
    hue = GetColorHue256(r, g, b);
    if (hue >= 0)
    {
        dist = (hue - override->bandHue) & 0xFF;
        if (dist > 128)
            dist = 256 - dist;
        // The band may already have shifted this color, so allow for that.
        reach = override->bandWidth + BAND_FEATHER + (override->bandShift < 0 ? -override->bandShift : override->bandShift);
        if (dist <= reach)
        {
            ComputeHueMatrix(override->bandMaxUp & 0xFF, &capped);
            RotateColor(color, &capped);
            return;
        }
    }
    RotateColor(color, mat);
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
    // A negative bandMaxUp pushes band colors down on every step, including
    // steps that otherwise wouldn't rotate at all (step 0 with no angleBias).
    bool8 forcedCap = (override != NULL && override->bandWidth != 0 && override->bandMaxUp < 0);
    s32 vividFactor = COLOR_VARIATION_SAT_VIVID;
    s32 coolIndex;
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

    if (override != NULL && override->bandWidth != 0)
        ApplyHueBand(palette, override, signedStep);

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

        // Apply hue rotation
        if (hasHue)
        {
            if (hasSplit && isSaturated)
            {
                // Split: warm rotates positive, cool rotates negative
                if (bucket == 0)
                    RotateColorBandCapped(&palette[i], angleIndex, &matPos, override);
                else
                    RotateColorBandCapped(&palette[i], coolIndex, &matNeg, override);
            }
            else if (!hasSplit)
            {
                RotateColorBandCapped(&palette[i], angleIndex, &matPos, override);
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
    struct HueMatrix mat;
    struct SideFades fades;
    bool8 hasFade = GetSideFades(override, hue * 3, &fades);
    u32 i;

    // Icons only know the hue direction; ±3 steps approximates the middle of
    // the main sprite's positive (+2..+3) and negative (-2..-4) step ranges.
    if (override != NULL && (override->tintStrength != 0 || override->tintDarken != 0 || override->tintLighten != 0))
        ApplyNeutralTint(palette, override, hue * 3);

    if (override != NULL && override->bandWidth != 0)
        ApplyHueBand(palette, override, hue * 3);

    if (hasFade)
    {
        for (i = 1; i < 16; i++)
            ApplySideFades(&palette[i], &fades);
    }
    else if (hue != 0)
    {
        ComputeHueMatrix((hue > 0) ? COLOR_VARIATION_ICON_ANGLE : 256 - COLOR_VARIATION_ICON_ANGLE, &mat);
        for (i = 1; i < 16; i++)
            RotateColor(&palette[i], &mat);
    }

    if (override != NULL && override->satMul != FP_SCALE)
    {
        for (i = 1; i < 16; i++)
            AdjustSaturation(&palette[i], override->satMul);
    }
}
