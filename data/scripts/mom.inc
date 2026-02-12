Mom_EventScript_UpdateLocation::
	goto_if_unset FLAG_HIDE_PETALBURG_CITY_WALLY, Mom_EventScript_UpdateLocationEnd
    call Mom_EventScript_SetAllFlags
	goto_if_eq VAR_BIRCH_STATE, 0, Mom_EventScript_MoveToHome
	goto_if_eq VAR_BIRCH_STATE, 1, Mom_EventScript_MoveToTown
	goto_if_eq VAR_BIRCH_STATE, 2, Mom_EventScript_MoveToHome
	goto_if_eq VAR_BIRCH_STATE, 3, Mom_EventScript_MoveToCity
	goto_if_eq VAR_BIRCH_STATE, 4, Mom_EventScript_MoveToHome
	goto_if_eq VAR_BIRCH_STATE, 5, Mom_EventScript_MoveToVacation
	goto_if_eq VAR_BIRCH_STATE, 6, Mom_EventScript_MoveToHome
    return
Mom_EventScript_UpdateLocationEnd::
	return

Mom_EventScript_SetAllFlags::
    setflag FLAG_HIDE_LITTLEROOT_TOWN_BRENDANS_HOUSE_MOM
    setflag FLAG_HIDE_LITTLEROOT_TOWN_MAYS_HOUSE_MOM
    setflag FLAG_HIDE_MOM_LITTLEROOT_LAKE
    setflag FLAG_HIDE_MOM_PRECIPICE_SPRING
    setflag FLAG_HIDE_MOM_OLDALE
    setflag FLAG_HIDE_MOM_PETALBURG
    setflag FLAG_HIDE_MOM_RUSTBORO
    setflag FLAG_HIDE_MOM_SLATEPORT
    setflag FLAG_HIDE_MOM_MAUVILLE
    setflag FLAG_HIDE_MOM_LILYCOVE
    setflag FLAG_HIDE_MOM_DEWFORD
    setflag FLAG_HIDE_MOM_LAVARIDGE
    setflag FLAG_HIDE_MOM_MT_PYRE
    setflag FLAG_HIDE_MOM_SOOTOPOLIS
    return

Mom_EventScript_MoveToHome::
	goto_if_eq VAR_BIRCH_STATE_RANDOM, 0, Mom_EventScript_ShowMom_LittlerootHome
	goto_if_eq VAR_BIRCH_STATE_RANDOM, 1, Mom_EventScript_ShowMom_LittlerootLake
    goto_if_eq VAR_BIRCH_STATE_RANDOM, 2, Mom_EventScript_ShowMom_LittlerootHome
	goto_if_eq VAR_BIRCH_STATE_RANDOM, 3, Mom_EventScript_ShowMom_PrecipiceSpring
    goto_if_eq VAR_BIRCH_STATE_RANDOM, 4, Mom_EventScript_ShowMom_LittlerootHome
	goto_if_eq VAR_BIRCH_STATE_RANDOM, 5, Mom_EventScript_ShowMom_LittlerootLake
    goto_if_eq VAR_BIRCH_STATE_RANDOM, 6, Mom_EventScript_ShowMom_LittlerootHome
	return
Mom_EventScript_MoveToTown::
	goto_if_eq VAR_BIRCH_STATE_RANDOM, 0, Mom_EventScript_ShowMom_Petalburg
	goto_if_eq VAR_BIRCH_STATE_RANDOM, 1, Mom_EventScript_ShowMom_Oldale
    goto_if_eq VAR_BIRCH_STATE_RANDOM, 2, Mom_EventScript_ShowMom_Petalburg
	goto_if_eq VAR_BIRCH_STATE_RANDOM, 3, Mom_EventScript_ShowMom_Oldale
    goto_if_eq VAR_BIRCH_STATE_RANDOM, 4, Mom_EventScript_ShowMom_Petalburg
	goto_if_eq VAR_BIRCH_STATE_RANDOM, 5, Mom_EventScript_ShowMom_Oldale
    goto_if_eq VAR_BIRCH_STATE_RANDOM, 6, Mom_EventScript_ShowMom_Petalburg
	return
Mom_EventScript_MoveToCity::
	goto_if_eq VAR_BIRCH_STATE_RANDOM, 0, Mom_EventScript_ShowMom_Rustboro
	goto_if_eq VAR_BIRCH_STATE_RANDOM, 1, Mom_EventScript_ShowMom_Rustboro
    goto_if_eq VAR_BIRCH_STATE_RANDOM, 2, Mom_EventScript_ShowMom_Mauville
	goto_if_eq VAR_BIRCH_STATE_RANDOM, 3, Mom_EventScript_ShowMom_Slateport
    goto_if_eq VAR_BIRCH_STATE_RANDOM, 4, Mom_EventScript_ShowMom_Mauville
	goto_if_eq VAR_BIRCH_STATE_RANDOM, 5, Mom_EventScript_ShowMom_Lilycove
    goto_if_eq VAR_BIRCH_STATE_RANDOM, 6, Mom_EventScript_ShowMom_Lilycove
	return
Mom_EventScript_MoveToVacation::
	goto_if_eq VAR_BIRCH_STATE_RANDOM, 0, Mom_EventScript_ShowMom_Dewford
	goto_if_eq VAR_BIRCH_STATE_RANDOM, 1, Mom_EventScript_ShowMom_Dewford
	goto_if_eq VAR_BIRCH_STATE_RANDOM, 2, Mom_EventScript_ShowMom_Lavaridge
	goto_if_eq VAR_BIRCH_STATE_RANDOM, 3, Mom_EventScript_ShowMom_MtPyre
    goto_if_eq VAR_BIRCH_STATE_RANDOM, 4, Mom_EventScript_ShowMom_Lavaridge
	goto_if_eq VAR_BIRCH_STATE_RANDOM, 5, Mom_EventScript_ShowMom_Sootopolis
	goto_if_eq VAR_BIRCH_STATE_RANDOM, 6, Mom_EventScript_ShowMom_Sootopolis
	return

Mom_EventScript_ShowMom_LittlerootHome::
    checkplayergender
    goto_if_eq VAR_RESULT, MALE, Mom_EventScript_ShowMom_LittlerootHomeBrendan
    goto_if_eq VAR_RESULT, FEMALE, Mom_EventScript_ShowMom_LittlerootHomeMay
    return
Mom_EventScript_ShowMom_LittlerootHomeBrendan::
    clearflag FLAG_HIDE_LITTLEROOT_TOWN_BRENDANS_HOUSE_MOM
    return
Mom_EventScript_ShowMom_LittlerootHomeMay::
    clearflag FLAG_HIDE_LITTLEROOT_TOWN_MAYS_HOUSE_MOM
    return
Mom_EventScript_ShowMom_LittlerootLake::
    clearflag FLAG_HIDE_MOM_LITTLEROOT_LAKE
    return
Mom_EventScript_ShowMom_PrecipiceSpring::
    clearflag FLAG_HIDE_MOM_PRECIPICE_SPRING
    return
Mom_EventScript_ShowMom_Oldale::
    clearflag FLAG_HIDE_MOM_OLDALE
    return
Mom_EventScript_ShowMom_Petalburg::
    clearflag FLAG_HIDE_MOM_PETALBURG
    return
Mom_EventScript_ShowMom_Rustboro::
    clearflag FLAG_HIDE_MOM_RUSTBORO
    return
Mom_EventScript_ShowMom_Slateport::
    clearflag FLAG_HIDE_MOM_SLATEPORT
    return
Mom_EventScript_ShowMom_Mauville::
    clearflag FLAG_HIDE_MOM_MAUVILLE
    return
Mom_EventScript_ShowMom_Lilycove::
    clearflag FLAG_HIDE_MOM_LILYCOVE
    return
Mom_EventScript_ShowMom_Dewford::
    clearflag FLAG_HIDE_MOM_DEWFORD
    return
Mom_EventScript_ShowMom_Lavaridge::
    clearflag FLAG_HIDE_MOM_LAVARIDGE
    return
Mom_EventScript_ShowMom_MtPyre::
    clearflag FLAG_HIDE_MOM_MT_PYRE
    return
Mom_EventScript_ShowMom_Sootopolis::
    clearflag FLAG_HIDE_MOM_SOOTOPOLIS
    return

@above: location update scripts
@below: object event scripts

Mom_EventScript_Littleroot_Lake::
    lock
    faceplayer
    goto_if_set FLAG_MOM_LITTLEROOT_LAKE_ITEM_GIVEN, Mom_EventScript_Littleroot_Lake_ItemGiven
    msgbox Mom_Text_Littleroot_Lake, MSGBOX_DEFAULT
    closemessage
    special GiveMomBerry
    msgbox Mom_Text_ReceiveItemFromMom, MSGBOX_DEFAULT
    closemessage
    call Mom_EventScript_ClearEventItemFlags
    setflag FLAG_MOM_LITTLEROOT_LAKE_ITEM_GIVEN
    release
    end
Mom_EventScript_Littleroot_Lake_ItemGiven::
    msgbox Mom_Text_Littleroot_Lake_Item_Given
    closemessage
    release
    end

Mom_EventScript_Precipice_Spring::
    lock
    faceplayer
    goto_if_set FLAG_MOM_PRECIPICE_SPRING_ITEM_GIVEN, Mom_EventScript_Precipice_SpringItemGiven
    msgbox Mom_Text_Precipice_Spring, MSGBOX_DEFAULT
    closemessage
    special GiveGroundMomItem
    msgbox Mom_Text_ReceiveItemFromMom, MSGBOX_DEFAULT
    closemessage
    call Mom_EventScript_ClearEventItemFlags
    setflag FLAG_MOM_PRECIPICE_SPRING_ITEM_GIVEN
    release
    end
Mom_EventScript_Precipice_SpringItemGiven::
    msgbox Mom_Text_Precipice_Spring_Item_Given, MSGBOX_DEFAULT
    closemessage
    release
    end

Mom_EventScript_Oldale::
    lock
    faceplayer
    goto_if_set FLAG_MOM_OLDALE_ITEM_GIVEN, Mom_EventScript_Oldale_ItemGiven
    msgbox Mom_Text_Oldale, MSGBOX_DEFAULT
    closemessage
    special GiveGroundMomItem
    msgbox Mom_Text_ReceiveItemFromMom, MSGBOX_DEFAULT
    closemessage
    call Mom_EventScript_ClearEventItemFlags
    setflag FLAG_MOM_OLDALE_ITEM_GIVEN
    release
    end
Mom_EventScript_Oldale_ItemGiven::
    msgbox Mom_Text_OldaleItemGiven, MSGBOX_DEFAULT
    closemessage
    release
    end

Mom_EventScript_Petalburg::
    lock
    faceplayer
    goto_if_set FLAG_MOM_PETALBURG_ITEM_GIVEN, Mom_EventScript_Petalburg_ItemGiven
    msgbox Mom_Text_Petalburg, MSGBOX_DEFAULT
    closemessage
    special GiveGroundMomItem
    msgbox Mom_Text_ReceiveItemFromMom, MSGBOX_DEFAULT
    closemessage
    call Mom_EventScript_ClearEventItemFlags
    setflag FLAG_MOM_PETALBURG_ITEM_GIVEN
    release
    end
Mom_EventScript_Petalburg_ItemGiven::
    msgbox Mom_Text_PetalburgItemGiven, MSGBOX_DEFAULT
    closemessage
    release
    end

Mom_EventScript_Rustboro::
    lock
    faceplayer
    goto_if_set FLAG_MOM_RUSTBORO_ITEM_GIVEN, Mom_EventScript_Rustboro_ItemGiven
    msgbox Mom_Text_Rustboro, MSGBOX_DEFAULT
    closemessage
    special GiveBoughtMomItem
    msgbox Mom_Text_ReceiveItemFromMom, MSGBOX_DEFAULT
    closemessage
    call Mom_EventScript_ClearEventItemFlags
    setflag FLAG_MOM_RUSTBORO_ITEM_GIVEN
    release
    end
Mom_EventScript_Rustboro_ItemGiven::
    msgbox Mom_Text_RustboroItemGiven, MSGBOX_DEFAULT
    closemessage
    release
    end

Mom_EventScript_Slateport::
    lock
    faceplayer
    goto_if_set FLAG_MOM_SLATEPORT_ITEM_GIVEN, Mom_EventScript_Slateport_ItemGiven
    msgbox Mom_Text_Slateport, MSGBOX_DEFAULT
    closemessage
    special GiveBoughtMomItem
    msgbox Mom_Text_ReceiveItemFromMom, MSGBOX_DEFAULT
    closemessage
    call Mom_EventScript_ClearEventItemFlags
    setflag FLAG_MOM_SLATEPORT_ITEM_GIVEN
    release
    end
Mom_EventScript_Slateport_ItemGiven::
    msgbox Mom_Text_SlateportItemGiven, MSGBOX_DEFAULT
    closemessage
    release
    end

Mom_EventScript_Mauville::
    lock
    faceplayer
    goto_if_set FLAG_MOM_MAUVILLE_ITEM_GIVEN, Mom_EventScript_Mauville_ItemGiven
    msgbox Mom_Text_Mauville, MSGBOX_DEFAULT
    closemessage
    special GiveBoughtMomItem
    msgbox Mom_Text_ReceiveItemFromMom, MSGBOX_DEFAULT
    closemessage
    call Mom_EventScript_ClearEventItemFlags
    setflag FLAG_MOM_MAUVILLE_ITEM_GIVEN
    release
    end
Mom_EventScript_Mauville_ItemGiven::
    msgbox Mom_Text_MauvilleItemGiven, MSGBOX_DEFAULT
    closemessage
    release
    end

Mom_EventScript_Lilycove::
    lock
    faceplayer
    goto_if_set FLAG_MOM_LILYCOVE_ITEM_GIVEN, Mom_EventScript_Lilycove_ItemGiven
    msgbox Mom_Text_Lilycove, MSGBOX_DEFAULT
    closemessage
    special GiveBoughtMomItem
    msgbox Mom_Text_ReceiveItemFromMom, MSGBOX_DEFAULT
    closemessage
    call Mom_EventScript_ClearEventItemFlags
    setflag FLAG_MOM_LILYCOVE_ITEM_GIVEN
    release
    end
Mom_EventScript_Lilycove_ItemGiven::
    msgbox Mom_Text_LilycoveItemGiven, MSGBOX_DEFAULT
    closemessage
    release
    end

Mom_EventScript_Dewford::
    lock
    faceplayer
    goto_if_set FLAG_MOM_DEWFORD_ITEM_GIVEN, Mom_EventScript_Dewford_ItemGiven
    msgbox Mom_Text_Dewford, MSGBOX_DEFAULT
    closemessage
    special GiveGroundMomItem
    msgbox Mom_Text_ReceiveItemFromMom, MSGBOX_DEFAULT
    closemessage
    call Mom_EventScript_ClearEventItemFlags
    setflag FLAG_MOM_DEWFORD_ITEM_GIVEN
    release
    end
Mom_EventScript_Dewford_ItemGiven::
    msgbox Mom_Text_DewfordItemGiven, MSGBOX_DEFAULT
    closemessage
    release
    end

Mom_EventScript_Lavaridge::
    lock
    faceplayer
    goto_if_set FLAG_MOM_LAVARIDGE_ITEM_GIVEN, Mom_EventScript_Lavaridge_ItemGiven
    msgbox Mom_Text_Lavaridge, MSGBOX_DEFAULT
    closemessage
    special GiveGroundMomItem
    msgbox Mom_Text_ReceiveItemFromMom, MSGBOX_DEFAULT
    closemessage
    call Mom_EventScript_ClearEventItemFlags
    setflag FLAG_MOM_LAVARIDGE_ITEM_GIVEN
    release
    end
Mom_EventScript_Lavaridge_ItemGiven::
    msgbox Mom_Text_LavaridgeItemGiven, MSGBOX_DEFAULT
    closemessage
    release
    end

Mom_EventScript_Mt_Pyre::
    lock
    faceplayer
    goto_if_set FLAG_MOM_MTPYRE_ITEM_GIVEN, Mom_EventScript_Mt_Pyre_ItemGiven
    msgbox Mom_Text_MtPyre, MSGBOX_DEFAULT
    closemessage
    special GiveMomBerry
    msgbox Mom_Text_ReceiveItemFromMom, MSGBOX_DEFAULT
    closemessage
    call Mom_EventScript_ClearEventItemFlags
    setflag FLAG_MOM_MTPYRE_ITEM_GIVEN
    release
    end
Mom_EventScript_Mt_Pyre_ItemGiven::
    msgbox Mom_Text_MtPyreItemGiven, MSGBOX_DEFAULT
    closemessage
    release
    end

Mom_EventScript_Sootopolis::
    lock
    faceplayer
    goto_if_set FLAG_MOM_SOOTOPOLIS_ITEM_GIVEN, Mom_EventScript_Sootopolis_ItemGiven
    msgbox Mom_Text_Sootopolis, MSGBOX_DEFAULT
    closemessage
    special GiveBoughtMomItem
    msgbox Mom_Text_ReceiveItemFromMom, MSGBOX_DEFAULT
    closemessage
    call Mom_EventScript_ClearEventItemFlags
    setflag FLAG_MOM_SOOTOPOLIS_ITEM_GIVEN
    release
    end
Mom_EventScript_Sootopolis_ItemGiven::
    msgbox Mom_Text_SootopolisItemGiven, MSGBOX_DEFAULT
    closemessage
    release
    end

Mom_EventScript_ClearEventItemFlags::
    clearflag FLAG_MOM_LITTLEROOT_LAKE_ITEM_GIVEN
    clearflag FLAG_MOM_PRECIPICE_SPRING_ITEM_GIVEN
    clearflag FLAG_MOM_OLDALE_ITEM_GIVEN
    clearflag FLAG_MOM_PETALBURG_ITEM_GIVEN
    clearflag FLAG_MOM_RUSTBORO_ITEM_GIVEN
    clearflag FLAG_MOM_SLATEPORT_ITEM_GIVEN
    clearflag FLAG_MOM_MAUVILLE_ITEM_GIVEN
    clearflag FLAG_MOM_LILYCOVE_ITEM_GIVEN
    clearflag FLAG_MOM_DEWFORD_ITEM_GIVEN
    clearflag FLAG_MOM_LAVARIDGE_ITEM_GIVEN
    clearflag FLAG_MOM_MTPYRE_ITEM_GIVEN
    clearflag FLAG_MOM_SOOTOPOLIS_ITEM_GIVEN
    return

Mom_Text_ReceiveItemFromMom:
    .string "You got {STR_VAR_3}\n"
    .string "from MOM!$"
Mom_Text_Littleroot_Lake:
    .string "Isn't this lake interesting, {PLAYER}?\p"
    .string "The water from the spring above created\n"
    .string "a sinkhole, which revealed an\l"
    .string "underground cavern.\p"
    .string "The berry trees grow well here, too.\n"
    .string "That attracts sea POKEMON from\l"
    .string "the caverns.\p"
    .string "It also attracts me to the lake!\n"
    .string "-giggles-\p"  
    .string "Would you like some berries, {PLAYER}?\n"
    .string "Here, try some that I picked.$"
Mom_Text_Littleroot_Lake_Item_Given:
    .string "Hey {PLAYER}, want to know what I\n"
    .string "was just thinking about?\p"
    .string "If I were a sea POKEMON, I would\n"
    .string "still get to live in LITTEROOT.\p"
    .string "We could be a family of KRABBY!\n"
    .string "COOKIE-COOKIE!$"
Mom_Text_Precipice_Spring:
    .string "Here, I saved this for you, sweetie.\n"
    .string "I saw it on the path during my walk.$"
Mom_Text_Precipice_Spring_Item_Given:
    .string "{PLAYER}, want to know something\n"
    .string "interesting about this place?\p"
    .string "Have you ever wondered why it's\n"
    .string "called PRECIPICE SPRING?\p"
    .string "It's because it was created by\n"
    .string "GROUDON's PRECIPICE BLADE!\p"
    .string "...What do you mean it took\n"
    .string "me long enough?!$"
Mom_Text_Oldale:
    .string "Be careful walking through here,\n"
    .string "{PLAYER}.\p"
    .string "I heard OLDALE has had a spike\n"
    .string "in gang activity lately.\p"
    .string "The wild DOG POKEMON have become\n"
    .string "dangerous here, too..\p"
    .string "Remember to ACT CASUAL and no one\n"
    .string "will notice you, okay sweetie?\p"
    .string "Here, take this before you go.\n"
    .string "I saw it on the path during my walk.$"
Mom_Text_OldaleItemGiven:
    .string "Be careful walking through here,\n"
    .string "{PLAYER}.\p"
    .string "I heard OLDALE has had a spike\n"
    .string "in gang activity lately.\p"
    .string "The wild DOG POKEMON have become\n"
    .string "dangerous here, too..\p"
    .string "Remember to ACT CASUAL and no one\n"
    .string "will notice you, okay sweetie?\p"
Mom_Text_Petalburg:
    .string "Hi, {PLAYER}! Are you here to see\n"
    .string "your father today, too?\p"
    .string "I found a couple of these on my walk-\n"
    .string "just enough to give one to each of you.$"
Mom_Text_PetalburgItemGiven:
    .string "Are you here to see NORMAN, too?\n"
    .string "It's like a family reunion!$"
Mom_Text_Rustboro:
    .string "Look who it is! I just picked this\n"
    .string "up for you!\p"
    .string "It was meant to be a surprise, but\n"
    .string "I think it still is? Here you go!$"
Mom_Text_RustboroItemGiven:
    .string "We meet again, my child...\n"
    .string "Bound together by the EVERSTONE\l"
    .string "of paternalism...\p"
    .string "...Was that a good impression of\n"
    .string "MR. STONE? -giggles-$"
Mom_Text_Slateport:
    .string "The market in SLATEPORT is\n"
    .string "so unique.\p"
    .string "I managed to find this and I\n"
    .string "immediately thought of you.$"
Mom_Text_SlateportItemGiven:
    .string "Do you think that guy over there\n"
    .string "knows he has a TANGELA in his seaweed?$"
Mom_Text_Mauville:
    .string "I have a knack for these kinds of games.\n"
    .string "Didn't you know?\p"
    .string "How else do you think we could afford\n"
    .string "that EXP. SHARE you found in your PC?\p"
    .string "The same way we could afford this!$"
Mom_Text_MauvilleItemGiven:
    .string "Lady luck is on my side tonight!$"
Mom_Text_Lilycove:
    .string "TEST-TEST....\n"
    .string "We're in LILYCOVE!$"
Mom_Text_LilycoveItemGiven:
    .string "TEST-TEST....\n"
    .string "We're in LILYCOVE!$"
Mom_Text_Dewford:
    .string "TEST-TEST....\n"
    .string "We're in DEWFORD!$"
Mom_Text_DewfordItemGiven:
    .string "TEST-TEST....\n"
    .string "We're in DEWFORD!$"
Mom_Text_Lavaridge:
    .string "TEST-TEST....\n"
    .string "We're in LAVARIDGE!$"
Mom_Text_LavaridgeItemGiven:
    .string "TEST-TEST....\n"
    .string "We're in LAVARIDGE!$"
Mom_Text_MtPyre:
    .string "TEST-TEST....\n"
    .string "We're in MT. PYRE!$"
Mom_Text_MtPyreItemGiven:
    .string "TEST-TEST....\n"
    .string "We're in MTPYRE!$"
Mom_Text_Sootopolis:
    .string "TEST-TEST....\n"
    .string "We're in SOOTOPOLIS!$"
Mom_Text_SootopolisItemGiven:
    .string "TEST-TEST....\n"
    .string "We're in SOOTOPOLIS!$"
