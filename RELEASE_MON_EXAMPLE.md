# Release Pokemon from Party - Special Functions

## Overview
Three new special functions have been added to allow scripts to release (remove) a Pokemon from the player's party based on species, similar to how in-game trades work.

## Special Functions

### 1. `GetReleaseMonSpeciesInfo`
- **Purpose**: Gets information about the species you want the player to release
- **Input**: `VAR_0x8004` = Species ID
- **Output**: Returns species ID to `VAR_RESULT`, buffers species name to `gStringVar1`
- **Usage**: Use this to tell the player what species you're looking for

### 2. `GetReleaseMonSpecies`
- **Purpose**: Gets the species of the selected party Pokemon
- **Input**: `VAR_0x8005` = Party slot (set by `ChoosePartyMon`)
- **Output**: Returns species ID to `VAR_RESULT`, or `SPECIES_NONE` if egg
- **Usage**: Check if the selected Pokemon matches the species you want

### 3. `ReleaseMonFromParty`
- **Purpose**: Removes the selected Pokemon from the party
- **Input**: `VAR_0x8005` = Party slot to release
- **Output**: None
- **Usage**: Actually removes the Pokemon from the party

### 4. `GetLandEncounter` `GetWaterEncounter` `GetFishingEncounter` `GetRockSmashEncounter`
- **Purpose**: Gets the species corresponding to a land wild encounter on a map
- **Input**:
  - `VAR_0x8004` = map identifier
    - `0xFFFF` = current player map
    - `(MAP_GROUP<<8)|MAP_NUM` = explicit map
  - `VAR_0x8005` = encounter slot (0..11)
    - `0xFFFF` = random slot (equal chance 1/12)
- **Output**:
  - `VAR_RESULT` = species ID (or 0 if none)
  - `gStringVar1` = species name string (for `{STR_VAR_1}` usage)
- **Usage**: Use this to show the wild mon that would appear at a given slot or random slot

### 5. `GetRandomEncounterSlot`
- **Purpose**: Returns a deterministic index from `0..N` based on the player Trainer ID and day count
- **Input**:
  - `VAR_0x8004` = `N` (max slot)
- **Output**:
  - `VAR_RESULT` = slot index in range `0..N` result
- **Usage**: Set `VAR_0x8004`, then call via `specialvar VAR_RESULT, GetRandomEncounterSlot` to drive slot-based behavior in scripts

## Example Script

Here's a complete example script that asks the player to release a specific Pokemon (e.g., SPECIES_ZIGZAGOON):

```c
YourMap_EventScript_ReleaseMonNPC::
	lock
	faceplayer
	@ Check if already completed
	goto_if_set FLAG_YOUR_RELEASE_MON_COMPLETED, YourMap_EventScript_AlreadyReleased
	
	@ Set the species we want (SPECIES_ZIGZAGOON as example)
	setvar VAR_0x8004, SPECIES_ZIGZAGOON
	specialvar VAR_RESULT, GetReleaseMonSpeciesInfo
	@ VAR_RESULT now contains the species, gStringVar1 contains the name
	
	@ Ask the player
	msgbox YourMap_Text_WouldYouReleaseYourMon, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, YourMap_EventScript_DeclineRelease
	
	@ Let player choose a Pokemon
	special ChoosePartyMon
	waitstate
	
	@ Store the selected party slot
	copyvar VAR_0x8005, VAR_0x8004
	
	@ Check if they canceled
	goto_if_eq VAR_0x8004, PARTY_NOTHING_CHOSEN, YourMap_EventScript_DeclineRelease
	
	@ Get the species of the selected mon
	specialvar VAR_RESULT, GetReleaseMonSpecies
	
	@ Check if it's the correct species (comparing to SPECIES_ZIGZAGOON)
	goto_if_ne VAR_RESULT, SPECIES_ZIGZAGOON, YourMap_EventScript_WrongSpecies
	
	@ Release the Pokemon!
	special ReleaseMonFromParty
	
	@ Show completion message
	msgbox YourMap_Text_ThankYouForReleasingMon, MSGBOX_DEFAULT
	setflag FLAG_YOUR_RELEASE_MON_COMPLETED
	@ Give reward here if desired
	release
	end

YourMap_EventScript_DeclineRelease::
	msgbox YourMap_Text_ThatsOkay, MSGBOX_DEFAULT
	release
	end

YourMap_EventScript_WrongSpecies::
	bufferspeciesname STR_VAR_1, SPECIES_ZIGZAGOON
	msgbox YourMap_Text_ThatsNotTheRightSpecies, MSGBOX_DEFAULT
	release
	end

YourMap_EventScript_AlreadyReleased::
	msgbox YourMap_Text_ThanksAgain, MSGBOX_DEFAULT
	release
	end
```

## Example Text Strings

```c
YourMap_Text_WouldYouReleaseYourMon:
	.string "I'm looking for someone who would\n"
	.string "be willing to release their {STR_VAR_1}.\p"
	.string "Would you do that for me?$"

YourMap_Text_ThankYouForReleasingMon:
	.string "Thank you so much!\n"
	.string "Your {STR_VAR_1} is now free!$"

YourMap_Text_ThatsOkay:
	.string "That's okay.\n"
	.string "Maybe another time.$"

YourMap_Text_ThatsNotTheRightSpecies:
	.string "Hmm, that's not a {STR_VAR_1}.\n"
	.string "I need specifically a {STR_VAR_1}.$"

YourMap_Text_ThanksAgain:
	.string "Thanks again for releasing\n"
	.string "that Pokémon earlier!$"
```

## Important Notes

1. **Always check if player has more than one Pokemon**: You may want to add a check using `CountPartyNonEggMons` before allowing the player to release a Pokemon to prevent releasing their last Pokemon.

2. **Variable Usage**:
   - `VAR_0x8004` = Input species ID / Selected party slot from ChoosePartyMon
   - `VAR_0x8005` = Party slot for the Pokemon to release
   - `VAR_RESULT` = Return value from special functions
   - `gStringVar1` = Species name (from GetReleaseMonSpeciesInfo)

3. **Safety**: The `ReleaseMonFromParty` function will automatically:
   - Zero out the Pokemon data
   - Compact the party (move remaining Pokemon up)
   - Recalculate the party count

4. **Species Constants**: Use the appropriate SPECIES_ constant from `include/constants/species.h`

## Testing Your Script

1. Build the ROM: `./build.sh` or `make`
2. Load the ROM in your emulator
3. Trigger your event
4. Verify that:
   - The correct species is displayed
   - Only the correct species can be released
   - The Pokemon is removed from the party
   - Remaining Pokemon shift up properly
