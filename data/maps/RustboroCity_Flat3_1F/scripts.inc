RustboroCity_Flat3_1F_MapScripts::
	.byte 0

RustboroCity_Flat3_1F_EventScript_Scientist::
	lock
	faceplayer
	msgbox RustboroCity_Flat3_1F_Text_ScientistDexTracker, MSGBOX_DEFAULT
	release
	end

RustboroCity_Flat3_1F_Text_ScientistDexTracker:
	.string "Do you use the DEX TRACKER?\p"
	.string "Apparently, the more you encounter\n"
	.string "the same POKéMON, the better\l"
	.string "the search function will be!\p"
	.string "Its accurate search capability\n"
	.string "allows you to find rare POKéMON.\p"
	.string "They say you can find POKéMON\n"
	.string "with strange moves, or unusual colors.$"
