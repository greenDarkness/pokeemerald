// Seasonal route weather.
//
// Each entry sets a map's weather for some seasons and, optionally, some days of those seasons:
//
//     .map       the map, e.g. MAP_ROUTE101
//     .seasons   seasons it applies to, season tags joined with | (leave out = every season):
//                    SEASON_TAG_SPRING   SEASON_TAG_SUMMER   SEASON_TAG_FALL   SEASON_TAG_WINTER
//                    SEASON_TAG_ALL
//     .days      days of the season it applies to, day tags joined with | (leave out = every day):
//                    DAY_TAG_MONDAY     (day 1)   DAY_TAG_TUESDAY  (day 2)   DAY_TAG_WEDNESDAY (day 3)
//                    DAY_TAG_THURSDAY   (day 4)   DAY_TAG_FRIDAY   (day 5)   DAY_TAG_SATURDAY  (day 6)
//                    DAY_TAG_SUNDAY     (day 7)   DAY_TAG_WEEKDAYS (days 1-5) DAY_TAG_WEEKEND  (days 6-7)
//                    DAY_TAG_ALL
//     .weather   any WEATHER_* from include/constants/weather.h (WEATHER_RAIN, WEATHER_SNOW,
//                WEATHER_DROUGHT, WEATHER_FOG_HORIZONTAL, WEATHER_ROUTE119_CYCLE, ...), or
//                WEATHER_REGULAR to use the map's normal weather (the one set in Porymap).
//
// Seasons are one week long and start on Monday, so the days of a season are Monday-Sunday.
// Maps with no entry, and days no entry covers, keep their normal weather.
//
// If several entries for a map apply on the same day, the most specific one wins: an entry
// with .days beats one with only .seasons, which beats one with neither. If they're equally
// specific, the one listed first wins. So give a map its season weather, then add entries for
// the days that should be different.
//
// The weather is picked when the map loads (entering it, or walking in from a connected map),
// so a new day or season shows on the next map change. A map script that uses setweather still
// overrides it, and resetweather goes back to the seasonal weather.
//
// The three lists are only for organizing; an entry works in any of them.
//
// Example (Route 101):
//
// static const struct SeasonalWeather sSeasonalWeatherHoenn[] = {
//     // Spring: normal weather, except rain on days 1 and 2.
//     {
//         .map = MAP_ROUTE101,
//         .seasons = SEASON_TAG_SPRING,
//         .weather = WEATHER_REGULAR,                        // same as leaving spring out
//     },
//     {
//         .map = MAP_ROUTE101,
//         .seasons = SEASON_TAG_SPRING,
//         .days = DAY_TAG_MONDAY | DAY_TAG_TUESDAY,
//         .weather = WEATHER_RAIN,
//     },
//     // Summer: harsh sun all week.
//     {
//         .map = MAP_ROUTE101,
//         .seasons = SEASON_TAG_SUMMER,
//         .weather = WEATHER_DROUGHT,
//     },
//     // Fall: fog on the weekend, normal weather the rest of the week.
//     {
//         .map = MAP_ROUTE101,
//         .seasons = SEASON_TAG_FALL,
//         .days = DAY_TAG_WEEKEND,
//         .weather = WEATHER_FOG_HORIZONTAL,
//     },
//     // Winter: snow, but normal weather on Sunday.
//     {
//         .map = MAP_ROUTE101,
//         .seasons = SEASON_TAG_WINTER,
//         .weather = WEATHER_SNOW,
//     },
//     {
//         .map = MAP_ROUTE101,
//         .seasons = SEASON_TAG_WINTER,
//         .days = DAY_TAG_SUNDAY,
//         .weather = WEATHER_REGULAR,
//     },
// };

// ================================================================ Kanto

static const struct SeasonalWeather sSeasonalWeatherKanto[] = {
    // {
    //     .map = MAP_ROUTE1,
    //     .seasons = SEASON_TAG_WINTER,
    //     .weather = WEATHER_SNOW,
    // },
};

// ================================================================ Johto

static const struct SeasonalWeather sSeasonalWeatherJohto[] = {
    // {
    //     .map = MAP_ROUTE29,
    //     .seasons = SEASON_TAG_SPRING,
    //     .days = DAY_TAG_WEEKEND,
    //     .weather = WEATHER_RAIN,
    // },
};

// ================================================================ Hoenn

//WEATHER_NONE
//WEATHER_SUNNY_CLOUDS
//WEATHER_SUNNY - Regular weather?
//WEATHER_RAIN
//WEATHER_SNOW
//WEATHER_RAIN_THUNDERSTORM
//WEATHER_FOG_HORIZONTAL
//WEATHER_VOLCANIC_ASH
//WEATHER_SANDSTORM
//WEATHER_FOG_DIAGONAL
//WEATHER_UNDERWATER
//WEATHER_SHADE
//WEATHER_DROUGHT
//WEATHER_DOWNPOUR
//WEATHER_UNDERWATER_BUBBLES
//WEATHER_ABNORMAL
//WEATHER_FLURRIES - WEATHER_SNOW without the darkening
//WEATHER_ROUTE119_CYCLE
//WEATHER_ROUTE123_CYCLE

static const struct SeasonalWeather sSeasonalWeatherHoenn[] = {
    {
         .map = MAP_ROUTE103,
    //     .seasons = SEASON_TAG_SUMMER,
         .weather = WEATHER_SNOW,
    },
};
