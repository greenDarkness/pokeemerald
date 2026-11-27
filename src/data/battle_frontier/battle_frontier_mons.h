const struct FacilityMon gBattleFrontierMons[NUM_FRONTIER_MONS] =
{
    [FRONTIER_MON_SUNKERN] = {
        .species = SPECIES_SUNKERN,
        .moves = {MOVE_MEGA_DRAIN, MOVE_HELPING_HAND, MOVE_SUNNY_DAY, MOVE_LIGHT_SCREEN},
        .itemTableId = BATTLE_FRONTIER_ITEM_LAX_INCENSE,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_HP,
        .nature = NATURE_RELAXED
    },
    [FRONTIER_MON_AZURILL] = {
        .species = SPECIES_AZURILL,
        .moves = {MOVE_WATER_PULSE, MOVE_ATTRACT, MOVE_SING, MOVE_CHARM},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHERI_BERRY,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_HP,
        .nature = NATURE_RASH
    },
    [FRONTIER_MON_CATERPIE] = {
        .species = SPECIES_CATERPIE,
        .moves = {MOVE_TACKLE, MOVE_STRING_SHOT, MOVE_NONE, MOVE_NONE},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {236, 196, 0, 0, 0, 76},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_SHIELD_DUST
    },
    [FRONTIER_MON_WEEDLE] = {
        .species = SPECIES_WEEDLE,
        .moves = {MOVE_POISON_STING, MOVE_STRING_SHOT, MOVE_NONE, MOVE_NONE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_HP,
        .evs = {196, 0, 0, 0, 196, 36},
        .nature = NATURE_CAREFUL,
        .ability = ABILITY_SHIELD_DUST
    },
    [FRONTIER_MON_WURMPLE] = {
        .species = SPECIES_WURMPLE,
        .moves = {MOVE_TACKLE, MOVE_STRING_SHOT, MOVE_POISON_STING, MOVE_NONE},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {236, 236, 0, 0, 0, 36},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_SHIELD_DUST
    },
    [FRONTIER_MON_RALTS] = {
        .species = SPECIES_RALTS,
        .moves = {MOVE_MEMENTO, MOVE_MUD_SLAP, MOVE_THUNDER_WAVE, MOVE_ATTRACT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LUM_BERRY,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 0, 0, 252, 0},
        .nature = NATURE_CALM,
        .ability = ABILITY_TRACE
    },
    [FRONTIER_MON_MAGIKARP] = {
        .species = SPECIES_MAGIKARP,
        .moves = {MOVE_FLAIL, MOVE_TACKLE, MOVE_NONE, MOVE_NONE},
        .itemTableId = BATTLE_FRONTIER_ITEM_SILK_SCARF,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {0, 196, 0, 0, 116, 196},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_SWIFT_SWIM
    },
    [FRONTIER_MON_FEEBAS] = {
        .species = SPECIES_FEEBAS,
        .moves = {MOVE_PROTECT, MOVE_TOXIC, MOVE_DIVE, MOVE_ATTRACT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {196, 0, 196, 0, 76, 0},
        .nature = NATURE_BOLD,
        .ability = ABILITY_SWIFT_SWIM
    },
    [FRONTIER_MON_METAPOD] = {
        .species = SPECIES_METAPOD,
        .moves = {MOVE_STRING_SHOT, MOVE_HARDEN, MOVE_TACKLE, MOVE_NONE},
        .itemTableId = BATTLE_FRONTIER_ITEM_FOCUS_BAND,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {0, 0, 252, 0, 0, 0},
        .nature = NATURE_LAX,
        .ability = ABILITY_SHED_SKIN
    },
    [FRONTIER_MON_KAKUNA] = {
        .species = SPECIES_KAKUNA,
        .moves = {MOVE_HARDEN, MOVE_NONE, MOVE_NONE, MOVE_NONE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LAX_INCENSE,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_HP,
        .nature = NATURE_BASHFUL
    },
    [FRONTIER_MON_PICHU] = {
        .species = SPECIES_PICHU,
        .moves = {MOVE_SWEET_KISS, MOVE_THUNDER_WAVE, MOVE_ATTRACT, MOVE_SHOCK_WAVE},
        .itemTableId = BATTLE_FRONTIER_ITEM_SITRUS_BERRY,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_HP,
        .nature = NATURE_RASH
    },
    [FRONTIER_MON_SILCOON] = {
        .species = SPECIES_SILCOON,
        .moves = {MOVE_HARDEN, MOVE_NONE, MOVE_NONE, MOVE_NONE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LAX_INCENSE,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_HP,
        .nature = NATURE_BASHFUL
    },
    [FRONTIER_MON_CASCOON] = {
        .species = SPECIES_CASCOON,
        .moves = {MOVE_HARDEN, MOVE_NONE, MOVE_NONE, MOVE_NONE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LAX_INCENSE,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_HP,
        .nature = NATURE_BASHFUL
    },
    [FRONTIER_MON_IGGLYBUFF] = {
        .species = SPECIES_IGGLYBUFF,
        .moves = {MOVE_SWEET_KISS, MOVE_SING, MOVE_ATTRACT, MOVE_SEISMIC_TOSS},
        .itemTableId = BATTLE_FRONTIER_ITEM_FOCUS_BAND,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .nature = NATURE_DOCILE
    },
    [FRONTIER_MON_WOOPER] = {
        .species = SPECIES_WOOPER,
        .moves = {MOVE_AMNESIA, MOVE_EARTHQUAKE, MOVE_SLUDGE_BOMB, MOVE_PROTECT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 4, 252, 0, 0, 0},
        .nature = NATURE_IMPISH,
        .ability = ABILITY_WATER_ABSORB
    },
    [FRONTIER_MON_TYROGUE] = {
        .species = SPECIES_TYROGUE,
        .moves = {MOVE_MACH_PUNCH, MOVE_PROTECT, MOVE_DOUBLE_TEAM, MOVE_FACADE},
        .itemTableId = BATTLE_FRONTIER_ITEM_FOCUS_BAND,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .nature = NATURE_DOCILE
    },
    [FRONTIER_MON_SENTRET] = {
        .species = SPECIES_SENTRET,
        .moves = {MOVE_QUICK_ATTACK, MOVE_FOLLOW_ME, MOVE_HELPING_HAND, MOVE_ASSIST},
        .itemTableId = BATTLE_FRONTIER_ITEM_KINGS_ROCK,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .nature = NATURE_DOCILE
    },
    [FRONTIER_MON_CLEFFA] = {
        .species = SPECIES_CLEFFA,
        .moves = {MOVE_METRONOME, MOVE_TOXIC, MOVE_WISH, MOVE_SEISMIC_TOSS},
        .itemTableId = BATTLE_FRONTIER_ITEM_ORAN_BERRY,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_HP,
        .evs = {36, 0, 0, 0, 236, 0},
        .nature = NATURE_CALM,
        .ability = ABILITY_CUTE_CHARM
    },
    [FRONTIER_MON_SEEDOT] = {
        .species = SPECIES_SEEDOT,
        .moves = {MOVE_EXPLOSION, MOVE_SLEEP_TALK, MOVE_NONE, MOVE_NONE},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {0, 252, 0, 0, 4, 252},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_CHLOROPHYLL
    },
    [FRONTIER_MON_LOTAD] = {
        .species = SPECIES_LOTAD,
        .moves = {MOVE_GIGA_DRAIN, MOVE_SURF, MOVE_RAIN_DANCE, MOVE_THIEF},
        .itemTableId = BATTLE_FRONTIER_ITEM_SITRUS_BERRY,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {36, 0, 36, 116, 196, 116},
        .nature = NATURE_CALM,
        .ability = ABILITY_SWIFT_SWIM
    },
    [FRONTIER_MON_POOCHYENA] = {
        .species = SPECIES_POOCHYENA,
        .moves = {MOVE_CRUNCH, MOVE_SWAGGER, MOVE_ROAR, MOVE_SAND_ATTACK},
        .itemTableId = BATTLE_FRONTIER_ITEM_PETAYA_BERRY,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_HP,
        .nature = NATURE_RELAXED
    },
    [FRONTIER_MON_SHEDINJA] = {
        .species = SPECIES_SHEDINJA,
        .moves = {MOVE_SWORDS_DANCE, MOVE_SHADOW_BALL, MOVE_HIDDEN_POWER, MOVE_PROTECT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LUM_BERRY,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {0, 252, 4, 0, 0, 252},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_WONDER_GUARD,
        .hiddenPowerType = TYPE_FIGHTING
    },
    [FRONTIER_MON_MAKUHITA] = {
        .species = SPECIES_MAKUHITA,
        .moves = {MOVE_FAKE_OUT, MOVE_SEISMIC_TOSS, MOVE_DETECT, MOVE_WHIRLWIND},
        .itemTableId = BATTLE_FRONTIER_ITEM_SCOPE_LENS,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .nature = NATURE_DOCILE
    },
    [FRONTIER_MON_WHISMUR] = {
        .species = SPECIES_WHISMUR,
        .moves = {MOVE_UPROAR, MOVE_SWAGGER, MOVE_BODY_SLAM, MOVE_SMELLING_SALT},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHERI_BERRY,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .nature = NATURE_RELAXED
    },
    [FRONTIER_MON_ZIGZAGOON] = {
        .species = SPECIES_ZIGZAGOON,
        .moves = {MOVE_EXTREME_SPEED, MOVE_BELLY_DRUM, MOVE_HIDDEN_POWER, MOVE_SHADOW_BALL},
        .itemTableId = BATTLE_FRONTIER_ITEM_SILK_SCARF,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {0, 252, 0, 0, 0, 252},
        .nature = NATURE_BRAVE,
        .ability = ABILITY_PICKUP,
        .hiddenPowerType = TYPE_BUG
    },
    [FRONTIER_MON_ZUBAT] = {
        .species = SPECIES_ZUBAT,
        .moves = {MOVE_POISON_FANG, MOVE_WHIRLWIND, MOVE_CONFUSE_RAY, MOVE_AERIAL_ACE},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHERI_BERRY,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .nature = NATURE_SASSY
    },
    [FRONTIER_MON_TOGEPI] = {
        .species = SPECIES_TOGEPI,
        .moves = {MOVE_BODY_SLAM, MOVE_ENCORE, MOVE_SHADOW_BALL, MOVE_SOFT_BOILED},
        .itemTableId = BATTLE_FRONTIER_ITEM_SITRUS_BERRY,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {236, 0, 236, 0, 0, 36},
        .nature = NATURE_IMPISH,
        .ability = ABILITY_SERENE_GRACE
    },
    [FRONTIER_MON_SPINARAK] = {
        .species = SPECIES_SPINARAK,
        .moves = {MOVE_SIGNAL_BEAM, MOVE_NIGHT_SHADE, MOVE_SPIDER_WEB, MOVE_SCARY_FACE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LIECHI_BERRY,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .nature = NATURE_QUIRKY
    },
    [FRONTIER_MON_MARILL] = {
        .species = SPECIES_MARILL,
        .moves = {MOVE_WATER_PULSE, MOVE_RAIN_DANCE, MOVE_LIGHT_SCREEN, MOVE_RETURN},
        .itemTableId = BATTLE_FRONTIER_ITEM_MYSTIC_WATER,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .nature = NATURE_GENTLE
    },
    [FRONTIER_MON_HOPPIP] = {
        .species = SPECIES_HOPPIP,
        .moves = {MOVE_MEGA_DRAIN, MOVE_LEECH_SEED, MOVE_SLEEP_POWDER, MOVE_STUN_SPORE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LAX_INCENSE,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_DEFENSE,
        .nature = NATURE_LAX
    },
    [FRONTIER_MON_SLUGMA] = {
        .species = SPECIES_SLUGMA,
        .moves = {MOVE_EMBER, MOVE_ROCK_SLIDE, MOVE_YAWN, MOVE_BODY_SLAM},
        .itemTableId = BATTLE_FRONTIER_ITEM_SITRUS_BERRY,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .nature = NATURE_SASSY
    },
    [FRONTIER_MON_SWINUB] = {
        .species = SPECIES_SWINUB,
        .moves = {MOVE_ICY_WIND, MOVE_DIG, MOVE_ROCK_TOMB, MOVE_ENDURE},
        .itemTableId = BATTLE_FRONTIER_ITEM_SITRUS_BERRY,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_ATTACK,
        .nature = NATURE_GENTLE
    },
    [FRONTIER_MON_SMEARGLE] = {
        .species = SPECIES_SMEARGLE,
        .moves = {MOVE_SPORE, MOVE_SPIKES, MOVE_WHIRLWIND, MOVE_DESTINY_BOND},
        .itemTableId = BATTLE_FRONTIER_ITEM_SALAC_BERRY,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_SPEED,
        .evs = {68, 0, 0, 0, 188, 252},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_OWN_TEMPO
    },
    [FRONTIER_MON_PIDGEY] = {
        .species = SPECIES_PIDGEY,
        .moves = {MOVE_GUST, MOVE_SAND_ATTACK, MOVE_WHIRLWIND, MOVE_QUICK_ATTACK},
        .itemTableId = BATTLE_FRONTIER_ITEM_SHARP_BEAK,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .nature = NATURE_LONELY
    },
    [FRONTIER_MON_RATTATA] = {
        .species = SPECIES_RATTATA,
        .moves = {MOVE_ENDEAVOR, MOVE_THUNDER_WAVE, MOVE_SUBSTITUTE, MOVE_TAUNT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_SPEED,
        .evs = {4, 0, 0, 0, 252, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_GUTS
    },
    [FRONTIER_MON_WYNAUT] = {
        .species = SPECIES_WYNAUT,
        .moves = {MOVE_ENCORE, MOVE_COUNTER, MOVE_MIRROR_COAT, MOVE_DESTINY_BOND},
        .itemTableId = BATTLE_FRONTIER_ITEM_LUM_BERRY,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 64, 0, 192, 0},
        .nature = NATURE_CAREFUL,
        .ability = ABILITY_SHADOW_TAG
    },
    [FRONTIER_MON_SKITTY] = {
        .species = SPECIES_SKITTY,
        .moves = {MOVE_DOUBLE_SLAP, MOVE_TACKLE, MOVE_SING, MOVE_ATTRACT},
        .itemTableId = BATTLE_FRONTIER_ITEM_ORAN_BERRY,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_DEFENSE | F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {0, 0, 0, 0, 0, 1},
        .nature = NATURE_HASTY,
        .ability = ABILITY_CUTE_CHARM
    },
    [FRONTIER_MON_SPEAROW] = {
        .species = SPECIES_SPEAROW,
        .moves = {MOVE_FURY_ATTACK, MOVE_PURSUIT, MOVE_MIRROR_MOVE, MOVE_PROTECT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LIECHI_BERRY,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_HP,
        .nature = NATURE_DOCILE
    },
    [FRONTIER_MON_HOOTHOOT] = {
        .species = SPECIES_HOOTHOOT,
        .moves = {MOVE_REFLECT, MOVE_RETURN, MOVE_PSYCHIC, MOVE_TOXIC},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_ATTACK,
        .evs = {0, 36, 0, 0, 228, 0},
        .nature = NATURE_SASSY,
        .ability = ABILITY_INSOMNIA
    },
    [FRONTIER_MON_DIGLETT] = {
        .species = SPECIES_DIGLETT,
        .moves = {MOVE_EARTHQUAKE, MOVE_ROCK_SLIDE, MOVE_SLUDGE_BOMB, MOVE_DOUBLE_EDGE},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {0, 252, 4, 0, 0, 252},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_ARENA_TRAP
    },
    [FRONTIER_MON_LEDYBA] = {
        .species = SPECIES_LEDYBA,
        .moves = {MOVE_PSYBEAM, MOVE_AGILITY, MOVE_BATON_PASS, MOVE_LIGHT_SCREEN},
        .itemTableId = BATTLE_FRONTIER_ITEM_SITRUS_BERRY,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_HP,
        .nature = NATURE_BASHFUL
    },
    [FRONTIER_MON_NINCADA] = {
        .species = SPECIES_NINCADA,
        .moves = {MOVE_MUD_SLAP, MOVE_DIG, MOVE_TOXIC, MOVE_PROTECT},
        .itemTableId = BATTLE_FRONTIER_ITEM_PECHA_BERRY,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .nature = NATURE_CALM
    },
    [FRONTIER_MON_SURSKIT] = {
        .species = SPECIES_SURSKIT,
        .moves = {MOVE_HIDDEN_POWER, MOVE_NONE, MOVE_NONE, MOVE_NONE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_HP,
        .nature = NATURE_HARDY,
        .ability = ABILITY_SWIFT_SWIM,
        .hiddenPowerType = TYPE_BUG
    },
    [FRONTIER_MON_JIGGLYPUFF] = {
        .species = SPECIES_JIGGLYPUFF,
        .moves = {MOVE_DISABLE, MOVE_COUNTER, MOVE_PERISH_SONG, MOVE_PROTECT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LUM_BERRY,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 252, 0, 0, 0},
        .nature = NATURE_HARDY,
        .ability = ABILITY_CUTE_CHARM
    },
    [FRONTIER_MON_TAILLOW] = {
        .species = SPECIES_TAILLOW,
        .moves = {MOVE_DOUBLE_EDGE, MOVE_AERIAL_ACE, MOVE_HIDDEN_POWER, MOVE_QUICK_ATTACK},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {0, 252, 4, 0, 0, 252},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_GUTS,
        .hiddenPowerType = TYPE_GROUND
    },
    [FRONTIER_MON_WINGULL] = {
        .species = SPECIES_WINGULL,
        .moves = {MOVE_WING_ATTACK, MOVE_NONE, MOVE_NONE, MOVE_NONE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_HP,
        .nature = NATURE_HARDY,
        .ability = ABILITY_KEEN_EYE
    },
    [FRONTIER_MON_NIDORAN_M] = {
        .species = SPECIES_NIDORAN_M,
        .moves = {MOVE_DOUBLE_KICK, MOVE_POISON_STING, MOVE_DISABLE, MOVE_HELPING_HAND},
        .itemTableId = BATTLE_FRONTIER_ITEM_SITRUS_BERRY,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .nature = NATURE_QUIRKY
    },
    [FRONTIER_MON_NIDORAN_F] = {
        .species = SPECIES_NIDORAN_F,
        .moves = {MOVE_CRUNCH, MOVE_DOUBLE_KICK, MOVE_FLATTER, MOVE_HELPING_HAND},
        .itemTableId = BATTLE_FRONTIER_ITEM_SITRUS_BERRY,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .nature = NATURE_QUIRKY
    },
    [FRONTIER_MON_KIRLIA] = {
        .species = SPECIES_KIRLIA,
        .moves = {MOVE_MEMENTO, MOVE_THUNDER_WAVE, MOVE_DISABLE, MOVE_LIGHT_SCREEN},
        .itemTableId = BATTLE_FRONTIER_ITEM_LUM_BERRY,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 0, 0, 252, 4},
        .nature = NATURE_CALM,
        .ability = ABILITY_TRACE
    },
    [FRONTIER_MON_MAREEP] = {
        .species = SPECIES_MAREEP,
        .moves = {MOVE_SHOCK_WAVE, MOVE_FLASH, MOVE_REFLECT, MOVE_COTTON_SPORE},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHERI_BERRY,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_HP,
        .nature = NATURE_RELAXED
    },
    [FRONTIER_MON_MEDITITE] = {
        .species = SPECIES_MEDITITE,
        .moves = {MOVE_SUBSTITUTE, MOVE_BRICK_BREAK, MOVE_SHADOW_BALL, MOVE_TOXIC},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {0, 252, 0, 0, 4, 252},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_PURE_POWER
    },
    [FRONTIER_MON_SLAKOTH] = {
        .species = SPECIES_SLAKOTH,
        .moves = {MOVE_YAWN, MOVE_SCRATCH, MOVE_ENCORE, MOVE_COUNTER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .nature = NATURE_QUIRKY
    },
    [FRONTIER_MON_PARAS] = {
        .species = SPECIES_PARAS,
        .moves = {MOVE_SPORE, MOVE_LEECH_LIFE, MOVE_SLASH, MOVE_LIGHT_SCREEN},
        .itemTableId = BATTLE_FRONTIER_ITEM_SITRUS_BERRY,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_HP,
        .nature = NATURE_RELAXED
    },
    [FRONTIER_MON_EKANS] = {
        .species = SPECIES_EKANS,
        .moves = {MOVE_ACID, MOVE_DIG, MOVE_SCREECH, MOVE_TORMENT},
        .itemTableId = BATTLE_FRONTIER_ITEM_POISON_BARB,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .nature = NATURE_DOCILE
    },
    [FRONTIER_MON_DITTO] = {
        .species = SPECIES_DITTO,
        .moves = {MOVE_TRANSFORM, MOVE_NONE, MOVE_NONE, MOVE_NONE},
        .itemTableId = BATTLE_FRONTIER_ITEM_METAL_POWDER,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 128, 0, 128, 0},
        .nature = NATURE_IMPISH,
        .ability = ABILITY_LIMBER
    },
    [FRONTIER_MON_BARBOACH] = {
        .species = SPECIES_BARBOACH,
        .moves = {MOVE_MAGNITUDE, MOVE_WATER_PULSE, MOVE_SPARK, MOVE_FUTURE_SIGHT},
        .itemTableId = BATTLE_FRONTIER_ITEM_FOCUS_BAND,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_ATTACK,
        .nature = NATURE_BASHFUL
    },
    [FRONTIER_MON_MEOWTH] = {
        .species = SPECIES_MEOWTH,
        .moves = {MOVE_FAKE_OUT, MOVE_DOUBLE_EDGE, MOVE_SHADOW_BALL, MOVE_TAUNT},
        .itemTableId = BATTLE_FRONTIER_ITEM_SITRUS_BERRY,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {36, 236, 0, 0, 36, 196},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_PICKUP
    },
    [FRONTIER_MON_PINECO] = {
        .species = SPECIES_PINECO,
        .moves = {MOVE_SPIKES, MOVE_EXPLOSION, MOVE_RAPID_SPIN, MOVE_TOXIC},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 4, 252, 0, 0, 0},
        .nature = NATURE_IMPISH,
        .ability = ABILITY_STURDY
    },
    [FRONTIER_MON_TRAPINCH] = {
        .species = SPECIES_TRAPINCH,
        .moves = {MOVE_EARTHQUAKE, MOVE_HIDDEN_POWER, MOVE_ROCK_SLIDE, MOVE_QUICK_ATTACK},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {172, 252, 40, 0, 44, 0},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_ARENA_TRAP,
        .hiddenPowerType = TYPE_BUG
    },
    [FRONTIER_MON_SPHEAL] = {
        .species = SPECIES_SPHEAL,
        .moves = {MOVE_ICE_BALL, MOVE_YAWN, MOVE_PROTECT, MOVE_ENCORE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_HP,
        .nature = NATURE_HARDY,
        .ability = ABILITY_THICK_FAT
    },
    [FRONTIER_MON_HORSEA] = {
        .species = SPECIES_HORSEA,
        .moves = {MOVE_RAIN_DANCE, MOVE_HYDRO_PUMP, MOVE_ICE_BEAM, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_MYSTIC_WATER,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {4, 0, 0, 252, 0, 252},
        .nature = NATURE_MODEST,
        .ability = ABILITY_SWIFT_SWIM,
        .hiddenPowerType = TYPE_GRASS
    },
    [FRONTIER_MON_SHROOMISH] = {
        .species = SPECIES_SHROOMISH,
        .moves = {MOVE_SPORE, MOVE_BULLET_SEED, MOVE_LEECH_SEED, MOVE_HEADBUTT},
        .itemTableId = BATTLE_FRONTIER_ITEM_MIRACLE_SEED,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_HP,
        .nature = NATURE_RELAXED
    },
    [FRONTIER_MON_SHUPPET] = {
        .species = SPECIES_SHUPPET,
        .moves = {MOVE_SHADOW_BALL, MOVE_HIDDEN_POWER, MOVE_SUBSTITUTE, MOVE_THUNDER_WAVE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {4, 252, 0, 0, 0, 252},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_INSOMNIA,
        .hiddenPowerType = TYPE_FIGHTING
    },
    [FRONTIER_MON_DUSKULL] = {
        .species = SPECIES_DUSKULL,
        .moves = {MOVE_REST, MOVE_SLEEP_TALK, MOVE_WILL_O_WISP, MOVE_NIGHT_SHADE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 252, 0, 4, 0},
        .nature = NATURE_BOLD,
        .ability = ABILITY_LEVITATE
    },
    [FRONTIER_MON_ELECTRIKE] = {
        .species = SPECIES_ELECTRIKE,
        .moves = {MOVE_SPARK, MOVE_THUNDER_WAVE, MOVE_ROAR, MOVE_QUICK_ATTACK},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHERI_BERRY,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_HP,
        .nature = NATURE_DOCILE
    },
    [FRONTIER_MON_VULPIX] = {
        .species = SPECIES_VULPIX,
        .moves = {MOVE_FIRE_BLAST, MOVE_FLAMETHROWER, MOVE_QUICK_ATTACK, MOVE_ROAR},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {4, 0, 0, 196, 0, 236},
        .nature = NATURE_MILD,
        .ability = ABILITY_FLASH_FIRE
    },
    [FRONTIER_MON_PIKACHU] = {
        .species = SPECIES_PIKACHU,
        .moves = {MOVE_VOLT_TACKLE, MOVE_HIDDEN_POWER, MOVE_SUBSTITUTE, MOVE_TOXIC},
        .itemTableId = BATTLE_FRONTIER_ITEM_LIGHT_BALL,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {0, 0, 4, 252, 0, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_STATIC,
        .hiddenPowerType = TYPE_ICE
    },
    [FRONTIER_MON_SANDSHREW] = {
        .species = SPECIES_SANDSHREW,
        .moves = {MOVE_SANDSTORM, MOVE_SWORDS_DANCE, MOVE_EARTHQUAKE, MOVE_ROCK_SLIDE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {248, 252, 0, 0, 8, 0},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_SAND_VEIL
    },
    [FRONTIER_MON_POLIWAG] = {
        .species = SPECIES_POLIWAG,
        .moves = {MOVE_RETURN, MOVE_HIDDEN_POWER, MOVE_BELLY_DRUM, MOVE_SUBSTITUTE},
        .itemTableId = BATTLE_FRONTIER_ITEM_SITRUS_BERRY,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {36, 196, 0, 0, 40, 196},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_WATER_ABSORB,
        .hiddenPowerType = TYPE_GHOST
    },
    [FRONTIER_MON_BELLSPROUT] = {
        .species = SPECIES_BELLSPROUT,
        .moves = {MOVE_SUNNY_DAY, MOVE_SOLAR_BEAM, MOVE_HIDDEN_POWER, MOVE_SLUDGE_BOMB},
        .itemTableId = BATTLE_FRONTIER_ITEM_SITRUS_BERRY,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {0, 80, 0, 200, 0, 200},
        .nature = NATURE_RASH,
        .ability = ABILITY_CHLOROPHYLL,
        .hiddenPowerType = TYPE_FIRE
    },
    [FRONTIER_MON_GEODUDE] = {
        .species = SPECIES_GEODUDE,
        .moves = {MOVE_ROCK_SLIDE, MOVE_EARTHQUAKE, MOVE_HIDDEN_POWER, MOVE_EXPLOSION},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {4, 252, 0, 0, 0, 252},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_ROCK_HEAD,
        .hiddenPowerType = TYPE_BUG
    },
    [FRONTIER_MON_DRATINI] = {
        .species = SPECIES_DRATINI,
        .moves = {MOVE_OUTRAGE, MOVE_THUNDER_WAVE, MOVE_SUPERSONIC, MOVE_WATER_PULSE},
        .itemTableId = BATTLE_FRONTIER_ITEM_PERSIM_BERRY,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_HP,
        .nature = NATURE_DOCILE
    },
    [FRONTIER_MON_SNUBBULL] = {
        .species = SPECIES_SNUBBULL,
        .moves = {MOVE_DOUBLE_EDGE, MOVE_SHADOW_BALL, MOVE_EARTHQUAKE, MOVE_THIEF},
        .itemTableId = BATTLE_FRONTIER_ITEM_SITRUS_BERRY,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {116, 196, 36, 0, 120, 36},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_INTIMIDATE
    },
    [FRONTIER_MON_REMORAID] = {
        .species = SPECIES_REMORAID,
        .moves = {MOVE_BUBBLE_BEAM, MOVE_AURORA_BEAM, MOVE_PSYBEAM, MOVE_PROTECT},
        .itemTableId = BATTLE_FRONTIER_ITEM_PETAYA_BERRY,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .nature = NATURE_QUIRKY
    },
    [FRONTIER_MON_LARVITAR] = {
        .species = SPECIES_LARVITAR,
        .moves = {MOVE_DRAGON_DANCE, MOVE_EARTHQUAKE, MOVE_ROCK_SLIDE, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_SOFT_SAND,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {0, 252, 4, 0, 0, 252},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_GUTS,
        .hiddenPowerType = TYPE_BUG
    },
    [FRONTIER_MON_BALTOY] = {
        .species = SPECIES_BALTOY,
        .moves = {MOVE_EARTHQUAKE, MOVE_SHADOW_BALL, MOVE_ROCK_TOMB, MOVE_EXPLOSION},
        .itemTableId = BATTLE_FRONTIER_ITEM_SITRUS_BERRY,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_DEFENSE | F_EV_SPREAD_ATTACK,
        .evs = {36, 196, 156, 0, 36, 76},
        .nature = NATURE_CAREFUL,
        .ability = ABILITY_LEVITATE
    },
    [FRONTIER_MON_SNORUNT] = {
        .species = SPECIES_SNORUNT,
        .moves = {MOVE_SPIKES, MOVE_ICE_BEAM, MOVE_ENDURE, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_SALAC_BERRY,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {0, 0, 4, 252, 0, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_INNER_FOCUS,
        .hiddenPowerType = TYPE_GRASS
    },
    [FRONTIER_MON_BAGON] = {
        .species = SPECIES_BAGON,
        .moves = {MOVE_DRAGON_DANCE, MOVE_DOUBLE_EDGE, MOVE_BRICK_BREAK, MOVE_HYDRO_PUMP},
        .itemTableId = BATTLE_FRONTIER_ITEM_LUM_BERRY,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {0, 252, 4, 0, 0, 252},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_ROCK_HEAD
    },
    [FRONTIER_MON_BELDUM] = {
        .species = SPECIES_BELDUM,
        .moves = {MOVE_TAKE_DOWN, MOVE_NONE, MOVE_NONE, MOVE_NONE},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .nature = NATURE_HARDY
    },
    [FRONTIER_MON_GULPIN] = {
        .species = SPECIES_GULPIN,
        .moves = {MOVE_SLUDGE_BOMB, MOVE_SHADOW_BALL, MOVE_COUNTER, MOVE_ACID_ARMOR},
        .itemTableId = BATTLE_FRONTIER_ITEM_SITRUS_BERRY,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_ATTACK,
        .evs = {0, 252, 0, 0, 252, 0},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_STICKY_HOLD
    },
    [FRONTIER_MON_VENONAT] = {
        .species = SPECIES_VENONAT,
        .moves = {MOVE_PSYBEAM, MOVE_SUPERSONIC, MOVE_STUN_SPORE, MOVE_SKILL_SWAP},
        .itemTableId = BATTLE_FRONTIER_ITEM_SITRUS_BERRY,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_HP,
        .nature = NATURE_RELAXED
    },
    [FRONTIER_MON_MANKEY] = {
        .species = SPECIES_MANKEY,
        .moves = {MOVE_CROSS_CHOP, MOVE_EARTHQUAKE, MOVE_ROCK_SLIDE, MOVE_FOCUS_PUNCH},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {0, 252, 0, 0, 4, 252},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_VITAL_SPIRIT
    },
    [FRONTIER_MON_MACHOP] = {
        .species = SPECIES_MACHOP,
        .moves = {MOVE_BULK_UP, MOVE_CROSS_CHOP, MOVE_HIDDEN_POWER, MOVE_ENDURE},
        .itemTableId = BATTLE_FRONTIER_ITEM_SITRUS_BERRY,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {36, 116, 40, 0, 240, 76},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_GUTS,
        .hiddenPowerType = TYPE_GHOST
    },
    [FRONTIER_MON_SHELLDER] = {
        .species = SPECIES_SHELLDER,
        .moves = {MOVE_ICICLE_SPEAR, MOVE_SUPERSONIC, MOVE_CLAMP, MOVE_WITHDRAW},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHERI_BERRY,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_HP,
        .nature = NATURE_RELAXED
    },
    [FRONTIER_MON_SMOOCHUM] = {
        .species = SPECIES_SMOOCHUM,
        .moves = {MOVE_CALM_MIND, MOVE_ICE_BEAM, MOVE_PSYCHIC, MOVE_SUBSTITUTE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {0, 0, 0, 252, 4, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_OBLIVIOUS
    },
    [FRONTIER_MON_NUMEL] = {
        .species = SPECIES_NUMEL,
        .moves = {MOVE_EARTHQUAKE, MOVE_FIRE_BLAST, MOVE_TOXIC, MOVE_PROTECT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {128, 120, 0, 152, 4, 104},
        .nature = NATURE_MILD,
        .ability = ABILITY_OBLIVIOUS
    },
    [FRONTIER_MON_CARVANHA] = {
        .species = SPECIES_CARVANHA,
        .moves = {MOVE_HYDRO_PUMP, MOVE_CRUNCH, MOVE_HIDDEN_POWER, MOVE_SUBSTITUTE},
        .itemTableId = BATTLE_FRONTIER_ITEM_SALAC_BERRY,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {0, 0, 0, 240, 0, 236},
        .nature = NATURE_MODEST,
        .ability = ABILITY_ROUGH_SKIN,
        .hiddenPowerType = TYPE_GRASS
    },
    [FRONTIER_MON_CORPHISH] = {
        .species = SPECIES_CORPHISH,
        .moves = {MOVE_CRABHAMMER, MOVE_SWORDS_DANCE, MOVE_ICE_BEAM, MOVE_SURF},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED | F_EV_SPREAD_DEFENSE | F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {88, 80, 68, 100, 100, 72},
        .nature = NATURE_HARDY,
        .ability = ABILITY_SHELL_ARMOR
    },
    [FRONTIER_MON_CHARMANDER] = {
        .species = SPECIES_CHARMANDER,
        .moves = {MOVE_FIRE_SPIN, MOVE_METAL_CLAW, MOVE_SMOKESCREEN, MOVE_SCARY_FACE},
        .itemTableId = BATTLE_FRONTIER_ITEM_FOCUS_BAND,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_HP,
        .nature = NATURE_DOCILE
    },
    [FRONTIER_MON_CYNDAQUIL] = {
        .species = SPECIES_CYNDAQUIL,
        .moves = {MOVE_HIDDEN_POWER, MOVE_QUICK_ATTACK, MOVE_ENDURE, MOVE_FLAMETHROWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_SALAC_BERRY,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {0, 0, 12, 200, 36, 236},
        .nature = NATURE_TIMID,
        .ability = ABILITY_BLAZE,
        .hiddenPowerType = TYPE_GRASS
    },
    [FRONTIER_MON_ABRA] = {
        .species = SPECIES_ABRA,
        .moves = {MOVE_CALM_MIND, MOVE_PSYCHIC, MOVE_FIRE_PUNCH, MOVE_SUBSTITUTE},
        .itemTableId = BATTLE_FRONTIER_ITEM_SALAC_BERRY,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {4, 0, 0, 252, 0, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_SYNCHRONIZE
    },
    [FRONTIER_MON_DODUO] = {
        .species = SPECIES_DODUO,
        .moves = {MOVE_DOUBLE_EDGE, MOVE_QUICK_ATTACK, MOVE_DRILL_PECK, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {0, 252, 0, 0, 4, 252},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_EARLY_BIRD,
        .hiddenPowerType = TYPE_GROUND
    },
    [FRONTIER_MON_GASTLY] = {
        .species = SPECIES_GASTLY,
        .moves = {MOVE_PSYCHIC, MOVE_THUNDERBOLT, MOVE_TAUNT, MOVE_WILL_O_WISP},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {0, 0, 0, 252, 4, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_LEVITATE
    },
    [FRONTIER_MON_SWABLU] = {
        .species = SPECIES_SWABLU,
        .moves = {MOVE_TAKE_DOWN, MOVE_DREAM_EATER, MOVE_SING, MOVE_ATTRACT},
        .itemTableId = BATTLE_FRONTIER_ITEM_MENTAL_HERB,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_HP,
        .nature = NATURE_LONELY
    },
    [FRONTIER_MON_TREECKO] = {
        .species = SPECIES_TREECKO,
        .moves = {MOVE_CRUNCH, MOVE_GIGA_DRAIN, MOVE_SUBSTITUTE, MOVE_AGILITY},
        .itemTableId = BATTLE_FRONTIER_ITEM_PETAYA_BERRY,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {0, 0, 0, 236, 0, 196},
        .nature = NATURE_TIMID,
        .ability = ABILITY_OVERGROW
    },
    [FRONTIER_MON_TORCHIC] = {
        .species = SPECIES_TORCHIC,
        .moves = {MOVE_FIRE_SPIN, MOVE_QUICK_ATTACK, MOVE_SAND_ATTACK, MOVE_DOUBLE_TEAM},
        .itemTableId = BATTLE_FRONTIER_ITEM_BRIGHT_POWDER,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_HP,
        .nature = NATURE_LONELY
    },
    [FRONTIER_MON_MUDKIP] = {
        .species = SPECIES_MUDKIP,
        .moves = {MOVE_WHIRLPOOL, MOVE_ENDEAVOR, MOVE_MUD_SPORT, MOVE_MUD_SLAP},
        .itemTableId = BATTLE_FRONTIER_ITEM_FOCUS_BAND,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_HP,
        .nature = NATURE_DOCILE
    },
    [FRONTIER_MON_SQUIRTLE] = {
        .species = SPECIES_SQUIRTLE,
        .moves = {MOVE_WATER_PULSE, MOVE_BITE, MOVE_WITHDRAW, MOVE_PROTECT},
        .itemTableId = BATTLE_FRONTIER_ITEM_SITRUS_BERRY,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_HP,
        .nature = NATURE_DOCILE
    },
    [FRONTIER_MON_TOTODILE] = {
        .species = SPECIES_TOTODILE,
        .moves = {MOVE_SLASH, MOVE_WATER_PULSE, MOVE_SCREECH, MOVE_ICY_WIND},
        .itemTableId = BATTLE_FRONTIER_ITEM_SCOPE_LENS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_HP,
        .nature = NATURE_DOCILE
    },
    [FRONTIER_MON_SLOWPOKE] = {
        .species = SPECIES_SLOWPOKE,
        .moves = {MOVE_CALM_MIND, MOVE_PSYCHIC, MOVE_REST, MOVE_SLEEP_TALK},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_HP,
        .evs = {116, 0, 0, 196, 196, 0},
        .nature = NATURE_MODEST,
        .ability = ABILITY_OWN_TEMPO
    },
    [FRONTIER_MON_BULBASAUR] = {
        .species = SPECIES_BULBASAUR,
        .moves = {MOVE_GIGA_DRAIN, MOVE_GROWTH, MOVE_LEECH_SEED, MOVE_SLUDGE_BOMB},
        .itemTableId = BATTLE_FRONTIER_ITEM_SITRUS_BERRY,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_HP,
        .evs = {0, 0, 0, 0, 236, 0},
        .nature = NATURE_SASSY,
        .ability = ABILITY_OVERGROW
    },
    [FRONTIER_MON_CHIKORITA] = {
        .species = SPECIES_CHIKORITA,
        .moves = {MOVE_SECRET_POWER, MOVE_BULLET_SEED, MOVE_MUD_SLAP, MOVE_LIGHT_SCREEN},
        .itemTableId = BATTLE_FRONTIER_ITEM_BRIGHT_POWDER,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_HP,
        .nature = NATURE_RELAXED
    },
    [FRONTIER_MON_ODDISH] = {
        .species = SPECIES_ODDISH,
        .moves = {MOVE_SUNNY_DAY, MOVE_SOLAR_BEAM, MOVE_HIDDEN_POWER, MOVE_BULLET_SEED},
        .itemTableId = BATTLE_FRONTIER_ITEM_SITRUS_BERRY,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED | F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {76, 0, 80, 236, 0, 116},
        .nature = NATURE_MODEST,
        .ability = ABILITY_CHLOROPHYLL,
        .hiddenPowerType = TYPE_ICE
    },
    [FRONTIER_MON_PSYDUCK] = {
        .species = SPECIES_PSYDUCK,
        .moves = {MOVE_CONFUSION, MOVE_FURY_SWIPES, MOVE_AERIAL_ACE, MOVE_DISABLE},
        .itemTableId = BATTLE_FRONTIER_ITEM_PERSIM_BERRY,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_ATTACK,
        .nature = NATURE_HASTY
    },
    [FRONTIER_MON_CUBONE] = {
        .species = SPECIES_CUBONE,
        .moves = {MOVE_SWORDS_DANCE, MOVE_EARTHQUAKE, MOVE_DOUBLE_EDGE, MOVE_SUBSTITUTE},
        .itemTableId = BATTLE_FRONTIER_ITEM_THICK_CLUB,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {0, 252, 0, 0, 4, 252},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_ROCK_HEAD
    },
    [FRONTIER_MON_GOLDEEN] = {
        .species = SPECIES_GOLDEEN,
        .moves = {MOVE_WATERFALL, MOVE_FURY_ATTACK, MOVE_AGILITY, MOVE_PSYBEAM},
        .itemTableId = BATTLE_FRONTIER_ITEM_PETAYA_BERRY,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_HP,
        .nature = NATURE_DOCILE
    },
    [FRONTIER_MON_NATU] = {
        .species = SPECIES_NATU,
        .moves = {MOVE_PSYCHIC, MOVE_SUBSTITUTE, MOVE_TOXIC, MOVE_PROTECT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {12, 0, 0, 244, 0, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_SYNCHRONIZE
    },
    [FRONTIER_MON_CLEFAIRY] = {
        .species = SPECIES_CLEFAIRY,
        .moves = {MOVE_THUNDER_WAVE, MOVE_SEISMIC_TOSS, MOVE_ENCORE, MOVE_SOFT_BOILED},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 4, 0, 252, 0},
        .nature = NATURE_CALM,
        .ability = ABILITY_CUTE_CHARM
    },
    [FRONTIER_MON_MAGNEMITE] = {
        .species = SPECIES_MAGNEMITE,
        .moves = {MOVE_THUNDERBOLT, MOVE_HIDDEN_POWER, MOVE_SUBSTITUTE, MOVE_TOXIC},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_SPEED | F_EV_SPREAD_HP,
        .evs = {200, 0, 0, 0, 132, 176},
        .nature = NATURE_TIMID,
        .ability = ABILITY_MAGNET_PULL,
        .hiddenPowerType = TYPE_WATER
    },
    [FRONTIER_MON_SEEL] = {
        .species = SPECIES_SEEL,
        .moves = {MOVE_AURORA_BEAM, MOVE_DIVE, MOVE_BODY_SLAM, MOVE_FAKE_OUT},
        .itemTableId = BATTLE_FRONTIER_ITEM_ASPEAR_BERRY,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_HP,
        .nature = NATURE_HARDY
    },
    [FRONTIER_MON_GRIMER] = {
        .species = SPECIES_GRIMER,
        .moves = {MOVE_REST, MOVE_SLEEP_TALK, MOVE_CURSE, MOVE_SLUDGE_BOMB},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 4, 0, 252, 0},
        .nature = NATURE_CAREFUL,
        .ability = ABILITY_STICKY_HOLD
    },
    [FRONTIER_MON_KRABBY] = {
        .species = SPECIES_KRABBY,
        .moves = {MOVE_DOUBLE_EDGE, MOVE_HIDDEN_POWER, MOVE_KNOCK_OFF, MOVE_SURF},
        .itemTableId = BATTLE_FRONTIER_ITEM_LUM_BERRY,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_ATTACK,
        .evs = {36, 236, 0, 236, 0, 0},
        .nature = NATURE_NAUGHTY,
        .ability = ABILITY_HYPER_CUTTER,
        .hiddenPowerType = TYPE_GHOST
    },
    [FRONTIER_MON_EXEGGCUTE] = {
        .species = SPECIES_EXEGGCUTE,
        .moves = {MOVE_EXPLOSION, MOVE_SUNNY_DAY, MOVE_SOLAR_BEAM, MOVE_PSYCHIC},
        .itemTableId = BATTLE_FRONTIER_ITEM_SITRUS_BERRY,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_HP,
        .evs = {36, 36, 36, 196, 156, 36},
        .nature = NATURE_MILD,
        .ability = ABILITY_CHLOROPHYLL
    },
    [FRONTIER_MON_EEVEE] = {
        .species = SPECIES_EEVEE,
        .moves = {MOVE_QUICK_ATTACK, MOVE_DOUBLE_EDGE, MOVE_HIDDEN_POWER, MOVE_FACADE},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {252, 188, 0, 0, 68, 0},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_RUN_AWAY,
        .hiddenPowerType = TYPE_FIGHTING
    },
    [FRONTIER_MON_DROWZEE] = {
        .species = SPECIES_DROWZEE,
        .moves = {MOVE_PSYCHIC, MOVE_TOXIC, MOVE_WISH, MOVE_PROTECT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 0, 4, 252, 0},
        .nature = NATURE_CALM,
        .ability = ABILITY_INSOMNIA
    },
    [FRONTIER_MON_VOLTORB] = {
        .species = SPECIES_VOLTORB,
        .moves = {MOVE_RAIN_DANCE, MOVE_THUNDER, MOVE_LIGHT_SCREEN, MOVE_EXPLOSION},
        .itemTableId = BATTLE_FRONTIER_ITEM_SILK_SCARF,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {0, 252, 0, 4, 0, 252},
        .nature = NATURE_NAIVE,
        .ability = ABILITY_STATIC
    },
    [FRONTIER_MON_CHINCHOU] = {
        .species = SPECIES_CHINCHOU,
        .moves = {MOVE_HYDRO_PUMP, MOVE_THUNDERBOLT, MOVE_ICE_BEAM, MOVE_THUNDER_WAVE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {0, 0, 0, 252, 4, 252},
        .nature = NATURE_MODEST,
        .ability = ABILITY_VOLT_ABSORB
    },
    [FRONTIER_MON_TEDDIURSA] = {
        .species = SPECIES_TEDDIURSA,
        .moves = {MOVE_SECRET_POWER, MOVE_FAKE_TEARS, MOVE_FAINT_ATTACK, MOVE_REST},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHESTO_BERRY,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .nature = NATURE_RELAXED
    },
    [FRONTIER_MON_DELIBIRD] = {
        .species = SPECIES_DELIBIRD,
        .moves = {MOVE_ICE_BEAM, MOVE_QUICK_ATTACK, MOVE_FOCUS_PUNCH, MOVE_AERIAL_ACE},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {0, 136, 0, 136, 0, 236},
        .nature = NATURE_NAUGHTY,
        .ability = ABILITY_HUSTLE
    },
    [FRONTIER_MON_HOUNDOUR] = {
        .species = SPECIES_HOUNDOUR,
        .moves = {MOVE_FIRE_BLAST, MOVE_CRUNCH, MOVE_HIDDEN_POWER, MOVE_PURSUIT},
        .itemTableId = BATTLE_FRONTIER_ITEM_SITRUS_BERRY,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {0, 0, 36, 200, 116, 156},
        .nature = NATURE_MODEST,
        .ability = ABILITY_EARLY_BIRD,
        .hiddenPowerType = TYPE_GRASS
    },
    [FRONTIER_MON_PHANPY] = {
        .species = SPECIES_PHANPY,
        .moves = {MOVE_ROLLOUT, MOVE_DEFENSE_CURL, MOVE_ROCK_TOMB, MOVE_SANDSTORM},
        .itemTableId = BATTLE_FRONTIER_ITEM_LAX_INCENSE,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .nature = NATURE_NAUGHTY
    },
    [FRONTIER_MON_SPOINK] = {
        .species = SPECIES_SPOINK,
        .moves = {MOVE_CALM_MIND, MOVE_SUBSTITUTE, MOVE_PSYCHIC, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_HP,
        .evs = {252, 0, 0, 4, 0, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_THICK_FAT,
        .hiddenPowerType = TYPE_GRASS
    },
    [FRONTIER_MON_ARON] = {
        .species = SPECIES_ARON,
        .moves = {MOVE_PROTECT, MOVE_DOUBLE_EDGE, MOVE_ROCK_SLIDE, MOVE_EARTHQUAKE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {168, 252, 0, 0, 0, 88},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_ROCK_HEAD
    },
    [FRONTIER_MON_LUVDISC] = {
        .species = SPECIES_LUVDISC,
        .moves = {MOVE_RAIN_DANCE, MOVE_SURF, MOVE_ICE_BEAM, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LUM_BERRY,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_DEFENSE,
        .evs = {28, 0, 228, 252, 0, 0},
        .nature = NATURE_MODEST,
        .ability = ABILITY_SWIFT_SWIM,
        .hiddenPowerType = TYPE_GRASS
    },
    [FRONTIER_MON_TENTACOOL] = {
        .species = SPECIES_TENTACOOL,
        .moves = {MOVE_RAPID_SPIN, MOVE_SURF, MOVE_TOXIC, MOVE_GIGA_DRAIN},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 4, 0, 252, 0},
        .nature = NATURE_CALM,
        .ability = ABILITY_LIQUID_OOZE
    },
    [FRONTIER_MON_CACNEA] = {
        .species = SPECIES_CACNEA,
        .moves = {MOVE_SPIKES, MOVE_GIGA_DRAIN, MOVE_ENCORE, MOVE_DESTINY_BOND},
        .itemTableId = BATTLE_FRONTIER_ITEM_SITRUS_BERRY,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {196, 0, 116, 0, 196, 0},
        .nature = NATURE_BOLD,
        .ability = ABILITY_SAND_VEIL
    },
    [FRONTIER_MON_UNOWN] = {
        .species = SPECIES_UNOWN,
        .moves = {MOVE_HIDDEN_POWER, MOVE_NONE, MOVE_NONE, MOVE_NONE},
        .itemTableId = BATTLE_FRONTIER_ITEM_TWISTED_SPOON,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {0, 0, 84, 244, 0, 180},
        .nature = NATURE_MODEST,
        .ability = ABILITY_LEVITATE,
        .hiddenPowerType = TYPE_PSYCHIC
    },
    [FRONTIER_MON_KOFFING] = {
        .species = SPECIES_KOFFING,
        .moves = {MOVE_SLUDGE_BOMB, MOVE_HIDDEN_POWER, MOVE_WILL_O_WISP, MOVE_PAIN_SPLIT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 4, 252, 0, 0, 0},
        .nature = NATURE_RELAXED,
        .ability = ABILITY_LEVITATE,
        .hiddenPowerType = TYPE_WATER
    },
    [FRONTIER_MON_STARYU] = {
        .species = SPECIES_STARYU,
        .moves = {MOVE_SURF, MOVE_THUNDERBOLT, MOVE_SUBSTITUTE, MOVE_RECOVER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {0, 0, 0, 252, 4, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_NATURAL_CURE
    },
    [FRONTIER_MON_SKIPLOOM] = {
        .species = SPECIES_SKIPLOOM,
        .moves = {MOVE_MEGA_DRAIN, MOVE_CONFUSION, MOVE_COTTON_SPORE, MOVE_SYNTHESIS},
        .itemTableId = BATTLE_FRONTIER_ITEM_PERSIM_BERRY,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_HP,
        .nature = NATURE_DOCILE
    },
    [FRONTIER_MON_NUZLEAF] = {
        .species = SPECIES_NUZLEAF,
        .moves = {MOVE_EXTRASENSORY, MOVE_FAKE_OUT, MOVE_RAZOR_WIND, MOVE_FAINT_ATTACK},
        .itemTableId = BATTLE_FRONTIER_ITEM_SITRUS_BERRY,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_HP,
        .nature = NATURE_HARDY
    },
    [FRONTIER_MON_LOMBRE] = {
        .species = SPECIES_LOMBRE,
        .moves = {MOVE_HYDRO_PUMP, MOVE_GIGA_DRAIN, MOVE_ICE_BEAM, MOVE_RAIN_DANCE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED | F_EV_SPREAD_HP,
        .evs = {160, 0, 0, 232, 0, 116},
        .nature = NATURE_MODEST,
        .ability = ABILITY_SWIFT_SWIM
    },
    [FRONTIER_MON_VIBRAVA] = {
        .species = SPECIES_VIBRAVA,
        .moves = {MOVE_EARTHQUAKE, MOVE_ROCK_SLIDE, MOVE_TOXIC, MOVE_PROTECT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {40, 252, 0, 0, 0, 216},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_LEVITATE
    },
    [FRONTIER_MON_RHYHORN] = {
        .species = SPECIES_RHYHORN,
        .moves = {MOVE_EARTHQUAKE, MOVE_ROCK_SLIDE, MOVE_MEGAHORN, MOVE_PROTECT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {204, 148, 20, 0, 0, 136},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_ROCK_HEAD
    },
    [FRONTIER_MON_CLAMPERL] = {
        .species = SPECIES_CLAMPERL,
        .moves = {MOVE_SURF, MOVE_ICE_BEAM, MOVE_HIDDEN_POWER, MOVE_TOXIC},
        .itemTableId = BATTLE_FRONTIER_ITEM_DEEP_SEA_TOOTH,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_HP,
        .evs = {252, 0, 0, 252, 0, 4},
        .nature = NATURE_MODEST,
        .ability = ABILITY_SHELL_ARMOR,
        .hiddenPowerType = TYPE_ELECTRIC
    },
    [FRONTIER_MON_PIDGEOTTO] = {
        .species = SPECIES_PIDGEOTTO,
        .moves = {MOVE_DOUBLE_EDGE, MOVE_QUICK_ATTACK, MOVE_AERIAL_ACE, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {0, 252, 0, 0, 4, 252},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_KEEN_EYE,
        .hiddenPowerType = TYPE_FIGHTING
    },
    [FRONTIER_MON_GROWLITHE] = {
        .species = SPECIES_GROWLITHE,
        .moves = {MOVE_AGILITY, MOVE_FIRE_BLAST, MOVE_HIDDEN_POWER, MOVE_SUBSTITUTE},
        .itemTableId = BATTLE_FRONTIER_ITEM_PETAYA_BERRY,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {4, 0, 0, 252, 0, 252},
        .nature = NATURE_MODEST,
        .ability = ABILITY_INTIMIDATE,
        .hiddenPowerType = TYPE_GRASS
    },
    [FRONTIER_MON_FARFETCHD] = {
        .species = SPECIES_FARFETCHD,
        .moves = {MOVE_SWORDS_DANCE, MOVE_SLASH, MOVE_AGILITY, MOVE_KNOCK_OFF},
        .itemTableId = BATTLE_FRONTIER_ITEM_STICK,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {4, 252, 0, 0, 0, 252},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_INNER_FOCUS
    },
    [FRONTIER_MON_OMANYTE] = {
        .species = SPECIES_OMANYTE,
        .moves = {MOVE_SPIKES, MOVE_TOXIC, MOVE_SURF, MOVE_ICE_BEAM},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 252, 0, 4, 0},
        .nature = NATURE_BOLD,
        .ability = ABILITY_SHELL_ARMOR
    },
    [FRONTIER_MON_KABUTO] = {
        .species = SPECIES_KABUTO,
        .moves = {MOVE_SURF, MOVE_RAIN_DANCE, MOVE_ICE_BEAM, MOVE_SUBSTITUTE},
        .itemTableId = BATTLE_FRONTIER_ITEM_PETAYA_BERRY,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {36, 0, 0, 236, 0, 236},
        .nature = NATURE_MODEST,
        .ability = ABILITY_SWIFT_SWIM
    },
    [FRONTIER_MON_LILEEP] = {
        .species = SPECIES_LILEEP,
        .moves = {MOVE_ROCK_SLIDE, MOVE_GIGA_DRAIN, MOVE_HIDDEN_POWER, MOVE_TOXIC},
        .itemTableId = BATTLE_FRONTIER_ITEM_SITRUS_BERRY,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_DEFENSE | F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {68, 188, 140, 32, 64, 12},
        .nature = NATURE_BRAVE,
        .ability = ABILITY_SUCTION_CUPS,
        .hiddenPowerType = TYPE_GROUND
    },
    [FRONTIER_MON_ANORITH] = {
        .species = SPECIES_ANORITH,
        .moves = {MOVE_SWORDS_DANCE, MOVE_HIDDEN_POWER, MOVE_ROCK_SLIDE, MOVE_BRICK_BREAK},
        .itemTableId = BATTLE_FRONTIER_ITEM_LUM_BERRY,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {0, 252, 0, 0, 4, 252},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_BATTLE_ARMOR,
        .hiddenPowerType = TYPE_BUG
    },
    [FRONTIER_MON_AIPOM] = {
        .species = SPECIES_AIPOM,
        .moves = {MOVE_DOUBLE_EDGE, MOVE_SHADOW_BALL, MOVE_THUNDER_WAVE, MOVE_FOCUS_PUNCH},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {4, 252, 0, 0, 0, 252},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_PICKUP
    },
    [FRONTIER_MON_ELEKID] = {
        .species = SPECIES_ELEKID,
        .moves = {MOVE_THUNDERBOLT, MOVE_ICE_PUNCH, MOVE_HIDDEN_POWER, MOVE_SUBSTITUTE},
        .itemTableId = BATTLE_FRONTIER_ITEM_PETAYA_BERRY,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {0, 0, 4, 252, 0, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_STATIC,
        .hiddenPowerType = TYPE_GRASS
    },
    [FRONTIER_MON_LOUDRED] = {
        .species = SPECIES_LOUDRED,
        .moves = {MOVE_RETURN, MOVE_SHADOW_BALL, MOVE_EARTHQUAKE, MOVE_FIRE_BLAST},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {100, 252, 0, 16, 0, 140},
        .nature = NATURE_NAUGHTY,
        .ability = ABILITY_SOUNDPROOF
    },
    [FRONTIER_MON_SPINDA] = {
        .species = SPECIES_SPINDA,
        .moves = {MOVE_HYPNOSIS, MOVE_THUNDER_PUNCH, MOVE_CALM_MIND, MOVE_BATON_PASS},
        .itemTableId = BATTLE_FRONTIER_ITEM_SALAC_BERRY,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_HP,
        .evs = {252, 0, 0, 4, 0, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_OWN_TEMPO
    },
    [FRONTIER_MON_NIDORINA] = {
        .species = SPECIES_NIDORINA,
        .moves = {MOVE_DOUBLE_KICK, MOVE_BITE, MOVE_AERIAL_ACE, MOVE_FLATTER},
        .itemTableId = BATTLE_FRONTIER_ITEM_FOCUS_BAND,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .nature = NATURE_QUIRKY
    },
    [FRONTIER_MON_NIDORINO] = {
        .species = SPECIES_NIDORINO,
        .moves = {MOVE_SLUDGE_BOMB, MOVE_RETURN, MOVE_DOUBLE_KICK, MOVE_FACADE},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {0, 252, 0, 0, 4, 252},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_POISON_POINT
    },
    [FRONTIER_MON_FLAAFFY] = {
        .species = SPECIES_FLAAFFY,
        .moves = {MOVE_THUNDERBOLT, MOVE_HEAL_BELL, MOVE_HIDDEN_POWER, MOVE_FIRE_PUNCH},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {4, 0, 0, 252, 0, 252},
        .nature = NATURE_MODEST,
        .ability = ABILITY_STATIC,
        .hiddenPowerType = TYPE_GRASS
    },
    [FRONTIER_MON_MAGBY] = {
        .species = SPECIES_MAGBY,
        .moves = {MOVE_FIRE_BLAST, MOVE_FOCUS_PUNCH, MOVE_TOXIC, MOVE_SUBSTITUTE},
        .itemTableId = BATTLE_FRONTIER_ITEM_SITRUS_BERRY,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {0, 76, 0, 116, 0, 252},
        .nature = NATURE_HASTY,
        .ability = ABILITY_FLAME_BODY
    },
    [FRONTIER_MON_NOSEPASS] = {
        .species = SPECIES_NOSEPASS,
        .moves = {MOVE_TAUNT, MOVE_FIRE_PUNCH, MOVE_BLOCK, MOVE_EXPLOSION},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {0, 0, 4, 252, 0, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_MAGNET_PULL
    },
    [FRONTIER_MON_CORSOLA] = {
        .species = SPECIES_CORSOLA,
        .moves = {MOVE_RECOVER, MOVE_SURF, MOVE_ICE_BEAM, MOVE_TOXIC},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_DEFENSE,
        .evs = {0, 0, 136, 252, 120, 0},
        .nature = NATURE_MODEST,
        .ability = ABILITY_NATURAL_CURE
    },
    [FRONTIER_MON_MAWILE] = {
        .species = SPECIES_MAWILE,
        .moves = {MOVE_SUBSTITUTE, MOVE_BATON_PASS, MOVE_SWORDS_DANCE, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {132, 252, 0, 0, 0, 124},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_INTIMIDATE,
        .hiddenPowerType = TYPE_STEEL
    },
    [FRONTIER_MON_BUTTERFREE] = {
        .species = SPECIES_BUTTERFREE,
        .moves = {MOVE_SLEEP_POWDER, MOVE_STUN_SPORE, MOVE_PSYCHIC, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_QUICK_CLAW,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {4, 0, 0, 252, 0, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_COMPOUND_EYES,
        .hiddenPowerType = TYPE_FIRE
    },
    [FRONTIER_MON_BEEDRILL] = {
        .species = SPECIES_BEEDRILL,
        .moves = {MOVE_SLUDGE_BOMB, MOVE_TWINEEDLE, MOVE_BRICK_BREAK, MOVE_DOUBLE_EDGE},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {4, 252, 0, 0, 0, 252},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_SWARM
    },
    [FRONTIER_MON_POLIWHIRL] = {
        .species = SPECIES_POLIWHIRL,
        .moves = {MOVE_BELLY_DRUM, MOVE_EARTHQUAKE, MOVE_HYPNOSIS, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {4, 252, 0, 0, 0, 252},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_WATER_ABSORB,
        .hiddenPowerType = TYPE_GHOST
    },
    [FRONTIER_MON_ONIX] = {
        .species = SPECIES_ONIX,
        .moves = {MOVE_SUNNY_DAY, MOVE_EXPLOSION, MOVE_BLOCK, MOVE_EARTHQUAKE},
        .itemTableId = BATTLE_FRONTIER_ITEM_SALAC_BERRY,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {112, 144, 0, 0, 0, 252},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_STURDY
    },
    [FRONTIER_MON_BEAUTIFLY] = {
        .species = SPECIES_BEAUTIFLY,
        .moves = {MOVE_STUN_SPORE, MOVE_SUBSTITUTE, MOVE_MORNING_SUN, MOVE_SILVER_WIND},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {0, 252, 0, 0, 0, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_SWARM
    },
    [FRONTIER_MON_DUSTOX] = {
        .species = SPECIES_DUSTOX,
        .moves = {MOVE_LIGHT_SCREEN, MOVE_MOONLIGHT, MOVE_WHIRLWIND, MOVE_SLUDGE_BOMB},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 4, 0, 0, 252, 0},
        .nature = NATURE_CAREFUL,
        .ability = ABILITY_SHIELD_DUST
    },
    [FRONTIER_MON_LEDIAN] = {
        .species = SPECIES_LEDIAN,
        .moves = {MOVE_REFLECT, MOVE_LIGHT_SCREEN, MOVE_BATON_PASS, MOVE_SUBSTITUTE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_HP,
        .evs = {252, 0, 4, 0, 0, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_EARLY_BIRD
    },
    [FRONTIER_MON_ARIADOS] = {
        .species = SPECIES_ARIADOS,
        .moves = {MOVE_SLUDGE_BOMB, MOVE_SIGNAL_BEAM, MOVE_SPIDER_WEB, MOVE_BATON_PASS},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 40, 108, 0, 108, 0},
        .nature = NATURE_IMPISH,
        .ability = ABILITY_INSOMNIA
    },
    [FRONTIER_MON_YANMA] = {
        .species = SPECIES_YANMA,
        .moves = {MOVE_SUBSTITUTE, MOVE_REVERSAL, MOVE_AERIAL_ACE, MOVE_PROTECT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LIECHI_BERRY,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {0, 252, 4, 0, 0, 252},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_SPEED_BOOST
    },
    [FRONTIER_MON_DELCATTY_1] = {
        .species = SPECIES_DELCATTY,
        .moves = {MOVE_CALM_MIND, MOVE_BATON_PASS, MOVE_SUBSTITUTE, MOVE_WISH},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_HP,
        .evs = {252, 0, 4, 0, 0, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_CUTE_CHARM
    },
    [FRONTIER_MON_SABLEYE_1] = {
        .species = SPECIES_SABLEYE,
        .moves = {MOVE_RECOVER, MOVE_KNOCK_OFF, MOVE_SEISMIC_TOSS, MOVE_PSYCH_UP},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 252, 0, 4, 0},
        .nature = NATURE_BOLD,
        .ability = ABILITY_KEEN_EYE
    },
    [FRONTIER_MON_LICKITUNG_1] = {
        .species = SPECIES_LICKITUNG,
        .moves = {MOVE_WISH, MOVE_PROTECT, MOVE_HEAL_BELL, MOVE_SEISMIC_TOSS},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 4, 0, 252, 0},
        .nature = NATURE_CAREFUL,
        .ability = ABILITY_OWN_TEMPO
    },
    [FRONTIER_MON_WEEPINBELL_1] = {
        .species = SPECIES_WEEPINBELL,
        .moves = {MOVE_SUNNY_DAY, MOVE_SLUDGE_BOMB, MOVE_HIDDEN_POWER, MOVE_SOLAR_BEAM},
        .itemTableId = BATTLE_FRONTIER_ITEM_POISON_BARB,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {0, 252, 0, 4, 0, 252},
        .nature = NATURE_NAIVE,
        .ability = ABILITY_CHLOROPHYLL,
        .hiddenPowerType = TYPE_GROUND
    },
    [FRONTIER_MON_GRAVELER_1] = {
        .species = SPECIES_GRAVELER,
        .moves = {MOVE_ROCK_SLIDE, MOVE_EARTHQUAKE, MOVE_DOUBLE_EDGE, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {212, 252, 0, 0, 0, 44},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_ROCK_HEAD,
        .hiddenPowerType = TYPE_BUG
    },
    [FRONTIER_MON_GLOOM_1] = {
        .species = SPECIES_GLOOM,
        .moves = {MOVE_SLUDGE_BOMB, MOVE_GIGA_DRAIN, MOVE_LEECH_SEED, MOVE_MOONLIGHT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 4, 0, 252, 0},
        .nature = NATURE_CAREFUL,
        .ability = ABILITY_CHLOROPHYLL
    },
    [FRONTIER_MON_PORYGON_1] = {
        .species = SPECIES_PORYGON,
        .moves = {MOVE_THUNDER_WAVE, MOVE_RECOVER, MOVE_THUNDERBOLT, MOVE_ICE_BEAM},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {32, 0, 0, 252, 4, 220},
        .nature = NATURE_MODEST,
        .ability = ABILITY_TRACE
    },
    [FRONTIER_MON_KADABRA_1] = {
        .species = SPECIES_KADABRA,
        .moves = {MOVE_CONFUSION, MOVE_ROLE_PLAY, MOVE_FUTURE_SIGHT, MOVE_DISABLE},
        .itemTableId = BATTLE_FRONTIER_ITEM_TWISTED_SPOON,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .nature = NATURE_IMPISH
    },
    [FRONTIER_MON_WAILMER_1] = {
        .species = SPECIES_WAILMER,
        .moves = {MOVE_CURSE, MOVE_REST, MOVE_HIDDEN_POWER, MOVE_EARTHQUAKE},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHESTO_BERRY,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_DEFENSE,
        .evs = {48, 0, 188, 0, 252, 20},
        .nature = NATURE_CAREFUL,
        .ability = ABILITY_WATER_VEIL,
        .hiddenPowerType = TYPE_ROCK
    },
    [FRONTIER_MON_ROSELIA_1] = {
        .species = SPECIES_ROSELIA,
        .moves = {MOVE_SPIKES, MOVE_GIGA_DRAIN, MOVE_STUN_SPORE, MOVE_SYNTHESIS},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 4, 0, 252, 0},
        .nature = NATURE_CALM,
        .ability = ABILITY_NATURAL_CURE
    },
    [FRONTIER_MON_VOLBEAT_1] = {
        .species = SPECIES_VOLBEAT,
        .moves = {MOVE_TAIL_GLOW, MOVE_BATON_PASS, MOVE_SUBSTITUTE, MOVE_THUNDERBOLT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_HP,
        .evs = {252, 0, 0, 4, 0, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_SWARM
    },
    [FRONTIER_MON_ILLUMISE_1] = {
        .species = SPECIES_ILLUMISE,
        .moves = {MOVE_SUBSTITUTE, MOVE_BATON_PASS, MOVE_ICE_PUNCH, MOVE_THUNDERBOLT},
        .itemTableId = BATTLE_FRONTIER_ITEM_SALAC_BERRY,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED | F_EV_SPREAD_HP,
        .evs = {96, 0, 0, 252, 0, 160},
        .nature = NATURE_TIMID,
        .ability = ABILITY_OBLIVIOUS
    },
    [FRONTIER_MON_IVYSAUR_1] = {
        .species = SPECIES_IVYSAUR,
        .moves = {MOVE_LEECH_SEED, MOVE_SLEEP_POWDER, MOVE_SYNTHESIS, MOVE_MAGICAL_LEAF},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 0, 0, 224, 32},
        .nature = NATURE_CALM,
        .ability = ABILITY_OVERGROW
    },
    [FRONTIER_MON_CHARMELEON_1] = {
        .species = SPECIES_CHARMELEON,
        .moves = {MOVE_FIRE_BLAST, MOVE_OVERHEAT, MOVE_HIDDEN_POWER, MOVE_DRAGON_CLAW},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {4, 0, 0, 252, 0, 252},
        .nature = NATURE_MODEST,
        .ability = ABILITY_BLAZE,
        .hiddenPowerType = TYPE_GRASS
    },
    [FRONTIER_MON_WARTORTLE_1] = {
        .species = SPECIES_WARTORTLE,
        .moves = {MOVE_RAPID_SPIN, MOVE_SURF, MOVE_TOXIC, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {248, 0, 188, 0, 0, 72},
        .nature = NATURE_BOLD,
        .ability = ABILITY_TORRENT,
        .hiddenPowerType = TYPE_GRASS
    },
    [FRONTIER_MON_PARASECT_1] = {
        .species = SPECIES_PARASECT,
        .moves = {MOVE_SPORE, MOVE_AROMATHERAPY, MOVE_SLUDGE_BOMB, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 64, 168, 0, 0, 24},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_EFFECT_SPORE,
        .hiddenPowerType = TYPE_ROCK
    },
    [FRONTIER_MON_MACHOKE_1] = {
        .species = SPECIES_MACHOKE,
        .moves = {MOVE_BULK_UP, MOVE_REVENGE, MOVE_HIDDEN_POWER, MOVE_EARTHQUAKE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {208, 252, 0, 0, 0, 44},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_GUTS,
        .hiddenPowerType = TYPE_GHOST
    },
    [FRONTIER_MON_HAUNTER_1] = {
        .species = SPECIES_HAUNTER,
        .moves = {MOVE_THUNDERBOLT, MOVE_PSYCHIC, MOVE_WILL_O_WISP, MOVE_TAUNT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {160, 0, 92, 32, 0, 224},
        .nature = NATURE_TIMID,
        .ability = ABILITY_LEVITATE
    },
    [FRONTIER_MON_BAYLEEF_1] = {
        .species = SPECIES_BAYLEEF,
        .moves = {MOVE_LIGHT_SCREEN, MOVE_GIGA_DRAIN, MOVE_SYNTHESIS, MOVE_REFLECT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 4, 0, 252, 0},
        .nature = NATURE_CALM,
        .ability = ABILITY_OVERGROW
    },
    [FRONTIER_MON_QUILAVA_1] = {
        .species = SPECIES_QUILAVA,
        .moves = {MOVE_FIRE_BLAST, MOVE_HIDDEN_POWER, MOVE_TOXIC, MOVE_QUICK_ATTACK},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {0, 4, 0, 252, 0, 252},
        .nature = NATURE_HASTY,
        .ability = ABILITY_BLAZE,
        .hiddenPowerType = TYPE_GRASS
    },
    [FRONTIER_MON_CROCONAW_1] = {
        .species = SPECIES_CROCONAW,
        .moves = {MOVE_SWORDS_DANCE, MOVE_DOUBLE_EDGE, MOVE_BRICK_BREAK, MOVE_SURF},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {0, 252, 0, 4, 0, 252},
        .nature = NATURE_NAIVE,
        .ability = ABILITY_TORRENT
    },
    [FRONTIER_MON_TOGETIC_1] = {
        .species = SPECIES_TOGETIC,
        .moves = {MOVE_ANCIENT_POWER, MOVE_BATON_PASS, MOVE_SOFT_BOILED, MOVE_FIRE_BLAST},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 120, 0, 136, 0},
        .nature = NATURE_CALM,
        .ability = ABILITY_SERENE_GRACE
    },
    [FRONTIER_MON_MURKROW_1] = {
        .species = SPECIES_MURKROW,
        .moves = {MOVE_DRILL_PECK, MOVE_HIDDEN_POWER, MOVE_SHADOW_BALL, MOVE_DOUBLE_EDGE},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {4, 252, 0, 0, 0, 252},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_INSOMNIA,
        .hiddenPowerType = TYPE_GROUND
    },
    [FRONTIER_MON_WOBBUFFET_1] = {
        .species = SPECIES_WOBBUFFET,
        .moves = {MOVE_COUNTER, MOVE_MIRROR_COAT, MOVE_ENCORE, MOVE_SAFEGUARD},
        .itemTableId = BATTLE_FRONTIER_ITEM_LUM_BERRY,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_DEFENSE,
        .evs = {4, 0, 252, 0, 252, 0},
        .nature = NATURE_CAREFUL,
        .ability = ABILITY_SHADOW_TAG
    },
    [FRONTIER_MON_PLUSLE_1] = {
        .species = SPECIES_PLUSLE,
        .moves = {MOVE_THUNDERBOLT, MOVE_HIDDEN_POWER, MOVE_SUBSTITUTE, MOVE_BATON_PASS},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {4, 0, 0, 252, 0, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_PLUS,
        .hiddenPowerType = TYPE_ICE
    },
    [FRONTIER_MON_MINUN_1] = {
        .species = SPECIES_MINUN,
        .moves = {MOVE_WISH, MOVE_ENCORE, MOVE_BATON_PASS, MOVE_THUNDERBOLT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_HP,
        .evs = {252, 0, 0, 4, 0, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_MINUS
    },
    [FRONTIER_MON_GROVYLE_1] = {
        .species = SPECIES_GROVYLE,
        .moves = {MOVE_SUBSTITUTE, MOVE_ENDEAVOR, MOVE_LEAF_BLADE, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_PETAYA_BERRY,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {0, 0, 0, 252, 4, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_OVERGROW,
        .hiddenPowerType = TYPE_ICE
    },
    [FRONTIER_MON_COMBUSKEN_1] = {
        .species = SPECIES_COMBUSKEN,
        .moves = {MOVE_FIRE_BLAST, MOVE_HIDDEN_POWER, MOVE_SKY_UPPERCUT, MOVE_SUBSTITUTE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {0, 4, 0, 252, 0, 252},
        .nature = NATURE_MILD,
        .ability = ABILITY_BLAZE,
        .hiddenPowerType = TYPE_GRASS
    },
    [FRONTIER_MON_MARSHTOMP_1] = {
        .species = SPECIES_MARSHTOMP,
        .moves = {MOVE_EARTHQUAKE, MOVE_ICE_BEAM, MOVE_TOXIC, MOVE_PROTECT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {248, 0, 192, 0, 24, 44},
        .nature = NATURE_RELAXED,
        .ability = ABILITY_TORRENT
    },
    [FRONTIER_MON_PONYTA_1] = {
        .species = SPECIES_PONYTA,
        .moves = {MOVE_DOUBLE_EDGE, MOVE_FIRE_BLAST, MOVE_HIDDEN_POWER, MOVE_QUICK_ATTACK},
        .itemTableId = BATTLE_FRONTIER_ITEM_SILK_SCARF,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {0, 252, 0, 4, 0, 252},
        .nature = NATURE_HASTY,
        .ability = ABILITY_FLASH_FIRE,
        .hiddenPowerType = TYPE_GRASS
    },
    [FRONTIER_MON_AZUMARILL_1] = {
        .species = SPECIES_AZUMARILL,
        .moves = {MOVE_SUBSTITUTE, MOVE_FOCUS_PUNCH, MOVE_HIDDEN_POWER, MOVE_SING},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {252, 252, 0, 0, 0, 4},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_HUGE_POWER,
        .hiddenPowerType = TYPE_GHOST
    },
    [FRONTIER_MON_SUDOWOODO_1] = {
        .species = SPECIES_SUDOWOODO,
        .moves = {MOVE_ROCK_SLIDE, MOVE_EARTHQUAKE, MOVE_FOCUS_PUNCH, MOVE_EXPLOSION},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {252, 252, 0, 0, 0, 4},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_ROCK_HEAD
    },
    [FRONTIER_MON_MAGCARGO_1] = {
        .species = SPECIES_MAGCARGO,
        .moves = {MOVE_FLAMETHROWER, MOVE_TOXIC, MOVE_REST, MOVE_SLEEP_TALK},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 0, 4, 252, 0},
        .nature = NATURE_SASSY,
        .ability = ABILITY_FLAME_BODY
    },
    [FRONTIER_MON_PUPITAR_1] = {
        .species = SPECIES_PUPITAR,
        .moves = {MOVE_DRAGON_DANCE, MOVE_EARTHQUAKE, MOVE_ROCK_SLIDE, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {0, 252, 4, 0, 0, 252},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_SHED_SKIN,
        .hiddenPowerType = TYPE_BUG
    },
    [FRONTIER_MON_SEALEO_1] = {
        .species = SPECIES_SEALEO,
        .moves = {MOVE_SURF, MOVE_ICE_BEAM, MOVE_HIDDEN_POWER, MOVE_SUBSTITUTE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {0, 0, 0, 236, 48, 224},
        .nature = NATURE_TIMID,
        .ability = ABILITY_THICK_FAT,
        .hiddenPowerType = TYPE_GRASS
    },
    [FRONTIER_MON_RATICATE_1] = {
        .species = SPECIES_RATICATE,
        .moves = {MOVE_DOUBLE_EDGE, MOVE_SHADOW_BALL, MOVE_HIDDEN_POWER, MOVE_QUICK_ATTACK},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {4, 252, 0, 0, 0, 252},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_GUTS,
        .hiddenPowerType = TYPE_GROUND
    },
    [FRONTIER_MON_MASQUERAIN_1] = {
        .species = SPECIES_MASQUERAIN,
        .moves = {MOVE_STUN_SPORE, MOVE_ICE_BEAM, MOVE_HYDRO_PUMP, MOVE_SUBSTITUTE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {4, 0, 0, 252, 0, 252},
        .nature = NATURE_MODEST,
        .ability = ABILITY_INTIMIDATE
    },
    [FRONTIER_MON_FURRET_1] = {
        .species = SPECIES_FURRET,
        .moves = {MOVE_DOUBLE_EDGE, MOVE_SHADOW_BALL, MOVE_FOCUS_PUNCH, MOVE_TRICK},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {4, 252, 0, 0, 0, 252},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_KEEN_EYE
    },
    [FRONTIER_MON_DUNSPARCE_1] = {
        .species = SPECIES_DUNSPARCE,
        .moves = {MOVE_HEADBUTT, MOVE_THUNDER_WAVE, MOVE_SHADOW_BALL, MOVE_ROCK_SLIDE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_DEFENSE | F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {204, 136, 64, 0, 104, 0},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_SERENE_GRACE
    },
    [FRONTIER_MON_DRAGONAIR_1] = {
        .species = SPECIES_DRAGONAIR,
        .moves = {MOVE_THUNDER_WAVE, MOVE_REST, MOVE_THUNDERBOLT, MOVE_ICE_BEAM},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_SPEED | F_EV_SPREAD_HP,
        .evs = {252, 0, 0, 0, 160, 96},
        .nature = NATURE_CALM,
        .ability = ABILITY_SHED_SKIN
    },
    [FRONTIER_MON_MIGHTYENA_1] = {
        .species = SPECIES_MIGHTYENA,
        .moves = {MOVE_HEAL_BELL, MOVE_ROAR, MOVE_CRUNCH, MOVE_TOXIC},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 252, 0, 4, 0},
        .nature = NATURE_BOLD,
        .ability = ABILITY_INTIMIDATE
    },
    [FRONTIER_MON_LINOONE_1] = {
        .species = SPECIES_LINOONE,
        .moves = {MOVE_BELLY_DRUM, MOVE_EXTREME_SPEED, MOVE_SHADOW_BALL, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {108, 252, 12, 0, 0, 136},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_PICKUP,
        .hiddenPowerType = TYPE_GROUND
    },
    [FRONTIER_MON_CASTFORM_1] = {
        .species = SPECIES_CASTFORM,
        .moves = {MOVE_THUNDERBOLT, MOVE_FLAMETHROWER, MOVE_ICE_BEAM, MOVE_THUNDER_WAVE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_HP,
        .evs = {128, 0, 0, 252, 128, 0},
        .nature = NATURE_MODEST,
        .ability = ABILITY_FORECAST
    },
    [FRONTIER_MON_SHELGON_1] = {
        .species = SPECIES_SHELGON,
        .moves = {MOVE_DRAGON_DANCE, MOVE_DOUBLE_EDGE, MOVE_ROCK_SLIDE, MOVE_BRICK_BREAK},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {4, 252, 0, 0, 0, 252},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_ROCK_HEAD
    },
    [FRONTIER_MON_METANG_1] = {
        .species = SPECIES_METANG,
        .moves = {MOVE_EARTHQUAKE, MOVE_PSYCHIC, MOVE_TOXIC, MOVE_PROTECT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 60, 0, 196, 0},
        .nature = NATURE_RELAXED,
        .ability = ABILITY_CLEAR_BODY
    },
    [FRONTIER_MON_WIGGLYTUFF_1] = {
        .species = SPECIES_WIGGLYTUFF,
        .moves = {MOVE_COUNTER, MOVE_WISH, MOVE_PROTECT, MOVE_RETURN},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 252, 0, 4, 0},
        .nature = NATURE_IMPISH,
        .ability = ABILITY_CUTE_CHARM
    },
    [FRONTIER_MON_SUNFLORA_1] = {
        .species = SPECIES_SUNFLORA,
        .moves = {MOVE_SUNNY_DAY, MOVE_SOLAR_BEAM, MOVE_HIDDEN_POWER, MOVE_GRASS_WHISTLE},
        .itemTableId = BATTLE_FRONTIER_ITEM_PETAYA_BERRY,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {36, 0, 0, 252, 0, 220},
        .nature = NATURE_MODEST,
        .ability = ABILITY_CHLOROPHYLL,
        .hiddenPowerType = TYPE_FIRE
    },
    [FRONTIER_MON_CHIMECHO_1] = {
        .species = SPECIES_CHIMECHO,
        .moves = {MOVE_PSYCHIC, MOVE_HEAL_BELL, MOVE_CALM_MIND, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 188, 0, 20, 48},
        .nature = NATURE_BOLD,
        .ability = ABILITY_LEVITATE,
        .hiddenPowerType = TYPE_FIRE
    },
    [FRONTIER_MON_GLIGAR_1] = {
        .species = SPECIES_GLIGAR,
        .moves = {MOVE_SWORDS_DANCE, MOVE_EARTHQUAKE, MOVE_AERIAL_ACE, MOVE_QUICK_ATTACK},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {0, 252, 0, 0, 4, 252},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_HYPER_CUTTER
    },
    [FRONTIER_MON_QWILFISH_1] = {
        .species = SPECIES_QWILFISH,
        .moves = {MOVE_SUBSTITUTE, MOVE_SLUDGE_BOMB, MOVE_DESTINY_BOND, MOVE_SPIKES},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_HP,
        .evs = {252, 0, 0, 0, 24, 232},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_POISON_POINT
    },
    [FRONTIER_MON_SNEASEL_1] = {
        .species = SPECIES_SNEASEL,
        .moves = {MOVE_SHADOW_BALL, MOVE_BRICK_BREAK, MOVE_DOUBLE_EDGE, MOVE_SWORDS_DANCE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LIECHI_BERRY,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {0, 252, 4, 0, 0, 252},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_INNER_FOCUS
    },
    [FRONTIER_MON_PELIPPER_1] = {
        .species = SPECIES_PELIPPER,
        .moves = {MOVE_SURF, MOVE_ICE_BEAM, MOVE_REST, MOVE_SLEEP_TALK},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 252, 0, 0, 4},
        .nature = NATURE_BOLD,
        .ability = ABILITY_KEEN_EYE
    },
    [FRONTIER_MON_SWELLOW_1] = {
        .species = SPECIES_SWELLOW,
        .moves = {MOVE_AERIAL_ACE, MOVE_RETURN, MOVE_HIDDEN_POWER, MOVE_FACADE},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {4, 252, 0, 0, 0, 252},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_GUTS,
        .hiddenPowerType = TYPE_FIGHTING
    },
    [FRONTIER_MON_LAIRON_1] = {
        .species = SPECIES_LAIRON,
        .moves = {MOVE_ROCK_SLIDE, MOVE_TOXIC, MOVE_ROAR, MOVE_PROTECT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 4, 0, 0, 252, 0},
        .nature = NATURE_CAREFUL,
        .ability = ABILITY_ROCK_HEAD
    },
    [FRONTIER_MON_TANGELA_1] = {
        .species = SPECIES_TANGELA,
        .moves = {MOVE_SUNNY_DAY, MOVE_SLEEP_POWDER, MOVE_SOLAR_BEAM, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {4, 0, 0, 252, 0, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_CHLOROPHYLL,
        .hiddenPowerType = TYPE_FIRE
    },
    [FRONTIER_MON_ARBOK_1] = {
        .species = SPECIES_ARBOK,
        .moves = {MOVE_SLUDGE_BOMB, MOVE_EARTHQUAKE, MOVE_ROCK_SLIDE, MOVE_SLEEP_TALK},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {0, 252, 4, 0, 0, 252},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_INTIMIDATE
    },
    [FRONTIER_MON_PERSIAN_1] = {
        .species = SPECIES_PERSIAN,
        .moves = {MOVE_RETURN, MOVE_HIDDEN_POWER, MOVE_SHADOW_BALL, MOVE_FAKE_OUT},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {80, 252, 0, 0, 0, 176},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_LIMBER,
        .hiddenPowerType = TYPE_GROUND
    },
    [FRONTIER_MON_SEADRA_1] = {
        .species = SPECIES_SEADRA,
        .moves = {MOVE_HYDRO_PUMP, MOVE_ICE_BEAM, MOVE_HIDDEN_POWER, MOVE_SUBSTITUTE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {4, 0, 0, 252, 0, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_POISON_POINT,
        .hiddenPowerType = TYPE_GRASS
    },
    [FRONTIER_MON_KECLEON_1] = {
        .species = SPECIES_KECLEON,
        .moves = {MOVE_RETURN, MOVE_SHADOW_BALL, MOVE_FOCUS_PUNCH, MOVE_TRICK},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {252, 252, 0, 0, 4, 0},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_COLOR_CHANGE
    },
    [FRONTIER_MON_VIGOROTH_1] = {
        .species = SPECIES_VIGOROTH,
        .moves = {MOVE_RETURN, MOVE_EARTHQUAKE, MOVE_SLACK_OFF, MOVE_BULK_UP},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 4, 0, 252, 0},
        .nature = NATURE_CAREFUL,
        .ability = ABILITY_VITAL_SPIRIT
    },
    [FRONTIER_MON_LUNATONE_1] = {
        .species = SPECIES_LUNATONE,
        .moves = {MOVE_HYPNOSIS, MOVE_PSYCHIC, MOVE_ICE_BEAM, MOVE_CALM_MIND},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 240, 0, 0, 16},
        .nature = NATURE_MODEST,
        .ability = ABILITY_LEVITATE
    },
    [FRONTIER_MON_SOLROCK_1] = {
        .species = SPECIES_SOLROCK,
        .moves = {MOVE_REFLECT, MOVE_EARTHQUAKE, MOVE_ROCK_SLIDE, MOVE_EXPLOSION},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 76, 176, 0, 0, 4},
        .nature = NATURE_IMPISH,
        .ability = ABILITY_LEVITATE
    },
    [FRONTIER_MON_NOCTOWL_1] = {
        .species = SPECIES_NOCTOWL,
        .moves = {MOVE_PSYCHIC, MOVE_HYPNOSIS, MOVE_WHIRLWIND, MOVE_REFLECT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_HP,
        .evs = {252, 0, 0, 92, 164, 0},
        .nature = NATURE_CALM,
        .ability = ABILITY_INSOMNIA
    },
    [FRONTIER_MON_SANDSLASH_1] = {
        .species = SPECIES_SANDSLASH,
        .moves = {MOVE_EARTHQUAKE, MOVE_ROCK_SLIDE, MOVE_HIDDEN_POWER, MOVE_SWORDS_DANCE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {128, 252, 0, 0, 0, 128},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_SAND_VEIL,
        .hiddenPowerType = TYPE_BUG
    },
    [FRONTIER_MON_VENOMOTH_1] = {
        .species = SPECIES_VENOMOTH,
        .moves = {MOVE_SLEEP_POWDER, MOVE_BATON_PASS, MOVE_SUBSTITUTE, MOVE_SLUDGE_BOMB},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_HP,
        .evs = {252, 0, 4, 0, 0, 252},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_SHIELD_DUST
    },
    [FRONTIER_MON_CHANSEY_1] = {
        .species = SPECIES_CHANSEY,
        .moves = {MOVE_SOFT_BOILED, MOVE_SEISMIC_TOSS, MOVE_THUNDER_WAVE, MOVE_AROMATHERAPY},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 252, 0, 4, 0},
        .nature = NATURE_BOLD,
        .ability = ABILITY_NATURAL_CURE
    },
    [FRONTIER_MON_SEAKING_1] = {
        .species = SPECIES_SEAKING,
        .moves = {MOVE_RAIN_DANCE, MOVE_SURF, MOVE_MEGAHORN, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_ATTACK,
        .evs = {0, 184, 0, 252, 0, 72},
        .nature = NATURE_MILD,
        .ability = ABILITY_SWIFT_SWIM,
        .hiddenPowerType = TYPE_GRASS
    },
    [FRONTIER_MON_JUMPLUFF_1] = {
        .species = SPECIES_JUMPLUFF,
        .moves = {MOVE_ENCORE, MOVE_LEECH_SEED, MOVE_SUBSTITUTE, MOVE_SLEEP_POWDER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_SPEED | F_EV_SPREAD_HP,
        .evs = {188, 0, 0, 0, 104, 216},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_CHLOROPHYLL
    },
    [FRONTIER_MON_PILOSWINE_1] = {
        .species = SPECIES_PILOSWINE,
        .moves = {MOVE_EARTHQUAKE, MOVE_ROCK_SLIDE, MOVE_ICE_BEAM, MOVE_DOUBLE_EDGE},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {212, 252, 0, 0, 0, 44},
        .nature = NATURE_NAUGHTY,
        .ability = ABILITY_OBLIVIOUS
    },
    [FRONTIER_MON_GOLBAT_1] = {
        .species = SPECIES_GOLBAT,
        .moves = {MOVE_SLUDGE_BOMB, MOVE_AERIAL_ACE, MOVE_HIDDEN_POWER, MOVE_QUICK_ATTACK},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {0, 252, 4, 0, 0, 252},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_INNER_FOCUS,
        .hiddenPowerType = TYPE_GROUND
    },
    [FRONTIER_MON_PRIMEAPE_1] = {
        .species = SPECIES_PRIMEAPE,
        .moves = {MOVE_CROSS_CHOP, MOVE_FOCUS_PUNCH, MOVE_ROCK_SLIDE, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {4, 252, 0, 0, 0, 252},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_VITAL_SPIRIT,
        .hiddenPowerType = TYPE_GHOST
    },
    [FRONTIER_MON_HITMONLEE_1] = {
        .species = SPECIES_HITMONLEE,
        .moves = {MOVE_BRICK_BREAK, MOVE_HIDDEN_POWER, MOVE_ROCK_SLIDE, MOVE_MACH_PUNCH},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {4, 252, 0, 0, 0, 252},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_LIMBER,
        .hiddenPowerType = TYPE_GHOST
    },
    [FRONTIER_MON_HITMONCHAN_1] = {
        .species = SPECIES_HITMONCHAN,
        .moves = {MOVE_SKY_UPPERCUT, MOVE_MACH_PUNCH, MOVE_HIDDEN_POWER, MOVE_ROCK_SLIDE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {88, 252, 0, 0, 0, 168},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_KEEN_EYE,
        .hiddenPowerType = TYPE_GHOST
    },
    [FRONTIER_MON_GIRAFARIG_1] = {
        .species = SPECIES_GIRAFARIG,
        .moves = {MOVE_PSYCHIC, MOVE_BATON_PASS, MOVE_CALM_MIND, MOVE_AMNESIA},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 4, 252, 0, 0, 0},
        .nature = NATURE_IMPISH,
        .ability = ABILITY_INNER_FOCUS
    },
    [FRONTIER_MON_HITMONTOP_1] = {
        .species = SPECIES_HITMONTOP,
        .moves = {MOVE_BULK_UP, MOVE_HI_JUMP_KICK, MOVE_ROCK_SLIDE, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {144, 252, 0, 0, 0, 112},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_INTIMIDATE,
        .hiddenPowerType = TYPE_GHOST
    },
    [FRONTIER_MON_BANETTE_1] = {
        .species = SPECIES_BANETTE,
        .moves = {MOVE_HIDDEN_POWER, MOVE_KNOCK_OFF, MOVE_SHADOW_BALL, MOVE_WILL_O_WISP},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {112, 252, 0, 0, 0, 144},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_INSOMNIA,
        .hiddenPowerType = TYPE_FIGHTING
    },
    [FRONTIER_MON_NINJASK_1] = {
        .species = SPECIES_NINJASK,
        .moves = {MOVE_SILVER_WIND, MOVE_AERIAL_ACE, MOVE_HIDDEN_POWER, MOVE_SHADOW_BALL},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {60, 252, 0, 0, 0, 196},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_SPEED_BOOST,
        .hiddenPowerType = TYPE_ROCK
    },
    [FRONTIER_MON_SEVIPER_1] = {
        .species = SPECIES_SEVIPER,
        .moves = {MOVE_SLUDGE_BOMB, MOVE_EARTHQUAKE, MOVE_FLAMETHROWER, MOVE_GIGA_DRAIN},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {0, 252, 0, 112, 0, 144},
        .nature = NATURE_NAUGHTY,
        .ability = ABILITY_SHED_SKIN
    },
    [FRONTIER_MON_ZANGOOSE_1] = {
        .species = SPECIES_ZANGOOSE,
        .moves = {MOVE_SWORDS_DANCE, MOVE_RETURN, MOVE_SHADOW_BALL, MOVE_BRICK_BREAK},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {4, 252, 0, 0, 0, 252},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_IMMUNITY
    },
    [FRONTIER_MON_CAMERUPT_1] = {
        .species = SPECIES_CAMERUPT,
        .moves = {MOVE_EARTHQUAKE, MOVE_FLAMETHROWER, MOVE_REST, MOVE_SLEEP_TALK},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {92, 176, 0, 0, 240, 0},
        .nature = NATURE_BRAVE,
        .ability = ABILITY_MAGMA_ARMOR
    },
    [FRONTIER_MON_SHARPEDO_1] = {
        .species = SPECIES_SHARPEDO,
        .moves = {MOVE_HYDRO_PUMP, MOVE_ICE_BEAM, MOVE_CRUNCH, MOVE_ENDURE},
        .itemTableId = BATTLE_FRONTIER_ITEM_SALAC_BERRY,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {4, 0, 0, 252, 0, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_ROUGH_SKIN
    },
    [FRONTIER_MON_TROPIUS_1] = {
        .species = SPECIES_TROPIUS,
        .moves = {MOVE_SUNNY_DAY, MOVE_SOLAR_BEAM, MOVE_SYNTHESIS, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {68, 0, 0, 208, 0, 232},
        .nature = NATURE_MODEST,
        .ability = ABILITY_CHLOROPHYLL,
        .hiddenPowerType = TYPE_FIRE
    },
    [FRONTIER_MON_MAGNETON_1] = {
        .species = SPECIES_MAGNETON,
        .moves = {MOVE_THUNDERBOLT, MOVE_HIDDEN_POWER, MOVE_TOXIC, MOVE_PROTECT},
        .itemTableId = BATTLE_FRONTIER_ITEM_MAGNET,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {4, 0, 0, 252, 0, 252},
        .nature = NATURE_MODEST,
        .ability = ABILITY_MAGNET_PULL,
        .hiddenPowerType = TYPE_FIRE
    },
    [FRONTIER_MON_MANTINE_1] = {
        .species = SPECIES_MANTINE,
        .moves = {MOVE_SURF, MOVE_ICE_BEAM, MOVE_TOXIC, MOVE_HAZE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 108, 0, 140, 8},
        .nature = NATURE_CALM,
        .ability = ABILITY_WATER_ABSORB
    },
    [FRONTIER_MON_STANTLER_1] = {
        .species = SPECIES_STANTLER,
        .moves = {MOVE_HYPNOSIS, MOVE_CALM_MIND, MOVE_THUNDERBOLT, MOVE_PSYCHIC},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED | F_EV_SPREAD_HP,
        .evs = {96, 0, 0, 252, 0, 160},
        .nature = NATURE_TIMID,
        .ability = ABILITY_INTIMIDATE
    },
    [FRONTIER_MON_ABSOL_1] = {
        .species = SPECIES_ABSOL,
        .moves = {MOVE_CALM_MIND, MOVE_THUNDERBOLT, MOVE_ICE_BEAM, MOVE_BATON_PASS},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_HP,
        .evs = {252, 0, 0, 40, 0, 216},
        .nature = NATURE_TIMID,
        .ability = ABILITY_PRESSURE
    },
    [FRONTIER_MON_SWALOT_1] = {
        .species = SPECIES_SWALOT,
        .moves = {MOVE_SLUDGE_BOMB, MOVE_HIDDEN_POWER, MOVE_SHADOW_BALL, MOVE_COUNTER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {232, 140, 0, 0, 136, 0},
        .nature = NATURE_CAREFUL,
        .ability = ABILITY_STICKY_HOLD,
        .hiddenPowerType = TYPE_GROUND
    },
    [FRONTIER_MON_CRAWDAUNT_1] = {
        .species = SPECIES_CRAWDAUNT,
        .moves = {MOVE_DOUBLE_EDGE, MOVE_HIDDEN_POWER, MOVE_BRICK_BREAK, MOVE_SURF},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {0, 252, 0, 132, 0, 124},
        .nature = NATURE_NAUGHTY,
        .ability = ABILITY_HYPER_CUTTER,
        .hiddenPowerType = TYPE_GHOST
    },
    [FRONTIER_MON_PIDGEOT_1] = {
        .species = SPECIES_PIDGEOT,
        .moves = {MOVE_DOUBLE_EDGE, MOVE_QUICK_ATTACK, MOVE_AERIAL_ACE, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {0, 252, 4, 0, 0, 252},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_KEEN_EYE,
        .hiddenPowerType = TYPE_GROUND
    },
    [FRONTIER_MON_GRUMPIG_1] = {
        .species = SPECIES_GRUMPIG,
        .moves = {MOVE_CALM_MIND, MOVE_PSYCHIC, MOVE_HIDDEN_POWER, MOVE_SUBSTITUTE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED | F_EV_SPREAD_HP,
        .evs = {200, 0, 0, 220, 0, 88},
        .nature = NATURE_CALM,
        .ability = ABILITY_THICK_FAT,
        .hiddenPowerType = TYPE_DARK
    },
    [FRONTIER_MON_TORKOAL_1] = {
        .species = SPECIES_TORKOAL,
        .moves = {MOVE_FLAMETHROWER, MOVE_EXPLOSION, MOVE_TOXIC, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_HP,
        .evs = {252, 0, 32, 148, 76, 0},
        .nature = NATURE_RELAXED,
        .ability = ABILITY_WHITE_SMOKE,
        .hiddenPowerType = TYPE_GRASS
    },
    [FRONTIER_MON_KINGLER_1] = {
        .species = SPECIES_KINGLER,
        .moves = {MOVE_DOUBLE_EDGE, MOVE_HIDDEN_POWER, MOVE_SURF, MOVE_BLIZZARD},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {0, 252, 0, 4, 0, 252},
        .nature = NATURE_NAUGHTY,
        .ability = ABILITY_HYPER_CUTTER,
        .hiddenPowerType = TYPE_GHOST
    },
    [FRONTIER_MON_CACTURNE_1] = {
        .species = SPECIES_CACTURNE,
        .moves = {MOVE_ENDURE, MOVE_DESTINY_BOND, MOVE_FOCUS_PUNCH, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_SALAC_BERRY,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {4, 252, 0, 0, 0, 252},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_SAND_VEIL,
        .hiddenPowerType = TYPE_GHOST
    },
    [FRONTIER_MON_BELLOSSOM_1] = {
        .species = SPECIES_BELLOSSOM,
        .moves = {MOVE_SUNNY_DAY, MOVE_SLEEP_POWDER, MOVE_SOLAR_BEAM, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {4, 0, 0, 252, 0, 252},
        .nature = NATURE_MODEST,
        .ability = ABILITY_CHLOROPHYLL,
        .hiddenPowerType = TYPE_FIRE
    },
    [FRONTIER_MON_OCTILLERY_1] = {
        .species = SPECIES_OCTILLERY,
        .moves = {MOVE_SURF, MOVE_FIRE_BLAST, MOVE_HIDDEN_POWER, MOVE_THUNDER_WAVE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_HP,
        .evs = {244, 0, 12, 252, 0, 0},
        .nature = NATURE_MODEST,
        .ability = ABILITY_SUCTION_CUPS,
        .hiddenPowerType = TYPE_ELECTRIC
    },
    [FRONTIER_MON_HUNTAIL_1] = {
        .species = SPECIES_HUNTAIL,
        .moves = {MOVE_RAIN_DANCE, MOVE_HYDRO_PUMP, MOVE_ICE_BEAM, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LUM_BERRY,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {4, 0, 0, 252, 0, 252},
        .nature = NATURE_MODEST,
        .ability = ABILITY_SWIFT_SWIM,
        .hiddenPowerType = TYPE_ELECTRIC
    },
    [FRONTIER_MON_GOREBYSS_1] = {
        .species = SPECIES_GOREBYSS,
        .moves = {MOVE_RAIN_DANCE, MOVE_SURF, MOVE_ICE_BEAM, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {56, 0, 0, 252, 0, 200},
        .nature = NATURE_MODEST,
        .ability = ABILITY_SWIFT_SWIM,
        .hiddenPowerType = TYPE_GRASS
    },
    [FRONTIER_MON_RELICANTH_1] = {
        .species = SPECIES_RELICANTH,
        .moves = {MOVE_ROCK_SLIDE, MOVE_EARTHQUAKE, MOVE_TOXIC, MOVE_PROTECT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 156, 0, 100, 0},
        .nature = NATURE_IMPISH,
        .ability = ABILITY_SWIFT_SWIM
    },
    [FRONTIER_MON_OMASTAR_1] = {
        .species = SPECIES_OMASTAR,
        .moves = {MOVE_RAIN_DANCE, MOVE_HYDRO_PUMP, MOVE_ICE_BEAM, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED | F_EV_SPREAD_HP,
        .evs = {116, 0, 0, 252, 0, 140},
        .nature = NATURE_MODEST,
        .ability = ABILITY_SWIFT_SWIM,
        .hiddenPowerType = TYPE_ELECTRIC
    },
    [FRONTIER_MON_KABUTOPS_1] = {
        .species = SPECIES_KABUTOPS,
        .moves = {MOVE_ENDURE, MOVE_FLAIL, MOVE_SWORDS_DANCE, MOVE_ROCK_SLIDE},
        .itemTableId = BATTLE_FRONTIER_ITEM_SALAC_BERRY,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {12, 252, 68, 0, 0, 176},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_BATTLE_ARMOR
    },
    [FRONTIER_MON_POLIWRATH_1] = {
        .species = SPECIES_POLIWRATH,
        .moves = {MOVE_SUBSTITUTE, MOVE_BELLY_DRUM, MOVE_BRICK_BREAK, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_SALAC_BERRY,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {28, 252, 24, 0, 0, 204},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_WATER_ABSORB,
        .hiddenPowerType = TYPE_GHOST
    },
    [FRONTIER_MON_SCYTHER_1] = {
        .species = SPECIES_SCYTHER,
        .moves = {MOVE_SILVER_WIND, MOVE_AERIAL_ACE, MOVE_DOUBLE_EDGE, MOVE_QUICK_ATTACK},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {4, 252, 0, 0, 0, 252},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_SWARM
    },
    [FRONTIER_MON_PINSIR_1] = {
        .species = SPECIES_PINSIR,
        .moves = {MOVE_SWORDS_DANCE, MOVE_EARTHQUAKE, MOVE_HIDDEN_POWER, MOVE_ROCK_SLIDE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {4, 252, 0, 0, 0, 252},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_HYPER_CUTTER,
        .hiddenPowerType = TYPE_BUG
    },
    [FRONTIER_MON_POLITOED_1] = {
        .species = SPECIES_POLITOED,
        .moves = {MOVE_SURF, MOVE_HYPNOSIS, MOVE_ICE_BEAM, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED | F_EV_SPREAD_HP,
        .evs = {252, 0, 0, 156, 0, 100},
        .nature = NATURE_BOLD,
        .ability = ABILITY_WATER_ABSORB,
        .hiddenPowerType = TYPE_GRASS
    },
    [FRONTIER_MON_CLOYSTER_1] = {
        .species = SPECIES_CLOYSTER,
        .moves = {MOVE_SPIKES, MOVE_SURF, MOVE_RAPID_SPIN, MOVE_EXPLOSION},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_SPEED,
        .evs = {72, 0, 0, 8, 244, 184},
        .nature = NATURE_TIMID,
        .ability = ABILITY_SHELL_ARMOR
    },
    [FRONTIER_MON_DELCATTY_2] = {
        .species = SPECIES_DELCATTY,
        .moves = {MOVE_BODY_SLAM, MOVE_WISH, MOVE_PROTECT, MOVE_HEAL_BELL},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_HP,
        .evs = {248, 0, 8, 0, 252, 0},
        .nature = NATURE_CAREFUL,
        .ability = ABILITY_CUTE_CHARM
    },
    [FRONTIER_MON_SABLEYE_2] = {
        .species = SPECIES_SABLEYE,
        .moves = {MOVE_SEISMIC_TOSS, MOVE_TOXIC, MOVE_KNOCK_OFF, MOVE_RECOVER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 252, 0, 4, 0},
        .nature = NATURE_IMPISH,
        .ability = ABILITY_KEEN_EYE
    },
    [FRONTIER_MON_LICKITUNG_2] = {
        .species = SPECIES_LICKITUNG,
        .moves = {MOVE_CURSE, MOVE_REST, MOVE_SLEEP_TALK, MOVE_BODY_SLAM},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 4, 0, 0, 252, 0},
        .nature = NATURE_CAREFUL,
        .ability = ABILITY_OWN_TEMPO
    },
    [FRONTIER_MON_WEEPINBELL_2] = {
        .species = SPECIES_WEEPINBELL,
        .moves = {MOVE_SLUDGE_BOMB, MOVE_RAZOR_LEAF, MOVE_SLEEP_POWDER, MOVE_REFLECT},
        .itemTableId = BATTLE_FRONTIER_ITEM_SCOPE_LENS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_DEFENSE,
        .nature = NATURE_QUIRKY
    },
    [FRONTIER_MON_GRAVELER_2] = {
        .species = SPECIES_GRAVELER,
        .moves = {MOVE_EARTHQUAKE, MOVE_ROCK_SLIDE, MOVE_TOXIC, MOVE_SUBSTITUTE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {0, 252, 4, 0, 0, 252},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_ROCK_HEAD
    },
    [FRONTIER_MON_GLOOM_2] = {
        .species = SPECIES_GLOOM,
        .moves = {MOVE_SUNNY_DAY, MOVE_SOLAR_BEAM, MOVE_HIDDEN_POWER, MOVE_SYNTHESIS},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {80, 0, 0, 252, 0, 176},
        .nature = NATURE_MODEST,
        .ability = ABILITY_CHLOROPHYLL,
        .hiddenPowerType = TYPE_FIRE
    },
    [FRONTIER_MON_PORYGON_2] = {
        .species = SPECIES_PORYGON,
        .moves = {MOVE_THUNDERBOLT, MOVE_ICE_BEAM, MOVE_RECOVER, MOVE_TOXIC},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 0, 4, 252, 0},
        .nature = NATURE_CALM,
        .ability = ABILITY_TRACE
    },
    [FRONTIER_MON_KADABRA_2] = {
        .species = SPECIES_KADABRA,
        .moves = {MOVE_PSYCHIC, MOVE_THUNDER_WAVE, MOVE_REFLECT, MOVE_RECOVER},
        .itemTableId = BATTLE_FRONTIER_ITEM_TWISTED_SPOON,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .nature = NATURE_MODEST
    },
    [FRONTIER_MON_WAILMER_2] = {
        .species = SPECIES_WAILMER,
        .moves = {MOVE_WATER_SPOUT, MOVE_SELF_DESTRUCT, MOVE_HYDRO_PUMP, MOVE_ROAR},
        .itemTableId = BATTLE_FRONTIER_ITEM_LUM_BERRY,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {4, 0, 0, 252, 0, 252},
        .nature = NATURE_HASTY,
        .ability = ABILITY_WATER_VEIL
    },
    [FRONTIER_MON_ROSELIA_2] = {
        .species = SPECIES_ROSELIA,
        .moves = {MOVE_SPIKES, MOVE_LEECH_SEED, MOVE_AROMATHERAPY, MOVE_STUN_SPORE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 4, 0, 252, 0},
        .nature = NATURE_CALM,
        .ability = ABILITY_NATURAL_CURE
    },
    [FRONTIER_MON_VOLBEAT_2] = {
        .species = SPECIES_VOLBEAT,
        .moves = {MOVE_TAIL_GLOW, MOVE_THUNDERBOLT, MOVE_ICE_PUNCH, MOVE_MOONLIGHT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {0, 0, 0, 252, 4, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_ILLUMINATE
    },
    [FRONTIER_MON_ILLUMISE_2] = {
        .species = SPECIES_ILLUMISE,
        .moves = {MOVE_THUNDERBOLT, MOVE_ICE_PUNCH, MOVE_WISH, MOVE_THUNDER_WAVE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 160, 96, 0, 0},
        .nature = NATURE_MODEST,
        .ability = ABILITY_OBLIVIOUS
    },
    [FRONTIER_MON_IVYSAUR_2] = {
        .species = SPECIES_IVYSAUR,
        .moves = {MOVE_LEECH_SEED, MOVE_SYNTHESIS, MOVE_SWORDS_DANCE, MOVE_SLUDGE_BOMB},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 0, 0, 224, 32},
        .nature = NATURE_CAREFUL,
        .ability = ABILITY_OVERGROW
    },
    [FRONTIER_MON_CHARMELEON_2] = {
        .species = SPECIES_CHARMELEON,
        .moves = {MOVE_FIRE_BLAST, MOVE_HIDDEN_POWER, MOVE_DRAGON_CLAW, MOVE_SUBSTITUTE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {0, 0, 0, 252, 4, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_BLAZE,
        .hiddenPowerType = TYPE_GRASS
    },
    [FRONTIER_MON_WARTORTLE_2] = {
        .species = SPECIES_WARTORTLE,
        .moves = {MOVE_RAPID_SPIN, MOVE_SURF, MOVE_FORESIGHT, MOVE_REST},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 252, 0, 4, 0},
        .nature = NATURE_BOLD,
        .ability = ABILITY_TORRENT
    },
    [FRONTIER_MON_PARASECT_2] = {
        .species = SPECIES_PARASECT,
        .moves = {MOVE_SPORE, MOVE_SWORDS_DANCE, MOVE_SLUDGE_BOMB, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {252, 252, 0, 0, 0, 4},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_EFFECT_SPORE,
        .hiddenPowerType = TYPE_ROCK
    },
    [FRONTIER_MON_MACHOKE_2] = {
        .species = SPECIES_MACHOKE,
        .moves = {MOVE_CROSS_CHOP, MOVE_HIDDEN_POWER, MOVE_ROCK_SLIDE, MOVE_EARTHQUAKE},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {208, 252, 0, 0, 0, 44},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_GUTS,
        .hiddenPowerType = TYPE_GHOST
    },
    [FRONTIER_MON_HAUNTER_2] = {
        .species = SPECIES_HAUNTER,
        .moves = {MOVE_THUNDERBOLT, MOVE_PSYCHIC, MOVE_EXPLOSION, MOVE_ENDURE},
        .itemTableId = BATTLE_FRONTIER_ITEM_SALAC_BERRY,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {4, 0, 0, 252, 0, 252},
        .nature = NATURE_HASTY,
        .ability = ABILITY_LEVITATE
    },
    [FRONTIER_MON_BAYLEEF_2] = {
        .species = SPECIES_BAYLEEF,
        .moves = {MOVE_GIGA_DRAIN, MOVE_LEECH_SEED, MOVE_SYNTHESIS, MOVE_TOXIC},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 220, 0, 0, 34},
        .nature = NATURE_BOLD,
        .ability = ABILITY_OVERGROW
    },
    [FRONTIER_MON_QUILAVA_2] = {
        .species = SPECIES_QUILAVA,
        .moves = {MOVE_FIRE_BLAST, MOVE_HIDDEN_POWER, MOVE_SUBSTITUTE, MOVE_TOXIC},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {0, 0, 4, 252, 0, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_BLAZE,
        .hiddenPowerType = TYPE_GRASS
    },
    [FRONTIER_MON_CROCONAW_2] = {
        .species = SPECIES_CROCONAW,
        .moves = {MOVE_SUBSTITUTE, MOVE_HYDRO_PUMP, MOVE_ICE_BEAM, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_SALAC_BERRY,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {4, 0, 0, 252, 0, 252},
        .nature = NATURE_MODEST,
        .ability = ABILITY_TORRENT,
        .hiddenPowerType = TYPE_GRASS
    },
    [FRONTIER_MON_TOGETIC_2] = {
        .species = SPECIES_TOGETIC,
        .moves = {MOVE_FOLLOW_ME, MOVE_COUNTER, MOVE_SOFT_BOILED, MOVE_THUNDER_WAVE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 160, 0, 96, 0},
        .nature = NATURE_CALM,
        .ability = ABILITY_SERENE_GRACE
    },
    [FRONTIER_MON_MURKROW_2] = {
        .species = SPECIES_MURKROW,
        .moves = {MOVE_DRILL_PECK, MOVE_THUNDER_WAVE, MOVE_TAUNT, MOVE_PURSUIT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {4, 252, 0, 0, 0, 252},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_INSOMNIA
    },
    [FRONTIER_MON_WOBBUFFET_2] = {
        .species = SPECIES_WOBBUFFET,
        .moves = {MOVE_COUNTER, MOVE_SAFEGUARD, MOVE_ENCORE, MOVE_DESTINY_BOND},
        .itemTableId = BATTLE_FRONTIER_ITEM_LUM_BERRY,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_DEFENSE,
        .evs = {0, 0, 252, 0, 44, 212},
        .nature = NATURE_TIMID,
        .ability = ABILITY_SHADOW_TAG
    },
    [FRONTIER_MON_PLUSLE_2] = {
        .species = SPECIES_PLUSLE,
        .moves = {MOVE_ENCORE, MOVE_THUNDERBOLT, MOVE_THUNDER_WAVE, MOVE_WISH},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {4, 0, 0, 252, 0, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_PLUS
    },
    [FRONTIER_MON_MINUN_2] = {
        .species = SPECIES_MINUN,
        .moves = {MOVE_THUNDER_WAVE, MOVE_BATON_PASS, MOVE_THUNDERBOLT, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {0, 0, 0, 252, 4, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_MINUS,
        .hiddenPowerType = TYPE_GRASS
    },
    [FRONTIER_MON_GROVYLE_2] = {
        .species = SPECIES_GROVYLE,
        .moves = {MOVE_SUBSTITUTE, MOVE_LEECH_SEED, MOVE_HIDDEN_POWER, MOVE_LEAF_BLADE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {0, 0, 0, 252, 4, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_OVERGROW,
        .hiddenPowerType = TYPE_ICE
    },
    [FRONTIER_MON_COMBUSKEN_2] = {
        .species = SPECIES_COMBUSKEN,
        .moves = {MOVE_ENDURE, MOVE_SWORDS_DANCE, MOVE_REVERSAL, MOVE_FIRE_BLAST},
        .itemTableId = BATTLE_FRONTIER_ITEM_SALAC_BERRY,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {0, 252, 0, 4, 0, 252},
        .nature = NATURE_HASTY,
        .ability = ABILITY_BLAZE
    },
    [FRONTIER_MON_MARSHTOMP_2] = {
        .species = SPECIES_MARSHTOMP,
        .moves = {MOVE_SUBSTITUTE, MOVE_HYDRO_PUMP, MOVE_ICE_BEAM, MOVE_EARTHQUAKE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {0, 4, 0, 252, 0, 252},
        .nature = NATURE_MILD,
        .ability = ABILITY_TORRENT
    },
    [FRONTIER_MON_PONYTA_2] = {
        .species = SPECIES_PONYTA,
        .moves = {MOVE_SUNNY_DAY, MOVE_FIRE_BLAST, MOVE_SOLAR_BEAM, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LUM_BERRY,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {0, 4, 0, 252, 0, 252},
        .nature = NATURE_HASTY,
        .ability = ABILITY_RUN_AWAY,
        .hiddenPowerType = TYPE_GROUND
    },
    [FRONTIER_MON_AZUMARILL_2] = {
        .species = SPECIES_AZUMARILL,
        .moves = {MOVE_FOCUS_PUNCH, MOVE_RETURN, MOVE_HIDDEN_POWER, MOVE_IRON_TAIL},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {240, 244, 0, 0, 0, 24},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_HUGE_POWER,
        .hiddenPowerType = TYPE_GHOST
    },
    [FRONTIER_MON_SUDOWOODO_2] = {
        .species = SPECIES_SUDOWOODO,
        .moves = {MOVE_SUBSTITUTE, MOVE_FOCUS_PUNCH, MOVE_ROCK_SLIDE, MOVE_EXPLOSION},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {252, 252, 0, 0, 0, 4},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_ROCK_HEAD
    },
    [FRONTIER_MON_MAGCARGO_2] = {
        .species = SPECIES_MAGCARGO,
        .moves = {MOVE_FLAMETHROWER, MOVE_HIDDEN_POWER, MOVE_TOXIC, MOVE_REFRESH},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED | F_EV_SPREAD_HP,
        .evs = {200, 0, 0, 136, 0, 172},
        .nature = NATURE_MODEST,
        .ability = ABILITY_FLAME_BODY,
        .hiddenPowerType = TYPE_GRASS
    },
    [FRONTIER_MON_PUPITAR_2] = {
        .species = SPECIES_PUPITAR,
        .moves = {MOVE_ROCK_SLIDE, MOVE_DRAGON_DANCE, MOVE_EARTHQUAKE, MOVE_PROTECT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {4, 252, 0, 0, 0, 252},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_SHED_SKIN
    },
    [FRONTIER_MON_SEALEO_2] = {
        .species = SPECIES_SEALEO,
        .moves = {MOVE_SURF, MOVE_ICE_BEAM, MOVE_HIDDEN_POWER, MOVE_YAWN},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {0, 0, 0, 236, 44, 228},
        .nature = NATURE_TIMID,
        .ability = ABILITY_THICK_FAT,
        .hiddenPowerType = TYPE_GRASS
    },
    [FRONTIER_MON_RATICATE_2] = {
        .species = SPECIES_RATICATE,
        .moves = {MOVE_SUBSTITUTE, MOVE_REVERSAL, MOVE_RETURN, MOVE_SHADOW_BALL},
        .itemTableId = BATTLE_FRONTIER_ITEM_LIECHI_BERRY,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {0, 252, 4, 0, 0, 252},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_GUTS
    },
    [FRONTIER_MON_MASQUERAIN_2] = {
        .species = SPECIES_MASQUERAIN,
        .moves = {MOVE_TOXIC, MOVE_PROTECT, MOVE_HYDRO_PUMP, MOVE_ICE_BEAM},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 252, 0, 4, 0},
        .nature = NATURE_BOLD,
        .ability = ABILITY_INTIMIDATE
    },
    [FRONTIER_MON_FURRET_2] = {
        .species = SPECIES_FURRET,
        .moves = {MOVE_ENDURE, MOVE_RETURN, MOVE_SHADOW_BALL, MOVE_REVERSAL},
        .itemTableId = BATTLE_FRONTIER_ITEM_SALAC_BERRY,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {0, 252, 4, 0, 0, 252},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_KEEN_EYE
    },
    [FRONTIER_MON_DUNSPARCE_2] = {
        .species = SPECIES_DUNSPARCE,
        .moves = {MOVE_BODY_SLAM, MOVE_HEADBUTT, MOVE_ANCIENT_POWER, MOVE_PURSUIT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_SP_ATTACK,
        .evs = {64, 0, 0, 192, 252, 0},
        .nature = NATURE_SASSY,
        .ability = ABILITY_SERENE_GRACE
    },
    [FRONTIER_MON_DRAGONAIR_2] = {
        .species = SPECIES_DRAGONAIR,
        .moves = {MOVE_DRAGON_DANCE, MOVE_DOUBLE_EDGE, MOVE_IRON_TAIL, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {0, 252, 0, 0, 4, 252},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_SHED_SKIN,
        .hiddenPowerType = TYPE_FIGHTING
    },
    [FRONTIER_MON_MIGHTYENA_2] = {
        .species = SPECIES_MIGHTYENA,
        .moves = {MOVE_HEAL_BELL, MOVE_RETURN, MOVE_SHADOW_BALL, MOVE_HOWL},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {40, 252, 0, 0, 0, 216},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_INTIMIDATE
    },
    [FRONTIER_MON_LINOONE_2] = {
        .species = SPECIES_LINOONE,
        .moves = {MOVE_SUBSTITUTE, MOVE_BELLY_DRUM, MOVE_FLAIL, MOVE_SHADOW_BALL},
        .itemTableId = BATTLE_FRONTIER_ITEM_SALAC_BERRY,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {120, 252, 0, 0, 0, 136},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_PICKUP
    },
    [FRONTIER_MON_CASTFORM_2] = {
        .species = SPECIES_CASTFORM,
        .moves = {MOVE_THUNDERBOLT, MOVE_FLAMETHROWER, MOVE_SOLAR_BEAM, MOVE_SUNNY_DAY},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_HP,
        .evs = {128, 0, 0, 252, 128, 0},
        .nature = NATURE_MODEST,
        .ability = ABILITY_FORECAST
    },
    [FRONTIER_MON_SHELGON_2] = {
        .species = SPECIES_SHELGON,
        .moves = {MOVE_WISH, MOVE_TOXIC, MOVE_PROTECT, MOVE_BODY_SLAM},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 252, 0, 0, 4},
        .nature = NATURE_IMPISH,
        .ability = ABILITY_ROCK_HEAD
    },
    [FRONTIER_MON_METANG_2] = {
        .species = SPECIES_METANG,
        .moves = {MOVE_PSYCHIC, MOVE_THUNDER_PUNCH, MOVE_EARTHQUAKE, MOVE_TOXIC},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_HP,
        .evs = {252, 0, 32, 216, 8, 0},
        .nature = NATURE_QUIET,
        .ability = ABILITY_CLEAR_BODY
    },
    [FRONTIER_MON_WIGGLYTUFF_2] = {
        .species = SPECIES_WIGGLYTUFF,
        .moves = {MOVE_REFLECT, MOVE_BODY_SLAM, MOVE_WISH, MOVE_THUNDER_WAVE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_DEFENSE,
        .evs = {4, 0, 252, 0, 252, 0},
        .nature = NATURE_IMPISH,
        .ability = ABILITY_CUTE_CHARM
    },
    [FRONTIER_MON_SUNFLORA_2] = {
        .species = SPECIES_SUNFLORA,
        .moves = {MOVE_RAZOR_LEAF, MOVE_HIDDEN_POWER, MOVE_LEECH_SEED, MOVE_SYNTHESIS},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 0, 68, 188, 0},
        .nature = NATURE_CALM,
        .ability = ABILITY_CHLOROPHYLL,
        .hiddenPowerType = TYPE_FIRE
    },
    [FRONTIER_MON_CHIMECHO_2] = {
        .species = SPECIES_CHIMECHO,
        .moves = {MOVE_PSYCHIC, MOVE_TOXIC, MOVE_HEAL_BELL, MOVE_REFLECT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 156, 0, 100, 0},
        .nature = NATURE_BOLD,
        .ability = ABILITY_LEVITATE
    },
    [FRONTIER_MON_GLIGAR_2] = {
        .species = SPECIES_GLIGAR,
        .moves = {MOVE_EARTHQUAKE, MOVE_AERIAL_ACE, MOVE_IRON_TAIL, MOVE_QUICK_ATTACK},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {0, 252, 0, 0, 4, 252},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_HYPER_CUTTER
    },
    [FRONTIER_MON_QWILFISH_2] = {
        .species = SPECIES_QWILFISH,
        .moves = {MOVE_SWORDS_DANCE, MOVE_SLUDGE_BOMB, MOVE_HYDRO_PUMP, MOVE_SELF_DESTRUCT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {4, 252, 0, 0, 0, 252},
        .nature = NATURE_HASTY,
        .ability = ABILITY_POISON_POINT
    },
    [FRONTIER_MON_SNEASEL_2] = {
        .species = SPECIES_SNEASEL,
        .moves = {MOVE_SWORDS_DANCE, MOVE_DOUBLE_EDGE, MOVE_HIDDEN_POWER, MOVE_SHADOW_BALL},
        .itemTableId = BATTLE_FRONTIER_ITEM_LUM_BERRY,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {68, 252, 0, 0, 0, 188},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_INNER_FOCUS,
        .hiddenPowerType = TYPE_GROUND
    },
    [FRONTIER_MON_PELIPPER_2] = {
        .species = SPECIES_PELIPPER,
        .moves = {MOVE_ICE_BEAM, MOVE_HYDRO_PUMP, MOVE_HIDDEN_POWER, MOVE_AGILITY},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {16, 0, 16, 252, 96, 128},
        .nature = NATURE_MODEST,
        .ability = ABILITY_KEEN_EYE,
        .hiddenPowerType = TYPE_GRASS
    },
    [FRONTIER_MON_SWELLOW_2] = {
        .species = SPECIES_SWELLOW,
        .moves = {MOVE_SUBSTITUTE, MOVE_ENDEAVOR, MOVE_BATON_PASS, MOVE_RETURN},
        .itemTableId = BATTLE_FRONTIER_ITEM_LIECHI_BERRY,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {4, 252, 0, 0, 0, 252},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_GUTS
    },
    [FRONTIER_MON_LAIRON_2] = {
        .species = SPECIES_LAIRON,
        .moves = {MOVE_ROCK_SLIDE, MOVE_IRON_TAIL, MOVE_EARTHQUAKE, MOVE_DOUBLE_EDGE},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {252, 252, 0, 0, 0, 4},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_ROCK_HEAD
    },
    [FRONTIER_MON_TANGELA_2] = {
        .species = SPECIES_TANGELA,
        .moves = {MOVE_SLEEP_POWDER, MOVE_STUN_SPORE, MOVE_GIGA_DRAIN, MOVE_LEECH_SEED},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 252, 0, 0, 4},
        .nature = NATURE_BOLD,
        .ability = ABILITY_CHLOROPHYLL
    },
    [FRONTIER_MON_ARBOK_2] = {
        .species = SPECIES_ARBOK,
        .moves = {MOVE_SLUDGE_BOMB, MOVE_EARTHQUAKE, MOVE_HIDDEN_POWER, MOVE_GLARE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {0, 252, 0, 0, 4, 252},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_INTIMIDATE,
        .hiddenPowerType = TYPE_GHOST
    },
    [FRONTIER_MON_PERSIAN_2] = {
        .species = SPECIES_PERSIAN,
        .moves = {MOVE_SUBSTITUTE, MOVE_RETURN, MOVE_SHADOW_BALL, MOVE_HYPNOSIS},
        .itemTableId = BATTLE_FRONTIER_ITEM_LIECHI_BERRY,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {80, 252, 0, 0, 0, 176},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_LIMBER
    },
    [FRONTIER_MON_SEADRA_2] = {
        .species = SPECIES_SEADRA,
        .moves = {MOVE_SURF, MOVE_ICE_BEAM, MOVE_HIDDEN_POWER, MOVE_SUBSTITUTE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {4, 0, 0, 252, 0, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_POISON_POINT,
        .hiddenPowerType = TYPE_GRASS
    },
    [FRONTIER_MON_KECLEON_2] = {
        .species = SPECIES_KECLEON,
        .moves = {MOVE_BODY_SLAM, MOVE_THUNDER_WAVE, MOVE_PROTECT, MOVE_SHADOW_BALL},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 4, 0, 0, 252, 0},
        .nature = NATURE_CAREFUL,
        .ability = ABILITY_COLOR_CHANGE
    },
    [FRONTIER_MON_VIGOROTH_2] = {
        .species = SPECIES_VIGOROTH,
        .moves = {MOVE_ENDURE, MOVE_FLAIL, MOVE_REVERSAL, MOVE_SHADOW_BALL},
        .itemTableId = BATTLE_FRONTIER_ITEM_SALAC_BERRY,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {0, 252, 0, 0, 4, 252},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_VITAL_SPIRIT
    },
    [FRONTIER_MON_LUNATONE_2] = {
        .species = SPECIES_LUNATONE,
        .moves = {MOVE_PSYCHIC, MOVE_ICE_BEAM, MOVE_CALM_MIND, MOVE_SUBSTITUTE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {236, 0, 244, 0, 0, 28},
        .nature = NATURE_BOLD,
        .ability = ABILITY_LEVITATE
    },
    [FRONTIER_MON_SOLROCK_2] = {
        .species = SPECIES_SOLROCK,
        .moves = {MOVE_ROCK_SLIDE, MOVE_EARTHQUAKE, MOVE_EXPLOSION, MOVE_SHADOW_BALL},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {248, 252, 0, 0, 0, 8},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_LEVITATE
    },
    [FRONTIER_MON_NOCTOWL_2] = {
        .species = SPECIES_NOCTOWL,
        .moves = {MOVE_RETURN, MOVE_AERIAL_ACE, MOVE_SHADOW_BALL, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {4, 252, 0, 0, 0, 252},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_INSOMNIA,
        .hiddenPowerType = TYPE_GROUND
    },
    [FRONTIER_MON_SANDSLASH_2] = {
        .species = SPECIES_SANDSLASH,
        .moves = {MOVE_EARTHQUAKE, MOVE_ROCK_SLIDE, MOVE_HIDDEN_POWER, MOVE_RETURN},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {128, 252, 0, 0, 0, 128},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_SAND_VEIL,
        .hiddenPowerType = TYPE_BUG
    },
    [FRONTIER_MON_VENOMOTH_2] = {
        .species = SPECIES_VENOMOTH,
        .moves = {MOVE_SLEEP_POWDER, MOVE_SLUDGE_BOMB, MOVE_STUN_SPORE, MOVE_SIGNAL_BEAM},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {4, 252, 0, 0, 0, 252},
        .nature = NATURE_HASTY,
        .ability = ABILITY_SHIELD_DUST
    },
    [FRONTIER_MON_CHANSEY_2] = {
        .species = SPECIES_CHANSEY,
        .moves = {MOVE_SOFT_BOILED, MOVE_SEISMIC_TOSS, MOVE_THUNDER_WAVE, MOVE_TOXIC},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 252, 0, 4, 0},
        .nature = NATURE_BOLD,
        .ability = ABILITY_NATURAL_CURE
    },
    [FRONTIER_MON_SEAKING_2] = {
        .species = SPECIES_SEAKING,
        .moves = {MOVE_AGILITY, MOVE_SUBSTITUTE, MOVE_HYDRO_PUMP, MOVE_FLAIL},
        .itemTableId = BATTLE_FRONTIER_ITEM_LIECHI_BERRY,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_ATTACK,
        .evs = {0, 252, 0, 184, 0, 72},
        .nature = NATURE_LONELY,
        .ability = ABILITY_WATER_VEIL
    },
    [FRONTIER_MON_JUMPLUFF_2] = {
        .species = SPECIES_JUMPLUFF,
        .moves = {MOVE_AERIAL_ACE, MOVE_SWORDS_DANCE, MOVE_LEECH_SEED, MOVE_SLEEP_POWDER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {148, 140, 0, 0, 0, 220},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_CHLOROPHYLL
    },
    [FRONTIER_MON_PILOSWINE_2] = {
        .species = SPECIES_PILOSWINE,
        .moves = {MOVE_EARTHQUAKE, MOVE_ICE_BEAM, MOVE_TOXIC, MOVE_PROTECT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {248, 16, 152, 92, 0, 0},
        .nature = NATURE_BRAVE,
        .ability = ABILITY_OBLIVIOUS
    },
    [FRONTIER_MON_GOLBAT_2] = {
        .species = SPECIES_GOLBAT,
        .moves = {MOVE_SLUDGE_BOMB, MOVE_AERIAL_ACE, MOVE_HIDDEN_POWER, MOVE_GIGA_DRAIN},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {4, 252, 0, 0, 0, 252},
        .nature = NATURE_NAIVE,
        .ability = ABILITY_INNER_FOCUS,
        .hiddenPowerType = TYPE_GROUND
    },
    [FRONTIER_MON_PRIMEAPE_2] = {
        .species = SPECIES_PRIMEAPE,
        .moves = {MOVE_BULK_UP, MOVE_CROSS_CHOP, MOVE_SUBSTITUTE, MOVE_ROCK_SLIDE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {32, 252, 0, 0, 0, 224},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_VITAL_SPIRIT
    },
    [FRONTIER_MON_HITMONLEE_2] = {
        .species = SPECIES_HITMONLEE,
        .moves = {MOVE_BRICK_BREAK, MOVE_HIDDEN_POWER, MOVE_EARTHQUAKE, MOVE_MACH_PUNCH},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {4, 252, 0, 0, 0, 252},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_LIMBER,
        .hiddenPowerType = TYPE_GHOST
    },
    [FRONTIER_MON_HITMONCHAN_2] = {
        .species = SPECIES_HITMONCHAN,
        .moves = {MOVE_SKY_UPPERCUT, MOVE_HIDDEN_POWER, MOVE_EARTHQUAKE, MOVE_MACH_PUNCH},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {0, 252, 0, 0, 4, 252},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_KEEN_EYE,
        .hiddenPowerType = TYPE_GHOST
    },
    [FRONTIER_MON_GIRAFARIG_2] = {
        .species = SPECIES_GIRAFARIG,
        .moves = {MOVE_PSYCHIC, MOVE_BATON_PASS, MOVE_CALM_MIND, MOVE_REST},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 4, 252, 0, 0, 0},
        .nature = NATURE_IMPISH,
        .ability = ABILITY_INNER_FOCUS
    },
    [FRONTIER_MON_HITMONTOP_2] = {
        .species = SPECIES_HITMONTOP,
        .moves = {MOVE_HI_JUMP_KICK, MOVE_HIDDEN_POWER, MOVE_MACH_PUNCH, MOVE_ROCK_SLIDE},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {4, 252, 0, 0, 0, 252},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_INTIMIDATE,
        .hiddenPowerType = TYPE_GHOST
    },
    [FRONTIER_MON_BANETTE_2] = {
        .species = SPECIES_BANETTE,
        .moves = {MOVE_SHADOW_BALL, MOVE_HIDDEN_POWER, MOVE_WILL_O_WISP, MOVE_DESTINY_BOND},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {112, 252, 0, 0, 0, 144},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_INSOMNIA,
        .hiddenPowerType = TYPE_FIGHTING
    },
    [FRONTIER_MON_NINJASK_2] = {
        .species = SPECIES_NINJASK,
        .moves = {MOVE_SWORDS_DANCE, MOVE_SILVER_WIND, MOVE_AERIAL_ACE, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_SOFT_SAND,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_ATTACK,
        .evs = {68, 252, 188, 0, 0, 0},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_SPEED_BOOST,
        .hiddenPowerType = TYPE_GROUND
    },
    [FRONTIER_MON_SEVIPER_2] = {
        .species = SPECIES_SEVIPER,
        .moves = {MOVE_FLAMETHROWER, MOVE_GIGA_DRAIN, MOVE_CRUNCH, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {4, 0, 0, 252, 0, 252},
        .nature = NATURE_MODEST,
        .ability = ABILITY_SHED_SKIN,
        .hiddenPowerType = TYPE_PSYCHIC
    },
    [FRONTIER_MON_ZANGOOSE_2] = {
        .species = SPECIES_ZANGOOSE,
        .moves = {MOVE_COUNTER, MOVE_TAUNT, MOVE_RETURN, MOVE_SHADOW_BALL},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {28, 252, 0, 0, 0, 228},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_IMMUNITY
    },
    [FRONTIER_MON_CAMERUPT_2] = {
        .species = SPECIES_CAMERUPT,
        .moves = {MOVE_EARTHQUAKE, MOVE_OVERHEAT, MOVE_ROCK_SLIDE, MOVE_EXPLOSION},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_ATTACK,
        .evs = {4, 252, 0, 252, 0, 0},
        .nature = NATURE_BRAVE,
        .ability = ABILITY_MAGMA_ARMOR
    },
    [FRONTIER_MON_SHARPEDO_2] = {
        .species = SPECIES_SHARPEDO,
        .moves = {MOVE_EARTHQUAKE, MOVE_HIDDEN_POWER, MOVE_CRUNCH, MOVE_HYPER_BEAM},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {0, 252, 0, 4, 0, 252},
        .nature = NATURE_HASTY,
        .ability = ABILITY_ROUGH_SKIN,
        .hiddenPowerType = TYPE_FLYING
    },
    [FRONTIER_MON_TROPIUS_2] = {
        .species = SPECIES_TROPIUS,
        .moves = {MOVE_SWORDS_DANCE, MOVE_AERIAL_ACE, MOVE_EARTHQUAKE, MOVE_SUBSTITUTE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {196, 188, 0, 0, 124, 0},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_CHLOROPHYLL
    },
    [FRONTIER_MON_MAGNETON_2] = {
        .species = SPECIES_MAGNETON,
        .moves = {MOVE_THUNDERBOLT, MOVE_HIDDEN_POWER, MOVE_TOXIC, MOVE_THUNDER_WAVE},
        .itemTableId = BATTLE_FRONTIER_ITEM_MAGNET,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {4, 0, 0, 252, 0, 252},
        .nature = NATURE_MODEST,
        .ability = ABILITY_MAGNET_PULL,
        .hiddenPowerType = TYPE_FIRE
    },
    [FRONTIER_MON_MANTINE_2] = {
        .species = SPECIES_MANTINE,
        .moves = {MOVE_REST, MOVE_SLEEP_TALK, MOVE_SURF, MOVE_ICE_BEAM},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 108, 140, 0, 8},
        .nature = NATURE_CALM,
        .ability = ABILITY_WATER_ABSORB
    },
    [FRONTIER_MON_STANTLER_2] = {
        .species = SPECIES_STANTLER,
        .moves = {MOVE_RETURN, MOVE_SHADOW_BALL, MOVE_EARTHQUAKE, MOVE_THUNDERBOLT},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {4, 252, 0, 0, 0, 252},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_INTIMIDATE
    },
    [FRONTIER_MON_ABSOL_2] = {
        .species = SPECIES_ABSOL,
        .moves = {MOVE_SHADOW_BALL, MOVE_BODY_SLAM, MOVE_QUICK_ATTACK, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {4, 252, 0, 0, 0, 252},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_PRESSURE,
        .hiddenPowerType = TYPE_FIGHTING
    },
    [FRONTIER_MON_SWALOT_2] = {
        .species = SPECIES_SWALOT,
        .moves = {MOVE_SLUDGE_BOMB, MOVE_HIDDEN_POWER, MOVE_TOXIC, MOVE_EXPLOSION},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 4, 0, 252, 0},
        .nature = NATURE_CAREFUL,
        .ability = ABILITY_LIQUID_OOZE,
        .hiddenPowerType = TYPE_GROUND
    },
    [FRONTIER_MON_CRAWDAUNT_2] = {
        .species = SPECIES_CRAWDAUNT,
        .moves = {MOVE_SURF, MOVE_CRUNCH, MOVE_ICE_BEAM, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {4, 0, 0, 252, 0, 252},
        .nature = NATURE_MODEST,
        .ability = ABILITY_SHELL_ARMOR,
        .hiddenPowerType = TYPE_ELECTRIC
    },
    [FRONTIER_MON_PIDGEOT_2] = {
        .species = SPECIES_PIDGEOT,
        .moves = {MOVE_SUBSTITUTE, MOVE_RETURN, MOVE_AERIAL_ACE, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_SILK_SCARF,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {4, 252, 0, 0, 0, 252},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_KEEN_EYE,
        .hiddenPowerType = TYPE_GROUND
    },
    [FRONTIER_MON_GRUMPIG_2] = {
        .species = SPECIES_GRUMPIG,
        .moves = {MOVE_CALM_MIND, MOVE_PSYCHIC, MOVE_HIDDEN_POWER, MOVE_TRICK},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED | F_EV_SPREAD_HP,
        .evs = {200, 0, 0, 220, 0, 88},
        .nature = NATURE_CALM,
        .ability = ABILITY_THICK_FAT,
        .hiddenPowerType = TYPE_DARK
    },
    [FRONTIER_MON_TORKOAL_2] = {
        .species = SPECIES_TORKOAL,
        .moves = {MOVE_FIRE_BLAST, MOVE_EXPLOSION, MOVE_TOXIC, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_HP,
        .evs = {252, 0, 0, 92, 164, 0},
        .nature = NATURE_RELAXED,
        .ability = ABILITY_WHITE_SMOKE,
        .hiddenPowerType = TYPE_GRASS
    },
    [FRONTIER_MON_KINGLER_2] = {
        .species = SPECIES_KINGLER,
        .moves = {MOVE_ENDURE, MOVE_FLAIL, MOVE_HIDDEN_POWER, MOVE_SWORDS_DANCE},
        .itemTableId = BATTLE_FRONTIER_ITEM_SALAC_BERRY,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {0, 252, 0, 4, 0, 252},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_HYPER_CUTTER,
        .hiddenPowerType = TYPE_GHOST
    },
    [FRONTIER_MON_CACTURNE_2] = {
        .species = SPECIES_CACTURNE,
        .moves = {MOVE_SPIKES, MOVE_ENDURE, MOVE_DESTINY_BOND, MOVE_FAINT_ATTACK},
        .itemTableId = BATTLE_FRONTIER_ITEM_SALAC_BERRY,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_HP,
        .evs = {252, 0, 0, 0, 4, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_SAND_VEIL
    },
    [FRONTIER_MON_BELLOSSOM_2] = {
        .species = SPECIES_BELLOSSOM,
        .moves = {MOVE_SLEEP_POWDER, MOVE_GIGA_DRAIN, MOVE_STUN_SPORE, MOVE_MOONLIGHT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 4, 0, 252, 0},
        .nature = NATURE_CALM,
        .ability = ABILITY_CHLOROPHYLL
    },
    [FRONTIER_MON_OCTILLERY_2] = {
        .species = SPECIES_OCTILLERY,
        .moves = {MOVE_SURF, MOVE_ICE_BEAM, MOVE_HIDDEN_POWER, MOVE_SUBSTITUTE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED | F_EV_SPREAD_HP,
        .evs = {112, 0, 0, 252, 0, 144},
        .nature = NATURE_MODEST,
        .ability = ABILITY_SUCTION_CUPS,
        .hiddenPowerType = TYPE_ELECTRIC
    },
    [FRONTIER_MON_HUNTAIL_2] = {
        .species = SPECIES_HUNTAIL,
        .moves = {MOVE_RAIN_DANCE, MOVE_HYDRO_PUMP, MOVE_ICE_BEAM, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {4, 0, 0, 252, 0, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_SWIFT_SWIM,
        .hiddenPowerType = TYPE_GRASS
    },
    [FRONTIER_MON_GOREBYSS_2] = {
        .species = SPECIES_GOREBYSS,
        .moves = {MOVE_RAIN_DANCE, MOVE_HYDRO_PUMP, MOVE_ICE_BEAM, MOVE_SURF},
        .itemTableId = BATTLE_FRONTIER_ITEM_MYSTIC_WATER,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {0, 0, 4, 252, 0, 252},
        .nature = NATURE_MODEST,
        .ability = ABILITY_SWIFT_SWIM
    },
    [FRONTIER_MON_RELICANTH_2] = {
        .species = SPECIES_RELICANTH,
        .moves = {MOVE_ROCK_SLIDE, MOVE_EARTHQUAKE, MOVE_REST, MOVE_SLEEP_TALK},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 252, 0, 0, 4},
        .nature = NATURE_IMPISH,
        .ability = ABILITY_SWIFT_SWIM
    },
    [FRONTIER_MON_OMASTAR_2] = {
        .species = SPECIES_OMASTAR,
        .moves = {MOVE_SPIKES, MOVE_SURF, MOVE_ICE_BEAM, MOVE_THIEF},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 200, 56, 0, 0},
        .nature = NATURE_BOLD,
        .ability = ABILITY_SHELL_ARMOR
    },
    [FRONTIER_MON_KABUTOPS_2] = {
        .species = SPECIES_KABUTOPS,
        .moves = {MOVE_KNOCK_OFF, MOVE_ROCK_SLIDE, MOVE_HIDDEN_POWER, MOVE_SWORDS_DANCE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {70, 252, 0, 0, 0, 188},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_BATTLE_ARMOR,
        .hiddenPowerType = TYPE_GROUND
    },
    [FRONTIER_MON_POLIWRATH_2] = {
        .species = SPECIES_POLIWRATH,
        .moves = {MOVE_SUBSTITUTE, MOVE_HYPNOSIS, MOVE_FOCUS_PUNCH, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {252, 252, 0, 0, 4, 0},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_WATER_ABSORB,
        .hiddenPowerType = TYPE_GHOST
    },
    [FRONTIER_MON_SCYTHER_2] = {
        .species = SPECIES_SCYTHER,
        .moves = {MOVE_AERIAL_ACE, MOVE_REVERSAL, MOVE_SUBSTITUTE, MOVE_SWORDS_DANCE},
        .itemTableId = BATTLE_FRONTIER_ITEM_SALAC_BERRY,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {4, 252, 0, 0, 0, 252},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_SWARM
    },
    [FRONTIER_MON_PINSIR_2] = {
        .species = SPECIES_PINSIR,
        .moves = {MOVE_EARTHQUAKE, MOVE_HIDDEN_POWER, MOVE_ROCK_SLIDE, MOVE_RETURN},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {4, 252, 0, 0, 0, 252},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_HYPER_CUTTER,
        .hiddenPowerType = TYPE_BUG
    },
    [FRONTIER_MON_POLITOED_2] = {
        .species = SPECIES_POLITOED,
        .moves = {MOVE_SURF, MOVE_ICE_BEAM, MOVE_HIDDEN_POWER, MOVE_COUNTER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED | F_EV_SPREAD_HP,
        .evs = {136, 0, 4, 252, 20, 96},
        .nature = NATURE_MODEST,
        .ability = ABILITY_DAMP,
        .hiddenPowerType = TYPE_ELECTRIC
    },
    [FRONTIER_MON_CLOYSTER_2] = {
        .species = SPECIES_CLOYSTER,
        .moves = {MOVE_SPIKES, MOVE_EXPLOSION, MOVE_SURF, MOVE_ICE_BEAM},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_SPEED,
        .evs = {72, 0, 0, 8, 244, 184},
        .nature = NATURE_TIMID,
        .ability = ABILITY_SHELL_ARMOR
    },
    [FRONTIER_MON_DUGTRIO_1] = {
        .species = SPECIES_DUGTRIO,
        .moves = {MOVE_EARTHQUAKE, MOVE_HIDDEN_POWER, MOVE_AERIAL_ACE, MOVE_ROCK_SLIDE},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {0, 252, 0, 0, 4, 252},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_ARENA_TRAP,
        .hiddenPowerType = TYPE_BUG
    },
    [FRONTIER_MON_MEDICHAM_1] = {
        .species = SPECIES_MEDICHAM,
        .moves = {MOVE_BRICK_BREAK, MOVE_SHADOW_BALL, MOVE_ROCK_SLIDE, MOVE_FAKE_OUT},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {4, 252, 0, 0, 0, 252},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_PURE_POWER
    },
    [FRONTIER_MON_MISDREAVUS_1] = {
        .species = SPECIES_MISDREAVUS,
        .moves = {MOVE_MEAN_LOOK, MOVE_PERISH_SONG, MOVE_TAUNT, MOVE_PROTECT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 176, 0, 16, 64},
        .nature = NATURE_CALM,
        .ability = ABILITY_LEVITATE
    },
    [FRONTIER_MON_FEAROW_1] = {
        .species = SPECIES_FEAROW,
        .moves = {MOVE_QUICK_ATTACK, MOVE_DRILL_PECK, MOVE_RETURN, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {4, 252, 0, 0, 0, 252},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_KEEN_EYE,
        .hiddenPowerType = TYPE_GROUND
    },
    [FRONTIER_MON_GRANBULL_1] = {
        .species = SPECIES_GRANBULL,
        .moves = {MOVE_BULK_UP, MOVE_RETURN, MOVE_EARTHQUAKE, MOVE_REST},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {172, 252, 16, 0, 12, 56},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_INTIMIDATE
    },
    [FRONTIER_MON_JYNX_1] = {
        .species = SPECIES_JYNX,
        .moves = {MOVE_CALM_MIND, MOVE_ICE_BEAM, MOVE_LOVELY_KISS, MOVE_SUBSTITUTE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {0, 0, 0, 252, 4, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_OBLIVIOUS
    },
    [FRONTIER_MON_DUSCLOPS_1] = {
        .species = SPECIES_DUSCLOPS,
        .moves = {MOVE_SHADOW_BALL, MOVE_FOCUS_PUNCH, MOVE_WILL_O_WISP, MOVE_PAIN_SPLIT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 36, 20, 0, 200, 0},
        .nature = NATURE_CAREFUL,
        .ability = ABILITY_PRESSURE
    },
    [FRONTIER_MON_DODRIO_1] = {
        .species = SPECIES_DODRIO,
        .moves = {MOVE_DRILL_PECK, MOVE_RETURN, MOVE_QUICK_ATTACK, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {4, 252, 0, 0, 0, 252},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_EARLY_BIRD,
        .hiddenPowerType = TYPE_GROUND
    },
    [FRONTIER_MON_MR_MIME_1] = {
        .species = SPECIES_MR_MIME,
        .moves = {MOVE_CALM_MIND, MOVE_PSYCHIC, MOVE_TORMENT, MOVE_TRICK},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 0, 40, 196, 20},
        .nature = NATURE_CALM,
        .ability = ABILITY_SOUNDPROOF
    },
    [FRONTIER_MON_LANTURN_1] = {
        .species = SPECIES_LANTURN,
        .moves = {MOVE_THUNDERBOLT, MOVE_SURF, MOVE_REST, MOVE_SLEEP_TALK},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_SP_ATTACK,
        .evs = {40, 0, 0, 252, 216, 0},
        .nature = NATURE_MODEST,
        .ability = ABILITY_VOLT_ABSORB
    },
    [FRONTIER_MON_BRELOOM_1] = {
        .species = SPECIES_BRELOOM,
        .moves = {MOVE_SPORE, MOVE_FOCUS_PUNCH, MOVE_SKY_UPPERCUT, MOVE_MACH_PUNCH},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {0, 252, 4, 0, 0, 252},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_EFFECT_SPORE
    },
    [FRONTIER_MON_FORRETRESS_1] = {
        .species = SPECIES_FORRETRESS,
        .moves = {MOVE_SPIKES, MOVE_RAPID_SPIN, MOVE_HIDDEN_POWER, MOVE_EARTHQUAKE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 4, 0, 252, 0},
        .nature = NATURE_CAREFUL,
        .ability = ABILITY_STURDY,
        .hiddenPowerType = TYPE_BUG
    },
    [FRONTIER_MON_WHISCASH_1] = {
        .species = SPECIES_WHISCASH,
        .moves = {MOVE_EARTHQUAKE, MOVE_SUBSTITUTE, MOVE_ICE_BEAM, MOVE_TOXIC},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {172, 160, 0, 0, 0, 176},
        .nature = NATURE_NAUGHTY,
        .ability = ABILITY_OBLIVIOUS
    },
    [FRONTIER_MON_XATU_1] = {
        .species = SPECIES_XATU,
        .moves = {MOVE_REFLECT, MOVE_CALM_MIND, MOVE_PSYCHIC, MOVE_WISH},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_HP,
        .evs = {252, 0, 0, 32, 0, 224},
        .nature = NATURE_TIMID,
        .ability = ABILITY_SYNCHRONIZE
    },
    [FRONTIER_MON_SKARMORY_1] = {
        .species = SPECIES_SKARMORY,
        .moves = {MOVE_SPIKES, MOVE_ROAR, MOVE_DRILL_PECK, MOVE_PROTECT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 0, 0, 252, 4},
        .nature = NATURE_CAREFUL,
        .ability = ABILITY_KEEN_EYE
    },
    [FRONTIER_MON_MAROWAK_1] = {
        .species = SPECIES_MAROWAK,
        .moves = {MOVE_SWORDS_DANCE, MOVE_BONEMERANG, MOVE_ROCK_SLIDE, MOVE_FOCUS_PUNCH},
        .itemTableId = BATTLE_FRONTIER_ITEM_THICK_CLUB,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {4, 252, 0, 0, 0, 252},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_ROCK_HEAD
    },
    [FRONTIER_MON_QUAGSIRE_1] = {
        .species = SPECIES_QUAGSIRE,
        .moves = {MOVE_REST, MOVE_CURSE, MOVE_EARTHQUAKE, MOVE_ICE_BEAM},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 216, 0, 40, 0},
        .nature = NATURE_RELAXED,
        .ability = ABILITY_WATER_ABSORB
    },
    [FRONTIER_MON_CLEFABLE_1] = {
        .species = SPECIES_CLEFABLE,
        .moves = {MOVE_BELLY_DRUM, MOVE_METEOR_MASH, MOVE_SOFT_BOILED, MOVE_RETURN},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {224, 252, 0, 0, 0, 32},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_CUTE_CHARM
    },
    [FRONTIER_MON_HARIYAMA_1] = {
        .species = SPECIES_HARIYAMA,
        .moves = {MOVE_CROSS_CHOP, MOVE_BRICK_BREAK, MOVE_KNOCK_OFF, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_ATTACK,
        .evs = {0, 252, 228, 0, 0, 28},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_GUTS,
        .hiddenPowerType = TYPE_BUG
    },
    [FRONTIER_MON_RAICHU_1] = {
        .species = SPECIES_RAICHU,
        .moves = {MOVE_THUNDERBOLT, MOVE_ENCORE, MOVE_HIDDEN_POWER, MOVE_FOCUS_PUNCH},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {0, 68, 0, 200, 0, 240},
        .nature = NATURE_MILD,
        .ability = ABILITY_STATIC,
        .hiddenPowerType = TYPE_GRASS
    },
    [FRONTIER_MON_DEWGONG_1] = {
        .species = SPECIES_DEWGONG,
        .moves = {MOVE_ICE_BEAM, MOVE_SURF, MOVE_REST, MOVE_SLEEP_TALK},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 0, 0, 252, 4},
        .nature = NATURE_CALM,
        .ability = ABILITY_THICK_FAT
    },
    [FRONTIER_MON_MANECTRIC_1] = {
        .species = SPECIES_MANECTRIC,
        .moves = {MOVE_THUNDERBOLT, MOVE_SUBSTITUTE, MOVE_CRUNCH, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_PETAYA_BERRY,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {44, 0, 0, 248, 0, 216},
        .nature = NATURE_TIMID,
        .ability = ABILITY_STATIC,
        .hiddenPowerType = TYPE_ICE
    },
    [FRONTIER_MON_VILEPLUME_1] = {
        .species = SPECIES_VILEPLUME,
        .moves = {MOVE_SUNNY_DAY, MOVE_SLEEP_POWDER, MOVE_SOLAR_BEAM, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED | F_EV_SPREAD_HP,
        .evs = {120, 0, 0, 252, 0, 136},
        .nature = NATURE_MODEST,
        .ability = ABILITY_CHLOROPHYLL,
        .hiddenPowerType = TYPE_FIRE
    },
    [FRONTIER_MON_VICTREEBEL_1] = {
        .species = SPECIES_VICTREEBEL,
        .moves = {MOVE_SWORDS_DANCE, MOVE_SLUDGE_BOMB, MOVE_HIDDEN_POWER, MOVE_SLEEP_POWDER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {0, 252, 4, 0, 0, 252},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_CHLOROPHYLL,
        .hiddenPowerType = TYPE_GROUND
    },
    [FRONTIER_MON_ELECTRODE_1] = {
        .species = SPECIES_ELECTRODE,
        .moves = {MOVE_THUNDERBOLT, MOVE_THUNDER_WAVE, MOVE_HIDDEN_POWER, MOVE_EXPLOSION},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {0, 164, 0, 168, 0, 176},
        .nature = NATURE_HASTY,
        .ability = ABILITY_STATIC,
        .hiddenPowerType = TYPE_GRASS
    },
    [FRONTIER_MON_EXPLOUD_1] = {
        .species = SPECIES_EXPLOUD,
        .moves = {MOVE_RETURN, MOVE_SHADOW_BALL, MOVE_SUBSTITUTE, MOVE_FLAMETHROWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {220, 180, 0, 0, 0, 108},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_SOUNDPROOF
    },
    [FRONTIER_MON_SHIFTRY_1] = {
        .species = SPECIES_SHIFTRY,
        .moves = {MOVE_EXPLOSION, MOVE_SUNNY_DAY, MOVE_SOLAR_BEAM, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {136, 92, 0, 252, 0, 28},
        .nature = NATURE_RASH,
        .ability = ABILITY_CHLOROPHYLL,
        .hiddenPowerType = TYPE_FIRE
    },
    [FRONTIER_MON_GLALIE_1] = {
        .species = SPECIES_GLALIE,
        .moves = {MOVE_SPIKES, MOVE_EXPLOSION, MOVE_EARTHQUAKE, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_HP,
        .evs = {252, 4, 0, 0, 0, 252},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_INNER_FOCUS,
        .hiddenPowerType = TYPE_GRASS
    },
    [FRONTIER_MON_LUDICOLO_1] = {
        .species = SPECIES_LUDICOLO,
        .moves = {MOVE_SURF, MOVE_LEECH_SEED, MOVE_TOXIC, MOVE_SYNTHESIS},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 0, 36, 220, 0},
        .nature = NATURE_CALM,
        .ability = ABILITY_RAIN_DISH
    },
    [FRONTIER_MON_HYPNO_1] = {
        .species = SPECIES_HYPNO,
        .moves = {MOVE_SEISMIC_TOSS, MOVE_THUNDER_WAVE, MOVE_WISH, MOVE_REFLECT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 192, 0, 0, 64},
        .nature = NATURE_BOLD,
        .ability = ABILITY_INSOMNIA
    },
    [FRONTIER_MON_GOLEM_1] = {
        .species = SPECIES_GOLEM,
        .moves = {MOVE_EARTHQUAKE, MOVE_ROCK_SLIDE, MOVE_HIDDEN_POWER, MOVE_EXPLOSION},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {200, 220, 0, 0, 0, 88},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_ROCK_HEAD,
        .hiddenPowerType = TYPE_BUG
    },
    [FRONTIER_MON_RHYDON_1] = {
        .species = SPECIES_RHYDON,
        .moves = {MOVE_MEGAHORN, MOVE_EARTHQUAKE, MOVE_ROCK_SLIDE, MOVE_FOCUS_PUNCH},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {252, 252, 0, 0, 0, 4},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_ROCK_HEAD
    },
    [FRONTIER_MON_ALAKAZAM_1] = {
        .species = SPECIES_ALAKAZAM,
        .moves = {MOVE_CALM_MIND, MOVE_PSYCHIC, MOVE_FIRE_PUNCH, MOVE_THUNDER_PUNCH},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {4, 0, 0, 252, 0, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_SYNCHRONIZE
    },
    [FRONTIER_MON_WEEZING_1] = {
        .species = SPECIES_WEEZING,
        .moves = {MOVE_SLUDGE_BOMB, MOVE_HAZE, MOVE_WILL_O_WISP, MOVE_PAIN_SPLIT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 252, 0, 0, 4},
        .nature = NATURE_IMPISH,
        .ability = ABILITY_LEVITATE
    },
    [FRONTIER_MON_KANGASKHAN_1] = {
        .species = SPECIES_KANGASKHAN,
        .moves = {MOVE_RETURN, MOVE_SHADOW_BALL, MOVE_REST, MOVE_EARTHQUAKE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {212, 252, 0, 0, 0, 44},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_EARLY_BIRD
    },
    [FRONTIER_MON_ELECTABUZZ_1] = {
        .species = SPECIES_ELECTABUZZ,
        .moves = {MOVE_THUNDERBOLT, MOVE_ICE_PUNCH, MOVE_HIDDEN_POWER, MOVE_CROSS_CHOP},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {0, 84, 0, 208, 0, 216},
        .nature = NATURE_MODEST,
        .ability = ABILITY_STATIC,
        .hiddenPowerType = TYPE_GRASS
    },
    [FRONTIER_MON_TAUROS_1] = {
        .species = SPECIES_TAUROS,
        .moves = {MOVE_RETURN, MOVE_EARTHQUAKE, MOVE_HIDDEN_POWER, MOVE_DOUBLE_EDGE},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {4, 252, 0, 0, 0, 252},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_INTIMIDATE,
        .hiddenPowerType = TYPE_GHOST
    },
    [FRONTIER_MON_SLOWBRO_1] = {
        .species = SPECIES_SLOWBRO,
        .moves = {MOVE_SURF, MOVE_PSYCHIC, MOVE_REST, MOVE_CALM_MIND},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 176, 28, 52, 0},
        .nature = NATURE_BOLD,
        .ability = ABILITY_OWN_TEMPO
    },
    [FRONTIER_MON_SLOWKING_1] = {
        .species = SPECIES_SLOWKING,
        .moves = {MOVE_SURF, MOVE_PSYCHIC, MOVE_CALM_MIND, MOVE_REST},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 252, 0, 0, 4},
        .nature = NATURE_BOLD,
        .ability = ABILITY_OWN_TEMPO
    },
    [FRONTIER_MON_MILTANK_1] = {
        .species = SPECIES_MILTANK,
        .moves = {MOVE_CURSE, MOVE_BODY_SLAM, MOVE_MILK_DRINK, MOVE_HEAL_BELL},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {220, 192, 96, 0, 0, 0},
        .nature = NATURE_IMPISH,
        .ability = ABILITY_THICK_FAT
    },
    [FRONTIER_MON_ALTARIA_1] = {
        .species = SPECIES_ALTARIA,
        .moves = {MOVE_TOXIC, MOVE_FIRE_BLAST, MOVE_HAZE, MOVE_REST},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_HP,
        .evs = {248, 0, 44, 0, 216, 0},
        .nature = NATURE_CALM,
        .ability = ABILITY_NATURAL_CURE
    },
    [FRONTIER_MON_NIDOQUEEN_1] = {
        .species = SPECIES_NIDOQUEEN,
        .moves = {MOVE_EARTHQUAKE, MOVE_SHADOW_BALL, MOVE_SLUDGE_BOMB, MOVE_SUPERPOWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {4, 252, 0, 0, 0, 252},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_POISON_POINT
    },
    [FRONTIER_MON_NIDOKING_1] = {
        .species = SPECIES_NIDOKING,
        .moves = {MOVE_EARTHQUAKE, MOVE_MEGAHORN, MOVE_SLUDGE_BOMB, MOVE_ICE_BEAM},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {4, 252, 0, 0, 0, 252},
        .nature = NATURE_HASTY,
        .ability = ABILITY_POISON_POINT
    },
    [FRONTIER_MON_MAGMAR_1] = {
        .species = SPECIES_MAGMAR,
        .moves = {MOVE_FLAMETHROWER, MOVE_CROSS_CHOP, MOVE_THUNDER_PUNCH, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {0, 64, 0, 252, 0, 192},
        .nature = NATURE_MILD,
        .ability = ABILITY_FLAME_BODY,
        .hiddenPowerType = TYPE_GRASS
    },
    [FRONTIER_MON_CRADILY_1] = {
        .species = SPECIES_CRADILY,
        .moves = {MOVE_ROCK_SLIDE, MOVE_EARTHQUAKE, MOVE_TOXIC, MOVE_RECOVER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {252, 164, 0, 0, 92, 0},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_SUCTION_CUPS
    },
    [FRONTIER_MON_ARMALDO_1] = {
        .species = SPECIES_ARMALDO,
        .moves = {MOVE_HIDDEN_POWER, MOVE_RAPID_SPIN, MOVE_ROCK_BLAST, MOVE_SWORDS_DANCE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {188, 252, 68, 0, 0, 0},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_BATTLE_ARMOR,
        .hiddenPowerType = TYPE_BUG
    },
    [FRONTIER_MON_GOLDUCK_1] = {
        .species = SPECIES_GOLDUCK,
        .moves = {MOVE_CALM_MIND, MOVE_SURF, MOVE_ICE_BEAM, MOVE_ENDURE},
        .itemTableId = BATTLE_FRONTIER_ITEM_SALAC_BERRY,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {0, 0, 4, 252, 0, 252},
        .nature = NATURE_MODEST,
        .ability = ABILITY_CLOUD_NINE
    },
    [FRONTIER_MON_RAPIDASH_1] = {
        .species = SPECIES_RAPIDASH,
        .moves = {MOVE_SUNNY_DAY, MOVE_SOLAR_BEAM, MOVE_FLAMETHROWER, MOVE_HYPNOSIS},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {4, 0, 0, 252, 0, 252},
        .nature = NATURE_MODEST,
        .ability = ABILITY_FLASH_FIRE
    },
    [FRONTIER_MON_MUK_1] = {
        .species = SPECIES_MUK,
        .moves = {MOVE_SLUDGE_BOMB, MOVE_SUBSTITUTE, MOVE_FOCUS_PUNCH, MOVE_FIRE_BLAST},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {252, 196, 0, 0, 60, 0},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_STICKY_HOLD
    },
    [FRONTIER_MON_GENGAR_1] = {
        .species = SPECIES_GENGAR,
        .moves = {MOVE_WILL_O_WISP, MOVE_TAUNT, MOVE_THUNDERBOLT, MOVE_ICE_PUNCH},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_SPEED | F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {248, 0, 44, 8, 112, 96},
        .nature = NATURE_TIMID,
        .ability = ABILITY_LEVITATE
    },
    [FRONTIER_MON_AMPHAROS_1] = {
        .species = SPECIES_AMPHAROS,
        .moves = {MOVE_FOCUS_PUNCH, MOVE_THUNDERBOLT, MOVE_REFLECT, MOVE_THUNDER_WAVE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {252, 156, 0, 100, 0, 0},
        .nature = NATURE_QUIET,
        .ability = ABILITY_STATIC
    },
    [FRONTIER_MON_SCIZOR_1] = {
        .species = SPECIES_SCIZOR,
        .moves = {MOVE_SWORDS_DANCE, MOVE_SILVER_WIND, MOVE_HIDDEN_POWER, MOVE_AGILITY},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {108, 252, 0, 0, 0, 148},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_SWARM,
        .hiddenPowerType = TYPE_ROCK
    },
    [FRONTIER_MON_HERACROSS_1] = {
        .species = SPECIES_HERACROSS,
        .moves = {MOVE_MEGAHORN, MOVE_ROCK_SLIDE, MOVE_FOCUS_PUNCH, MOVE_SWORDS_DANCE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {4, 252, 0, 0, 0, 252},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_GUTS
    },
    [FRONTIER_MON_URSARING_1] = {
        .species = SPECIES_URSARING,
        .moves = {MOVE_RETURN, MOVE_EARTHQUAKE, MOVE_HIDDEN_POWER, MOVE_FOCUS_PUNCH},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {160, 252, 0, 0, 0, 96},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_GUTS,
        .hiddenPowerType = TYPE_GHOST
    },
    [FRONTIER_MON_HOUNDOOM_1] = {
        .species = SPECIES_HOUNDOOM,
        .moves = {MOVE_PURSUIT, MOVE_FIRE_BLAST, MOVE_CRUNCH, MOVE_WILL_O_WISP},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {56, 0, 0, 252, 12, 188},
        .nature = NATURE_TIMID,
        .ability = ABILITY_EARLY_BIRD
    },
    [FRONTIER_MON_DONPHAN_1] = {
        .species = SPECIES_DONPHAN,
        .moves = {MOVE_EARTHQUAKE, MOVE_ROCK_SLIDE, MOVE_REST, MOVE_RAPID_SPIN},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 44, 188, 0, 0, 24},
        .nature = NATURE_IMPISH,
        .ability = ABILITY_STURDY
    },
    [FRONTIER_MON_CLAYDOL_1] = {
        .species = SPECIES_CLAYDOL,
        .moves = {MOVE_RAPID_SPIN, MOVE_EARTHQUAKE, MOVE_PSYCHIC, MOVE_EXPLOSION},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {252, 216, 0, 32, 0, 8},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_LEVITATE
    },
    [FRONTIER_MON_WAILORD_1] = {
        .species = SPECIES_WAILORD,
        .moves = {MOVE_HYDRO_PUMP, MOVE_ICE_BEAM, MOVE_HIDDEN_POWER, MOVE_SELF_DESTRUCT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_SP_ATTACK,
        .evs = {0, 0, 0, 252, 172, 84},
        .nature = NATURE_MODEST,
        .ability = ABILITY_WATER_VEIL,
        .hiddenPowerType = TYPE_ELECTRIC
    },
    [FRONTIER_MON_NINETALES_1] = {
        .species = SPECIES_NINETALES,
        .moves = {MOVE_FIRE_BLAST, MOVE_GRUDGE, MOVE_HIDDEN_POWER, MOVE_WILL_O_WISP},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED | F_EV_SPREAD_HP,
        .evs = {100, 0, 0, 252, 0, 156},
        .nature = NATURE_TIMID,
        .ability = ABILITY_FLASH_FIRE,
        .hiddenPowerType = TYPE_GRASS
    },
    [FRONTIER_MON_MACHAMP_1] = {
        .species = SPECIES_MACHAMP,
        .moves = {MOVE_BULK_UP, MOVE_CROSS_CHOP, MOVE_ROCK_SLIDE, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {0, 252, 0, 0, 4, 252},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_GUTS,
        .hiddenPowerType = TYPE_GHOST
    },
    [FRONTIER_MON_SHUCKLE_1] = {
        .species = SPECIES_SHUCKLE,
        .moves = {MOVE_TOXIC, MOVE_ENCORE, MOVE_PROTECT, MOVE_WRAP},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 228, 0, 28, 0},
        .nature = NATURE_CAREFUL,
        .ability = ABILITY_STURDY
    },
    [FRONTIER_MON_STEELIX_1] = {
        .species = SPECIES_STEELIX,
        .moves = {MOVE_EARTHQUAKE, MOVE_TOXIC, MOVE_ROAR, MOVE_REST},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 36, 0, 0, 220, 0},
        .nature = NATURE_IMPISH,
        .ability = ABILITY_STURDY
    },
    [FRONTIER_MON_TENTACRUEL_1] = {
        .species = SPECIES_TENTACRUEL,
        .moves = {MOVE_HYDRO_PUMP, MOVE_SLUDGE_BOMB, MOVE_SUBSTITUTE, MOVE_SWORDS_DANCE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {0, 232, 0, 92, 0, 184},
        .nature = NATURE_HASTY,
        .ability = ABILITY_CLEAR_BODY
    },
    [FRONTIER_MON_AERODACTYL_1] = {
        .species = SPECIES_AERODACTYL,
        .moves = {MOVE_ROCK_SLIDE, MOVE_DOUBLE_EDGE, MOVE_EARTHQUAKE, MOVE_AERIAL_ACE},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {4, 252, 0, 0, 0, 252},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_ROCK_HEAD
    },
    [FRONTIER_MON_PORYGON2_1] = {
        .species = SPECIES_PORYGON2,
        .moves = {MOVE_RECOVER, MOVE_ICE_BEAM, MOVE_THUNDERBOLT, MOVE_THUNDER_WAVE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 200, 0, 56, 0},
        .nature = NATURE_BOLD,
        .ability = ABILITY_TRACE
    },
    [FRONTIER_MON_GARDEVOIR_1] = {
        .species = SPECIES_GARDEVOIR,
        .moves = {MOVE_CALM_MIND, MOVE_PSYCHIC, MOVE_THUNDERBOLT, MOVE_WILL_O_WISP},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {56, 0, 0, 252, 0, 200},
        .nature = NATURE_TIMID,
        .ability = ABILITY_TRACE
    },
    [FRONTIER_MON_EXEGGUTOR_1] = {
        .species = SPECIES_EXEGGUTOR,
        .moves = {MOVE_SLEEP_POWDER, MOVE_SUNNY_DAY, MOVE_SOLAR_BEAM, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {52, 0, 0, 252, 0, 204},
        .nature = NATURE_MODEST,
        .ability = ABILITY_CHLOROPHYLL,
        .hiddenPowerType = TYPE_ICE
    },
    [FRONTIER_MON_STARMIE_1] = {
        .species = SPECIES_STARMIE,
        .moves = {MOVE_HYDRO_PUMP, MOVE_ICE_BEAM, MOVE_THUNDERBOLT, MOVE_RAPID_SPIN},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {4, 0, 0, 252, 0, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_NATURAL_CURE
    },
    [FRONTIER_MON_FLYGON_1] = {
        .species = SPECIES_FLYGON,
        .moves = {MOVE_EARTHQUAKE, MOVE_ROCK_SLIDE, MOVE_HIDDEN_POWER, MOVE_SUBSTITUTE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {4, 252, 0, 0, 0, 252},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_LEVITATE,
        .hiddenPowerType = TYPE_BUG
    },
    [FRONTIER_MON_VENUSAUR_1] = {
        .species = SPECIES_VENUSAUR,
        .moves = {MOVE_SLEEP_POWDER, MOVE_LEECH_SEED, MOVE_SLUDGE_BOMB, MOVE_SYNTHESIS},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 84, 0, 148, 24},
        .nature = NATURE_CAREFUL,
        .ability = ABILITY_OVERGROW
    },
    [FRONTIER_MON_VAPOREON_1] = {
        .species = SPECIES_VAPOREON,
        .moves = {MOVE_SURF, MOVE_SUBSTITUTE, MOVE_WISH, MOVE_BATON_PASS},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {248, 0, 228, 0, 32, 0},
        .nature = NATURE_BOLD,
        .ability = ABILITY_WATER_ABSORB
    },
    [FRONTIER_MON_JOLTEON_1] = {
        .species = SPECIES_JOLTEON,
        .moves = {MOVE_THUNDERBOLT, MOVE_HIDDEN_POWER, MOVE_BATON_PASS, MOVE_ROAR},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {4, 0, 0, 252, 0, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_VOLT_ABSORB,
        .hiddenPowerType = TYPE_ICE
    },
    [FRONTIER_MON_FLAREON_1] = {
        .species = SPECIES_FLAREON,
        .moves = {MOVE_DOUBLE_EDGE, MOVE_FIRE_BLAST, MOVE_HIDDEN_POWER, MOVE_QUICK_ATTACK},
        .itemTableId = BATTLE_FRONTIER_ITEM_SILK_SCARF,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {0, 228, 0, 28, 0, 252},
        .nature = NATURE_LONELY,
        .ability = ABILITY_FLASH_FIRE,
        .hiddenPowerType = TYPE_GRASS
    },
    [FRONTIER_MON_MEGANIUM_1] = {
        .species = SPECIES_MEGANIUM,
        .moves = {MOVE_LEECH_SEED, MOVE_SYNTHESIS, MOVE_COUNTER, MOVE_LIGHT_SCREEN},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 132, 72, 52, 0},
        .nature = NATURE_BOLD,
        .ability = ABILITY_OVERGROW
    },
    [FRONTIER_MON_ESPEON_1] = {
        .species = SPECIES_ESPEON,
        .moves = {MOVE_PSYCHIC, MOVE_CALM_MIND, MOVE_SUBSTITUTE, MOVE_BATON_PASS},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_HP,
        .evs = {200, 0, 0, 0, 236, 72},
        .nature = NATURE_TIMID,
        .ability = ABILITY_SYNCHRONIZE
    },
    [FRONTIER_MON_UMBREON_1] = {
        .species = SPECIES_UMBREON,
        .moves = {MOVE_CURSE, MOVE_BATON_PASS, MOVE_TAUNT, MOVE_MOONLIGHT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 120, 0, 136, 0},
        .nature = NATURE_CAREFUL,
        .ability = ABILITY_SYNCHRONIZE
    },
    [FRONTIER_MON_BLASTOISE_1] = {
        .species = SPECIES_BLASTOISE,
        .moves = {MOVE_SURF, MOVE_ICE_BEAM, MOVE_RAPID_SPIN, MOVE_ROAR},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 148, 68, 0, 40},
        .nature = NATURE_BOLD,
        .ability = ABILITY_TORRENT
    },
    [FRONTIER_MON_FERALIGATR_1] = {
        .species = SPECIES_FERALIGATR,
        .moves = {MOVE_EARTHQUAKE, MOVE_AERIAL_ACE, MOVE_SWORDS_DANCE, MOVE_HYDRO_PUMP},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {170, 252, 0, 0, 0, 88},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_TORRENT
    },
    [FRONTIER_MON_AGGRON_1] = {
        .species = SPECIES_AGGRON,
        .moves = {MOVE_SUBSTITUTE, MOVE_FOCUS_PUNCH, MOVE_ROCK_SLIDE, MOVE_THUNDER_WAVE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {228, 252, 0, 0, 0, 28},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_STURDY
    },
    [FRONTIER_MON_BLAZIKEN_1] = {
        .species = SPECIES_BLAZIKEN,
        .moves = {MOVE_SWORDS_DANCE, MOVE_SKY_UPPERCUT, MOVE_ROCK_SLIDE, MOVE_FIRE_BLAST},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {80, 232, 0, 0, 0, 196},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_BLAZE
    },
    [FRONTIER_MON_WALREIN_1] = {
        .species = SPECIES_WALREIN,
        .moves = {MOVE_ICE_BEAM, MOVE_EARTHQUAKE, MOVE_SURF, MOVE_TOXIC},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 116, 140, 0, 0},
        .nature = NATURE_MODEST,
        .ability = ABILITY_THICK_FAT
    },
    [FRONTIER_MON_SCEPTILE_1] = {
        .species = SPECIES_SCEPTILE,
        .moves = {MOVE_LEAF_BLADE, MOVE_LEECH_SEED, MOVE_SUBSTITUTE, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {0, 0, 4, 252, 0, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_OVERGROW,
        .hiddenPowerType = TYPE_ICE
    },
    [FRONTIER_MON_CHARIZARD_1] = {
        .species = SPECIES_CHARIZARD,
        .moves = {MOVE_FIRE_BLAST, MOVE_HIDDEN_POWER, MOVE_SUBSTITUTE, MOVE_FOCUS_PUNCH},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {0, 4, 0, 252, 0, 252},
        .nature = NATURE_HASTY,
        .ability = ABILITY_BLAZE,
        .hiddenPowerType = TYPE_GRASS
    },
    [FRONTIER_MON_TYPHLOSION_1] = {
        .species = SPECIES_TYPHLOSION,
        .moves = {MOVE_SUBSTITUTE, MOVE_SUNNY_DAY, MOVE_FLAMETHROWER, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_PETAYA_BERRY,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {60, 0, 0, 252, 12, 184},
        .nature = NATURE_TIMID,
        .ability = ABILITY_BLAZE,
        .hiddenPowerType = TYPE_GRASS
    },
    [FRONTIER_MON_LAPRAS_1] = {
        .species = SPECIES_LAPRAS,
        .moves = {MOVE_ICE_BEAM, MOVE_THUNDERBOLT, MOVE_REST, MOVE_SLEEP_TALK},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 164, 92, 0, 0},
        .nature = NATURE_BOLD,
        .ability = ABILITY_WATER_ABSORB
    },
    [FRONTIER_MON_CROBAT_1] = {
        .species = SPECIES_CROBAT,
        .moves = {MOVE_SLUDGE_BOMB, MOVE_AERIAL_ACE, MOVE_HIDDEN_POWER, MOVE_SHADOW_BALL},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {80, 252, 0, 0, 0, 176},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_INNER_FOCUS,
        .hiddenPowerType = TYPE_GROUND
    },
    [FRONTIER_MON_SWAMPERT_1] = {
        .species = SPECIES_SWAMPERT,
        .moves = {MOVE_EARTHQUAKE, MOVE_ICE_BEAM, MOVE_PROTECT, MOVE_HYDRO_PUMP},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {248, 0, 216, 0, 44, 0},
        .nature = NATURE_RELAXED,
        .ability = ABILITY_TORRENT
    },
    [FRONTIER_MON_GYARADOS_1] = {
        .species = SPECIES_GYARADOS,
        .moves = {MOVE_DRAGON_DANCE, MOVE_HIDDEN_POWER, MOVE_EARTHQUAKE, MOVE_DOUBLE_EDGE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {68, 252, 0, 0, 0, 188},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_INTIMIDATE,
        .hiddenPowerType = TYPE_FLYING
    },
    [FRONTIER_MON_SNORLAX_1] = {
        .species = SPECIES_SNORLAX,
        .moves = {MOVE_CURSE, MOVE_BODY_SLAM, MOVE_SHADOW_BALL, MOVE_EARTHQUAKE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {204, 224, 60, 0, 8, 12},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_IMMUNITY
    },
    [FRONTIER_MON_KINGDRA_1] = {
        .species = SPECIES_KINGDRA,
        .moves = {MOVE_RAIN_DANCE, MOVE_SURF, MOVE_ICE_BEAM, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LUM_BERRY,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED | F_EV_SPREAD_HP,
        .evs = {104, 0, 0, 252, 0, 152},
        .nature = NATURE_MODEST,
        .ability = ABILITY_SWIFT_SWIM,
        .hiddenPowerType = TYPE_GRASS
    },
    [FRONTIER_MON_BLISSEY_1] = {
        .species = SPECIES_BLISSEY,
        .moves = {MOVE_SOFT_BOILED, MOVE_ICE_BEAM, MOVE_SEISMIC_TOSS, MOVE_THUNDER_WAVE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_DEFENSE,
        .evs = {0, 0, 252, 252, 0, 4},
        .nature = NATURE_MODEST,
        .ability = ABILITY_NATURAL_CURE
    },
    [FRONTIER_MON_MILOTIC_1] = {
        .species = SPECIES_MILOTIC,
        .moves = {MOVE_SURF, MOVE_RECOVER, MOVE_TOXIC, MOVE_REFRESH},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 252, 4, 0, 0},
        .nature = NATURE_BOLD,
        .ability = ABILITY_MARVEL_SCALE
    },
    [FRONTIER_MON_ARCANINE_1] = {
        .species = SPECIES_ARCANINE,
        .moves = {MOVE_HOWL, MOVE_HIDDEN_POWER, MOVE_EXTREME_SPEED, MOVE_FLAMETHROWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {252, 252, 0, 0, 0, 4},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_INTIMIDATE,
        .hiddenPowerType = TYPE_FIGHTING
    },
    [FRONTIER_MON_SALAMENCE_1] = {
        .species = SPECIES_SALAMENCE,
        .moves = {MOVE_DRAGON_CLAW, MOVE_BRICK_BREAK, MOVE_HIDDEN_POWER, MOVE_FIRE_BLAST},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {0, 4, 0, 252, 0, 252},
        .nature = NATURE_RASH,
        .ability = ABILITY_INTIMIDATE,
        .hiddenPowerType = TYPE_GRASS
    },
    [FRONTIER_MON_METAGROSS_1] = {
        .species = SPECIES_METAGROSS,
        .moves = {MOVE_METEOR_MASH, MOVE_EXPLOSION, MOVE_EARTHQUAKE, MOVE_ROCK_SLIDE},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {128, 252, 0, 0, 0, 128},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_CLEAR_BODY
    },
    [FRONTIER_MON_SLAKING_1] = {
        .species = SPECIES_SLAKING,
        .moves = {MOVE_RETURN, MOVE_EARTHQUAKE, MOVE_SHADOW_BALL, MOVE_FOCUS_PUNCH},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {4, 252, 0, 0, 0, 252},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_TRUANT
    },
    [FRONTIER_MON_DUGTRIO_2] = {
        .species = SPECIES_DUGTRIO,
        .moves = {MOVE_EARTHQUAKE, MOVE_ROCK_SLIDE, MOVE_PROTECT, MOVE_SLUDGE_BOMB},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {0, 252, 0, 0, 4, 252},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_ARENA_TRAP
    },
    [FRONTIER_MON_MEDICHAM_2] = {
        .species = SPECIES_MEDICHAM,
        .moves = {MOVE_BULK_UP, MOVE_BRICK_BREAK, MOVE_SHADOW_BALL, MOVE_RECOVER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {4, 252, 0, 0, 0, 252},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_PURE_POWER
    },
    [FRONTIER_MON_MAROWAK_2] = {
        .species = SPECIES_MAROWAK,
        .moves = {MOVE_BONEMERANG, MOVE_ROCK_SLIDE, MOVE_DOUBLE_EDGE, MOVE_FOCUS_PUNCH},
        .itemTableId = BATTLE_FRONTIER_ITEM_THICK_CLUB,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {252, 252, 0, 0, 4, 0},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_ROCK_HEAD
    },
    [FRONTIER_MON_QUAGSIRE_2] = {
        .species = SPECIES_QUAGSIRE,
        .moves = {MOVE_COUNTER, MOVE_EARTHQUAKE, MOVE_TOXIC, MOVE_SURF},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_DEFENSE | F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {252, 68, 100, 0, 88, 0},
        .nature = NATURE_RELAXED,
        .ability = ABILITY_WATER_ABSORB
    },
    [FRONTIER_MON_MISDREAVUS_2] = {
        .species = SPECIES_MISDREAVUS,
        .moves = {MOVE_CALM_MIND, MOVE_PSYCHIC, MOVE_THUNDERBOLT, MOVE_SUBSTITUTE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 176, 0, 16, 64},
        .nature = NATURE_CALM,
        .ability = ABILITY_LEVITATE
    },
    [FRONTIER_MON_FEAROW_2] = {
        .species = SPECIES_FEAROW,
        .moves = {MOVE_DRILL_PECK, MOVE_QUICK_ATTACK, MOVE_DOUBLE_EDGE, MOVE_TOXIC},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {4, 252, 0, 0, 0, 252},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_KEEN_EYE
    },
    [FRONTIER_MON_GRANBULL_2] = {
        .species = SPECIES_GRANBULL,
        .moves = {MOVE_FOCUS_PUNCH, MOVE_SUBSTITUTE, MOVE_RETURN, MOVE_SHADOW_BALL},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {196, 252, 36, 0, 24, 0},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_INTIMIDATE
    },
    [FRONTIER_MON_JYNX_2] = {
        .species = SPECIES_JYNX,
        .moves = {MOVE_MEAN_LOOK, MOVE_LOVELY_KISS, MOVE_CALM_MIND, MOVE_ICE_BEAM},
        .itemTableId = BATTLE_FRONTIER_ITEM_LUM_BERRY,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {0, 0, 0, 252, 4, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_OBLIVIOUS
    },
    [FRONTIER_MON_DUSCLOPS_2] = {
        .species = SPECIES_DUSCLOPS,
        .moves = {MOVE_CALM_MIND, MOVE_ICE_BEAM, MOVE_WILL_O_WISP, MOVE_PAIN_SPLIT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 200, 56, 0, 0},
        .nature = NATURE_BOLD,
        .ability = ABILITY_PRESSURE
    },
    [FRONTIER_MON_DODRIO_2] = {
        .species = SPECIES_DODRIO,
        .moves = {MOVE_SUBSTITUTE, MOVE_FLAIL, MOVE_AGILITY, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LIECHI_BERRY,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {248, 252, 8, 0, 0, 0},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_EARLY_BIRD,
        .hiddenPowerType = TYPE_GHOST
    },
    [FRONTIER_MON_MR_MIME_2] = {
        .species = SPECIES_MR_MIME,
        .moves = {MOVE_HYPNOSIS, MOVE_CALM_MIND, MOVE_PSYCHIC, MOVE_THUNDERBOLT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 0, 60, 196, 0},
        .nature = NATURE_CALM,
        .ability = ABILITY_SOUNDPROOF
    },
    [FRONTIER_MON_LANTURN_2] = {
        .species = SPECIES_LANTURN,
        .moves = {MOVE_THUNDERBOLT, MOVE_THUNDER_WAVE, MOVE_CONFUSE_RAY, MOVE_SURF},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_SP_ATTACK,
        .evs = {40, 0, 0, 252, 216, 0},
        .nature = NATURE_MODEST,
        .ability = ABILITY_VOLT_ABSORB
    },
    [FRONTIER_MON_BRELOOM_2] = {
        .species = SPECIES_BRELOOM,
        .moves = {MOVE_SPORE, MOVE_MACH_PUNCH, MOVE_BRICK_BREAK, MOVE_PROTECT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {4, 252, 0, 0, 0, 252},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_EFFECT_SPORE
    },
    [FRONTIER_MON_FORRETRESS_2] = {
        .species = SPECIES_FORRETRESS,
        .moves = {MOVE_SPIKES, MOVE_RAPID_SPIN, MOVE_HIDDEN_POWER, MOVE_REST},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 12, 0, 0, 244, 0},
        .nature = NATURE_IMPISH,
        .ability = ABILITY_STURDY,
        .hiddenPowerType = TYPE_BUG
    },
    [FRONTIER_MON_SKARMORY_2] = {
        .species = SPECIES_SKARMORY,
        .moves = {MOVE_SPIKES, MOVE_DRILL_PECK, MOVE_HIDDEN_POWER, MOVE_TAUNT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {4, 252, 0, 0, 0, 252},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_KEEN_EYE,
        .hiddenPowerType = TYPE_GROUND
    },
    [FRONTIER_MON_WHISCASH_2] = {
        .species = SPECIES_WHISCASH,
        .moves = {MOVE_EARTHQUAKE, MOVE_ICE_BEAM, MOVE_TOXIC, MOVE_PROTECT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 4, 252, 0, 0, 0},
        .nature = NATURE_RELAXED,
        .ability = ABILITY_OBLIVIOUS
    },
    [FRONTIER_MON_XATU_2] = {
        .species = SPECIES_XATU,
        .moves = {MOVE_CALM_MIND, MOVE_THIEF, MOVE_PSYCHIC, MOVE_WISH},
        .itemTableId = BATTLE_FRONTIER_ITEM_LUM_BERRY,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_HP,
        .evs = {252, 0, 0, 32, 0, 224},
        .nature = NATURE_TIMID,
        .ability = ABILITY_SYNCHRONIZE
    },
    [FRONTIER_MON_CLEFABLE_2] = {
        .species = SPECIES_CLEFABLE,
        .moves = {MOVE_CALM_MIND, MOVE_SOFT_BOILED, MOVE_ICE_BEAM, MOVE_THUNDERBOLT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_HP,
        .evs = {252, 0, 72, 184, 0, 0},
        .nature = NATURE_MODEST,
        .ability = ABILITY_CUTE_CHARM
    },
    [FRONTIER_MON_HARIYAMA_2] = {
        .species = SPECIES_HARIYAMA,
        .moves = {MOVE_KNOCK_OFF, MOVE_BRICK_BREAK, MOVE_REST, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_DEFENSE,
        .evs = {0, 4, 252, 0, 252, 0},
        .nature = NATURE_IMPISH,
        .ability = ABILITY_THICK_FAT,
        .hiddenPowerType = TYPE_BUG
    },
    [FRONTIER_MON_RAICHU_2] = {
        .species = SPECIES_RAICHU,
        .moves = {MOVE_THUNDERBOLT, MOVE_FOCUS_PUNCH, MOVE_SURF, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {4, 0, 0, 252, 0, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_STATIC,
        .hiddenPowerType = TYPE_ICE
    },
    [FRONTIER_MON_DEWGONG_2] = {
        .species = SPECIES_DEWGONG,
        .moves = {MOVE_ICE_BEAM, MOVE_SURF, MOVE_HIDDEN_POWER, MOVE_ENCORE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_HP,
        .evs = {252, 0, 0, 252, 0, 4},
        .nature = NATURE_MODEST,
        .ability = ABILITY_THICK_FAT,
        .hiddenPowerType = TYPE_ELECTRIC
    },
    [FRONTIER_MON_MANECTRIC_2] = {
        .species = SPECIES_MANECTRIC,
        .moves = {MOVE_THUNDERBOLT, MOVE_THUNDER_WAVE, MOVE_CRUNCH, MOVE_BITE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {4, 0, 0, 252, 0, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_STATIC
    },
    [FRONTIER_MON_VILEPLUME_2] = {
        .species = SPECIES_VILEPLUME,
        .moves = {MOVE_SLEEP_POWDER, MOVE_SWORDS_DANCE, MOVE_SLUDGE_BOMB, MOVE_MOONLIGHT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {224, 252, 0, 0, 0, 32},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_CHLOROPHYLL
    },
    [FRONTIER_MON_VICTREEBEL_2] = {
        .species = SPECIES_VICTREEBEL,
        .moves = {MOVE_SLUDGE_BOMB, MOVE_HIDDEN_POWER, MOVE_STUN_SPORE, MOVE_SYNTHESIS},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 176, 0, 60, 20},
        .nature = NATURE_IMPISH,
        .ability = ABILITY_CHLOROPHYLL,
        .hiddenPowerType = TYPE_GROUND
    },
    [FRONTIER_MON_ELECTRODE_2] = {
        .species = SPECIES_ELECTRODE,
        .moves = {MOVE_THUNDERBOLT, MOVE_SUBSTITUTE, MOVE_EXPLOSION, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_PETAYA_BERRY,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED | F_EV_SPREAD_DEFENSE,
        .evs = {12, 32, 116, 252, 0, 96},
        .nature = NATURE_RASH,
        .ability = ABILITY_STATIC,
        .hiddenPowerType = TYPE_ICE
    },
    [FRONTIER_MON_EXPLOUD_2] = {
        .species = SPECIES_EXPLOUD,
        .moves = {MOVE_DOUBLE_EDGE, MOVE_FIRE_BLAST, MOVE_COUNTER, MOVE_PROTECT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_DEFENSE | F_EV_SPREAD_ATTACK,
        .evs = {28, 204, 92, 0, 172, 12},
        .nature = NATURE_BRAVE,
        .ability = ABILITY_SOUNDPROOF
    },
    [FRONTIER_MON_SHIFTRY_2] = {
        .species = SPECIES_SHIFTRY,
        .moves = {MOVE_BRICK_BREAK, MOVE_SHADOW_BALL, MOVE_QUICK_ATTACK, MOVE_EXPLOSION},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {60, 252, 0, 0, 0, 196},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_EARLY_BIRD
    },
    [FRONTIER_MON_GLALIE_2] = {
        .species = SPECIES_GLALIE,
        .moves = {MOVE_SPIKES, MOVE_EXPLOSION, MOVE_TAUNT, MOVE_ICE_BEAM},
        .itemTableId = BATTLE_FRONTIER_ITEM_SALAC_BERRY,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_SPEED,
        .evs = {40, 0, 0, 0, 252, 216},
        .nature = NATURE_TIMID,
        .ability = ABILITY_INNER_FOCUS
    },
    [FRONTIER_MON_LUDICOLO_2] = {
        .species = SPECIES_LUDICOLO,
        .moves = {MOVE_RAIN_DANCE, MOVE_SURF, MOVE_ICE_BEAM, MOVE_LEECH_SEED},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED | F_EV_SPREAD_HP,
        .evs = {152, 0, 0, 252, 0, 104},
        .nature = NATURE_MODEST,
        .ability = ABILITY_SWIFT_SWIM
    },
    [FRONTIER_MON_HYPNO_2] = {
        .species = SPECIES_HYPNO,
        .moves = {MOVE_WISH, MOVE_PSYCHIC, MOVE_TOXIC, MOVE_THUNDER_WAVE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {96, 0, 80, 252, 80, 0},
        .nature = NATURE_HARDY,
        .ability = ABILITY_INSOMNIA
    },
    [FRONTIER_MON_GOLEM_2] = {
        .species = SPECIES_GOLEM,
        .moves = {MOVE_EARTHQUAKE, MOVE_ROCK_SLIDE, MOVE_REST, MOVE_SLEEP_TALK},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {252, 240, 16, 0, 0, 0},
        .nature = NATURE_IMPISH,
        .ability = ABILITY_STURDY
    },
    [FRONTIER_MON_RHYDON_2] = {
        .species = SPECIES_RHYDON,
        .moves = {MOVE_EARTHQUAKE, MOVE_SWORDS_DANCE, MOVE_ROCK_SLIDE, MOVE_SUBSTITUTE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {212, 122, 0, 0, 0, 176},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_LIGHTNING_ROD
    },
    [FRONTIER_MON_ALAKAZAM_2] = {
        .species = SPECIES_ALAKAZAM,
        .moves = {MOVE_PSYCHIC, MOVE_SUBSTITUTE, MOVE_FIRE_PUNCH, MOVE_TRICK},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {4, 0, 0, 252, 0, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_SYNCHRONIZE
    },
    [FRONTIER_MON_WEEZING_2] = {
        .species = SPECIES_WEEZING,
        .moves = {MOVE_SLUDGE_BOMB, MOVE_FLAMETHROWER, MOVE_HAZE, MOVE_EXPLOSION},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {252, 178, 0, 68, 0, 12},
        .nature = NATURE_BRAVE,
        .ability = ABILITY_LEVITATE
    },
    [FRONTIER_MON_KANGASKHAN_2] = {
        .species = SPECIES_KANGASKHAN,
        .moves = {MOVE_RETURN, MOVE_EARTHQUAKE, MOVE_SHADOW_BALL, MOVE_FOCUS_PUNCH},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {4, 252, 0, 0, 0, 252},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_EARLY_BIRD
    },
    [FRONTIER_MON_ELECTABUZZ_2] = {
        .species = SPECIES_ELECTABUZZ,
        .moves = {MOVE_THUNDERBOLT, MOVE_SUBSTITUTE, MOVE_FOCUS_PUNCH, MOVE_THUNDER_WAVE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {0, 172, 0, 98, 0, 240},
        .nature = NATURE_HASTY,
        .ability = ABILITY_STATIC
    },
    [FRONTIER_MON_TAUROS_2] = {
        .species = SPECIES_TAUROS,
        .moves = {MOVE_RETURN, MOVE_SUBSTITUTE, MOVE_TOXIC, MOVE_PROTECT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {208, 108, 8, 0, 0, 184},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_INTIMIDATE
    },
    [FRONTIER_MON_SLOWBRO_2] = {
        .species = SPECIES_SLOWBRO,
        .moves = {MOVE_SUBSTITUTE, MOVE_FOCUS_PUNCH, MOVE_SURF, MOVE_PSYCHIC},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {252, 96, 0, 160, 0, 0},
        .nature = NATURE_RELAXED,
        .ability = ABILITY_OWN_TEMPO
    },
    [FRONTIER_MON_SLOWKING_2] = {
        .species = SPECIES_SLOWKING,
        .moves = {MOVE_SURF, MOVE_CALM_MIND, MOVE_PSYCHIC, MOVE_ICE_BEAM},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 180, 76, 0, 0},
        .nature = NATURE_BOLD,
        .ability = ABILITY_OWN_TEMPO
    },
    [FRONTIER_MON_MILTANK_2] = {
        .species = SPECIES_MILTANK,
        .moves = {MOVE_PSYCH_UP, MOVE_BODY_SLAM, MOVE_EARTHQUAKE, MOVE_MILK_DRINK},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {220, 192, 96, 0, 0, 0},
        .nature = NATURE_IMPISH,
        .ability = ABILITY_THICK_FAT
    },
    [FRONTIER_MON_ALTARIA_2] = {
        .species = SPECIES_ALTARIA,
        .moves = {MOVE_DRAGON_DANCE, MOVE_AERIAL_ACE, MOVE_EARTHQUAKE, MOVE_REST},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {188, 252, 44, 0, 0, 24},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_NATURAL_CURE
    },
    [FRONTIER_MON_NIDOQUEEN_2] = {
        .species = SPECIES_NIDOQUEEN,
        .moves = {MOVE_EARTHQUAKE, MOVE_SHADOW_BALL, MOVE_SLUDGE_BOMB, MOVE_ICE_BEAM},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {128, 252, 0, 56, 0, 72},
        .nature = NATURE_LONELY,
        .ability = ABILITY_POISON_POINT
    },
    [FRONTIER_MON_NIDOKING_2] = {
        .species = SPECIES_NIDOKING,
        .moves = {MOVE_EARTHQUAKE, MOVE_MEGAHORN, MOVE_THUNDERBOLT, MOVE_ICE_BEAM},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {0, 240, 0, 120, 0, 148},
        .nature = NATURE_LONELY,
        .ability = ABILITY_POISON_POINT
    },
    [FRONTIER_MON_MAGMAR_2] = {
        .species = SPECIES_MAGMAR,
        .moves = {MOVE_FIRE_BLAST, MOVE_THUNDER_PUNCH, MOVE_HIDDEN_POWER, MOVE_FOCUS_PUNCH},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHARCOAL,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {0, 64, 0, 252, 0, 192},
        .nature = NATURE_HASTY,
        .ability = ABILITY_FLAME_BODY,
        .hiddenPowerType = TYPE_GRASS
    },
    [FRONTIER_MON_CRADILY_2] = {
        .species = SPECIES_CRADILY,
        .moves = {MOVE_AMNESIA, MOVE_BARRIER, MOVE_TOXIC, MOVE_REST},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 164, 0, 92, 0},
        .nature = NATURE_CAREFUL,
        .ability = ABILITY_SUCTION_CUPS
    },
    [FRONTIER_MON_ARMALDO_2] = {
        .species = SPECIES_ARMALDO,
        .moves = {MOVE_HIDDEN_POWER, MOVE_KNOCK_OFF, MOVE_ROCK_BLAST, MOVE_SWORDS_DANCE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {202, 252, 56, 0, 0, 0},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_BATTLE_ARMOR,
        .hiddenPowerType = TYPE_BUG
    },
    [FRONTIER_MON_GOLDUCK_2] = {
        .species = SPECIES_GOLDUCK,
        .moves = {MOVE_CALM_MIND, MOVE_HYDRO_PUMP, MOVE_PSYCHIC, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {4, 0, 0, 252, 0, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_CLOUD_NINE,
        .hiddenPowerType = TYPE_GRASS
    },
    [FRONTIER_MON_RAPIDASH_2] = {
        .species = SPECIES_RAPIDASH,
        .moves = {MOVE_HYPNOSIS, MOVE_OVERHEAT, MOVE_IRON_TAIL, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {0, 252, 0, 110, 0, 148},
        .nature = NATURE_HASTY,
        .ability = ABILITY_FLASH_FIRE,
        .hiddenPowerType = TYPE_ROCK
    },
    [FRONTIER_MON_MUK_2] = {
        .species = SPECIES_MUK,
        .moves = {MOVE_CURSE, MOVE_SLUDGE_BOMB, MOVE_FIRE_BLAST, MOVE_EXPLOSION},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {252, 156, 0, 0, 100, 0},
        .nature = NATURE_SASSY,
        .ability = ABILITY_STICKY_HOLD
    },
    [FRONTIER_MON_GENGAR_2] = {
        .species = SPECIES_GENGAR,
        .moves = {MOVE_THUNDERBOLT, MOVE_ICE_PUNCH, MOVE_GIGA_DRAIN, MOVE_EXPLOSION},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {0, 0, 0, 252, 4, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_LEVITATE
    },
    [FRONTIER_MON_AMPHAROS_2] = {
        .species = SPECIES_AMPHAROS,
        .moves = {MOVE_FOCUS_PUNCH, MOVE_THUNDERBOLT, MOVE_SUBSTITUTE, MOVE_THUNDER_WAVE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {252, 156, 0, 100, 0, 0},
        .nature = NATURE_QUIET,
        .ability = ABILITY_STATIC
    },
    [FRONTIER_MON_SCIZOR_2] = {
        .species = SPECIES_SCIZOR,
        .moves = {MOVE_AGILITY, MOVE_ENDURE, MOVE_REVERSAL, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LIECHI_BERRY,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {108, 252, 0, 0, 0, 148},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_SWARM,
        .hiddenPowerType = TYPE_GHOST
    },
    [FRONTIER_MON_HERACROSS_2] = {
        .species = SPECIES_HERACROSS,
        .moves = {MOVE_SWORDS_DANCE, MOVE_MEGAHORN, MOVE_SUBSTITUTE, MOVE_ROCK_SLIDE},
        .itemTableId = BATTLE_FRONTIER_ITEM_SALAC_BERRY,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {0, 252, 4, 0, 0, 252},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_SWARM
    },
    [FRONTIER_MON_URSARING_2] = {
        .species = SPECIES_URSARING,
        .moves = {MOVE_SWORDS_DANCE, MOVE_RETURN, MOVE_HIDDEN_POWER, MOVE_FOCUS_PUNCH},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {160, 252, 0, 0, 0, 96},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_GUTS,
        .hiddenPowerType = TYPE_GHOST
    },
    [FRONTIER_MON_HOUNDOOM_2] = {
        .species = SPECIES_HOUNDOOM,
        .moves = {MOVE_CRUNCH, MOVE_PROTECT, MOVE_OVERHEAT, MOVE_FLAMETHROWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_WHITE_HERB,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED | F_EV_SPREAD_HP,
        .evs = {152, 0, 0, 180, 0, 176},
        .nature = NATURE_MODEST,
        .ability = ABILITY_FLASH_FIRE
    },
    [FRONTIER_MON_DONPHAN_2] = {
        .species = SPECIES_DONPHAN,
        .moves = {MOVE_EARTHQUAKE, MOVE_ROCK_SLIDE, MOVE_HIDDEN_POWER, MOVE_RAPID_SPIN},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {80, 188, 216, 0, 0, 24},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_STURDY,
        .hiddenPowerType = TYPE_BUG
    },
    [FRONTIER_MON_CLAYDOL_2] = {
        .species = SPECIES_CLAYDOL,
        .moves = {MOVE_RAPID_SPIN, MOVE_EARTHQUAKE, MOVE_EXPLOSION, MOVE_SHADOW_BALL},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {252, 216, 0, 32, 0, 8},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_LEVITATE
    },
    [FRONTIER_MON_WAILORD_2] = {
        .species = SPECIES_WAILORD,
        .moves = {MOVE_SURF, MOVE_REST, MOVE_SLEEP_TALK, MOVE_ROAR},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_DEFENSE,
        .evs = {0, 0, 212, 0, 252, 44},
        .nature = NATURE_CALM,
        .ability = ABILITY_WATER_VEIL
    },
    [FRONTIER_MON_NINETALES_2] = {
        .species = SPECIES_NINETALES,
        .moves = {MOVE_FIRE_BLAST, MOVE_HIDDEN_POWER, MOVE_WILL_O_WISP, MOVE_GRUDGE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LUM_BERRY,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {0, 0, 0, 252, 4, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_FLASH_FIRE,
        .hiddenPowerType = TYPE_WATER
    },
    [FRONTIER_MON_MACHAMP_2] = {
        .species = SPECIES_MACHAMP,
        .moves = {MOVE_CROSS_CHOP, MOVE_ROCK_SLIDE, MOVE_FOCUS_PUNCH, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {0, 252, 4, 0, 0, 252},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_GUTS,
        .hiddenPowerType = TYPE_GHOST
    },
    [FRONTIER_MON_SHUCKLE_2] = {
        .species = SPECIES_SHUCKLE,
        .moves = {MOVE_DEFENSE_CURL, MOVE_ROLLOUT, MOVE_REST, MOVE_TOXIC},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {252, 252, 0, 0, 4, 0},
        .nature = NATURE_CAREFUL,
        .ability = ABILITY_STURDY
    },
    [FRONTIER_MON_STEELIX_2] = {
        .species = SPECIES_STEELIX,
        .moves = {MOVE_EARTHQUAKE, MOVE_IRON_TAIL, MOVE_DOUBLE_EDGE, MOVE_EXPLOSION},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {144, 228, 0, 0, 136, 0},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_ROCK_HEAD
    },
    [FRONTIER_MON_TENTACRUEL_2] = {
        .species = SPECIES_TENTACRUEL,
        .moves = {MOVE_SURF, MOVE_ICE_BEAM, MOVE_GIGA_DRAIN, MOVE_SUBSTITUTE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {72, 0, 0, 252, 0, 184},
        .nature = NATURE_TIMID,
        .ability = ABILITY_LIQUID_OOZE
    },
    [FRONTIER_MON_AERODACTYL_2] = {
        .species = SPECIES_AERODACTYL,
        .moves = {MOVE_ROCK_SLIDE, MOVE_DOUBLE_EDGE, MOVE_EARTHQUAKE, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {4, 252, 0, 0, 0, 252},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_ROCK_HEAD,
        .hiddenPowerType = TYPE_GHOST
    },
    [FRONTIER_MON_PORYGON2_2] = {
        .species = SPECIES_PORYGON2,
        .moves = {MOVE_RECOVER, MOVE_ICE_BEAM, MOVE_THUNDERBOLT, MOVE_TOXIC},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 200, 0, 56, 0},
        .nature = NATURE_BOLD,
        .ability = ABILITY_TRACE
    },
    [FRONTIER_MON_GARDEVOIR_2] = {
        .species = SPECIES_GARDEVOIR,
        .moves = {MOVE_CALM_MIND, MOVE_PSYCHIC, MOVE_MEAN_LOOK, MOVE_HYPNOSIS},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {4, 0, 0, 252, 0, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_TRACE
    },
    [FRONTIER_MON_EXEGGUTOR_2] = {
        .species = SPECIES_EXEGGUTOR,
        .moves = {MOVE_PSYCHIC, MOVE_SLEEP_POWDER, MOVE_LEECH_SEED, MOVE_EXPLOSION},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED | F_EV_SPREAD_HP,
        .evs = {96, 0, 0, 252, 96, 60},
        .nature = NATURE_MODEST,
        .ability = ABILITY_CHLOROPHYLL
    },
    [FRONTIER_MON_STARMIE_2] = {
        .species = SPECIES_STARMIE,
        .moves = {MOVE_RAPID_SPIN, MOVE_RECOVER, MOVE_SURF, MOVE_THUNDER_WAVE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_HP,
        .evs = {252, 0, 0, 40, 0, 216},
        .nature = NATURE_TIMID,
        .ability = ABILITY_NATURAL_CURE
    },
    [FRONTIER_MON_FLYGON_2] = {
        .species = SPECIES_FLYGON,
        .moves = {MOVE_PROTECT, MOVE_EARTHQUAKE, MOVE_ROCK_SLIDE, MOVE_TOXIC},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 196, 0, 28, 32},
        .nature = NATURE_IMPISH,
        .ability = ABILITY_LEVITATE
    },
    [FRONTIER_MON_VENUSAUR_2] = {
        .species = SPECIES_VENUSAUR,
        .moves = {MOVE_CURSE, MOVE_SLUDGE_BOMB, MOVE_EARTHQUAKE, MOVE_SLEEP_POWDER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {208, 192, 0, 0, 0, 108},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_OVERGROW
    },
    [FRONTIER_MON_VAPOREON_2] = {
        .species = SPECIES_VAPOREON,
        .moves = {MOVE_SURF, MOVE_WISH, MOVE_PROTECT, MOVE_TOXIC},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 228, 0, 28, 0},
        .nature = NATURE_BOLD,
        .ability = ABILITY_WATER_ABSORB
    },
    [FRONTIER_MON_JOLTEON_2] = {
        .species = SPECIES_JOLTEON,
        .moves = {MOVE_THUNDERBOLT, MOVE_THUNDER_WAVE, MOVE_HIDDEN_POWER, MOVE_PROTECT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {0, 0, 0, 252, 4, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_VOLT_ABSORB,
        .hiddenPowerType = TYPE_ICE
    },
    [FRONTIER_MON_FLAREON_2] = {
        .species = SPECIES_FLAREON,
        .moves = {MOVE_FIRE_BLAST, MOVE_HIDDEN_POWER, MOVE_SUBSTITUTE, MOVE_BATON_PASS},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED | F_EV_SPREAD_DEFENSE,
        .evs = {8, 0, 84, 192, 0, 224},
        .nature = NATURE_MODEST,
        .ability = ABILITY_FLASH_FIRE,
        .hiddenPowerType = TYPE_GRASS
    },
    [FRONTIER_MON_MEGANIUM_2] = {
        .species = SPECIES_MEGANIUM,
        .moves = {MOVE_SWORDS_DANCE, MOVE_EARTHQUAKE, MOVE_HIDDEN_POWER, MOVE_SYNTHESIS},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {76, 252, 0, 0, 0, 180},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_OVERGROW,
        .hiddenPowerType = TYPE_FLYING
    },
    [FRONTIER_MON_ESPEON_2] = {
        .species = SPECIES_ESPEON,
        .moves = {MOVE_CALM_MIND, MOVE_PSYCHIC, MOVE_BATON_PASS, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {0, 0, 0, 252, 4, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_SYNCHRONIZE,
        .hiddenPowerType = TYPE_FIRE
    },
    [FRONTIER_MON_UMBREON_2] = {
        .species = SPECIES_UMBREON,
        .moves = {MOVE_WISH, MOVE_PROTECT, MOVE_BATON_PASS, MOVE_TOXIC},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 120, 0, 136, 0},
        .nature = NATURE_SASSY,
        .ability = ABILITY_SYNCHRONIZE
    },
    [FRONTIER_MON_BLASTOISE_2] = {
        .species = SPECIES_BLASTOISE,
        .moves = {MOVE_SURF, MOVE_ICE_BEAM, MOVE_SLEEP_TALK, MOVE_REST},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 148, 68, 0, 40},
        .nature = NATURE_BOLD,
        .ability = ABILITY_TORRENT
    },
    [FRONTIER_MON_FERALIGATR_2] = {
        .species = SPECIES_FERALIGATR,
        .moves = {MOVE_SUBSTITUTE, MOVE_FOCUS_PUNCH, MOVE_SURF, MOVE_REST},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_DEFENSE | F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {136, 116, 136, 120, 0, 0},
        .nature = NATURE_BRAVE,
        .ability = ABILITY_TORRENT
    },
    [FRONTIER_MON_AGGRON_2] = {
        .species = SPECIES_AGGRON,
        .moves = {MOVE_EARTHQUAKE, MOVE_ROCK_SLIDE, MOVE_DOUBLE_EDGE, MOVE_FOCUS_PUNCH},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {140, 252, 0, 0, 0, 116},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_ROCK_HEAD
    },
    [FRONTIER_MON_BLAZIKEN_2] = {
        .species = SPECIES_BLAZIKEN,
        .moves = {MOVE_FIRE_BLAST, MOVE_THUNDER_PUNCH, MOVE_HIDDEN_POWER, MOVE_SKY_UPPERCUT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {16, 56, 0, 240, 0, 196},
        .nature = NATURE_RASH,
        .ability = ABILITY_BLAZE,
        .hiddenPowerType = TYPE_ICE
    },
    [FRONTIER_MON_WALREIN_2] = {
        .species = SPECIES_WALREIN,
        .moves = {MOVE_CURSE, MOVE_EARTHQUAKE, MOVE_ICE_BEAM, MOVE_REST},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 0, 4, 252, 0},
        .nature = NATURE_SASSY,
        .ability = ABILITY_THICK_FAT
    },
    [FRONTIER_MON_SCEPTILE_2] = {
        .species = SPECIES_SCEPTILE,
        .moves = {MOVE_LEAF_BLADE, MOVE_HIDDEN_POWER, MOVE_ENDEAVOR, MOVE_SUBSTITUTE},
        .itemTableId = BATTLE_FRONTIER_ITEM_PETAYA_BERRY,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {28, 0, 0, 252, 0, 228},
        .nature = NATURE_TIMID,
        .ability = ABILITY_OVERGROW,
        .hiddenPowerType = TYPE_ICE
    },
    [FRONTIER_MON_CHARIZARD_2] = {
        .species = SPECIES_CHARIZARD,
        .moves = {MOVE_FIRE_BLAST, MOVE_DRAGON_CLAW, MOVE_FLAMETHROWER, MOVE_PROTECT},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHARCOAL,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {4, 0, 0, 252, 0, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_BLAZE
    },
    [FRONTIER_MON_TYPHLOSION_2] = {
        .species = SPECIES_TYPHLOSION,
        .moves = {MOVE_FIRE_BLAST, MOVE_FOCUS_PUNCH, MOVE_THUNDER_PUNCH, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {32, 16, 0, 252, 0, 208},
        .nature = NATURE_RASH,
        .ability = ABILITY_BLAZE,
        .hiddenPowerType = TYPE_GRASS
    },
    [FRONTIER_MON_LAPRAS_2] = {
        .species = SPECIES_LAPRAS,
        .moves = {MOVE_ICE_BEAM, MOVE_THUNDERBOLT, MOVE_TOXIC, MOVE_ROAR},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 164, 92, 0, 0},
        .nature = NATURE_BOLD,
        .ability = ABILITY_WATER_ABSORB
    },
    [FRONTIER_MON_CROBAT_2] = {
        .species = SPECIES_CROBAT,
        .moves = {MOVE_SLEEP_TALK, MOVE_WHIRLWIND, MOVE_NONE, MOVE_NONE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_HP,
        .evs = {252, 0, 4, 0, 0, 252},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_INNER_FOCUS
    },
    [FRONTIER_MON_SWAMPERT_2] = {
        .species = SPECIES_SWAMPERT,
        .moves = {MOVE_SURF, MOVE_TOXIC, MOVE_PROTECT, MOVE_REFRESH},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {248, 0, 204, 0, 44, 12},
        .nature = NATURE_BOLD,
        .ability = ABILITY_TORRENT
    },
    [FRONTIER_MON_GYARADOS_2] = {
        .species = SPECIES_GYARADOS,
        .moves = {MOVE_DRAGON_DANCE, MOVE_HIDDEN_POWER, MOVE_EARTHQUAKE, MOVE_REST},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_SPEED | F_EV_SPREAD_HP,
        .evs = {216, 16, 0, 0, 88, 188},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_INTIMIDATE,
        .hiddenPowerType = TYPE_FLYING
    },
    [FRONTIER_MON_SNORLAX_2] = {
        .species = SPECIES_SNORLAX,
        .moves = {MOVE_CURSE, MOVE_REST, MOVE_RETURN, MOVE_SHADOW_BALL},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_DEFENSE | F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {144, 96, 132, 0, 136, 0},
        .nature = NATURE_CAREFUL,
        .ability = ABILITY_IMMUNITY
    },
    [FRONTIER_MON_KINGDRA_2] = {
        .species = SPECIES_KINGDRA,
        .moves = {MOVE_ICE_BEAM, MOVE_RAIN_DANCE, MOVE_SURF, MOVE_HYDRO_PUMP},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_HP,
        .evs = {240, 0, 0, 252, 0, 16},
        .nature = NATURE_MODEST,
        .ability = ABILITY_SWIFT_SWIM
    },
    [FRONTIER_MON_BLISSEY_2] = {
        .species = SPECIES_BLISSEY,
        .moves = {MOVE_CALM_MIND, MOVE_SOFT_BOILED, MOVE_ICE_BEAM, MOVE_THUNDERBOLT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_DEFENSE,
        .evs = {0, 0, 252, 252, 0, 4},
        .nature = NATURE_MODEST,
        .ability = ABILITY_NATURAL_CURE
    },
    [FRONTIER_MON_MILOTIC_2] = {
        .species = SPECIES_MILOTIC,
        .moves = {MOVE_ICE_BEAM, MOVE_SURF, MOVE_RECOVER, MOVE_TOXIC},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_HP,
        .evs = {252, 0, 4, 252, 0, 0},
        .nature = NATURE_BOLD,
        .ability = ABILITY_MARVEL_SCALE
    },
    [FRONTIER_MON_ARCANINE_2] = {
        .species = SPECIES_ARCANINE,
        .moves = {MOVE_EXTREME_SPEED, MOVE_IRON_TAIL, MOVE_HIDDEN_POWER, MOVE_OVERHEAT},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {184, 252, 0, 0, 0, 72},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_INTIMIDATE,
        .hiddenPowerType = TYPE_GHOST
    },
    [FRONTIER_MON_SALAMENCE_2] = {
        .species = SPECIES_SALAMENCE,
        .moves = {MOVE_AERIAL_ACE, MOVE_EARTHQUAKE, MOVE_ROCK_SLIDE, MOVE_BRICK_BREAK},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {4, 252, 0, 0, 0, 252},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_INTIMIDATE
    },
    [FRONTIER_MON_METAGROSS_2] = {
        .species = SPECIES_METAGROSS,
        .moves = {MOVE_AGILITY, MOVE_METEOR_MASH, MOVE_EARTHQUAKE, MOVE_EXPLOSION},
        .itemTableId = BATTLE_FRONTIER_ITEM_LUM_BERRY,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {168, 252, 0, 0, 0, 88},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_CLEAR_BODY
    },
    [FRONTIER_MON_SLAKING_2] = {
        .species = SPECIES_SLAKING,
        .moves = {MOVE_DOUBLE_EDGE, MOVE_SHADOW_BALL, MOVE_FOCUS_PUNCH, MOVE_HYPER_BEAM},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {0, 252, 0, 0, 4, 252},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_TRUANT
    },
    [FRONTIER_MON_DUGTRIO_3] = {
        .species = SPECIES_DUGTRIO,
        .moves = {MOVE_EARTHQUAKE, MOVE_HIDDEN_POWER, MOVE_AERIAL_ACE, MOVE_BEAT_UP},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {0, 252, 0, 0, 4, 252},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_ARENA_TRAP,
        .hiddenPowerType = TYPE_BUG
    },
    [FRONTIER_MON_MEDICHAM_3] = {
        .species = SPECIES_MEDICHAM,
        .moves = {MOVE_BRICK_BREAK, MOVE_FOCUS_PUNCH, MOVE_SUBSTITUTE, MOVE_SHADOW_BALL},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {4, 252, 0, 0, 0, 252},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_PURE_POWER
    },
    [FRONTIER_MON_MISDREAVUS_3] = {
        .species = SPECIES_MISDREAVUS,
        .moves = {MOVE_HIDDEN_POWER, MOVE_THUNDERBOLT, MOVE_THUNDER_WAVE, MOVE_IMPRISON},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {236, 0, 68, 124, 80, 0},
        .nature = NATURE_MODEST,
        .ability = ABILITY_LEVITATE,
        .hiddenPowerType = TYPE_ICE
    },
    [FRONTIER_MON_FEAROW_3] = {
        .species = SPECIES_FEAROW,
        .moves = {MOVE_DRILL_PECK, MOVE_DOUBLE_EDGE, MOVE_PROTECT, MOVE_SCARY_FACE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {4, 252, 0, 0, 0, 252},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_KEEN_EYE
    },
    [FRONTIER_MON_GRANBULL_3] = {
        .species = SPECIES_GRANBULL,
        .moves = {MOVE_RETURN, MOVE_FOCUS_PUNCH, MOVE_SHADOW_BALL, MOVE_HEAL_BELL},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 40, 216, 0, 0, 0},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_INTIMIDATE
    },
    [FRONTIER_MON_JYNX_3] = {
        .species = SPECIES_JYNX,
        .moves = {MOVE_LOVELY_KISS, MOVE_FAKE_TEARS, MOVE_PSYCHIC, MOVE_ICE_BEAM},
        .itemTableId = BATTLE_FRONTIER_ITEM_LUM_BERRY,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {0, 0, 0, 252, 4, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_OBLIVIOUS
    },
    [FRONTIER_MON_DUSCLOPS_3] = {
        .species = SPECIES_DUSCLOPS,
        .moves = {MOVE_SHADOW_BALL, MOVE_REST, MOVE_SLEEP_TALK, MOVE_WILL_O_WISP},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 36, 20, 0, 200, 0},
        .nature = NATURE_CAREFUL,
        .ability = ABILITY_PRESSURE
    },
    [FRONTIER_MON_DODRIO_3] = {
        .species = SPECIES_DODRIO,
        .moves = {MOVE_DRILL_PECK, MOVE_HIDDEN_POWER, MOVE_QUICK_ATTACK, MOVE_RETURN},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {4, 252, 0, 0, 0, 252},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_EARLY_BIRD,
        .hiddenPowerType = TYPE_FIGHTING
    },
    [FRONTIER_MON_MR_MIME_3] = {
        .species = SPECIES_MR_MIME,
        .moves = {MOVE_PSYCHIC, MOVE_THUNDERBOLT, MOVE_FIRE_PUNCH, MOVE_THUNDER_WAVE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LUM_BERRY,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {0, 0, 0, 252, 4, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_SOUNDPROOF
    },
    [FRONTIER_MON_LANTURN_3] = {
        .species = SPECIES_LANTURN,
        .moves = {MOVE_THUNDERBOLT, MOVE_SURF, MOVE_ICE_BEAM, MOVE_THUNDER_WAVE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_SP_ATTACK,
        .evs = {52, 0, 0, 252, 204, 0},
        .nature = NATURE_MODEST,
        .ability = ABILITY_VOLT_ABSORB
    },
    [FRONTIER_MON_BRELOOM_3] = {
        .species = SPECIES_BRELOOM,
        .moves = {MOVE_SPORE, MOVE_FOCUS_PUNCH, MOVE_MACH_PUNCH, MOVE_SUBSTITUTE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {0, 252, 4, 0, 0, 252},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_EFFECT_SPORE
    },
    [FRONTIER_MON_FORRETRESS_3] = {
        .species = SPECIES_FORRETRESS,
        .moves = {MOVE_SPIKES, MOVE_RAPID_SPIN, MOVE_EXPLOSION, MOVE_EARTHQUAKE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 4, 0, 252, 0},
        .nature = NATURE_CAREFUL,
        .ability = ABILITY_STURDY
    },
    [FRONTIER_MON_WHISCASH_3] = {
        .species = SPECIES_WHISCASH,
        .moves = {MOVE_EARTHQUAKE, MOVE_ICE_BEAM, MOVE_REST, MOVE_SLEEP_TALK},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 252, 0, 4, 0},
        .nature = NATURE_RELAXED,
        .ability = ABILITY_OBLIVIOUS
    },
    [FRONTIER_MON_XATU_3] = {
        .species = SPECIES_XATU,
        .moves = {MOVE_PSYCHIC, MOVE_THUNDER_WAVE, MOVE_REFLECT, MOVE_REST},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_HP,
        .evs = {252, 0, 0, 4, 0, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_EARLY_BIRD
    },
    [FRONTIER_MON_SKARMORY_3] = {
        .species = SPECIES_SKARMORY,
        .moves = {MOVE_SPIKES, MOVE_WHIRLWIND, MOVE_TOXIC, MOVE_PURSUIT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 156, 100, 0, 0},
        .nature = NATURE_BOLD,
        .ability = ABILITY_KEEN_EYE
    },
    [FRONTIER_MON_MAROWAK_3] = {
        .species = SPECIES_MAROWAK,
        .moves = {MOVE_EARTHQUAKE, MOVE_ROCK_SLIDE, MOVE_SWORDS_DANCE, MOVE_PROTECT},
        .itemTableId = BATTLE_FRONTIER_ITEM_THICK_CLUB,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {132, 252, 0, 0, 0, 124},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_LIGHTNING_ROD
    },
    [FRONTIER_MON_QUAGSIRE_3] = {
        .species = SPECIES_QUAGSIRE,
        .moves = {MOVE_EARTHQUAKE, MOVE_REST, MOVE_CURSE, MOVE_SURF},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 216, 0, 40, 0},
        .nature = NATURE_RELAXED,
        .ability = ABILITY_WATER_ABSORB
    },
    [FRONTIER_MON_CLEFABLE_3] = {
        .species = SPECIES_CLEFABLE,
        .moves = {MOVE_SHADOW_BALL, MOVE_RETURN, MOVE_COUNTER, MOVE_SOFT_BOILED},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 72, 184, 0, 0, 0},
        .nature = NATURE_IMPISH,
        .ability = ABILITY_CUTE_CHARM
    },
    [FRONTIER_MON_HARIYAMA_3] = {
        .species = SPECIES_HARIYAMA,
        .moves = {MOVE_KNOCK_OFF, MOVE_BRICK_BREAK, MOVE_ROCK_SLIDE, MOVE_FOCUS_PUNCH},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_DEFENSE | F_EV_SPREAD_ATTACK,
        .evs = {0, 96, 192, 0, 192, 28},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_GUTS
    },
    [FRONTIER_MON_RAICHU_3] = {
        .species = SPECIES_RAICHU,
        .moves = {MOVE_THUNDERBOLT, MOVE_HIDDEN_POWER, MOVE_THUNDER_WAVE, MOVE_WISH},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED | F_EV_SPREAD_HP,
        .evs = {108, 0, 0, 252, 0, 148},
        .nature = NATURE_TIMID,
        .ability = ABILITY_STATIC,
        .hiddenPowerType = TYPE_GRASS
    },
    [FRONTIER_MON_DEWGONG_3] = {
        .species = SPECIES_DEWGONG,
        .moves = {MOVE_ICE_BEAM, MOVE_PERISH_SONG, MOVE_SURF, MOVE_ENCORE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED | F_EV_SPREAD_HP,
        .evs = {100, 0, 20, 136, 0, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_THICK_FAT
    },
    [FRONTIER_MON_MANECTRIC_3] = {
        .species = SPECIES_MANECTRIC,
        .moves = {MOVE_THUNDERBOLT, MOVE_THUNDER_WAVE, MOVE_CRUNCH, MOVE_SUBSTITUTE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {4, 0, 0, 252, 0, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_STATIC
    },
    [FRONTIER_MON_VILEPLUME_3] = {
        .species = SPECIES_VILEPLUME,
        .moves = {MOVE_AROMATHERAPY, MOVE_SLEEP_POWDER, MOVE_SLUDGE_BOMB, MOVE_LEECH_SEED},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 0, 0, 224, 32},
        .nature = NATURE_CAREFUL,
        .ability = ABILITY_CHLOROPHYLL
    },
    [FRONTIER_MON_VICTREEBEL_3] = {
        .species = SPECIES_VICTREEBEL,
        .moves = {MOVE_SUNNY_DAY, MOVE_SOLAR_BEAM, MOVE_HIDDEN_POWER, MOVE_SLEEP_POWDER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LUM_BERRY,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {0, 0, 0, 252, 36, 220},
        .nature = NATURE_MODEST,
        .ability = ABILITY_CHLOROPHYLL,
        .hiddenPowerType = TYPE_FIRE
    },
    [FRONTIER_MON_ELECTRODE_3] = {
        .species = SPECIES_ELECTRODE,
        .moves = {MOVE_THUNDERBOLT, MOVE_EXPLOSION, MOVE_THUNDER_WAVE, MOVE_THUNDER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {4, 0, 0, 252, 0, 252},
        .nature = NATURE_MODEST,
        .ability = ABILITY_STATIC
    },
    [FRONTIER_MON_EXPLOUD_3] = {
        .species = SPECIES_EXPLOUD,
        .moves = {MOVE_EARTHQUAKE, MOVE_SHADOW_BALL, MOVE_BRICK_BREAK, MOVE_RETURN},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {4, 252, 0, 0, 0, 252},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_SOUNDPROOF
    },
    [FRONTIER_MON_SHIFTRY_3] = {
        .species = SPECIES_SHIFTRY,
        .moves = {MOVE_BRICK_BREAK, MOVE_SHADOW_BALL, MOVE_SWORDS_DANCE, MOVE_EXPLOSION},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {60, 252, 0, 0, 0, 196},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_EARLY_BIRD
    },
    [FRONTIER_MON_GLALIE_3] = {
        .species = SPECIES_GLALIE,
        .moves = {MOVE_SPIKES, MOVE_ICE_BEAM, MOVE_EXPLOSION, MOVE_EARTHQUAKE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LUM_BERRY,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_HP,
        .evs = {248, 8, 0, 0, 0, 252},
        .nature = NATURE_HASTY,
        .ability = ABILITY_INNER_FOCUS
    },
    [FRONTIER_MON_LUDICOLO_3] = {
        .species = SPECIES_LUDICOLO,
        .moves = {MOVE_FAKE_OUT, MOVE_ICE_BEAM, MOVE_HYDRO_PUMP, MOVE_GIGA_DRAIN},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {0, 4, 0, 252, 0, 252},
        .nature = NATURE_MILD,
        .ability = ABILITY_SWIFT_SWIM
    },
    [FRONTIER_MON_HYPNO_3] = {
        .species = SPECIES_HYPNO,
        .moves = {MOVE_BATON_PASS, MOVE_CALM_MIND, MOVE_PSYCHIC, MOVE_TAUNT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 252, 0, 0, 0},
        .nature = NATURE_BOLD,
        .ability = ABILITY_INSOMNIA
    },
    [FRONTIER_MON_GOLEM_3] = {
        .species = SPECIES_GOLEM,
        .moves = {MOVE_EARTHQUAKE, MOVE_EXPLOSION, MOVE_ROCK_SLIDE, MOVE_ROCK_BLAST},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {248, 252, 0, 0, 8, 0},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_ROCK_HEAD
    },
    [FRONTIER_MON_RHYDON_3] = {
        .species = SPECIES_RHYDON,
        .moves = {MOVE_EARTHQUAKE, MOVE_ROCK_SLIDE, MOVE_MEGAHORN, MOVE_SUBSTITUTE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {252, 252, 0, 0, 4, 0},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_ROCK_HEAD
    },
    [FRONTIER_MON_ALAKAZAM_3] = {
        .species = SPECIES_ALAKAZAM,
        .moves = {MOVE_PSYCHIC, MOVE_CALM_MIND, MOVE_SUBSTITUTE, MOVE_ENCORE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {4, 0, 0, 252, 0, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_SYNCHRONIZE
    },
    [FRONTIER_MON_WEEZING_3] = {
        .species = SPECIES_WEEZING,
        .moves = {MOVE_WILL_O_WISP, MOVE_SLUDGE_BOMB, MOVE_EXPLOSION, MOVE_PAIN_SPLIT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 252, 0, 0, 4},
        .nature = NATURE_IMPISH,
        .ability = ABILITY_LEVITATE
    },
    [FRONTIER_MON_KANGASKHAN_3] = {
        .species = SPECIES_KANGASKHAN,
        .moves = {MOVE_PROTECT, MOVE_WISH, MOVE_TOXIC, MOVE_RETURN},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 128, 0, 128, 0},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_EARLY_BIRD
    },
    [FRONTIER_MON_ELECTABUZZ_3] = {
        .species = SPECIES_ELECTABUZZ,
        .moves = {MOVE_THUNDERBOLT, MOVE_LIGHT_SCREEN, MOVE_COUNTER, MOVE_TOXIC},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {84, 0, 0, 208, 0, 216},
        .nature = NATURE_MODEST,
        .ability = ABILITY_STATIC
    },
    [FRONTIER_MON_TAUROS_3] = {
        .species = SPECIES_TAUROS,
        .moves = {MOVE_EARTHQUAKE, MOVE_HIDDEN_POWER, MOVE_DOUBLE_EDGE, MOVE_BODY_SLAM},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {4, 252, 0, 0, 0, 252},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_INTIMIDATE,
        .hiddenPowerType = TYPE_GHOST
    },
    [FRONTIER_MON_SLOWBRO_3] = {
        .species = SPECIES_SLOWBRO,
        .moves = {MOVE_CURSE, MOVE_EARTHQUAKE, MOVE_ICE_BEAM, MOVE_REST},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {252, 176, 0, 0, 80, 0},
        .nature = NATURE_BRAVE,
        .ability = ABILITY_OWN_TEMPO
    },
    [FRONTIER_MON_SLOWKING_3] = {
        .species = SPECIES_SLOWKING,
        .moves = {MOVE_SURF, MOVE_CALM_MIND, MOVE_PSYCHIC, MOVE_THUNDER_WAVE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 180, 76, 0, 0},
        .nature = NATURE_BOLD,
        .ability = ABILITY_OWN_TEMPO
    },
    [FRONTIER_MON_MILTANK_3] = {
        .species = SPECIES_MILTANK,
        .moves = {MOVE_CURSE, MOVE_BODY_SLAM, MOVE_MILK_DRINK, MOVE_EARTHQUAKE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {248, 0, 120, 0, 0, 140},
        .nature = NATURE_IMPISH,
        .ability = ABILITY_THICK_FAT
    },
    [FRONTIER_MON_ALTARIA_3] = {
        .species = SPECIES_ALTARIA,
        .moves = {MOVE_DRAGON_DANCE, MOVE_EARTHQUAKE, MOVE_AERIAL_ACE, MOVE_HEAL_BELL},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {128, 252, 0, 0, 0, 128},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_NATURAL_CURE
    },
    [FRONTIER_MON_NIDOQUEEN_3] = {
        .species = SPECIES_NIDOQUEEN,
        .moves = {MOVE_EARTHQUAKE, MOVE_SUPERPOWER, MOVE_SLUDGE_BOMB, MOVE_PROTECT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {4, 252, 0, 0, 0, 252},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_POISON_POINT
    },
    [FRONTIER_MON_NIDOKING_3] = {
        .species = SPECIES_NIDOKING,
        .moves = {MOVE_SUBSTITUTE, MOVE_FOCUS_PUNCH, MOVE_THUNDERBOLT, MOVE_ICE_BEAM},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_HP,
        .evs = {200, 56, 0, 252, 0, 0},
        .nature = NATURE_MILD,
        .ability = ABILITY_POISON_POINT
    },
    [FRONTIER_MON_MAGMAR_3] = {
        .species = SPECIES_MAGMAR,
        .moves = {MOVE_FIRE_BLAST, MOVE_THUNDER_PUNCH, MOVE_HIDDEN_POWER, MOVE_SUBSTITUTE},
        .itemTableId = BATTLE_FRONTIER_ITEM_PETAYA_BERRY,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {0, 0, 0, 252, 4, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_FLAME_BODY,
        .hiddenPowerType = TYPE_WATER
    },
    [FRONTIER_MON_CRADILY_3] = {
        .species = SPECIES_CRADILY,
        .moves = {MOVE_ROCK_SLIDE, MOVE_BARRIER, MOVE_MIRROR_COAT, MOVE_RECOVER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 164, 0, 92, 0},
        .nature = NATURE_CAREFUL,
        .ability = ABILITY_SUCTION_CUPS
    },
    [FRONTIER_MON_ARMALDO_3] = {
        .species = SPECIES_ARMALDO,
        .moves = {MOVE_HIDDEN_POWER, MOVE_ROCK_BLAST, MOVE_EARTHQUAKE, MOVE_KNOCK_OFF},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {202, 252, 56, 0, 0, 0},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_BATTLE_ARMOR,
        .hiddenPowerType = TYPE_BUG
    },
    [FRONTIER_MON_GOLDUCK_3] = {
        .species = SPECIES_GOLDUCK,
        .moves = {MOVE_CALM_MIND, MOVE_ICE_BEAM, MOVE_SURF, MOVE_RAIN_DANCE},
        .itemTableId = BATTLE_FRONTIER_ITEM_SALAC_BERRY,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {0, 0, 0, 252, 4, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_CLOUD_NINE
    },
    [FRONTIER_MON_RAPIDASH_3] = {
        .species = SPECIES_RAPIDASH,
        .moves = {MOVE_DOUBLE_EDGE, MOVE_OVERHEAT, MOVE_HIDDEN_POWER, MOVE_QUICK_ATTACK},
        .itemTableId = BATTLE_FRONTIER_ITEM_SILK_SCARF,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {0, 252, 0, 4, 0, 252},
        .nature = NATURE_NAIVE,
        .ability = ABILITY_FLASH_FIRE,
        .hiddenPowerType = TYPE_GROUND
    },
    [FRONTIER_MON_MUK_3] = {
        .species = SPECIES_MUK,
        .moves = {MOVE_SLUDGE_BOMB, MOVE_FOCUS_PUNCH, MOVE_SHADOW_PUNCH, MOVE_EXPLOSION},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {252, 252, 0, 0, 4, 0},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_STICKY_HOLD
    },
    [FRONTIER_MON_GENGAR_3] = {
        .species = SPECIES_GENGAR,
        .moves = {MOVE_HYPNOSIS, MOVE_EXPLOSION, MOVE_ICE_PUNCH, MOVE_THUNDERBOLT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_SPEED | F_EV_SPREAD_HP,
        .evs = {168, 0, 0, 0, 164, 176},
        .nature = NATURE_HASTY,
        .ability = ABILITY_LEVITATE
    },
    [FRONTIER_MON_AMPHAROS_3] = {
        .species = SPECIES_AMPHAROS,
        .moves = {MOVE_THUNDERBOLT, MOVE_HIDDEN_POWER, MOVE_TOXIC, MOVE_HEAL_BELL},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 88, 32, 120, 16},
        .nature = NATURE_MODEST,
        .ability = ABILITY_STATIC,
        .hiddenPowerType = TYPE_ICE
    },
    [FRONTIER_MON_SCIZOR_3] = {
        .species = SPECIES_SCIZOR,
        .moves = {MOVE_SILVER_WIND, MOVE_HIDDEN_POWER, MOVE_STEEL_WING, MOVE_QUICK_ATTACK},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {30, 252, 0, 0, 0, 228},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_SWARM,
        .hiddenPowerType = TYPE_ROCK
    },
    [FRONTIER_MON_HERACROSS_3] = {
        .species = SPECIES_HERACROSS,
        .moves = {MOVE_MEGAHORN, MOVE_BRICK_BREAK, MOVE_ROCK_SLIDE, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {4, 252, 0, 0, 0, 252},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_SWARM,
        .hiddenPowerType = TYPE_GHOST
    },
    [FRONTIER_MON_URSARING_3] = {
        .species = SPECIES_URSARING,
        .moves = {MOVE_BULK_UP, MOVE_RETURN, MOVE_HIDDEN_POWER, MOVE_EARTHQUAKE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 4, 0, 252, 0},
        .nature = NATURE_CAREFUL,
        .ability = ABILITY_GUTS,
        .hiddenPowerType = TYPE_GHOST
    },
    [FRONTIER_MON_HOUNDOOM_3] = {
        .species = SPECIES_HOUNDOOM,
        .moves = {MOVE_CRUNCH, MOVE_FIRE_BLAST, MOVE_TAUNT, MOVE_SUBSTITUTE},
        .itemTableId = BATTLE_FRONTIER_ITEM_PETAYA_BERRY,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {0, 0, 4, 252, 0, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_FLASH_FIRE
    },
    [FRONTIER_MON_DONPHAN_3] = {
        .species = SPECIES_DONPHAN,
        .moves = {MOVE_EARTHQUAKE, MOVE_RAPID_SPIN, MOVE_ANCIENT_POWER, MOVE_ROCK_SLIDE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 44, 188, 0, 0, 24},
        .nature = NATURE_IMPISH,
        .ability = ABILITY_STURDY
    },
    [FRONTIER_MON_CLAYDOL_3] = {
        .species = SPECIES_CLAYDOL,
        .moves = {MOVE_EARTHQUAKE, MOVE_PSYCHIC, MOVE_ICE_BEAM, MOVE_RAPID_SPIN},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHERI_BERRY,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {0, 128, 0, 128, 0, 252},
        .nature = NATURE_HASTY,
        .ability = ABILITY_LEVITATE
    },
    [FRONTIER_MON_WAILORD_3] = {
        .species = SPECIES_WAILORD,
        .moves = {MOVE_WATER_SPOUT, MOVE_SELF_DESTRUCT, MOVE_ICE_BEAM, MOVE_HYDRO_PUMP},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {0, 36, 0, 252, 0, 220},
        .nature = NATURE_RASH,
        .ability = ABILITY_WATER_VEIL
    },
    [FRONTIER_MON_NINETALES_3] = {
        .species = SPECIES_NINETALES,
        .moves = {MOVE_WILL_O_WISP, MOVE_FLAMETHROWER, MOVE_HIDDEN_POWER, MOVE_FIRE_BLAST},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {4, 0, 0, 252, 0, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_FLASH_FIRE,
        .hiddenPowerType = TYPE_GRASS
    },
    [FRONTIER_MON_MACHAMP_3] = {
        .species = SPECIES_MACHAMP,
        .moves = {MOVE_CROSS_CHOP, MOVE_ENCORE, MOVE_BULK_UP, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_DEFENSE | F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {132, 204, 88, 0, 0, 84},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_GUTS,
        .hiddenPowerType = TYPE_GHOST
    },
    [FRONTIER_MON_SHUCKLE_3] = {
        .species = SPECIES_SHUCKLE,
        .moves = {MOVE_TOXIC, MOVE_ENCORE, MOVE_WRAP, MOVE_REST},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 4, 0, 252, 0},
        .nature = NATURE_CAREFUL,
        .ability = ABILITY_STURDY
    },
    [FRONTIER_MON_STEELIX_3] = {
        .species = SPECIES_STEELIX,
        .moves = {MOVE_EARTHQUAKE, MOVE_IRON_TAIL, MOVE_HIDDEN_POWER, MOVE_PROTECT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {252, 104, 0, 0, 152, 0},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_ROCK_HEAD,
        .hiddenPowerType = TYPE_GHOST
    },
    [FRONTIER_MON_TENTACRUEL_3] = {
        .species = SPECIES_TENTACRUEL,
        .moves = {MOVE_BARRIER, MOVE_GIGA_DRAIN, MOVE_HYDRO_PUMP, MOVE_MIRROR_COAT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_HP,
        .evs = {252, 0, 80, 0, 0, 176},
        .nature = NATURE_TIMID,
        .ability = ABILITY_LIQUID_OOZE
    },
    [FRONTIER_MON_AERODACTYL_3] = {
        .species = SPECIES_AERODACTYL,
        .moves = {MOVE_EARTHQUAKE, MOVE_ROCK_SLIDE, MOVE_DOUBLE_EDGE, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {4, 252, 0, 0, 0, 252},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_ROCK_HEAD,
        .hiddenPowerType = TYPE_BUG
    },
    [FRONTIER_MON_PORYGON2_3] = {
        .species = SPECIES_PORYGON2,
        .moves = {MOVE_ICE_BEAM, MOVE_RECOVER, MOVE_THUNDERBOLT, MOVE_PSYCHIC},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 0, 4, 252, 0},
        .nature = NATURE_CALM,
        .ability = ABILITY_TRACE
    },
    [FRONTIER_MON_GARDEVOIR_3] = {
        .species = SPECIES_GARDEVOIR,
        .moves = {MOVE_SUBSTITUTE, MOVE_CALM_MIND, MOVE_PSYCHIC, MOVE_HYPNOSIS},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {4, 0, 0, 252, 0, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_TRACE
    },
    [FRONTIER_MON_EXEGGUTOR_3] = {
        .species = SPECIES_EXEGGUTOR,
        .moves = {MOVE_SLEEP_POWDER, MOVE_STUN_SPORE, MOVE_EXPLOSION, MOVE_SOLAR_BEAM},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED | F_EV_SPREAD_HP,
        .evs = {96, 0, 0, 252, 96, 60},
        .nature = NATURE_MODEST,
        .ability = ABILITY_CHLOROPHYLL
    },
    [FRONTIER_MON_STARMIE_3] = {
        .species = SPECIES_STARMIE,
        .moves = {MOVE_HYDRO_PUMP, MOVE_PSYCHIC, MOVE_ICE_BEAM, MOVE_PROTECT},
        .itemTableId = BATTLE_FRONTIER_ITEM_MYSTIC_WATER,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {0, 0, 0, 252, 4, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_NATURAL_CURE
    },
    [FRONTIER_MON_FLYGON_3] = {
        .species = SPECIES_FLYGON,
        .moves = {MOVE_EARTHQUAKE, MOVE_ROCK_SLIDE, MOVE_HIDDEN_POWER, MOVE_GUST},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {0, 252, 0, 4, 0, 252},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_LEVITATE,
        .hiddenPowerType = TYPE_BUG
    },
    [FRONTIER_MON_VENUSAUR_3] = {
        .species = SPECIES_VENUSAUR,
        .moves = {MOVE_RAZOR_LEAF, MOVE_HIDDEN_POWER, MOVE_LEECH_SEED, MOVE_SLEEP_POWDER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED | F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 64, 80, 56, 56},
        .nature = NATURE_CALM,
        .ability = ABILITY_OVERGROW,
        .hiddenPowerType = TYPE_FIRE
    },
    [FRONTIER_MON_VAPOREON_3] = {
        .species = SPECIES_VAPOREON,
        .moves = {MOVE_SURF, MOVE_HAZE, MOVE_WISH, MOVE_ICE_BEAM},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 228, 0, 28, 0},
        .nature = NATURE_BOLD,
        .ability = ABILITY_WATER_ABSORB
    },
    [FRONTIER_MON_JOLTEON_3] = {
        .species = SPECIES_JOLTEON,
        .moves = {MOVE_THUNDERBOLT, MOVE_SUBSTITUTE, MOVE_TOXIC, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_PETAYA_BERRY,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {4, 0, 0, 252, 0, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_VOLT_ABSORB,
        .hiddenPowerType = TYPE_ICE
    },
    [FRONTIER_MON_FLAREON_3] = {
        .species = SPECIES_FLAREON,
        .moves = {MOVE_WISH, MOVE_PROTECT, MOVE_FLAMETHROWER, MOVE_FIRE_BLAST},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 0, 4, 252, 0},
        .nature = NATURE_CALM,
        .ability = ABILITY_FLASH_FIRE
    },
    [FRONTIER_MON_MEGANIUM_3] = {
        .species = SPECIES_MEGANIUM,
        .moves = {MOVE_LEECH_SEED, MOVE_RAZOR_LEAF, MOVE_BODY_SLAM, MOVE_SYNTHESIS},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 252, 0, 4, 0},
        .nature = NATURE_BOLD,
        .ability = ABILITY_OVERGROW
    },
    [FRONTIER_MON_ESPEON_3] = {
        .species = SPECIES_ESPEON,
        .moves = {MOVE_PSYCHIC, MOVE_BATON_PASS, MOVE_CALM_MIND, MOVE_WISH},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {0, 0, 0, 252, 4, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_SYNCHRONIZE
    },
    [FRONTIER_MON_UMBREON_3] = {
        .species = SPECIES_UMBREON,
        .moves = {MOVE_MEAN_LOOK, MOVE_TOXIC, MOVE_TAUNT, MOVE_MOONLIGHT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_HP,
        .evs = {252, 0, 32, 0, 0, 224},
        .nature = NATURE_CAREFUL,
        .ability = ABILITY_SYNCHRONIZE
    },
    [FRONTIER_MON_BLASTOISE_3] = {
        .species = SPECIES_BLASTOISE,
        .moves = {MOVE_SURF, MOVE_RAPID_SPIN, MOVE_ICE_BEAM, MOVE_REST},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 148, 68, 0, 40},
        .nature = NATURE_BOLD,
        .ability = ABILITY_TORRENT
    },
    [FRONTIER_MON_FERALIGATR_3] = {
        .species = SPECIES_FERALIGATR,
        .moves = {MOVE_EARTHQUAKE, MOVE_SWORDS_DANCE, MOVE_SURF, MOVE_BRICK_BREAK},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_HP,
        .evs = {252, 4, 0, 252, 0, 0},
        .nature = NATURE_QUIET,
        .ability = ABILITY_TORRENT
    },
    [FRONTIER_MON_AGGRON_3] = {
        .species = SPECIES_AGGRON,
        .moves = {MOVE_ROCK_SLIDE, MOVE_ICE_BEAM, MOVE_SUBSTITUTE, MOVE_FOCUS_PUNCH},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {228, 252, 0, 28, 0, 0},
        .nature = NATURE_BRAVE,
        .ability = ABILITY_STURDY
    },
    [FRONTIER_MON_BLAZIKEN_3] = {
        .species = SPECIES_BLAZIKEN,
        .moves = {MOVE_SUBSTITUTE, MOVE_SWORDS_DANCE, MOVE_REVERSAL, MOVE_FIRE_BLAST},
        .itemTableId = BATTLE_FRONTIER_ITEM_SALAC_BERRY,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {80, 252, 0, 0, 0, 176},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_BLAZE
    },
    [FRONTIER_MON_WALREIN_3] = {
        .species = SPECIES_WALREIN,
        .moves = {MOVE_ICE_BEAM, MOVE_SURF, MOVE_TOXIC, MOVE_REST},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 252, 0, 4, 0},
        .nature = NATURE_BOLD,
        .ability = ABILITY_THICK_FAT
    },
    [FRONTIER_MON_SCEPTILE_3] = {
        .species = SPECIES_SCEPTILE,
        .moves = {MOVE_SWORDS_DANCE, MOVE_LEAF_BLADE, MOVE_AERIAL_ACE, MOVE_BRICK_BREAK},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {0, 252, 0, 36, 0, 220},
        .nature = NATURE_HASTY,
        .ability = ABILITY_OVERGROW
    },
    [FRONTIER_MON_CHARIZARD_3] = {
        .species = SPECIES_CHARIZARD,
        .moves = {MOVE_BLAST_BURN, MOVE_SUBSTITUTE, MOVE_HIDDEN_POWER, MOVE_FLAMETHROWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_PETAYA_BERRY,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {20, 0, 0, 236, 0, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_BLAZE,
        .hiddenPowerType = TYPE_GRASS
    },
    [FRONTIER_MON_TYPHLOSION_3] = {
        .species = SPECIES_TYPHLOSION,
        .moves = {MOVE_BLAST_BURN, MOVE_SUBSTITUTE, MOVE_FLAMETHROWER, MOVE_ENDURE},
        .itemTableId = BATTLE_FRONTIER_ITEM_PETAYA_BERRY,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {0, 4, 0, 252, 0, 252},
        .nature = NATURE_RASH,
        .ability = ABILITY_BLAZE
    },
    [FRONTIER_MON_LAPRAS_3] = {
        .species = SPECIES_LAPRAS,
        .moves = {MOVE_HYDRO_PUMP, MOVE_ICE_BEAM, MOVE_THUNDERBOLT, MOVE_HEAL_BELL},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED | F_EV_SPREAD_DEFENSE,
        .evs = {0, 0, 120, 252, 0, 136},
        .nature = NATURE_MODEST,
        .ability = ABILITY_WATER_ABSORB
    },
    [FRONTIER_MON_CROBAT_3] = {
        .species = SPECIES_CROBAT,
        .moves = {MOVE_SLUDGE_BOMB, MOVE_SHADOW_BALL, MOVE_AERIAL_ACE, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {4, 252, 0, 0, 0, 252},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_INNER_FOCUS,
        .hiddenPowerType = TYPE_FIGHTING
    },
    [FRONTIER_MON_SWAMPERT_3] = {
        .species = SPECIES_SWAMPERT,
        .moves = {MOVE_HYDRO_PUMP, MOVE_ICE_BEAM, MOVE_EARTHQUAKE, MOVE_FOCUS_PUNCH},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_HP,
        .evs = {240, 0, 0, 236, 0, 32},
        .nature = NATURE_QUIET,
        .ability = ABILITY_TORRENT
    },
    [FRONTIER_MON_GYARADOS_3] = {
        .species = SPECIES_GYARADOS,
        .moves = {MOVE_EARTHQUAKE, MOVE_HIDDEN_POWER, MOVE_DRAGON_DANCE, MOVE_PROTECT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LUM_BERRY,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {4, 252, 0, 0, 0, 252},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_INTIMIDATE,
        .hiddenPowerType = TYPE_FLYING
    },
    [FRONTIER_MON_SNORLAX_3] = {
        .species = SPECIES_SNORLAX,
        .moves = {MOVE_SELF_DESTRUCT, MOVE_BODY_SLAM, MOVE_SHADOW_BALL, MOVE_FOCUS_PUNCH},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_DEFENSE | F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {92, 116, 144, 0, 144, 12},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_IMMUNITY
    },
    [FRONTIER_MON_KINGDRA_3] = {
        .species = SPECIES_KINGDRA,
        .moves = {MOVE_ICE_BEAM, MOVE_RAIN_DANCE, MOVE_HYDRO_PUMP, MOVE_PROTECT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LUM_BERRY,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {0, 0, 0, 252, 4, 252},
        .nature = NATURE_MODEST,
        .ability = ABILITY_SWIFT_SWIM
    },
    [FRONTIER_MON_BLISSEY_3] = {
        .species = SPECIES_BLISSEY,
        .moves = {MOVE_SOFT_BOILED, MOVE_TOXIC, MOVE_HEAL_BELL, MOVE_SEISMIC_TOSS},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_DEFENSE,
        .evs = {4, 0, 252, 0, 252, 0},
        .nature = NATURE_CALM,
        .ability = ABILITY_NATURAL_CURE
    },
    [FRONTIER_MON_MILOTIC_3] = {
        .species = SPECIES_MILOTIC,
        .moves = {MOVE_RECOVER, MOVE_SURF, MOVE_TOXIC, MOVE_MIRROR_COAT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 252, 4, 0, 0},
        .nature = NATURE_BOLD,
        .ability = ABILITY_MARVEL_SCALE
    },
    [FRONTIER_MON_ARCANINE_3] = {
        .species = SPECIES_ARCANINE,
        .moves = {MOVE_FIRE_BLAST, MOVE_HIDDEN_POWER, MOVE_CRUNCH, MOVE_EXTREME_SPEED},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_HP,
        .evs = {184, 0, 0, 252, 0, 72},
        .nature = NATURE_MODEST,
        .ability = ABILITY_INTIMIDATE,
        .hiddenPowerType = TYPE_GRASS
    },
    [FRONTIER_MON_SALAMENCE_3] = {
        .species = SPECIES_SALAMENCE,
        .moves = {MOVE_DRAGON_DANCE, MOVE_AERIAL_ACE, MOVE_ROCK_SLIDE, MOVE_EARTHQUAKE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {116, 252, 0, 0, 0, 140},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_INTIMIDATE
    },
    [FRONTIER_MON_METAGROSS_3] = {
        .species = SPECIES_METAGROSS,
        .moves = {MOVE_METEOR_MASH, MOVE_EARTHQUAKE, MOVE_PROTECT, MOVE_EXPLOSION},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {252, 176, 80, 0, 0, 0},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_CLEAR_BODY
    },
    [FRONTIER_MON_SLAKING_3] = {
        .species = SPECIES_SLAKING,
        .moves = {MOVE_EARTHQUAKE, MOVE_SHADOW_BALL, MOVE_FOCUS_PUNCH, MOVE_DOUBLE_EDGE},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {4, 252, 0, 0, 0, 252},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_TRUANT
    },
    [FRONTIER_MON_DUGTRIO_4] = {
        .species = SPECIES_DUGTRIO,
        .moves = {MOVE_EARTHQUAKE, MOVE_ROCK_SLIDE, MOVE_HIDDEN_POWER, MOVE_SLUDGE_BOMB},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {0, 252, 0, 0, 4, 252},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_ARENA_TRAP,
        .hiddenPowerType = TYPE_GHOST
    },
    [FRONTIER_MON_MEDICHAM_4] = {
        .species = SPECIES_MEDICHAM,
        .moves = {MOVE_SUBSTITUTE, MOVE_REVERSAL, MOVE_FOCUS_PUNCH, MOVE_SHADOW_BALL},
        .itemTableId = BATTLE_FRONTIER_ITEM_SALAC_BERRY,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {4, 252, 0, 0, 0, 252},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_PURE_POWER
    },
    [FRONTIER_MON_MISDREAVUS_4] = {
        .species = SPECIES_MISDREAVUS,
        .moves = {MOVE_THUNDERBOLT, MOVE_HIDDEN_POWER, MOVE_THUNDER_WAVE, MOVE_PAIN_SPLIT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 84, 0, 172, 0},
        .nature = NATURE_CALM,
        .ability = ABILITY_LEVITATE,
        .hiddenPowerType = TYPE_ICE
    },
    [FRONTIER_MON_FEAROW_4] = {
        .species = SPECIES_FEAROW,
        .moves = {MOVE_DRILL_PECK, MOVE_DOUBLE_EDGE, MOVE_PROTECT, MOVE_LEER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {4, 252, 0, 0, 0, 252},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_KEEN_EYE
    },
    [FRONTIER_MON_GRANBULL_4] = {
        .species = SPECIES_GRANBULL,
        .moves = {MOVE_BULK_UP, MOVE_RETURN, MOVE_EARTHQUAKE, MOVE_SHADOW_BALL},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 40, 216, 0, 0, 0},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_INTIMIDATE
    },
    [FRONTIER_MON_JYNX_4] = {
        .species = SPECIES_JYNX,
        .moves = {MOVE_MEAN_LOOK, MOVE_PERISH_SONG, MOVE_SUBSTITUTE, MOVE_LOVELY_KISS},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_HP,
        .evs = {252, 0, 0, 0, 70, 184},
        .nature = NATURE_TIMID,
        .ability = ABILITY_OBLIVIOUS
    },
    [FRONTIER_MON_DUSCLOPS_4] = {
        .species = SPECIES_DUSCLOPS,
        .moves = {MOVE_WILL_O_WISP, MOVE_PAIN_SPLIT, MOVE_REST, MOVE_SHADOW_BALL},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 4, 0, 252, 0},
        .nature = NATURE_CALM,
        .ability = ABILITY_PRESSURE
    },
    [FRONTIER_MON_DODRIO_4] = {
        .species = SPECIES_DODRIO,
        .moves = {MOVE_DRILL_PECK, MOVE_HIDDEN_POWER, MOVE_QUICK_ATTACK, MOVE_DOUBLE_EDGE},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {4, 252, 0, 0, 0, 252},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_EARLY_BIRD,
        .hiddenPowerType = TYPE_FIGHTING
    },
    [FRONTIER_MON_MR_MIME_4] = {
        .species = SPECIES_MR_MIME,
        .moves = {MOVE_CALM_MIND, MOVE_PSYCHIC, MOVE_THUNDERBOLT, MOVE_FIRE_PUNCH},
        .itemTableId = BATTLE_FRONTIER_ITEM_LUM_BERRY,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {0, 0, 0, 252, 4, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_SOUNDPROOF
    },
    [FRONTIER_MON_LANTURN_4] = {
        .species = SPECIES_LANTURN,
        .moves = {MOVE_THUNDERBOLT, MOVE_SURF, MOVE_ICE_BEAM, MOVE_HYDRO_PUMP},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_SP_ATTACK,
        .evs = {0, 0, 0, 252, 252, 4},
        .nature = NATURE_MODEST,
        .ability = ABILITY_VOLT_ABSORB
    },
    [FRONTIER_MON_BRELOOM_4] = {
        .species = SPECIES_BRELOOM,
        .moves = {MOVE_SPORE, MOVE_FOCUS_PUNCH, MOVE_MACH_PUNCH, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {0, 252, 4, 0, 0, 252},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_EFFECT_SPORE,
        .hiddenPowerType = TYPE_GHOST
    },
    [FRONTIER_MON_FORRETRESS_4] = {
        .species = SPECIES_FORRETRESS,
        .moves = {MOVE_SPIKES, MOVE_RAPID_SPIN, MOVE_EXPLOSION, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 4, 0, 252, 0},
        .nature = NATURE_CAREFUL,
        .ability = ABILITY_STURDY,
        .hiddenPowerType = TYPE_BUG
    },
    [FRONTIER_MON_WHISCASH_4] = {
        .species = SPECIES_WHISCASH,
        .moves = {MOVE_SUBSTITUTE, MOVE_EARTHQUAKE, MOVE_SURF, MOVE_ICE_BEAM},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {172, 148, 0, 144, 0, 44},
        .nature = NATURE_MILD,
        .ability = ABILITY_OBLIVIOUS
    },
    [FRONTIER_MON_XATU_4] = {
        .species = SPECIES_XATU,
        .moves = {MOVE_CALM_MIND, MOVE_PSYCHIC, MOVE_HIDDEN_POWER, MOVE_SUBSTITUTE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {0, 0, 0, 252, 4, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_SYNCHRONIZE,
        .hiddenPowerType = TYPE_FIRE
    },
    [FRONTIER_MON_SKARMORY_4] = {
        .species = SPECIES_SKARMORY,
        .moves = {MOVE_HIDDEN_POWER, MOVE_SPIKES, MOVE_WHIRLWIND, MOVE_TOXIC},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {0, 252, 4, 0, 0, 252},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_KEEN_EYE,
        .hiddenPowerType = TYPE_GROUND
    },
    [FRONTIER_MON_MAROWAK_4] = {
        .species = SPECIES_MAROWAK,
        .moves = {MOVE_EARTHQUAKE, MOVE_ROCK_SLIDE, MOVE_SWORDS_DANCE, MOVE_DOUBLE_EDGE},
        .itemTableId = BATTLE_FRONTIER_ITEM_THICK_CLUB,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {4, 252, 0, 0, 0, 252},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_ROCK_HEAD
    },
    [FRONTIER_MON_QUAGSIRE_4] = {
        .species = SPECIES_QUAGSIRE,
        .moves = {MOVE_EARTHQUAKE, MOVE_REST, MOVE_CURSE, MOVE_TOXIC},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 216, 0, 40, 0},
        .nature = NATURE_RELAXED,
        .ability = ABILITY_WATER_ABSORB
    },
    [FRONTIER_MON_CLEFABLE_4] = {
        .species = SPECIES_CLEFABLE,
        .moves = {MOVE_SUBSTITUTE, MOVE_FOCUS_PUNCH, MOVE_FLAMETHROWER, MOVE_SHADOW_BALL},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {244, 144, 0, 120, 0, 0},
        .nature = NATURE_QUIET,
        .ability = ABILITY_CUTE_CHARM
    },
    [FRONTIER_MON_HARIYAMA_4] = {
        .species = SPECIES_HARIYAMA,
        .moves = {MOVE_FAKE_OUT, MOVE_BRICK_BREAK, MOVE_PROTECT, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {252, 252, 0, 0, 0, 4},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_GUTS,
        .hiddenPowerType = TYPE_GHOST
    },
    [FRONTIER_MON_RAICHU_4] = {
        .species = SPECIES_RAICHU,
        .moves = {MOVE_THUNDERBOLT, MOVE_HIDDEN_POWER, MOVE_SUBSTITUTE, MOVE_REVERSAL},
        .itemTableId = BATTLE_FRONTIER_ITEM_SALAC_BERRY,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {0, 252, 0, 120, 0, 136},
        .nature = NATURE_LONELY,
        .ability = ABILITY_STATIC,
        .hiddenPowerType = TYPE_GRASS
    },
    [FRONTIER_MON_DEWGONG_4] = {
        .species = SPECIES_DEWGONG,
        .moves = {MOVE_SURF, MOVE_REST, MOVE_SLEEP_TALK, MOVE_BLIZZARD},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_HP,
        .evs = {252, 0, 4, 100, 152, 0},
        .nature = NATURE_CALM,
        .ability = ABILITY_THICK_FAT
    },
    [FRONTIER_MON_MANECTRIC_4] = {
        .species = SPECIES_MANECTRIC,
        .moves = {MOVE_THUNDERBOLT, MOVE_THUNDER_WAVE, MOVE_CRUNCH, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {4, 0, 0, 252, 0, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_STATIC,
        .hiddenPowerType = TYPE_ICE
    },
    [FRONTIER_MON_VILEPLUME_4] = {
        .species = SPECIES_VILEPLUME,
        .moves = {MOVE_AROMATHERAPY, MOVE_LEECH_SEED, MOVE_GIGA_DRAIN, MOVE_SLEEP_POWDER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 156, 0, 96, 0},
        .nature = NATURE_CALM,
        .ability = ABILITY_CHLOROPHYLL
    },
    [FRONTIER_MON_VICTREEBEL_4] = {
        .species = SPECIES_VICTREEBEL,
        .moves = {MOVE_SUNNY_DAY, MOVE_SOLAR_BEAM, MOVE_HIDDEN_POWER, MOVE_SLUDGE_BOMB},
        .itemTableId = BATTLE_FRONTIER_ITEM_SOFT_SAND,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {0, 252, 0, 4, 0, 252},
        .nature = NATURE_NAUGHTY,
        .ability = ABILITY_CHLOROPHYLL,
        .hiddenPowerType = TYPE_GROUND
    },
    [FRONTIER_MON_ELECTRODE_4] = {
        .species = SPECIES_ELECTRODE,
        .moves = {MOVE_THUNDERBOLT, MOVE_EXPLOSION, MOVE_THUNDER_WAVE, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {4, 0, 0, 252, 0, 252},
        .nature = NATURE_MODEST,
        .ability = ABILITY_STATIC,
        .hiddenPowerType = TYPE_ICE
    },
    [FRONTIER_MON_EXPLOUD_4] = {
        .species = SPECIES_EXPLOUD,
        .moves = {MOVE_EARTHQUAKE, MOVE_HYPER_VOICE, MOVE_DYNAMIC_PUNCH, MOVE_BODY_SLAM},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {252, 252, 0, 0, 4, 0},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_SOUNDPROOF
    },
    [FRONTIER_MON_SHIFTRY_4] = {
        .species = SPECIES_SHIFTRY,
        .moves = {MOVE_SWORDS_DANCE, MOVE_BRICK_BREAK, MOVE_SHADOW_BALL, MOVE_DOUBLE_EDGE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LUM_BERRY,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {0, 252, 0, 0, 32, 224},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_CHLOROPHYLL
    },
    [FRONTIER_MON_GLALIE_4] = {
        .species = SPECIES_GLALIE,
        .moves = {MOVE_SPIKES, MOVE_EXPLOSION, MOVE_EARTHQUAKE, MOVE_TOXIC},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_HP,
        .evs = {252, 4, 0, 0, 0, 252},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_INNER_FOCUS
    },
    [FRONTIER_MON_LUDICOLO_4] = {
        .species = SPECIES_LUDICOLO,
        .moves = {MOVE_HYDRO_PUMP, MOVE_RAIN_DANCE, MOVE_GIGA_DRAIN, MOVE_ICE_BEAM},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_DEFENSE,
        .evs = {0, 0, 252, 212, 0, 44},
        .nature = NATURE_MODEST,
        .ability = ABILITY_SWIFT_SWIM
    },
    [FRONTIER_MON_HYPNO_4] = {
        .species = SPECIES_HYPNO,
        .moves = {MOVE_BATON_PASS, MOVE_CALM_MIND, MOVE_PSYCHIC, MOVE_PROTECT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 252, 0, 0, 0},
        .nature = NATURE_BOLD,
        .ability = ABILITY_INSOMNIA
    },
    [FRONTIER_MON_GOLEM_4] = {
        .species = SPECIES_GOLEM,
        .moves = {MOVE_EARTHQUAKE, MOVE_EXPLOSION, MOVE_ROCK_SLIDE, MOVE_DOUBLE_EDGE},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {168, 252, 0, 0, 0, 88},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_ROCK_HEAD
    },
    [FRONTIER_MON_RHYDON_4] = {
        .species = SPECIES_RHYDON,
        .moves = {MOVE_EARTHQUAKE, MOVE_ROCK_SLIDE, MOVE_MEGAHORN, MOVE_SWORDS_DANCE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {252, 252, 0, 0, 4, 0},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_ROCK_HEAD
    },
    [FRONTIER_MON_ALAKAZAM_4] = {
        .species = SPECIES_ALAKAZAM,
        .moves = {MOVE_PSYCHIC, MOVE_FOCUS_PUNCH, MOVE_SHADOW_BALL, MOVE_TRICK},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {0, 252, 0, 4, 0, 252},
        .nature = NATURE_LONELY,
        .ability = ABILITY_SYNCHRONIZE
    },
    [FRONTIER_MON_WEEZING_4] = {
        .species = SPECIES_WEEZING,
        .moves = {MOVE_WILL_O_WISP, MOVE_SLUDGE_BOMB, MOVE_EXPLOSION, MOVE_HAZE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 252, 0, 0, 4},
        .nature = NATURE_IMPISH,
        .ability = ABILITY_LEVITATE
    },
    [FRONTIER_MON_KANGASKHAN_4] = {
        .species = SPECIES_KANGASKHAN,
        .moves = {MOVE_SUBSTITUTE, MOVE_FOCUS_PUNCH, MOVE_RETURN, MOVE_SHADOW_BALL},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {212, 252, 44, 0, 0, 0},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_EARLY_BIRD
    },
    [FRONTIER_MON_ELECTABUZZ_4] = {
        .species = SPECIES_ELECTABUZZ,
        .moves = {MOVE_THUNDERBOLT, MOVE_ICE_PUNCH, MOVE_HIDDEN_POWER, MOVE_THIEF},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {4, 0, 0, 252, 0, 252},
        .nature = NATURE_MODEST,
        .ability = ABILITY_STATIC,
        .hiddenPowerType = TYPE_GRASS
    },
    [FRONTIER_MON_TAUROS_4] = {
        .species = SPECIES_TAUROS,
        .moves = {MOVE_EARTHQUAKE, MOVE_HIDDEN_POWER, MOVE_DOUBLE_EDGE, MOVE_HYPER_BEAM},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {4, 252, 0, 0, 0, 252},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_INTIMIDATE,
        .hiddenPowerType = TYPE_GHOST
    },
    [FRONTIER_MON_SLOWBRO_4] = {
        .species = SPECIES_SLOWBRO,
        .moves = {MOVE_SURF, MOVE_CALM_MIND, MOVE_REST, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 252, 0, 4, 0},
        .nature = NATURE_BOLD,
        .ability = ABILITY_OBLIVIOUS,
        .hiddenPowerType = TYPE_ELECTRIC
    },
    [FRONTIER_MON_SLOWKING_4] = {
        .species = SPECIES_SLOWKING,
        .moves = {MOVE_SURF, MOVE_CALM_MIND, MOVE_PSYCHIC, MOVE_FIRE_BLAST},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 180, 76, 0, 0},
        .nature = NATURE_BOLD,
        .ability = ABILITY_OWN_TEMPO
    },
    [FRONTIER_MON_MILTANK_4] = {
        .species = SPECIES_MILTANK,
        .moves = {MOVE_MILK_DRINK, MOVE_HEAL_BELL, MOVE_BODY_SLAM, MOVE_THUNDER_WAVE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 4, 252, 0, 0, 0},
        .nature = NATURE_IMPISH,
        .ability = ABILITY_THICK_FAT
    },
    [FRONTIER_MON_ALTARIA_4] = {
        .species = SPECIES_ALTARIA,
        .moves = {MOVE_REST, MOVE_DRAGON_DANCE, MOVE_TOXIC, MOVE_HEAL_BELL},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_HP,
        .evs = {248, 0, 44, 0, 216, 0},
        .nature = NATURE_CALM,
        .ability = ABILITY_NATURAL_CURE
    },
    [FRONTIER_MON_NIDOQUEEN_4] = {
        .species = SPECIES_NIDOQUEEN,
        .moves = {MOVE_EARTHQUAKE, MOVE_SUPERPOWER, MOVE_SLUDGE_BOMB, MOVE_ICE_BEAM},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {4, 252, 0, 0, 0, 252},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_POISON_POINT
    },
    [FRONTIER_MON_NIDOKING_4] = {
        .species = SPECIES_NIDOKING,
        .moves = {MOVE_EARTHQUAKE, MOVE_MEGAHORN, MOVE_SHADOW_BALL, MOVE_ICE_BEAM},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {4, 252, 0, 0, 0, 252},
        .nature = NATURE_HASTY,
        .ability = ABILITY_POISON_POINT
    },
    [FRONTIER_MON_MAGMAR_4] = {
        .species = SPECIES_MAGMAR,
        .moves = {MOVE_THUNDER_PUNCH, MOVE_FLAMETHROWER, MOVE_SUBSTITUTE, MOVE_FIRE_BLAST},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHARCOAL,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {0, 0, 0, 252, 4, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_FLAME_BODY
    },
    [FRONTIER_MON_CRADILY_4] = {
        .species = SPECIES_CRADILY,
        .moves = {MOVE_GIGA_DRAIN, MOVE_HIDDEN_POWER, MOVE_TOXIC, MOVE_RECOVER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_HP,
        .evs = {252, 0, 0, 164, 92, 0},
        .nature = NATURE_MODEST,
        .ability = ABILITY_SUCTION_CUPS,
        .hiddenPowerType = TYPE_FIRE
    },
    [FRONTIER_MON_ARMALDO_4] = {
        .species = SPECIES_ARMALDO,
        .moves = {MOVE_HIDDEN_POWER, MOVE_ROCK_BLAST, MOVE_EARTHQUAKE, MOVE_SWORDS_DANCE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_HP,
        .evs = {196, 60, 0, 0, 252, 0},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_BATTLE_ARMOR,
        .hiddenPowerType = TYPE_BUG
    },
    [FRONTIER_MON_GOLDUCK_4] = {
        .species = SPECIES_GOLDUCK,
        .moves = {MOVE_CALM_MIND, MOVE_ICE_BEAM, MOVE_SURF, MOVE_HYDRO_PUMP},
        .itemTableId = BATTLE_FRONTIER_ITEM_SALAC_BERRY,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {0, 0, 0, 252, 4, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_CLOUD_NINE
    },
    [FRONTIER_MON_RAPIDASH_4] = {
        .species = SPECIES_RAPIDASH,
        .moves = {MOVE_SUNNY_DAY, MOVE_FIRE_BLAST, MOVE_SOLAR_BEAM, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LUM_BERRY,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {0, 4, 0, 252, 0, 252},
        .nature = NATURE_NAIVE,
        .ability = ABILITY_FLASH_FIRE,
        .hiddenPowerType = TYPE_GROUND
    },
    [FRONTIER_MON_MUK_4] = {
        .species = SPECIES_MUK,
        .moves = {MOVE_SLUDGE_BOMB, MOVE_ICE_PUNCH, MOVE_EXPLOSION, MOVE_SHADOW_PUNCH},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {252, 252, 0, 0, 4, 0},
        .nature = NATURE_BRAVE,
        .ability = ABILITY_STICKY_HOLD
    },
    [FRONTIER_MON_GENGAR_4] = {
        .species = SPECIES_GENGAR,
        .moves = {MOVE_MEAN_LOOK, MOVE_PERISH_SONG, MOVE_TAUNT, MOVE_DESTINY_BOND},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_HP,
        .evs = {248, 0, 0, 0, 60, 200},
        .nature = NATURE_TIMID,
        .ability = ABILITY_LEVITATE
    },
    [FRONTIER_MON_AMPHAROS_4] = {
        .species = SPECIES_AMPHAROS,
        .moves = {MOVE_THUNDERBOLT, MOVE_FOCUS_PUNCH, MOVE_COUNTER, MOVE_HEAL_BELL},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 20, 236, 0, 0, 0},
        .nature = NATURE_RELAXED,
        .ability = ABILITY_STATIC
    },
    [FRONTIER_MON_SCIZOR_4] = {
        .species = SPECIES_SCIZOR,
        .moves = {MOVE_SWORDS_DANCE, MOVE_AGILITY, MOVE_SILVER_WIND, MOVE_STEEL_WING},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {120, 252, 24, 0, 112, 0},
        .nature = NATURE_CAREFUL,
        .ability = ABILITY_SWARM
    },
    [FRONTIER_MON_HERACROSS_4] = {
        .species = SPECIES_HERACROSS,
        .moves = {MOVE_MEGAHORN, MOVE_BRICK_BREAK, MOVE_EARTHQUAKE, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {104, 176, 0, 0, 0, 228},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_SWARM,
        .hiddenPowerType = TYPE_GHOST
    },
    [FRONTIER_MON_URSARING_4] = {
        .species = SPECIES_URSARING,
        .moves = {MOVE_HYPER_BEAM, MOVE_RETURN, MOVE_BRICK_BREAK, MOVE_EARTHQUAKE},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {120, 252, 0, 0, 92, 44},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_GUTS
    },
    [FRONTIER_MON_HOUNDOOM_4] = {
        .species = SPECIES_HOUNDOOM,
        .moves = {MOVE_PURSUIT, MOVE_CRUNCH, MOVE_FIRE_BLAST, MOVE_FLAMETHROWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {56, 0, 0, 252, 12, 188},
        .nature = NATURE_TIMID,
        .ability = ABILITY_FLASH_FIRE
    },
    [FRONTIER_MON_DONPHAN_4] = {
        .species = SPECIES_DONPHAN,
        .moves = {MOVE_EARTHQUAKE, MOVE_RAPID_SPIN, MOVE_ROCK_SLIDE, MOVE_ROAR},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 44, 188, 0, 0, 24},
        .nature = NATURE_IMPISH,
        .ability = ABILITY_STURDY
    },
    [FRONTIER_MON_CLAYDOL_4] = {
        .species = SPECIES_CLAYDOL,
        .moves = {MOVE_REST, MOVE_RAPID_SPIN, MOVE_LIGHT_SCREEN, MOVE_COSMIC_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 128, 0, 128, 0},
        .nature = NATURE_CAREFUL,
        .ability = ABILITY_LEVITATE
    },
    [FRONTIER_MON_WAILORD_4] = {
        .species = SPECIES_WAILORD,
        .moves = {MOVE_WATER_SPOUT, MOVE_SELF_DESTRUCT, MOVE_ICE_BEAM, MOVE_SURF},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {0, 36, 0, 252, 0, 220},
        .nature = NATURE_RASH,
        .ability = ABILITY_WATER_VEIL
    },
    [FRONTIER_MON_NINETALES_4] = {
        .species = SPECIES_NINETALES,
        .moves = {MOVE_FLAMETHROWER, MOVE_WILL_O_WISP, MOVE_SUNNY_DAY, MOVE_OVERHEAT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_SP_ATTACK,
        .evs = {68, 0, 4, 252, 132, 52},
        .nature = NATURE_TIMID,
        .ability = ABILITY_FLASH_FIRE
    },
    [FRONTIER_MON_MACHAMP_4] = {
        .species = SPECIES_MACHAMP,
        .moves = {MOVE_ROCK_SLIDE, MOVE_CROSS_CHOP, MOVE_BULK_UP, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {0, 252, 0, 0, 4, 252},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_GUTS,
        .hiddenPowerType = TYPE_BUG
    },
    [FRONTIER_MON_SHUCKLE_4] = {
        .species = SPECIES_SHUCKLE,
        .moves = {MOVE_EARTHQUAKE, MOVE_ROCK_SLIDE, MOVE_TOXIC, MOVE_REST},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {252, 252, 0, 0, 4, 0},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_STURDY
    },
    [FRONTIER_MON_STEELIX_4] = {
        .species = SPECIES_STEELIX,
        .moves = {MOVE_EARTHQUAKE, MOVE_EXPLOSION, MOVE_ROAR, MOVE_TOXIC},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 36, 0, 0, 220, 0},
        .nature = NATURE_IMPISH,
        .ability = ABILITY_ROCK_HEAD
    },
    [FRONTIER_MON_TENTACRUEL_4] = {
        .species = SPECIES_TENTACRUEL,
        .moves = {MOVE_RAPID_SPIN, MOVE_SURF, MOVE_TOXIC, MOVE_ICE_BEAM},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_SPEED | F_EV_SPREAD_HP,
        .evs = {252, 0, 0, 0, 120, 136},
        .nature = NATURE_CALM,
        .ability = ABILITY_LIQUID_OOZE
    },
    [FRONTIER_MON_AERODACTYL_4] = {
        .species = SPECIES_AERODACTYL,
        .moves = {MOVE_EARTHQUAKE, MOVE_ROCK_SLIDE, MOVE_DOUBLE_EDGE, MOVE_SUBSTITUTE},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {4, 252, 0, 0, 0, 252},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_ROCK_HEAD
    },
    [FRONTIER_MON_PORYGON2_4] = {
        .species = SPECIES_PORYGON2,
        .moves = {MOVE_THUNDERBOLT, MOVE_ICE_BEAM, MOVE_CONVERSION_2, MOVE_RECOVER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 252, 0, 4, 0},
        .nature = NATURE_BOLD,
        .ability = ABILITY_TRACE
    },
    [FRONTIER_MON_GARDEVOIR_4] = {
        .species = SPECIES_GARDEVOIR,
        .moves = {MOVE_PSYCHIC, MOVE_WISH, MOVE_PROTECT, MOVE_ICE_PUNCH},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 0, 48, 176, 32},
        .nature = NATURE_CALM,
        .ability = ABILITY_TRACE
    },
    [FRONTIER_MON_EXEGGUTOR_4] = {
        .species = SPECIES_EXEGGUTOR,
        .moves = {MOVE_SUBSTITUTE, MOVE_SOLAR_BEAM, MOVE_LEECH_SEED, MOVE_EXPLOSION},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED | F_EV_SPREAD_HP,
        .evs = {124, 0, 0, 188, 96, 96},
        .nature = NATURE_MODEST,
        .ability = ABILITY_CHLOROPHYLL
    },
    [FRONTIER_MON_STARMIE_4] = {
        .species = SPECIES_STARMIE,
        .moves = {MOVE_HYDRO_PUMP, MOVE_PSYCHIC, MOVE_ICE_BEAM, MOVE_THUNDERBOLT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {0, 0, 0, 252, 4, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_NATURAL_CURE
    },
    [FRONTIER_MON_FLYGON_4] = {
        .species = SPECIES_FLYGON,
        .moves = {MOVE_EARTHQUAKE, MOVE_ROCK_SLIDE, MOVE_DRAGON_CLAW, MOVE_PROTECT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {0, 252, 0, 0, 4, 252},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_LEVITATE
    },
    [FRONTIER_MON_VENUSAUR_4] = {
        .species = SPECIES_VENUSAUR,
        .moves = {MOVE_SUBSTITUTE, MOVE_FRENZY_PLANT, MOVE_SLUDGE_BOMB, MOVE_GIGA_DRAIN},
        .itemTableId = BATTLE_FRONTIER_ITEM_PETAYA_BERRY,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED | F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 96, 56, 0, 104},
        .nature = NATURE_CALM,
        .ability = ABILITY_OVERGROW
    },
    [FRONTIER_MON_VAPOREON_4] = {
        .species = SPECIES_VAPOREON,
        .moves = {MOVE_ACID_ARMOR, MOVE_BATON_PASS, MOVE_ROAR, MOVE_SUBSTITUTE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 228, 0, 28, 0},
        .nature = NATURE_BOLD,
        .ability = ABILITY_WATER_ABSORB
    },
    [FRONTIER_MON_JOLTEON_4] = {
        .species = SPECIES_JOLTEON,
        .moves = {MOVE_THUNDERBOLT, MOVE_BATON_PASS, MOVE_HIDDEN_POWER, MOVE_THUNDER_WAVE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {4, 0, 0, 252, 0, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_VOLT_ABSORB,
        .hiddenPowerType = TYPE_ICE
    },
    [FRONTIER_MON_FLAREON_4] = {
        .species = SPECIES_FLAREON,
        .moves = {MOVE_SHADOW_BALL, MOVE_OVERHEAT, MOVE_FLAMETHROWER, MOVE_DOUBLE_EDGE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_ATTACK,
        .evs = {0, 252, 0, 252, 0, 4},
        .nature = NATURE_NAUGHTY,
        .ability = ABILITY_FLASH_FIRE
    },
    [FRONTIER_MON_MEGANIUM_4] = {
        .species = SPECIES_MEGANIUM,
        .moves = {MOVE_LEECH_SEED, MOVE_SYNTHESIS, MOVE_GIGA_DRAIN, MOVE_PROTECT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 132, 72, 52, 0},
        .nature = NATURE_BOLD,
        .ability = ABILITY_OVERGROW
    },
    [FRONTIER_MON_ESPEON_4] = {
        .species = SPECIES_ESPEON,
        .moves = {MOVE_PSYCHIC, MOVE_BATON_PASS, MOVE_CALM_MIND, MOVE_LIGHT_SCREEN},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {0, 0, 0, 252, 4, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_SYNCHRONIZE
    },
    [FRONTIER_MON_UMBREON_4] = {
        .species = SPECIES_UMBREON,
        .moves = {MOVE_HELPING_HAND, MOVE_BATON_PASS, MOVE_CURSE, MOVE_PROTECT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 252, 0, 4, 0},
        .nature = NATURE_CALM,
        .ability = ABILITY_SYNCHRONIZE
    },
    [FRONTIER_MON_BLASTOISE_4] = {
        .species = SPECIES_BLASTOISE,
        .moves = {MOVE_HYDRO_CANNON, MOVE_SUBSTITUTE, MOVE_ICY_WIND, MOVE_SURF},
        .itemTableId = BATTLE_FRONTIER_ITEM_PETAYA_BERRY,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED | F_EV_SPREAD_HP,
        .evs = {68, 0, 0, 240, 92, 108},
        .nature = NATURE_MODEST,
        .ability = ABILITY_TORRENT
    },
    [FRONTIER_MON_FERALIGATR_4] = {
        .species = SPECIES_FERALIGATR,
        .moves = {MOVE_EARTHQUAKE, MOVE_SWORDS_DANCE, MOVE_SURF, MOVE_ROCK_SLIDE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_HP,
        .evs = {252, 4, 0, 252, 0, 0},
        .nature = NATURE_QUIET,
        .ability = ABILITY_TORRENT
    },
    [FRONTIER_MON_AGGRON_4] = {
        .species = SPECIES_AGGRON,
        .moves = {MOVE_ROCK_SLIDE, MOVE_THUNDER_WAVE, MOVE_EARTHQUAKE, MOVE_SUBSTITUTE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {0, 252, 4, 0, 0, 252},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_ROCK_HEAD
    },
    [FRONTIER_MON_BLAZIKEN_4] = {
        .species = SPECIES_BLAZIKEN,
        .moves = {MOVE_SKY_UPPERCUT, MOVE_ROCK_SLIDE, MOVE_FOCUS_PUNCH, MOVE_FIRE_BLAST},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {4, 252, 0, 0, 0, 252},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_BLAZE
    },
    [FRONTIER_MON_WALREIN_4] = {
        .species = SPECIES_WALREIN,
        .moves = {MOVE_ICE_BEAM, MOVE_SURF, MOVE_ENCORE, MOVE_ICY_WIND},
        .itemTableId = BATTLE_FRONTIER_ITEM_NEVER_MELT_ICE,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_HP,
        .evs = {192, 0, 0, 56, 48, 212},
        .nature = NATURE_MODEST,
        .ability = ABILITY_THICK_FAT
    },
    [FRONTIER_MON_SCEPTILE_4] = {
        .species = SPECIES_SCEPTILE,
        .moves = {MOVE_LEAF_BLADE, MOVE_THUNDER_PUNCH, MOVE_HIDDEN_POWER, MOVE_FOCUS_PUNCH},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {0, 32, 0, 252, 0, 224},
        .nature = NATURE_HASTY,
        .ability = ABILITY_OVERGROW,
        .hiddenPowerType = TYPE_ICE
    },
    [FRONTIER_MON_CHARIZARD_4] = {
        .species = SPECIES_CHARIZARD,
        .moves = {MOVE_HIDDEN_POWER, MOVE_FIRE_BLAST, MOVE_SUBSTITUTE, MOVE_DRAGON_CLAW},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {0, 4, 0, 252, 0, 252},
        .nature = NATURE_HASTY,
        .ability = ABILITY_BLAZE,
        .hiddenPowerType = TYPE_GRASS
    },
    [FRONTIER_MON_TYPHLOSION_4] = {
        .species = SPECIES_TYPHLOSION,
        .moves = {MOVE_THUNDER_PUNCH, MOVE_SUBSTITUTE, MOVE_FLAMETHROWER, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_PETAYA_BERRY,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {4, 0, 0, 252, 0, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_BLAZE,
        .hiddenPowerType = TYPE_GRASS
    },
    [FRONTIER_MON_LAPRAS_4] = {
        .species = SPECIES_LAPRAS,
        .moves = {MOVE_CURSE, MOVE_RETURN, MOVE_SURF, MOVE_REST},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 80, 0, 176, 0},
        .nature = NATURE_SASSY,
        .ability = ABILITY_SHELL_ARMOR
    },
    [FRONTIER_MON_CROBAT_4] = {
        .species = SPECIES_CROBAT,
        .moves = {MOVE_SLUDGE_BOMB, MOVE_SHADOW_BALL, MOVE_AERIAL_ACE, MOVE_TAUNT},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {4, 252, 0, 0, 0, 252},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_INNER_FOCUS
    },
    [FRONTIER_MON_SWAMPERT_4] = {
        .species = SPECIES_SWAMPERT,
        .moves = {MOVE_SUBSTITUTE, MOVE_ENDEAVOR, MOVE_HYDRO_PUMP, MOVE_ROAR},
        .itemTableId = BATTLE_FRONTIER_ITEM_SALAC_BERRY,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {0, 0, 4, 252, 0, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_TORRENT
    },
    [FRONTIER_MON_GYARADOS_4] = {
        .species = SPECIES_GYARADOS,
        .moves = {MOVE_HIDDEN_POWER, MOVE_DRAGON_DANCE, MOVE_EARTHQUAKE, MOVE_SUBSTITUTE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LIECHI_BERRY,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {160, 252, 0, 0, 0, 96},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_INTIMIDATE,
        .hiddenPowerType = TYPE_FLYING
    },
    [FRONTIER_MON_SNORLAX_4] = {
        .species = SPECIES_SNORLAX,
        .moves = {MOVE_CURSE, MOVE_BODY_SLAM, MOVE_PROTECT, MOVE_SELF_DESTRUCT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {120, 0, 136, 0, 252, 0},
        .nature = NATURE_CAREFUL,
        .ability = ABILITY_IMMUNITY
    },
    [FRONTIER_MON_KINGDRA_4] = {
        .species = SPECIES_KINGDRA,
        .moves = {MOVE_RAIN_DANCE, MOVE_HYDRO_PUMP, MOVE_SUBSTITUTE, MOVE_ICE_BEAM},
        .itemTableId = BATTLE_FRONTIER_ITEM_PETAYA_BERRY,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_HP,
        .evs = {192, 0, 0, 252, 4, 60},
        .nature = NATURE_MODEST,
        .ability = ABILITY_SWIFT_SWIM
    },
    [FRONTIER_MON_BLISSEY_4] = {
        .species = SPECIES_BLISSEY,
        .moves = {MOVE_SOFT_BOILED, MOVE_ICE_BEAM, MOVE_SEISMIC_TOSS, MOVE_TOXIC},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_DEFENSE,
        .evs = {0, 0, 252, 252, 0, 4},
        .nature = NATURE_MODEST,
        .ability = ABILITY_NATURAL_CURE
    },
    [FRONTIER_MON_MILOTIC_4] = {
        .species = SPECIES_MILOTIC,
        .moves = {MOVE_RECOVER, MOVE_SURF, MOVE_TOXIC, MOVE_HYPNOSIS},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 252, 4, 0, 0},
        .nature = NATURE_BOLD,
        .ability = ABILITY_MARVEL_SCALE
    },
    [FRONTIER_MON_ARCANINE_4] = {
        .species = SPECIES_ARCANINE,
        .moves = {MOVE_FIRE_BLAST, MOVE_HIDDEN_POWER, MOVE_TOXIC, MOVE_EXTREME_SPEED},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {0, 4, 0, 252, 0, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_INTIMIDATE,
        .hiddenPowerType = TYPE_GRASS
    },
    [FRONTIER_MON_SALAMENCE_4] = {
        .species = SPECIES_SALAMENCE,
        .moves = {MOVE_FLAMETHROWER, MOVE_TOXIC, MOVE_WISH, MOVE_PROTECT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 252, 0, 4, 0},
        .nature = NATURE_BOLD,
        .ability = ABILITY_INTIMIDATE
    },
    [FRONTIER_MON_METAGROSS_4] = {
        .species = SPECIES_METAGROSS,
        .moves = {MOVE_PSYCHIC, MOVE_HIDDEN_POWER, MOVE_METEOR_MASH, MOVE_EXPLOSION},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_HP,
        .evs = {252, 0, 0, 252, 0, 4},
        .nature = NATURE_QUIET,
        .ability = ABILITY_CLEAR_BODY,
        .hiddenPowerType = TYPE_FIRE
    },
    [FRONTIER_MON_SLAKING_4] = {
        .species = SPECIES_SLAKING,
        .moves = {MOVE_SHADOW_BALL, MOVE_EARTHQUAKE, MOVE_RETURN, MOVE_SLACK_OFF},
        .itemTableId = BATTLE_FRONTIER_ITEM_LUM_BERRY,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {0, 252, 0, 0, 4, 252},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_TRUANT
    },
    [FRONTIER_MON_ARTICUNO_1] = {
        .species = SPECIES_ARTICUNO,
        .moves = {MOVE_ICE_BEAM, MOVE_REST, MOVE_SLEEP_TALK, MOVE_ROAR},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_HP,
        .evs = {252, 0, 0, 114, 136, 8},
        .nature = NATURE_CALM,
        .ability = ABILITY_PRESSURE
    },
    [FRONTIER_MON_ZAPDOS_1] = {
        .species = SPECIES_ZAPDOS,
        .moves = {MOVE_THUNDERBOLT, MOVE_HIDDEN_POWER, MOVE_THUNDER_WAVE, MOVE_ROAR},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {4, 0, 0, 252, 0, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_PRESSURE,
        .hiddenPowerType = TYPE_GRASS
    },
    [FRONTIER_MON_MOLTRES_1] = {
        .species = SPECIES_MOLTRES,
        .moves = {MOVE_FLAMETHROWER, MOVE_HIDDEN_POWER, MOVE_WILL_O_WISP, MOVE_ROAR},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {0, 0, 0, 252, 4, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_PRESSURE,
        .hiddenPowerType = TYPE_GRASS
    },
    [FRONTIER_MON_RAIKOU_1] = {
        .species = SPECIES_RAIKOU,
        .moves = {MOVE_CALM_MIND, MOVE_THUNDERBOLT, MOVE_HIDDEN_POWER, MOVE_SUBSTITUTE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {4, 0, 0, 252, 0, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_PRESSURE,
        .hiddenPowerType = TYPE_GRASS
    },
    [FRONTIER_MON_ENTEI_1] = {
        .species = SPECIES_ENTEI,
        .moves = {MOVE_SUBSTITUTE, MOVE_CALM_MIND, MOVE_FLAMETHROWER, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED | F_EV_SPREAD_HP,
        .evs = {132, 0, 0, 228, 0, 148},
        .nature = NATURE_TIMID,
        .ability = ABILITY_PRESSURE,
        .hiddenPowerType = TYPE_GRASS
    },
    [FRONTIER_MON_SUICUNE_1] = {
        .species = SPECIES_SUICUNE,
        .moves = {MOVE_CALM_MIND, MOVE_SURF, MOVE_REST, MOVE_ROAR},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {240, 0, 252, 0, 0, 16},
        .nature = NATURE_BOLD,
        .ability = ABILITY_PRESSURE
    },
    [FRONTIER_MON_REGIROCK_1] = {
        .species = SPECIES_REGIROCK,
        .moves = {MOVE_ROCK_SLIDE, MOVE_FOCUS_PUNCH, MOVE_THUNDER_WAVE, MOVE_EXPLOSION},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {252, 252, 0, 0, 4, 0},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_CLEAR_BODY
    },
    [FRONTIER_MON_REGICE_1] = {
        .species = SPECIES_REGICE,
        .moves = {MOVE_ICE_BEAM, MOVE_THUNDERBOLT, MOVE_EXPLOSION, MOVE_THUNDER_WAVE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {252, 104, 0, 152, 0, 0},
        .nature = NATURE_QUIET,
        .ability = ABILITY_CLEAR_BODY
    },
    [FRONTIER_MON_REGISTEEL_1] = {
        .species = SPECIES_REGISTEEL,
        .moves = {MOVE_THUNDER_WAVE, MOVE_SEISMIC_TOSS, MOVE_COUNTER, MOVE_EXPLOSION},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 0, 0, 212, 44},
        .nature = NATURE_CAREFUL,
        .ability = ABILITY_CLEAR_BODY
    },
    [FRONTIER_MON_LATIAS_1] = {
        .species = SPECIES_LATIAS,
        .moves = {MOVE_THUNDER, MOVE_ICE_BEAM, MOVE_RECOVER, MOVE_REFRESH},
        .itemTableId = BATTLE_FRONTIER_ITEM_SOUL_DEW,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED | F_EV_SPREAD_HP,
        .evs = {252, 0, 0, 116, 0, 140},
        .nature = NATURE_TIMID,
        .ability = ABILITY_LEVITATE
    },
    [FRONTIER_MON_LATIOS_1] = {
        .species = SPECIES_LATIOS,
        .moves = {MOVE_RECOVER, MOVE_REFRESH, MOVE_THUNDER, MOVE_ICE_BEAM},
        .itemTableId = BATTLE_FRONTIER_ITEM_SOUL_DEW,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {4, 0, 0, 252, 0, 252},
        .nature = NATURE_MODEST,
        .ability = ABILITY_LEVITATE
    },
    [FRONTIER_MON_ARTICUNO_2] = {
        .species = SPECIES_ARTICUNO,
        .moves = {MOVE_ICE_BEAM, MOVE_REST, MOVE_HEAL_BELL, MOVE_SLEEP_TALK},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_HP,
        .evs = {252, 0, 0, 114, 136, 8},
        .nature = NATURE_CALM,
        .ability = ABILITY_PRESSURE
    },
    [FRONTIER_MON_ZAPDOS_2] = {
        .species = SPECIES_ZAPDOS,
        .moves = {MOVE_THUNDERBOLT, MOVE_REST, MOVE_TOXIC, MOVE_ROAR},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 0, 0, 252, 4},
        .nature = NATURE_CALM,
        .ability = ABILITY_PRESSURE
    },
    [FRONTIER_MON_MOLTRES_2] = {
        .species = SPECIES_MOLTRES,
        .moves = {MOVE_PROTECT, MOVE_HIDDEN_POWER, MOVE_HEAT_WAVE, MOVE_OVERHEAT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {40, 0, 0, 252, 0, 216},
        .nature = NATURE_MODEST,
        .ability = ABILITY_PRESSURE,
        .hiddenPowerType = TYPE_GRASS
    },
    [FRONTIER_MON_RAIKOU_2] = {
        .species = SPECIES_RAIKOU,
        .moves = {MOVE_REST, MOVE_CALM_MIND, MOVE_THUNDERBOLT, MOVE_SLEEP_TALK},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED | F_EV_SPREAD_HP,
        .evs = {252, 0, 0, 120, 0, 136},
        .nature = NATURE_MODEST,
        .ability = ABILITY_PRESSURE
    },
    [FRONTIER_MON_ENTEI_2] = {
        .species = SPECIES_ENTEI,
        .moves = {MOVE_SUNNY_DAY, MOVE_CALM_MIND, MOVE_FLAMETHROWER, MOVE_SOLAR_BEAM},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED | F_EV_SPREAD_HP,
        .evs = {120, 0, 0, 240, 0, 148},
        .nature = NATURE_TIMID,
        .ability = ABILITY_PRESSURE
    },
    [FRONTIER_MON_SUICUNE_2] = {
        .species = SPECIES_SUICUNE,
        .moves = {MOVE_CALM_MIND, MOVE_HYDRO_PUMP, MOVE_ICE_BEAM, MOVE_ROAR},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {28, 0, 0, 252, 0, 228},
        .nature = NATURE_MODEST,
        .ability = ABILITY_PRESSURE
    },
    [FRONTIER_MON_REGIROCK_2] = {
        .species = SPECIES_REGIROCK,
        .moves = {MOVE_CURSE, MOVE_ROCK_SLIDE, MOVE_EARTHQUAKE, MOVE_EXPLOSION},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {252, 180, 0, 0, 76, 0},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_CLEAR_BODY
    },
    [FRONTIER_MON_REGICE_2] = {
        .species = SPECIES_REGICE,
        .moves = {MOVE_ICE_BEAM, MOVE_THUNDERBOLT, MOVE_THUNDER_WAVE, MOVE_PROTECT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_HP,
        .evs = {252, 0, 4, 252, 0, 0},
        .nature = NATURE_MODEST,
        .ability = ABILITY_CLEAR_BODY
    },
    [FRONTIER_MON_REGISTEEL_2] = {
        .species = SPECIES_REGISTEEL,
        .moves = {MOVE_THUNDER_WAVE, MOVE_COUNTER, MOVE_SEISMIC_TOSS, MOVE_PROTECT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 48, 0, 208, 0},
        .nature = NATURE_CAREFUL,
        .ability = ABILITY_CLEAR_BODY
    },
    [FRONTIER_MON_LATIAS_2] = {
        .species = SPECIES_LATIAS,
        .moves = {MOVE_DRAGON_CLAW, MOVE_CALM_MIND, MOVE_RECOVER, MOVE_REFRESH},
        .itemTableId = BATTLE_FRONTIER_ITEM_SOUL_DEW,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {240, 0, 172, 0, 0, 96},
        .nature = NATURE_BOLD,
        .ability = ABILITY_LEVITATE
    },
    [FRONTIER_MON_LATIOS_2] = {
        .species = SPECIES_LATIOS,
        .moves = {MOVE_CALM_MIND, MOVE_THUNDER, MOVE_SOLAR_BEAM, MOVE_DRAGON_CLAW},
        .itemTableId = BATTLE_FRONTIER_ITEM_SOUL_DEW,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {4, 0, 0, 252, 0, 252},
        .nature = NATURE_MODEST,
        .ability = ABILITY_LEVITATE
    },
    [FRONTIER_MON_ARTICUNO_3] = {
        .species = SPECIES_ARTICUNO,
        .moves = {MOVE_ICE_BEAM, MOVE_PROTECT, MOVE_SUBSTITUTE, MOVE_REST},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_HP,
        .evs = {252, 0, 0, 114, 136, 8},
        .nature = NATURE_CALM,
        .ability = ABILITY_PRESSURE
    },
    [FRONTIER_MON_ZAPDOS_3] = {
        .species = SPECIES_ZAPDOS,
        .moves = {MOVE_THUNDERBOLT, MOVE_DRILL_PECK, MOVE_HIDDEN_POWER, MOVE_AGILITY},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {0, 120, 0, 252, 0, 136},
        .nature = NATURE_MILD,
        .ability = ABILITY_PRESSURE,
        .hiddenPowerType = TYPE_GRASS
    },
    [FRONTIER_MON_MOLTRES_3] = {
        .species = SPECIES_MOLTRES,
        .moves = {MOVE_SUBSTITUTE, MOVE_OVERHEAT, MOVE_HIDDEN_POWER, MOVE_FIRE_BLAST},
        .itemTableId = BATTLE_FRONTIER_ITEM_PETAYA_BERRY,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {0, 0, 4, 252, 0, 252},
        .nature = NATURE_MODEST,
        .ability = ABILITY_PRESSURE,
        .hiddenPowerType = TYPE_GRASS
    },
    [FRONTIER_MON_RAIKOU_3] = {
        .species = SPECIES_RAIKOU,
        .moves = {MOVE_REST, MOVE_SLEEP_TALK, MOVE_THUNDERBOLT, MOVE_ROAR},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED | F_EV_SPREAD_HP,
        .evs = {252, 0, 0, 120, 0, 136},
        .nature = NATURE_MODEST,
        .ability = ABILITY_PRESSURE
    },
    [FRONTIER_MON_ENTEI_3] = {
        .species = SPECIES_ENTEI,
        .moves = {MOVE_CALM_MIND, MOVE_FLAMETHROWER, MOVE_REST, MOVE_SUBSTITUTE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {132, 0, 124, 0, 0, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_PRESSURE
    },
    [FRONTIER_MON_SUICUNE_3] = {
        .species = SPECIES_SUICUNE,
        .moves = {MOVE_SUBSTITUTE, MOVE_CALM_MIND, MOVE_SURF, MOVE_ICE_BEAM},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED | F_EV_SPREAD_HP,
        .evs = {252, 0, 0, 104, 0, 152},
        .nature = NATURE_MODEST,
        .ability = ABILITY_PRESSURE
    },
    [FRONTIER_MON_REGIROCK_3] = {
        .species = SPECIES_REGIROCK,
        .moves = {MOVE_ROCK_SLIDE, MOVE_SUPERPOWER, MOVE_EARTHQUAKE, MOVE_EXPLOSION},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {252, 252, 0, 0, 4, 0},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_CLEAR_BODY
    },
    [FRONTIER_MON_REGICE_3] = {
        .species = SPECIES_REGICE,
        .moves = {MOVE_ICE_BEAM, MOVE_PROTECT, MOVE_THUNDERBOLT, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {52, 0, 0, 252, 0, 204},
        .nature = NATURE_MODEST,
        .ability = ABILITY_CLEAR_BODY,
        .hiddenPowerType = TYPE_FIRE
    },
    [FRONTIER_MON_REGISTEEL_3] = {
        .species = SPECIES_REGISTEEL,
        .moves = {MOVE_SEISMIC_TOSS, MOVE_IRON_DEFENSE, MOVE_AMNESIA, MOVE_PROTECT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 48, 0, 208, 0},
        .nature = NATURE_CAREFUL,
        .ability = ABILITY_CLEAR_BODY
    },
    [FRONTIER_MON_LATIAS_3] = {
        .species = SPECIES_LATIAS,
        .moves = {MOVE_CALM_MIND, MOVE_RECOVER, MOVE_DRAGON_CLAW, MOVE_ICE_BEAM},
        .itemTableId = BATTLE_FRONTIER_ITEM_SOUL_DEW,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED | F_EV_SPREAD_HP,
        .evs = {160, 0, 0, 96, 0, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_LEVITATE
    },
    [FRONTIER_MON_LATIOS_3] = {
        .species = SPECIES_LATIOS,
        .moves = {MOVE_THUNDER, MOVE_RECOVER, MOVE_ICE_BEAM, MOVE_DRAGON_CLAW},
        .itemTableId = BATTLE_FRONTIER_ITEM_SOUL_DEW,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {4, 0, 0, 252, 0, 252},
        .nature = NATURE_MODEST,
        .ability = ABILITY_LEVITATE
    },
    [FRONTIER_MON_ARTICUNO_4] = {
        .species = SPECIES_ARTICUNO,
        .moves = {MOVE_ICE_BEAM, MOVE_REST, MOVE_HEAL_BELL, MOVE_HAZE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 0, 4, 252, 0},
        .nature = NATURE_CALM,
        .ability = ABILITY_PRESSURE
    },
    [FRONTIER_MON_ZAPDOS_4] = {
        .species = SPECIES_ZAPDOS,
        .moves = {MOVE_THUNDERBOLT, MOVE_HIDDEN_POWER, MOVE_THUNDER_WAVE, MOVE_PROTECT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LUM_BERRY,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {4, 0, 0, 252, 0, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_PRESSURE,
        .hiddenPowerType = TYPE_GRASS
    },
    [FRONTIER_MON_MOLTRES_4] = {
        .species = SPECIES_MOLTRES,
        .moves = {MOVE_FLAMETHROWER, MOVE_WILL_O_WISP, MOVE_HIDDEN_POWER, MOVE_PROTECT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {0, 0, 0, 252, 4, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_PRESSURE,
        .hiddenPowerType = TYPE_GRASS
    },
    [FRONTIER_MON_RAIKOU_4] = {
        .species = SPECIES_RAIKOU,
        .moves = {MOVE_THUNDERBOLT, MOVE_HIDDEN_POWER, MOVE_CALM_MIND, MOVE_PROTECT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LUM_BERRY,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {4, 0, 0, 252, 0, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_PRESSURE,
        .hiddenPowerType = TYPE_GRASS
    },
    [FRONTIER_MON_ENTEI_4] = {
        .species = SPECIES_ENTEI,
        .moves = {MOVE_FLAMETHROWER, MOVE_CALM_MIND, MOVE_SUBSTITUTE, MOVE_FIRE_BLAST},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_HP,
        .evs = {252, 0, 0, 4, 0, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_PRESSURE
    },
    [FRONTIER_MON_SUICUNE_4] = {
        .species = SPECIES_SUICUNE,
        .moves = {MOVE_CALM_MIND, MOVE_SURF, MOVE_REST, MOVE_SLEEP_TALK},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_HP,
        .evs = {240, 0, 0, 252, 0, 16},
        .nature = NATURE_MODEST,
        .ability = ABILITY_PRESSURE
    },
    [FRONTIER_MON_REGIROCK_4] = {
        .species = SPECIES_REGIROCK,
        .moves = {MOVE_EXPLOSION, MOVE_ROCK_SLIDE, MOVE_THUNDER_WAVE, MOVE_EARTHQUAKE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {252, 252, 0, 0, 4, 0},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_CLEAR_BODY
    },
    [FRONTIER_MON_REGICE_4] = {
        .species = SPECIES_REGICE,
        .moves = {MOVE_ICE_BEAM, MOVE_AMNESIA, MOVE_THUNDERBOLT, MOVE_SEISMIC_TOSS},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {248, 0, 124, 124, 0, 12},
        .nature = NATURE_MODEST,
        .ability = ABILITY_CLEAR_BODY
    },
    [FRONTIER_MON_REGISTEEL_4] = {
        .species = SPECIES_REGISTEEL,
        .moves = {MOVE_REST, MOVE_SEISMIC_TOSS, MOVE_AMNESIA, MOVE_COUNTER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 4, 0, 252, 0},
        .nature = NATURE_CAREFUL,
        .ability = ABILITY_CLEAR_BODY
    },
    [FRONTIER_MON_LATIAS_4] = {
        .species = SPECIES_LATIAS,
        .moves = {MOVE_CALM_MIND, MOVE_RECOVER, MOVE_DRAGON_CLAW, MOVE_THUNDER},
        .itemTableId = BATTLE_FRONTIER_ITEM_SOUL_DEW,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED | F_EV_SPREAD_HP,
        .evs = {160, 0, 0, 96, 0, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_LEVITATE
    },
    [FRONTIER_MON_LATIOS_4] = {
        .species = SPECIES_LATIOS,
        .moves = {MOVE_THUNDER, MOVE_DRAGON_CLAW, MOVE_RECOVER, MOVE_CALM_MIND},
        .itemTableId = BATTLE_FRONTIER_ITEM_SOUL_DEW,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {0, 0, 0, 252, 4, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_LEVITATE
    },
    [FRONTIER_MON_GENGAR_5] = {
        .species = SPECIES_GENGAR,
        .moves = {MOVE_ICE_PUNCH, MOVE_FIRE_PUNCH, MOVE_WILL_O_WISP, MOVE_DESTINY_BOND},
        .itemTableId = BATTLE_FRONTIER_ITEM_LUM_BERRY,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_HP,
        .evs = {252, 0, 0, 80, 0, 176},
        .nature = NATURE_TIMID,
        .ability = ABILITY_LEVITATE
    },
    [FRONTIER_MON_GENGAR_6] = {
        .species = SPECIES_GENGAR,
        .moves = {MOVE_THUNDERBOLT, MOVE_ICE_PUNCH, MOVE_EXPLOSION, MOVE_WILL_O_WISP},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_SPEED | F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {248, 0, 44, 8, 112, 96},
        .nature = NATURE_TIMID,
        .ability = ABILITY_LEVITATE
    },
    [FRONTIER_MON_GENGAR_7] = {
        .species = SPECIES_GENGAR,
        .moves = {MOVE_WILL_O_WISP, MOVE_EXPLOSION, MOVE_SHADOW_BALL, MOVE_TAUNT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {0, 252, 0, 4, 0, 252},
        .nature = NATURE_HASTY,
        .ability = ABILITY_LEVITATE
    },
    [FRONTIER_MON_GENGAR_8] = {
        .species = SPECIES_GENGAR,
        .moves = {MOVE_THUNDERBOLT, MOVE_ICE_PUNCH, MOVE_FIRE_PUNCH, MOVE_TAUNT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {0, 0, 0, 252, 4, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_LEVITATE
    },
    [FRONTIER_MON_URSARING_5] = {
        .species = SPECIES_URSARING,
        .moves = {MOVE_HIDDEN_POWER, MOVE_BODY_SLAM, MOVE_EARTHQUAKE, MOVE_FOCUS_PUNCH},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {160, 252, 0, 0, 0, 96},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_GUTS,
        .hiddenPowerType = TYPE_GHOST
    },
    [FRONTIER_MON_URSARING_6] = {
        .species = SPECIES_URSARING,
        .moves = {MOVE_HIDDEN_POWER, MOVE_BODY_SLAM, MOVE_EARTHQUAKE, MOVE_COUNTER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {160, 252, 0, 0, 0, 96},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_GUTS,
        .hiddenPowerType = TYPE_GHOST
    },
    [FRONTIER_MON_URSARING_7] = {
        .species = SPECIES_URSARING,
        .moves = {MOVE_HIDDEN_POWER, MOVE_BODY_SLAM, MOVE_EARTHQUAKE, MOVE_RETURN},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {160, 252, 0, 0, 0, 96},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_GUTS,
        .hiddenPowerType = TYPE_GHOST
    },
    [FRONTIER_MON_URSARING_8] = {
        .species = SPECIES_URSARING,
        .moves = {MOVE_HIDDEN_POWER, MOVE_BODY_SLAM, MOVE_EARTHQUAKE, MOVE_BRICK_BREAK},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {160, 252, 0, 0, 0, 96},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_GUTS,
        .hiddenPowerType = TYPE_GHOST
    },
    [FRONTIER_MON_MACHAMP_5] = {
        .species = SPECIES_MACHAMP,
        .moves = {MOVE_ROCK_SLIDE, MOVE_CROSS_CHOP, MOVE_BULK_UP, MOVE_EARTHQUAKE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {0, 252, 0, 0, 4, 252},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_GUTS
    },
    [FRONTIER_MON_MACHAMP_6] = {
        .species = SPECIES_MACHAMP,
        .moves = {MOVE_ROCK_SLIDE, MOVE_CROSS_CHOP, MOVE_BULK_UP, MOVE_LOW_KICK},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {0, 252, 0, 0, 4, 252},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_GUTS
    },
    [FRONTIER_MON_MACHAMP_7] = {
        .species = SPECIES_MACHAMP,
        .moves = {MOVE_CROSS_CHOP, MOVE_ROCK_SLIDE, MOVE_BULK_UP, MOVE_FACADE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {52, 252, 0, 0, 0, 204},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_GUTS
    },
    [FRONTIER_MON_MACHAMP_8] = {
        .species = SPECIES_MACHAMP,
        .moves = {MOVE_CROSS_CHOP, MOVE_ROCK_SLIDE, MOVE_BULK_UP, MOVE_BRICK_BREAK},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {52, 252, 0, 0, 0, 204},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_GUTS
    },
    [FRONTIER_MON_GARDEVOIR_5] = {
        .species = SPECIES_GARDEVOIR,
        .moves = {MOVE_PSYCHIC, MOVE_CALM_MIND, MOVE_THUNDERBOLT, MOVE_MEMENTO},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_HP,
        .evs = {252, 0, 4, 0, 0, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_TRACE
    },
    [FRONTIER_MON_GARDEVOIR_6] = {
        .species = SPECIES_GARDEVOIR,
        .moves = {MOVE_PSYCHIC, MOVE_CALM_MIND, MOVE_THUNDERBOLT, MOVE_REFLECT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_HP,
        .evs = {252, 0, 4, 0, 0, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_TRACE
    },
    [FRONTIER_MON_GARDEVOIR_7] = {
        .species = SPECIES_GARDEVOIR,
        .moves = {MOVE_PSYCHIC, MOVE_CALM_MIND, MOVE_THUNDERBOLT, MOVE_FIRE_PUNCH},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_HP,
        .evs = {252, 0, 4, 0, 0, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_TRACE
    },
    [FRONTIER_MON_GARDEVOIR_8] = {
        .species = SPECIES_GARDEVOIR,
        .moves = {MOVE_PSYCHIC, MOVE_CALM_MIND, MOVE_THUNDERBOLT, MOVE_SUBSTITUTE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_HP,
        .evs = {252, 0, 4, 0, 0, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_TRACE
    },
    [FRONTIER_MON_STARMIE_5] = {
        .species = SPECIES_STARMIE,
        .moves = {MOVE_HYDRO_PUMP, MOVE_THUNDERBOLT, MOVE_SUBSTITUTE, MOVE_ICE_BEAM},
        .itemTableId = BATTLE_FRONTIER_ITEM_PETAYA_BERRY,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {0, 0, 0, 252, 4, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_ILLUMINATE
    },
    [FRONTIER_MON_STARMIE_6] = {
        .species = SPECIES_STARMIE,
        .moves = {MOVE_RAPID_SPIN, MOVE_ICE_BEAM, MOVE_THUNDERBOLT, MOVE_SURF},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {4, 0, 0, 252, 0, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_NATURAL_CURE
    },
    [FRONTIER_MON_STARMIE_7] = {
        .species = SPECIES_STARMIE,
        .moves = {MOVE_RAPID_SPIN, MOVE_ICE_BEAM, MOVE_THUNDERBOLT, MOVE_RECOVER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {4, 0, 0, 252, 0, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_NATURAL_CURE
    },
    [FRONTIER_MON_STARMIE_8] = {
        .species = SPECIES_STARMIE,
        .moves = {MOVE_RAPID_SPIN, MOVE_ICE_BEAM, MOVE_THUNDERBOLT, MOVE_PSYCHIC},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {4, 0, 0, 252, 0, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_NATURAL_CURE
    },
    [FRONTIER_MON_LAPRAS_5] = {
        .species = SPECIES_LAPRAS,
        .moves = {MOVE_ICE_BEAM, MOVE_THUNDERBOLT, MOVE_HEAL_BELL, MOVE_SURF},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 0, 4, 252, 0},
        .nature = NATURE_CALM,
        .ability = ABILITY_WATER_ABSORB
    },
    [FRONTIER_MON_LAPRAS_6] = {
        .species = SPECIES_LAPRAS,
        .moves = {MOVE_ICE_BEAM, MOVE_THUNDERBOLT, MOVE_HEAL_BELL, MOVE_PERISH_SONG},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 0, 4, 252, 0},
        .nature = NATURE_CALM,
        .ability = ABILITY_WATER_ABSORB
    },
    [FRONTIER_MON_LAPRAS_7] = {
        .species = SPECIES_LAPRAS,
        .moves = {MOVE_ICE_BEAM, MOVE_THUNDERBOLT, MOVE_HEAL_BELL, MOVE_TOXIC},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 0, 4, 252, 0},
        .nature = NATURE_CALM,
        .ability = ABILITY_WATER_ABSORB
    },
    [FRONTIER_MON_LAPRAS_8] = {
        .species = SPECIES_LAPRAS,
        .moves = {MOVE_ICE_BEAM, MOVE_THUNDERBOLT, MOVE_HEAL_BELL, MOVE_ROAR},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 0, 4, 252, 0},
        .nature = NATURE_CALM,
        .ability = ABILITY_WATER_ABSORB
    },
    [FRONTIER_MON_SNORLAX_5] = {
        .species = SPECIES_SNORLAX,
        .moves = {MOVE_CURSE, MOVE_AMNESIA, MOVE_REST, MOVE_BODY_SLAM},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_HP,
        .evs = {208, 0, 252, 0, 48, 0},
        .nature = NATURE_CAREFUL,
        .ability = ABILITY_IMMUNITY
    },
    [FRONTIER_MON_SNORLAX_6] = {
        .species = SPECIES_SNORLAX,
        .moves = {MOVE_RETURN, MOVE_EARTHQUAKE, MOVE_SHADOW_BALL, MOVE_BRICK_BREAK},
        .itemTableId = BATTLE_FRONTIER_ITEM_LUM_BERRY,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {160, 96, 252, 0, 0, 0},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_THICK_FAT
    },
    [FRONTIER_MON_SNORLAX_7] = {
        .species = SPECIES_SNORLAX,
        .moves = {MOVE_RETURN, MOVE_EARTHQUAKE, MOVE_CURSE, MOVE_PROTECT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_DEFENSE | F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {160, 96, 252, 0, 0, 0},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_THICK_FAT
    },
    [FRONTIER_MON_SNORLAX_8] = {
        .species = SPECIES_SNORLAX,
        .moves = {MOVE_SHADOW_BALL, MOVE_SELF_DESTRUCT, MOVE_BODY_SLAM, MOVE_EARTHQUAKE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_DEFENSE | F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {92, 116, 144, 0, 144, 12},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_IMMUNITY
    },
    [FRONTIER_MON_SALAMENCE_5] = {
        .species = SPECIES_SALAMENCE,
        .moves = {MOVE_AERIAL_ACE, MOVE_EARTHQUAKE, MOVE_ROCK_SLIDE, MOVE_SUBSTITUTE},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED | F_EV_SPREAD_DEFENSE,
        .evs = {0, 0, 124, 248, 0, 136},
        .nature = NATURE_MODEST,
        .ability = ABILITY_INTIMIDATE
    },
    [FRONTIER_MON_SALAMENCE_6] = {
        .species = SPECIES_SALAMENCE,
        .moves = {MOVE_BRICK_BREAK, MOVE_ROCK_SLIDE, MOVE_EARTHQUAKE, MOVE_DRAGON_DANCE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {116, 252, 0, 0, 0, 140},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_INTIMIDATE
    },
    [FRONTIER_MON_SALAMENCE_7] = {
        .species = SPECIES_SALAMENCE,
        .moves = {MOVE_BRICK_BREAK, MOVE_ROCK_SLIDE, MOVE_EARTHQUAKE, MOVE_DRAGON_CLAW},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {116, 252, 0, 0, 0, 140},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_INTIMIDATE
    },
    [FRONTIER_MON_SALAMENCE_8] = {
        .species = SPECIES_SALAMENCE,
        .moves = {MOVE_BRICK_BREAK, MOVE_ROCK_SLIDE, MOVE_EARTHQUAKE, MOVE_FIRE_BLAST},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {116, 252, 0, 0, 0, 140},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_INTIMIDATE
    },
    [FRONTIER_MON_METAGROSS_5] = {
        .species = SPECIES_METAGROSS,
        .moves = {MOVE_METEOR_MASH, MOVE_EARTHQUAKE, MOVE_PURSUIT, MOVE_EXPLOSION},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {252, 116, 24, 0, 116, 0},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_CLEAR_BODY
    },
    [FRONTIER_MON_METAGROSS_6] = {
        .species = SPECIES_METAGROSS,
        .moves = {MOVE_METEOR_MASH, MOVE_HIDDEN_POWER, MOVE_PURSUIT, MOVE_EXPLOSION},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {252, 116, 24, 96, 20, 0},
        .nature = NATURE_BRAVE,
        .ability = ABILITY_CLEAR_BODY,
        .hiddenPowerType = TYPE_FIRE
    },
    [FRONTIER_MON_METAGROSS_7] = {
        .species = SPECIES_METAGROSS,
        .moves = {MOVE_METEOR_MASH, MOVE_EARTHQUAKE, MOVE_SHADOW_BALL, MOVE_PROTECT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LUM_BERRY,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {252, 192, 0, 0, 64, 0},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_CLEAR_BODY
    },
    [FRONTIER_MON_METAGROSS_8] = {
        .species = SPECIES_METAGROSS,
        .moves = {MOVE_METEOR_MASH, MOVE_EARTHQUAKE, MOVE_SLUDGE_BOMB, MOVE_SHADOW_BALL},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {252, 252, 0, 0, 0, 4},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_CLEAR_BODY
    },
    [FRONTIER_MON_REGIROCK_5] = {
        .species = SPECIES_REGIROCK,
        .moves = {MOVE_COUNTER, MOVE_ROCK_SLIDE, MOVE_CURSE, MOVE_EARTHQUAKE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {224, 252, 0, 0, 32, 0},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_CLEAR_BODY
    },
    [FRONTIER_MON_REGIROCK_6] = {
        .species = SPECIES_REGIROCK,
        .moves = {MOVE_EXPLOSION, MOVE_ROCK_SLIDE, MOVE_THUNDER_WAVE, MOVE_SUPERPOWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {252, 252, 0, 0, 4, 0},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_CLEAR_BODY
    },
    [FRONTIER_MON_REGICE_5] = {
        .species = SPECIES_REGICE,
        .moves = {MOVE_ICE_BEAM, MOVE_THUNDERBOLT, MOVE_THUNDER_WAVE, MOVE_PSYCH_UP},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {252, 104, 0, 152, 0, 0},
        .nature = NATURE_QUIET,
        .ability = ABILITY_CLEAR_BODY
    },
    [FRONTIER_MON_REGICE_6] = {
        .species = SPECIES_REGICE,
        .moves = {MOVE_ICE_BEAM, MOVE_THUNDERBOLT, MOVE_THUNDER_WAVE, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {252, 104, 0, 152, 0, 0},
        .nature = NATURE_QUIET,
        .ability = ABILITY_CLEAR_BODY,
        .hiddenPowerType = TYPE_FIRE
    },
    [FRONTIER_MON_REGISTEEL_5] = {
        .species = SPECIES_REGISTEEL,
        .moves = {MOVE_EXPLOSION, MOVE_SEISMIC_TOSS, MOVE_THUNDER_WAVE, MOVE_TOXIC},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 0, 0, 212, 44},
        .nature = NATURE_CAREFUL,
        .ability = ABILITY_CLEAR_BODY
    },
    [FRONTIER_MON_REGISTEEL_6] = {
        .species = SPECIES_REGISTEEL,
        .moves = {MOVE_EXPLOSION, MOVE_SEISMIC_TOSS, MOVE_THUNDER_WAVE, MOVE_CURSE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 0, 0, 212, 44},
        .nature = NATURE_CAREFUL,
        .ability = ABILITY_CLEAR_BODY
    },
    [FRONTIER_MON_LATIAS_5] = {
        .species = SPECIES_LATIAS,
        .moves = {MOVE_CALM_MIND, MOVE_RECOVER, MOVE_DRAGON_CLAW, MOVE_PSYCHIC},
        .itemTableId = BATTLE_FRONTIER_ITEM_SOUL_DEW,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED | F_EV_SPREAD_HP,
        .evs = {160, 0, 0, 96, 0, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_LEVITATE
    },
    [FRONTIER_MON_LATIAS_6] = {
        .species = SPECIES_LATIAS,
        .moves = {MOVE_REFLECT, MOVE_DRAGON_CLAW, MOVE_PROTECT, MOVE_THUNDER_WAVE},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHERI_BERRY,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_HP,
        .evs = {240, 0, 0, 16, 0, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_LEVITATE
    },
    [FRONTIER_MON_LATIAS_7] = {
        .species = SPECIES_LATIAS,
        .moves = {MOVE_REFLECT, MOVE_DRAGON_CLAW, MOVE_PROTECT, MOVE_SWAGGER},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHERI_BERRY,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_HP,
        .evs = {240, 0, 0, 16, 0, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_LEVITATE
    },
    [FRONTIER_MON_LATIAS_8] = {
        .species = SPECIES_LATIAS,
        .moves = {MOVE_REFLECT, MOVE_DRAGON_CLAW, MOVE_PROTECT, MOVE_LIGHT_SCREEN},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHERI_BERRY,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_HP,
        .evs = {240, 0, 0, 16, 0, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_LEVITATE
    },
    [FRONTIER_MON_LATIOS_5] = {
        .species = SPECIES_LATIOS,
        .moves = {MOVE_THUNDER, MOVE_DRAGON_CLAW, MOVE_RECOVER, MOVE_REFRESH},
        .itemTableId = BATTLE_FRONTIER_ITEM_SOUL_DEW,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {0, 0, 0, 252, 4, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_LEVITATE
    },
    [FRONTIER_MON_LATIOS_6] = {
        .species = SPECIES_LATIOS,
        .moves = {MOVE_THUNDER, MOVE_DRAGON_CLAW, MOVE_RECOVER, MOVE_SOLAR_BEAM},
        .itemTableId = BATTLE_FRONTIER_ITEM_SOUL_DEW,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {0, 0, 0, 252, 4, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_LEVITATE
    },
    [FRONTIER_MON_LATIOS_7] = {
        .species = SPECIES_LATIOS,
        .moves = {MOVE_PROTECT, MOVE_CALM_MIND, MOVE_DRAGON_CLAW, MOVE_SUBSTITUTE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {76, 0, 0, 180, 0, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_LEVITATE
    },
    [FRONTIER_MON_LATIOS_8] = {
        .species = SPECIES_LATIOS,
        .moves = {MOVE_PROTECT, MOVE_CALM_MIND, MOVE_DRAGON_CLAW, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {76, 0, 0, 180, 0, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_LEVITATE,
        .hiddenPowerType = TYPE_FIRE
    },
    [FRONTIER_MON_DRAGONITE_1] = {
        .species = SPECIES_DRAGONITE,
        .moves = {MOVE_AERIAL_ACE, MOVE_EARTHQUAKE, MOVE_DRAGON_DANCE, MOVE_THUNDER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {52, 204, 0, 68, 0, 184},
        .nature = NATURE_LONELY,
        .ability = ABILITY_INNER_FOCUS
    },
    [FRONTIER_MON_DRAGONITE_2] = {
        .species = SPECIES_DRAGONITE,
        .moves = {MOVE_SUBSTITUTE, MOVE_FOCUS_PUNCH, MOVE_THUNDERBOLT, MOVE_ICE_BEAM},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {120, 64, 0, 252, 68, 4},
        .nature = NATURE_BRAVE,
        .ability = ABILITY_INNER_FOCUS
    },
    [FRONTIER_MON_DRAGONITE_3] = {
        .species = SPECIES_DRAGONITE,
        .moves = {MOVE_ICE_BEAM, MOVE_FOCUS_PUNCH, MOVE_THUNDERBOLT, MOVE_FLAMETHROWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_ATTACK,
        .evs = {0, 178, 0, 252, 56, 24},
        .nature = NATURE_BRAVE,
        .ability = ABILITY_INNER_FOCUS
    },
    [FRONTIER_MON_DRAGONITE_4] = {
        .species = SPECIES_DRAGONITE,
        .moves = {MOVE_AERIAL_ACE, MOVE_EARTHQUAKE, MOVE_FOCUS_PUNCH, MOVE_THUNDER},
        .itemTableId = BATTLE_FRONTIER_ITEM_CHOICE_BAND,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {60, 252, 0, 0, 0, 196},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_INNER_FOCUS
    },
    [FRONTIER_MON_DRAGONITE_5] = {
        .species = SPECIES_DRAGONITE,
        .moves = {MOVE_DRAGON_DANCE, MOVE_DOUBLE_EDGE, MOVE_EARTHQUAKE, MOVE_AERIAL_ACE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LIECHI_BERRY,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_ATTACK,
        .evs = {4, 252, 0, 0, 180, 72},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_INNER_FOCUS
    },
    [FRONTIER_MON_DRAGONITE_6] = {
        .species = SPECIES_DRAGONITE,
        .moves = {MOVE_DRAGON_DANCE, MOVE_SUBSTITUTE, MOVE_HYPER_BEAM, MOVE_AERIAL_ACE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LIECHI_BERRY,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {44, 100, 0, 0, 152, 212},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_INNER_FOCUS
    },
    [FRONTIER_MON_DRAGONITE_7] = {
        .species = SPECIES_DRAGONITE,
        .moves = {MOVE_HEAL_BELL, MOVE_EARTHQUAKE, MOVE_DRAGON_DANCE, MOVE_AERIAL_ACE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {252, 252, 0, 4, 0, 0},
        .nature = NATURE_BRAVE,
        .ability = ABILITY_INNER_FOCUS
    },
    [FRONTIER_MON_DRAGONITE_8] = {
        .species = SPECIES_DRAGONITE,
        .moves = {MOVE_HEAL_BELL, MOVE_EARTHQUAKE, MOVE_DRAGON_DANCE, MOVE_FOCUS_PUNCH},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {252, 252, 0, 4, 0, 0},
        .nature = NATURE_BRAVE,
        .ability = ABILITY_INNER_FOCUS
    },
    [FRONTIER_MON_DRAGONITE_9] = {
        .species = SPECIES_DRAGONITE,
        .moves = {MOVE_HEAL_BELL, MOVE_EARTHQUAKE, MOVE_DRAGON_DANCE, MOVE_ICE_BEAM},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {252, 252, 0, 4, 0, 0},
        .nature = NATURE_BRAVE,
        .ability = ABILITY_INNER_FOCUS
    },
    [FRONTIER_MON_DRAGONITE_10] = {
        .species = SPECIES_DRAGONITE,
        .moves = {MOVE_HEAL_BELL, MOVE_EARTHQUAKE, MOVE_DRAGON_DANCE, MOVE_SUBSTITUTE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {252, 252, 0, 4, 0, 0},
        .nature = NATURE_BRAVE,
        .ability = ABILITY_INNER_FOCUS
    },
    [FRONTIER_MON_TYRANITAR_1] = {
        .species = SPECIES_TYRANITAR,
        .moves = {MOVE_DRAGON_DANCE, MOVE_ROCK_SLIDE, MOVE_EARTHQUAKE, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LUM_BERRY,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {4, 252, 0, 0, 0, 252},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_SAND_STREAM,
        .hiddenPowerType = TYPE_BUG
    },
    [FRONTIER_MON_TYRANITAR_2] = {
        .species = SPECIES_TYRANITAR,
        .moves = {MOVE_ROCK_SLIDE, MOVE_EARTHQUAKE, MOVE_HIDDEN_POWER, MOVE_FOCUS_PUNCH},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {248, 252, 0, 0, 8, 0},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_SAND_STREAM,
        .hiddenPowerType = TYPE_BUG
    },
    [FRONTIER_MON_TYRANITAR_3] = {
        .species = SPECIES_TYRANITAR,
        .moves = {MOVE_PURSUIT, MOVE_CRUNCH, MOVE_FIRE_BLAST, MOVE_ROAR},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_HP,
        .evs = {252, 0, 0, 252, 0, 4},
        .nature = NATURE_MODEST,
        .ability = ABILITY_SAND_STREAM
    },
    [FRONTIER_MON_TYRANITAR_4] = {
        .species = SPECIES_TYRANITAR,
        .moves = {MOVE_FIRE_BLAST, MOVE_HIDDEN_POWER, MOVE_ICE_BEAM, MOVE_BRICK_BREAK},
        .itemTableId = BATTLE_FRONTIER_ITEM_LUM_BERRY,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {0, 4, 0, 252, 0, 252},
        .nature = NATURE_HASTY,
        .ability = ABILITY_SAND_STREAM,
        .hiddenPowerType = TYPE_GRASS
    },
    [FRONTIER_MON_TYRANITAR_5] = {
        .species = SPECIES_TYRANITAR,
        .moves = {MOVE_ROCK_SLIDE, MOVE_EARTHQUAKE, MOVE_DRAGON_DANCE, MOVE_PROTECT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LUM_BERRY,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {4, 252, 0, 0, 0, 252},
        .nature = NATURE_JOLLY,
        .ability = ABILITY_SAND_STREAM
    },
    [FRONTIER_MON_TYRANITAR_6] = {
        .species = SPECIES_TYRANITAR,
        .moves = {MOVE_ROCK_SLIDE, MOVE_EARTHQUAKE, MOVE_PURSUIT, MOVE_CRUNCH},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 28, 0, 220, 8},
        .nature = NATURE_CALM,
        .ability = ABILITY_SAND_STREAM
    },
    [FRONTIER_MON_TYRANITAR_7] = {
        .species = SPECIES_TYRANITAR,
        .moves = {MOVE_ROCK_SLIDE, MOVE_EARTHQUAKE, MOVE_DRAGON_DANCE, MOVE_TAUNT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_ATTACK | F_EV_SPREAD_HP,
        .evs = {252, 252, 0, 0, 4, 0},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_SAND_STREAM
    },
    [FRONTIER_MON_TYRANITAR_8] = {
        .species = SPECIES_TYRANITAR,
        .moves = {MOVE_ROCK_SLIDE, MOVE_EARTHQUAKE, MOVE_DRAGON_DANCE, MOVE_CRUNCH},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {4, 252, 0, 0, 0, 252},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_SAND_STREAM
    },
    [FRONTIER_MON_TYRANITAR_9] = {
        .species = SPECIES_TYRANITAR,
        .moves = {MOVE_ROCK_SLIDE, MOVE_EARTHQUAKE, MOVE_DRAGON_DANCE, MOVE_FOCUS_PUNCH},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {4, 252, 0, 0, 0, 252},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_SAND_STREAM
    },
    [FRONTIER_MON_TYRANITAR_10] = {
        .species = SPECIES_TYRANITAR,
        .moves = {MOVE_ROCK_SLIDE, MOVE_EARTHQUAKE, MOVE_DRAGON_DANCE, MOVE_PURSUIT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_ATTACK,
        .evs = {4, 252, 0, 0, 0, 252},
        .nature = NATURE_ADAMANT,
        .ability = ABILITY_SAND_STREAM
    },
    [FRONTIER_MON_ARTICUNO_5] = {
        .species = SPECIES_ARTICUNO,
        .moves = {MOVE_ICE_BEAM, MOVE_REST, MOVE_HEAL_BELL, MOVE_ROAR},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 0, 4, 252, 0},
        .nature = NATURE_CALM,
        .ability = ABILITY_PRESSURE
    },
    [FRONTIER_MON_ARTICUNO_6] = {
        .species = SPECIES_ARTICUNO,
        .moves = {MOVE_ICE_BEAM, MOVE_REST, MOVE_HEAL_BELL, MOVE_TOXIC},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_DEFENSE | F_EV_SPREAD_HP,
        .evs = {252, 0, 0, 4, 252, 0},
        .nature = NATURE_CALM,
        .ability = ABILITY_PRESSURE
    },
    [FRONTIER_MON_ZAPDOS_5] = {
        .species = SPECIES_ZAPDOS,
        .moves = {MOVE_THUNDERBOLT, MOVE_BATON_PASS, MOVE_HIDDEN_POWER, MOVE_THUNDER_WAVE},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {4, 0, 0, 252, 0, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_PRESSURE,
        .hiddenPowerType = TYPE_GRASS
    },
    [FRONTIER_MON_ZAPDOS_6] = {
        .species = SPECIES_ZAPDOS,
        .moves = {MOVE_THUNDERBOLT, MOVE_HIDDEN_POWER, MOVE_BATON_PASS, MOVE_ROAR},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {4, 0, 0, 252, 0, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_PRESSURE,
        .hiddenPowerType = TYPE_GRASS
    },
    [FRONTIER_MON_MOLTRES_5] = {
        .species = SPECIES_MOLTRES,
        .moves = {MOVE_FLAMETHROWER, MOVE_WILL_O_WISP, MOVE_HIDDEN_POWER, MOVE_OVERHEAT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {0, 0, 0, 252, 4, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_PRESSURE,
        .hiddenPowerType = TYPE_GRASS
    },
    [FRONTIER_MON_MOLTRES_6] = {
        .species = SPECIES_MOLTRES,
        .moves = {MOVE_EXTRASENSORY, MOVE_HIDDEN_POWER, MOVE_WILL_O_WISP, MOVE_FLAMETHROWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_SALAC_BERRY,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {0, 0, 0, 252, 4, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_PRESSURE,
        .hiddenPowerType = TYPE_ICE
    },
    [FRONTIER_MON_RAIKOU_5] = {
        .species = SPECIES_RAIKOU,
        .moves = {MOVE_THUNDERBOLT, MOVE_CALM_MIND, MOVE_SUBSTITUTE, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_PETAYA_BERRY,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {4, 0, 0, 252, 0, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_PRESSURE,
        .hiddenPowerType = TYPE_ICE
    },
    [FRONTIER_MON_RAIKOU_6] = {
        .species = SPECIES_RAIKOU,
        .moves = {MOVE_THUNDERBOLT, MOVE_CALM_MIND, MOVE_HIDDEN_POWER, MOVE_CRUNCH},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {4, 0, 0, 252, 0, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_PRESSURE,
        .hiddenPowerType = TYPE_GRASS
    },
    [FRONTIER_MON_ENTEI_5] = {
        .species = SPECIES_ENTEI,
        .moves = {MOVE_FLAMETHROWER, MOVE_CALM_MIND, MOVE_SUBSTITUTE, MOVE_HIDDEN_POWER},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_HP,
        .evs = {252, 0, 0, 4, 0, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_PRESSURE,
        .hiddenPowerType = TYPE_FIGHTING
    },
    [FRONTIER_MON_ENTEI_6] = {
        .species = SPECIES_ENTEI,
        .moves = {MOVE_FLAMETHROWER, MOVE_CALM_MIND, MOVE_SUBSTITUTE, MOVE_PROTECT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SPEED | F_EV_SPREAD_HP,
        .evs = {252, 0, 0, 4, 0, 252},
        .nature = NATURE_TIMID,
        .ability = ABILITY_PRESSURE
    },
    [FRONTIER_MON_SUICUNE_5] = {
        .species = SPECIES_SUICUNE,
        .moves = {MOVE_HYDRO_PUMP, MOVE_ICE_BEAM, MOVE_CALM_MIND, MOVE_PROTECT},
        .itemTableId = BATTLE_FRONTIER_ITEM_LUM_BERRY,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED | F_EV_SPREAD_HP,
        .evs = {104, 0, 0, 252, 0, 152},
        .nature = NATURE_MODEST,
        .ability = ABILITY_PRESSURE
    },
    [FRONTIER_MON_SUICUNE_6] = {
        .species = SPECIES_SUICUNE,
        .moves = {MOVE_CALM_MIND, MOVE_SURF, MOVE_ICE_BEAM, MOVE_REST},
        .itemTableId = BATTLE_FRONTIER_ITEM_LEFTOVERS,
        .evSpread = F_EV_SPREAD_SP_ATTACK | F_EV_SPREAD_SPEED,
        .evs = {28, 0, 0, 252, 0, 228},
        .nature = NATURE_MODEST,
        .ability = ABILITY_PRESSURE
    }
};
