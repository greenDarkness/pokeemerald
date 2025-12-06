/^static const u16 s[A-Z][a-zA-Z]*LevelUpLearnset/ {
    name = $4
    gsub(/LevelUpLearnset.*/, "", name)
    gsub(/^s/, "", name)
    has_evo = 0
}

/LEVEL_UP_MOVE\( 0,/ {
    has_evo = 1
}

/LEVEL_UP_END/ {
    if (!has_evo) print name
}
