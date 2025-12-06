/^static const u16 s[A-Z][a-zA-Z]*LevelUpLearnset/ {
    name = $4
    gsub(/LevelUpLearnset.*/, "", name)
    gsub(/^s/, "", name)
    count = 0
    moves = ""
}

/LEVEL_UP_MOVE\( 0,/ {
    count++
    # Extract move name
    match($0, /MOVE_[A-Z_]+/)
    move = substr($0, RSTART, RLENGTH)
    if (moves == "") {
        moves = move
    } else {
        moves = moves ", " move
    }
}

/LEVEL_UP_END/ {
    if (count > 1) print name ": " moves
}
