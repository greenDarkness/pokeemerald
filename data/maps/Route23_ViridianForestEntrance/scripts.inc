Route23_ViridianForestEntrance_MapScripts::
	.byte 0

Route23_ViridianForestEntrance_EventScript_Guard::
	msgbox Route23_ViridianForest_Text_SleepingGuard, MSGBOX_NPC
	end

Route23_ViridianForest_Text_SleepingGuard:
    .string "Zzzz...\n"
	.string "The guard is asleep.\l"
	.string "Let's try not to wake him...$"