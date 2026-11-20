// Seasonal route weather.
//
// Each map gets one block that sets its weather for the whole year. Blocks go inside each
// other, and are written one after another with no commas between them:
//
//     MAP(map, ...)          the map, e.g. MAP_ROUTE101, then everything for that map
//     SPRING(...)            only in that season
//     SUMMER(...)
//     FALL(...)
//     WINTER(...)
//     ODD_YEARS(...)         only in years 1, 3, 5, ... (the game starts in year 1)
//     EVEN_YEARS(...)        only in years 2, 4, 6, ...
//     WEATHER(weather)       the weather, on every day the blocks around it allow
//     WEATHER(weather, days) the same, but only on some days of the season
//
// The season and year blocks can go in either order and inside each other as deep as you
// like; a WEATHER(...) applies when every block around it does.
//
// weather is any WEATHER_* from include/constants/weather.h (WEATHER_RAIN, WEATHER_SNOW,
// WEATHER_DROUGHT, WEATHER_FOG_HORIZONTAL, WEATHER_ROUTE119_CYCLE, ...), or WEATHER_REGULAR
// to use the map's normal weather (the one set in Porymap).
//
// days is day numbers, DAYS(1-3, 7) = days 1 to 3 and day 7 (each item is one day or a
// first-last range), or day tags joined with |:
//     DAY_TAG_MONDAY     (day 1)   DAY_TAG_TUESDAY  (day 2)   DAY_TAG_WEDNESDAY (day 3)
//     DAY_TAG_THURSDAY   (day 4)   DAY_TAG_FRIDAY   (day 5)   DAY_TAG_SATURDAY  (day 6)
//     DAY_TAG_SUNDAY     (day 7)   DAY_TAG_WEEKDAYS (days 1-5) DAY_TAG_WEEKEND  (days 6-7)
//
// Seasons are one week long and start on Monday, so the days of a season are Monday-Sunday.
// Maps with no block, and days nothing in the block covers, keep their normal weather.
//
// If several WEATHER(...) for a map apply on the same day, the most specific one wins: one
// with days beats one inside only a season block, which beats one inside only a year block,
// which beats one with none of them. Between two that are otherwise alike, the one inside a
// year block wins. If they're equally specific, the one listed first wins. So give a season
// its weather, then add the days and years that should be different.
//
// The weather is picked when the map loads (entering it, or walking in from a connected map),
// so a new day or season shows on the next map change. A map script that uses setweather still
// overrides it, and resetweather goes back to the seasonal weather.
//
// The three lists are only for organizing; a map's block works in any of them.
//
// Example (Route 101):
//
// static const u16 sSeasonalWeatherHoenn[] = {
//     MAP(MAP_ROUTE101,
//         // Spring: normal weather, except a thunderstorm on days 1-2 in odd years and
//         // rain on days 3-4 in even years.
//         SPRING(
//             ODD_YEARS(WEATHER(WEATHER_RAIN_THUNDERSTORM, DAYS(1-2)))
//             EVEN_YEARS(WEATHER(WEATHER_RAIN, DAYS(3-4)))
//         )
//         // Summer: harsh sun all week.
//         SUMMER(WEATHER(WEATHER_DROUGHT))
//         // Fall: fog on the weekend, normal weather the rest of the week.
//         FALL(WEATHER(WEATHER_FOG_HORIZONTAL, DAY_TAG_WEEKEND))
//         // Winter: snow, but normal weather on days 1 to 3 and day 7.
//         WINTER(
//             WEATHER(WEATHER_SNOW)
//             WEATHER(WEATHER_REGULAR, DAYS(1-3, 7))
//         )
//     )
// };

// ================================================================ Kanto

static const u16 sSeasonalWeatherKanto[] = {
    // MAP(MAP_ROUTE1,
    //     WINTER(WEATHER(WEATHER_SNOW))
    // )
};

// ================================================================ Johto

static const u16 sSeasonalWeatherJohto[] = {
    // MAP(MAP_ROUTE29,
    //     SPRING(WEATHER(WEATHER_RAIN, DAY_TAG_WEEKEND))
    // )
};

// ================================================================ Hoenn

//WEATHER_NONE
//WEATHER_SUNNY_CLOUDS
//WEATHER_SUNNY - Regular weather?
//WEATHER_RAIN
//WEATHER_SNOW - Wind-blown snowfall
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
//WEATHER_BLIZZARD - Snow driven sideways by gusts
//WEATHER_DIAMOND_DUST - Ice crystals glittering in clear air
//WEATHER_ROUTE119_CYCLE
//WEATHER_ROUTE123_CYCLE

static const u16 sSeasonalWeatherHoenn[] = {
    MAP(MAP_ROUTE103,
        ODD_YEARS(
        SPRING(WEATHER(WEATHER_NONE, DAYS(2,4,6)))
        SUMMER(WEATHER(WEATHER_NONE, DAYS(1,3,5,7)))
        FALL(WEATHER(WEATHER_NONE, DAYS(2,4,6)))
        WINTER(WEATHER(WEATHER_NONE, DAYS(1,3,5,7)))
        )
        EVEN_YEARS(
        SPRING(WEATHER(WEATHER_NONE, DAYS(1,3,5,7)))
        SUMMER(WEATHER(WEATHER_NONE, DAYS(2,4,6)))
        FALL(WEATHER(WEATHER_DIAMOND_DUST, DAYS(1,3,5,7)))
        WINTER(WEATHER(WEATHER_NONE, DAYS(2,4,6)))
        )
    )
};