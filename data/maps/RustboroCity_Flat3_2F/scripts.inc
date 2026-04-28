RustboroCity_Flat3_2F_MapScripts::
	.byte 0

RustboroCity_Flat3_2F_EventScript_Woman::
	lock
	faceplayer
	msgbox RustboroCity_Flat3_2F_Text_WomanPlaceholder, MSGBOX_DEFAULT
	release
	end

RustboroCity_Flat3_2F_EventScript_Girl::
	lock
	faceplayer
	msgbox RustboroCity_Flat3_2F_Text_GirlPlaceholder, MSGBOX_DEFAULT
	release
	end

RustboroCity_Flat3_2F_EventScript_SchoolKid::
	lock
	faceplayer
	msgbox RustboroCity_Flat3_2F_Text_SchoolKidPlaceholder, MSGBOX_DEFAULT
	release
	end

RustboroCity_Flat3_2F_Text_WomanPlaceholder:
	.string "Time really flies when I'm doing\n"
	.string "housework.\p"
	.string "Perhaps I should hire a POKéMON\n"
	.string "housekeeper.$"


RustboroCity_Flat3_2F_Text_GirlPlaceholder:
	.string "No, no way! I don't want to share\n"
	.string "a room with my little brother!\p"
	.string "I'm a grown-up woman!$"

RustboroCity_Flat3_2F_Text_SchoolKidPlaceholder:
	.string "My parents got a house while\n"
	.string "while raising two of us.\p"
	.string "I have the highest respect for\n"
	.string "my parents.$"
