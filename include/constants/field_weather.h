#ifndef GUARD_CONSTANTS_FIELD_WEATHER_H
#define GUARD_CONSTANTS_FIELD_WEATHER_H

// sPaletteColorMapTypes & field_effect_scripts
#define COLOR_MAP_NONE          0
#define COLOR_MAP_DARK_CONTRAST 1
#define COLOR_MAP_CONTRAST      2

#define MAX_RAIN_SPRITES             24
#define NUM_CLOUD_SPRITES            3
#define NUM_FOG_HORIZONTAL_SPRITES   20
#define NUM_ASH_SPRITES              20
#define NUM_FOG_DIAGONAL_SPRITES     20
#define NUM_SANDSTORM_SPRITES        20
#define NUM_SWIRL_SANDSTORM_SPRITES  5
#define NUM_SNOWFLAKE_SPRITES        24
#define NUM_BLIZZARD_FLAKE_SPRITES   10
#define NUM_BLIZZARD_OVERLAY_SPRITES 20
#define NUM_DIAMOND_DUST_SPRITES     24
#define NUM_LEAF_SPRITES             16

// The map tiles that move in WEATHER_WINDY and WEATHER_BLIZZARD (see the wind section of
// src/tileset_anims.c)
#define WIND_TILES_GRASS    0 // tall grass
#define WIND_TILES_FOLIAGE  1 // trees and bushes
#define WIND_TILES_COUNT    2

// What they look like
#define WIND_FRAME_STILL    0
#define WIND_FRAME_BREEZE   1
#define WIND_FRAME_GUST     2
#define WIND_FRAMES         3

// Controls how the weather should be changing the screen palettes.
#define WEATHER_PAL_STATE_CHANGING_WEATHER   0
#define WEATHER_PAL_STATE_SCREEN_FADING_IN   1
#define WEATHER_PAL_STATE_SCREEN_FADING_OUT  2
#define WEATHER_PAL_STATE_IDLE               3

// Modes for FadeScreen
#define FADE_FROM_BLACK  0
#define FADE_TO_BLACK    1
#define FADE_FROM_WHITE  2
#define FADE_TO_WHITE    3

#endif // GUARD_CONSTANTS_FIELD_WEATHER_H
