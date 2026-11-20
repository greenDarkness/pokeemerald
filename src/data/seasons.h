// Seasonal route weather.
//
// Each map, or group of maps that share their weather, gets one block that sets its weather
// for the whole year. Blocks go inside each other, and are written one after another with no
// commas between them:
//
//     MAP(map, ...)          the map, e.g. MAP_ROUTE101, then everything for that map
//     MAP(map, map, ...)     several maps that share the same weather, each followed by a
//                            comma, then everything for those maps
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
// WEATHER_DROUGHT, WEATHER_FOG_HORIZONTAL, WEATHER_WINDY, WEATHER_ROUTE119_CYCLE, ...), or
// WEATHER_REGULAR to use the map's normal weather (the one set in Porymap).
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
// its weather, then add the days and years that should be different. This also goes for a map
// that is in more than one block, e.g. a block of its own after a block it shares.
//
// The weather is picked when the map loads (entering it, or walking in from a connected map),
// so a new day or season shows on the next map change. A map script that uses setweather still
// overrides it, and resetweather goes back to the seasonal weather.
//
// Some maps change weather as you walk, with weather trigger tiles set in Porymap (Routes 113,
// 119, 120, 123 and Jagged Pass). On those maps the seasonal weather only covers the normal
// part: tiles that set the map's normal weather set the seasonal weather instead, but the
// special areas (Route 119's and 123's rain cycle, Route 113's ash, ...) keep their own weather.
// Leaving a building on Route 119 or 123 also still starts its rain cycle, as those buildings
// are in the rainy area.
//
// The three lists are only for organizing; a map's block works in any of them.
//
// tech/seasonal_weather.py ("Seasonal Weather.bat") writes a map's block from a table of
// seasons and days copied from a spreadsheet. With no map it writes MAP(MAP_UNDEFINED, ...),
// a block that does nothing until MAP_UNDEFINED is replaced with real maps.
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
//     // Routes 102 and 104 both get rain on spring weekends.
//     MAP(MAP_ROUTE102, MAP_ROUTE104,
//         SPRING(WEATHER(WEATHER_RAIN, DAY_TAG_WEEKEND))
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

//WEATHER_NONE - Instant weather change
//WEATHER_SUNNY_CLOUDS
//WEATHER_SUNNY - Gradual Weather change
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
//WEATHER_WINDY - Gusts that stir the trees and tall grass and carry leaves across the screen
//WEATHER_ROUTE119_CYCLE
//WEATHER_ROUTE123_CYCLE

static const u16 sSeasonalWeatherHoenn[] = {
    MAP(MAP_LITTLEROOT_TOWN, MAP_ROUTE101, MAP_OLDALE_TOWN, MAP_ROUTE102, MAP_ROUTE103,
         MAP_PETALBURG_CITY, MAP_ROUTE104, MAP_RUSTBORO_CITY, MAP_ROUTE115, MAP_ROUTE116, MAP_ROUTE110,
         MAP_MAUVILLE_CITY, MAP_ROUTE117, MAP_VERDANTURF_TOWN, MAP_ROUTE112,
        ODD_YEARS(
            SPRING(
                WEATHER(WEATHER_SUNNY, DAYS(1, 3, 5))
                WEATHER(WEATHER_DOWNPOUR, DAYS(2))
                WEATHER(WEATHER_RAIN_THUNDERSTORM, DAYS(4))
                WEATHER(WEATHER_RAIN, DAYS(6))
                WEATHER(WEATHER_SUNNY_CLOUDS, DAYS(7))
            )
            SUMMER(
                WEATHER(WEATHER_DROUGHT, DAYS(1, 6-7))
                WEATHER(WEATHER_SUNNY_CLOUDS, DAYS(2, 4))
                WEATHER(WEATHER_RAIN, DAYS(3))
                WEATHER(WEATHER_WINDY, DAYS(5))
            )
            FALL(
                WEATHER(WEATHER_SUNNY, DAYS(1, 4))
                WEATHER(WEATHER_WINDY, DAYS(2-3, 5, 7))
                WEATHER(WEATHER_FLURRIES, DAYS(6))
            )
            WINTER(
                WEATHER(WEATHER_FLURRIES, DAYS(1))
                WEATHER(WEATHER_SUNNY, DAYS(2, 5))
                WEATHER(WEATHER_SNOW, DAYS(3-4, 6-7))
            )
        )
        EVEN_YEARS(
            SPRING(
                WEATHER(WEATHER_DOWNPOUR, DAYS(1))
                WEATHER(WEATHER_SUNNY, DAYS(2, 4, 6))
                WEATHER(WEATHER_RAIN_THUNDERSTORM, DAYS(3))
                WEATHER(WEATHER_RAIN, DAYS(5, 7))
            )
            SUMMER(
                WEATHER(WEATHER_RAIN, DAYS(1))
                WEATHER(WEATHER_SUNNY_CLOUDS, DAYS(2-3))
                WEATHER(WEATHER_WINDY, DAYS(4, 6))
                WEATHER(WEATHER_DROUGHT, DAYS(5, 7))
            )
            FALL(
                WEATHER(WEATHER_WINDY, DAYS(1, 4, 6))
                WEATHER(WEATHER_RAIN, DAYS(2))
                WEATHER(WEATHER_SUNNY, DAYS(3, 5))
                WEATHER(WEATHER_FLURRIES, DAYS(7))
            )
            WINTER(
                WEATHER(WEATHER_SUNNY, DAYS(1))
                WEATHER(WEATHER_FLURRIES, DAYS(2))
                WEATHER(WEATHER_SNOW, DAYS(3-4))
                WEATHER(WEATHER_BLIZZARD, DAYS(5-7))
            )
        )
    )
        MAP(MAP_PETALBURG_WOODS,
        ODD_YEARS(
            SPRING(
                WEATHER(WEATHER_SUNNY, DAYS(1, 3, 5))
                WEATHER(WEATHER_DOWNPOUR, DAYS(2))
                WEATHER(WEATHER_RAIN_THUNDERSTORM, DAYS(4))
                WEATHER(WEATHER_RAIN, DAYS(6))
                WEATHER(WEATHER_SUNNY_CLOUDS, DAYS(7))
            )
            SUMMER(
                WEATHER(WEATHER_SUNNY, DAYS(1, 6-7))
                WEATHER(WEATHER_SUNNY_CLOUDS, DAYS(2, 4))
                WEATHER(WEATHER_RAIN, DAYS(3))
                WEATHER(WEATHER_WINDY, DAYS(5))
            )
            FALL(
                WEATHER(WEATHER_SUNNY, DAYS(1, 4))
                WEATHER(WEATHER_WINDY, DAYS(2-3, 5, 7))
                WEATHER(WEATHER_FLURRIES, DAYS(6))
            )
            WINTER(
                WEATHER(WEATHER_FLURRIES, DAYS(1))
                WEATHER(WEATHER_SUNNY, DAYS(2, 5))
                WEATHER(WEATHER_SNOW, DAYS(3-4, 6-7))
            )
        )
        EVEN_YEARS(
            SPRING(
                WEATHER(WEATHER_DOWNPOUR, DAYS(1))
                WEATHER(WEATHER_SUNNY, DAYS(2, 4, 6))
                WEATHER(WEATHER_RAIN_THUNDERSTORM, DAYS(3))
                WEATHER(WEATHER_RAIN, DAYS(5, 7))
            )
            SUMMER(
                WEATHER(WEATHER_RAIN, DAYS(1))
                WEATHER(WEATHER_SUNNY_CLOUDS, DAYS(2-3))
                WEATHER(WEATHER_WINDY, DAYS(4, 6))
                WEATHER(WEATHER_SUNNY, DAYS(5, 7))
            )
            FALL(
                WEATHER(WEATHER_WINDY, DAYS(1, 4, 6))
                WEATHER(WEATHER_RAIN, DAYS(2))
                WEATHER(WEATHER_SUNNY, DAYS(3, 5))
                WEATHER(WEATHER_FLURRIES, DAYS(7))
            )
            WINTER(
                WEATHER(WEATHER_SUNNY, DAYS(1))
                WEATHER(WEATHER_FLURRIES, DAYS(2))
                WEATHER(WEATHER_SNOW, DAYS(3-4))
                WEATHER(WEATHER_BLIZZARD, DAYS(5-7))
            )
        )
    )
        MAP(MAP_ROUTE105, MAP_ROUTE106, MAP_DEWFORD_TOWN, MAP_ROUTE107, MAP_ROUTE108, MAP_ROUTE109, MAP_SLATEPORT_CITY,
        ODD_YEARS(
            SPRING(
                WEATHER(WEATHER_SUNNY, DAYS(1, 3, 5))
                WEATHER(WEATHER_DOWNPOUR, DAYS(2))
                WEATHER(WEATHER_RAIN_THUNDERSTORM, DAYS(4))
                WEATHER(WEATHER_RAIN, DAYS(6))
                WEATHER(WEATHER_SUNNY_CLOUDS, DAYS(7))
            )
            SUMMER(
                WEATHER(WEATHER_DROUGHT, DAYS(1, 6-7))
                WEATHER(WEATHER_SUNNY_CLOUDS, DAYS(2, 4))
                WEATHER(WEATHER_RAIN, DAYS(3, 5))
            )
            FALL(
                WEATHER(WEATHER_SUNNY, DAYS(1, 4))
                WEATHER(WEATHER_RAIN, DAYS(2-3))
                WEATHER(WEATHER_DOWNPOUR, DAYS(5, 7))
                WEATHER(WEATHER_FLURRIES, DAYS(6))
            )
            WINTER(
                WEATHER(WEATHER_FLURRIES, DAYS(1))
                WEATHER(WEATHER_SUNNY, DAYS(2, 5))
                WEATHER(WEATHER_SNOW, DAYS(3-4, 6-7))
            )
        )
        EVEN_YEARS(
            SPRING(
                WEATHER(WEATHER_DOWNPOUR, DAYS(1))
                WEATHER(WEATHER_SUNNY, DAYS(2, 4, 6))
                WEATHER(WEATHER_RAIN_THUNDERSTORM, DAYS(3))
                WEATHER(WEATHER_RAIN, DAYS(5, 7))
            )
            SUMMER(
                WEATHER(WEATHER_RAIN, DAYS(1, 4, 6))
                WEATHER(WEATHER_SUNNY_CLOUDS, DAYS(2-3))
                WEATHER(WEATHER_DROUGHT, DAYS(5, 7))
            )
            FALL(
                WEATHER(WEATHER_RAIN, DAYS(1-2))
                WEATHER(WEATHER_SUNNY, DAYS(3, 5))
                WEATHER(WEATHER_RAIN_THUNDERSTORM, DAYS(4))
                WEATHER(WEATHER_DOWNPOUR, DAYS(6))
                WEATHER(WEATHER_FLURRIES, DAYS(7))
            )
            WINTER(
                WEATHER(WEATHER_SUNNY, DAYS(1))
                WEATHER(WEATHER_FLURRIES, DAYS(2))
                WEATHER(WEATHER_SNOW, DAYS(3-4))
                WEATHER(WEATHER_BLIZZARD, DAYS(5-7))
            )
        )
    )
        MAP(MAP_ROUTE112, MAP_LAVARIDGE_TOWN, MAP_FALLARBOR_TOWN, MAP_ROUTE114,
        ODD_YEARS(
            SPRING(
                WEATHER(WEATHER_SUNNY, DAYS(1, 3, 5))
                WEATHER(WEATHER_DROUGHT, DAYS(2, 4))
                WEATHER(WEATHER_RAIN, DAYS(6))
                WEATHER(WEATHER_SUNNY_CLOUDS, DAYS(7))
            )
            SUMMER(
                WEATHER(WEATHER_DROUGHT, DAYS(1, 6-7))
                WEATHER(WEATHER_SUNNY_CLOUDS, DAYS(2, 4))
                WEATHER(WEATHER_RAIN, DAYS(3))
                WEATHER(WEATHER_VOLCANIC_ASH, DAYS(5))
            )
            FALL(
                WEATHER(WEATHER_SUNNY, DAYS(1, 4))
                WEATHER(WEATHER_VOLCANIC_ASH, DAYS(2-3, 5, 7))
                WEATHER(WEATHER_FLURRIES, DAYS(6))
            )
            WINTER(
                WEATHER(WEATHER_FLURRIES, DAYS(1, 3, 6-7))
                WEATHER(WEATHER_SUNNY, DAYS(2, 4-5))
            )
        )
        EVEN_YEARS(
            SPRING(
                WEATHER(WEATHER_DROUGHT, DAYS(1, 3))
                WEATHER(WEATHER_SUNNY, DAYS(2, 4, 6))
                WEATHER(WEATHER_RAIN, DAYS(5, 7))
            )
            SUMMER(
                WEATHER(WEATHER_RAIN, DAYS(1))
                WEATHER(WEATHER_SUNNY_CLOUDS, DAYS(2-3))
                WEATHER(WEATHER_VOLCANIC_ASH, DAYS(4, 6))
                WEATHER(WEATHER_DROUGHT, DAYS(5, 7))
            )
            FALL(
                WEATHER(WEATHER_VOLCANIC_ASH, DAYS(1, 4, 6))
                WEATHER(WEATHER_RAIN, DAYS(2))
                WEATHER(WEATHER_SUNNY, DAYS(3, 5))
                WEATHER(WEATHER_FLURRIES, DAYS(7))
            )
            WINTER(
                WEATHER(WEATHER_SUNNY, DAYS(1-2))
                WEATHER(WEATHER_FLURRIES, DAYS(3-4))
                WEATHER(WEATHER_SNOW, DAYS(5-7))
            )
        )
    )



};