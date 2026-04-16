Route3_SouthEntrance_2F_MapScripts::
	.byte 0

Route3_SouthEntrance_2F_EventScript_LeftBinoculars::
	msgbox Route3_SouthEntrance_2F_Text_MeteorHitMtMoon, MSGBOX_SIGN
	end

Route3_SouthEntrance_2F_EventScript_RightBinoculars::
	msgbox Route3_SouthEntrance_2F_Text_DancingClefairy, MSGBOX_SIGN
	end

Route3_SouthEntrance_2F_Text_MeteorHitMtMoon:
    .string "Look! A meteor just smashed into\n"
    .string "MT MOON!$"

Route3_SouthEntrance_2F_Text_DancingClefairy:
    .string "A trio of CLEFAIRY can be seen\n"
    .string "dancing in the distance.$"