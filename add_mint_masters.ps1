$centers = @(
    "BattleFrontier_PokemonCenter_2F",
    "DewfordTown_PokemonCenter_2F",
    "EverGrandeCity_PokemonCenter_2F",
    "FallarborTown_PokemonCenter_2F",
    "FortreeCity_PokemonCenter_2F",
    "LavaridgeTown_PokemonCenter_2F",
    "LilycoveCity_PokemonCenter_2F",
    "MauvilleCity_PokemonCenter_2F",
    "MossdeepCity_PokemonCenter_2F",
    "OldaleTown_PokemonCenter_2F",
    "PacifidlogTown_PokemonCenter_2F",
    "RustboroCity_PokemonCenter_2F",
    "SlateportCity_PokemonCenter_2F",
    "SootopolisCity_PokemonCenter_2F",
    "VerdanturfTown_PokemonCenter_2F"
)

foreach ($center in $centers) {
    $mapPath = "data\maps\$center\map.json"
    $scriptPath = "data\maps\$center\scripts.inc"

    # Add script
    $scriptContent = Get-Content $scriptPath -Raw
    $newScript = "`n${center}_EventScript_MintMaster::`n`tcall Common_EventScript_MintMaster`n`tend`n"
    Set-Content $scriptPath ($scriptContent + $newScript)

    # Add object to map.json
    $json = Get-Content $mapPath -Raw | ConvertFrom-Json
    $mintMaster = @{
        graphics_id = "OBJ_EVENT_GFX_COOK"
        x = 0
        y = 8
        elevation = 3
        movement_type = "MOVEMENT_TYPE_FACE_RIGHT"
        movement_range_x = 0
        movement_range_y = 0
        trainer_type = "TRAINER_TYPE_NONE"
        trainer_sight_or_berry_tree_id = "0"
        script = "${center}_EventScript_MintMaster"
        flag = "0"
    }
    $json.object_events += $mintMaster
    $json | ConvertTo-Json -Depth 10 | Set-Content $mapPath

    Write-Host "Added Mint Master to $center"
}

Write-Host "`nAll Mint Masters added successfully!"
