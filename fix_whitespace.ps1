# Fix trailing whitespace in all modified files

# Fix README.md
$content = Get-Content README.md
$content = $content | ForEach-Object { $_.TrimEnd() }
$content | Set-Content README.md

# Fix add_mint_masters.ps1
$content = Get-Content add_mint_masters.ps1
$content = $content | ForEach-Object { $_.TrimEnd() }
$content | Set-Content add_mint_masters.ps1

# Fix src/field_specials.c
$content = Get-Content src/field_specials.c
$content = $content | ForEach-Object { $_.TrimEnd() }
$content | Set-Content src/field_specials.c

# Fix all script files - remove extra blank lines at EOF
$scriptFiles = @(
    'data/maps/BattleFrontier_PokemonCenter_2F/scripts.inc',
    'data/maps/DewfordTown_PokemonCenter_2F/scripts.inc',
    'data/maps/EverGrandeCity_PokemonCenter_2F/scripts.inc',
    'data/maps/FallarborTown_PokemonCenter_2F/scripts.inc',
    'data/maps/FortreeCity_PokemonCenter_2F/scripts.inc',
    'data/maps/LavaridgeTown_PokemonCenter_2F/scripts.inc',
    'data/maps/MauvilleCity_PokemonCenter_2F/scripts.inc',
    'data/maps/MossdeepCity_PokemonCenter_2F/scripts.inc',
    'data/maps/OldaleTown_PokemonCenter_2F/scripts.inc',
    'data/maps/PacifidlogTown_PokemonCenter_2F/scripts.inc',
    'data/maps/RustboroCity_PokemonCenter_2F/scripts.inc',
    'data/maps/SlateportCity_PokemonCenter_2F/scripts.inc',
    'data/maps/SootopolisCity_PokemonCenter_2F/scripts.inc',
    'data/maps/VerdanturfTown_PokemonCenter_2F/scripts.inc',
    'data/maps/LavaridgeTown_PokemonCenter_2F/scripts.inc',
    'data/maps/LilycoveCity_PokemonCenter_2F/scripts.inc'
)

foreach ($file in $scriptFiles) {
    $content = Get-Content $file
    # Remove trailing whitespace from each line
    $content = $content | ForEach-Object { $_.TrimEnd() }
    # Remove trailing empty lines while keeping exactly one newline at EOF
    while ($content[-1] -eq '') {
        $content = $content[0..($content.Length-2)]
    }
    $content | Set-Content $file
}

Write-Host "Whitespace fixed!"
