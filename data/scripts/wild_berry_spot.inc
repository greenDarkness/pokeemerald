WildBerrySpotScript::
	special WildBerrySpotInteraction
	goto_if_eq VAR_0x8004, 0, WildBerrySpot_Flowering
	lock
	faceplayer
	bufferitemname STR_VAR_1, VAR_0x8005
	msgbox WildBerrySpot_Text_WantToPick, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, WildBerrySpot_Pick
	release
	end

WildBerrySpot_Pick::
	special WildBerrySpotPick
	goto_if_eq VAR_0x8004, 0, WildBerrySpot_BagFull
	message WildBerrySpot_Text_Picked
	playfanfare MUS_OBTAIN_BERRY
	waitmessage
	waitfanfare
	release
	end

WildBerrySpot_BagFull::
	message WildBerrySpot_Text_BagFull
	waitmessage
	waitbuttonpress
	release
	end

WildBerrySpot_Flowering::
	lock
	faceplayer
	bufferitemname STR_VAR_1, VAR_0x8005
	message WildBerrySpot_Text_Flowering
	waitmessage
	waitbuttonpress
	release
	end

WildBerrySpot_Text_WantToPick:
	.string "Wild {STR_VAR_1} berries have grown!\p"
	.string "Want to pick them?$"

WildBerrySpot_Text_Picked:
	.string "You picked the wild {STR_VAR_1}!$"

WildBerrySpot_Text_BagFull:
	.string "Your BAG is full.$"

WildBerrySpot_Text_Flowering:
	.string "Wild {STR_VAR_1} berries are\n"
	.string "flowering here.$"
