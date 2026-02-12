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
    goto_if_set FLAG_MOM_BERRY_GIVEN, Mom_EventScript_Littleroot_Lake_BerryGiven
    msgbox Mom_Text_Littleroot_Lake, MSGBOX_DEFAULT
    closemessage
    special GiveMomBerry
    msgbox Mom_Text_Littleroot_Lake_ReceiveBerry, MSGBOX_DEFAULT
    closemessage
    setflag FLAG_MOM_BERRY_GIVEN
    release
    end
Mom_EventScript_Littleroot_Lake_BerryGiven::
    lock
    faceplayer
    msgbox Mom_Text_Littleroot_Lake_Berry_Given
    closemessage
    release
    end

Mom_EventScript_Precipice_Spring::
    lock
    faceplayer
    msgbox Mom_Text_Precipice_Spring, MSGBOX_DEFAULT
    closemessage
    clearflag FLAG_MOM_BERRY_GIVEN
    release
    end
Mom_EventScript_Oldale::
    lock
    faceplayer
    msgbox Mom_Text_Oldale, MSGBOX_DEFAULT
    closemessage
    clearflag FLAG_MOM_BERRY_GIVEN
    release
    end
Mom_EventScript_Petalburg::
    lock
    faceplayer
    msgbox Mom_Text_Petalburg, MSGBOX_DEFAULT
    closemessage
    clearflag FLAG_MOM_BERRY_GIVEN
    release
    end
Mom_EventScript_Rustboro::
    lock
    faceplayer
    msgbox Mom_Text_Rustboro, MSGBOX_DEFAULT
    closemessage
    clearflag FLAG_MOM_BERRY_GIVEN
    release
    end
Mom_EventScript_Slateport::
    lock
    faceplayer
    msgbox Mom_Text_Slateport, MSGBOX_DEFAULT
    closemessage
    clearflag FLAG_MOM_BERRY_GIVEN
    release
    end
Mom_EventScript_Mauville::
    lock
    faceplayer
    msgbox Mom_Text_Mauville, MSGBOX_DEFAULT
    closemessage
    clearflag FLAG_MOM_BERRY_GIVEN
    release
    end
Mom_EventScript_Lilycove::
    lock
    faceplayer
    msgbox Mom_Text_Lilycove, MSGBOX_DEFAULT
    closemessage
    clearflag FLAG_MOM_BERRY_GIVEN
    release
    end
Mom_EventScript_Dewford::
    lock
    faceplayer
    msgbox Mom_Text_Dewford, MSGBOX_DEFAULT
    closemessage
    clearflag FLAG_MOM_BERRY_GIVEN
    release
    end
Mom_EventScript_Lavaridge::
    lock
    faceplayer
    msgbox Mom_Text_Lavaridge, MSGBOX_DEFAULT
    closemessage
    clearflag FLAG_MOM_BERRY_GIVEN
    release
    end
Mom_EventScript_Mt_Pyre::
    lock
    faceplayer
    msgbox Mom_Text_MtPyre, MSGBOX_DEFAULT
    closemessage
    clearflag FLAG_MOM_BERRY_GIVEN
    release
    end
Mom_EventScript_Sootopolis::
    lock
    faceplayer
    msgbox Mom_Text_Sootopolis, MSGBOX_DEFAULT
    closemessage
    clearflag FLAG_MOM_BERRY_GIVEN
    release
    end

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
Mom_Text_Littleroot_Lake_ReceiveBerry::
    .string "You got {STR_VAR_3}\n"
    .string "from MOM!$"
Mom_Text_Littleroot_Lake_Berry_Given:
    .string "Hey {PLAYER}, want to know what I\n"
    .string "was just thinking about?\p"
    .string "If I were a sea POKEMON, I would\n"
    .string "still get to live in LITTEROOT.\p"
    .string "We could be a family of KRABBY!\n"
    .string "COOKIE-COOKIE!$"
Mom_Text_Precipice_Spring:
    .string "{PLAYER}, wanna know something\n"
    .string "interesting about this place?\p"
    .string "Have you ever wondered why it's\n"
    .string "called PRECIPICE SPRING?\p"
    .string "It's because it was created by\n"
    .string "GROUDON's PRECIPICE BLADE!\p"
    .string "...What do you mean it took\n"
    .string "me long enough?!$"
Mom_Text_Oldale:
    .string "TEST-TEST....\n"
    .string "We're in OLDALE!$"
Mom_Text_Petalburg:
    .string "TEST-TEST....\n"
    .string "We're in PETALBURG!$"
Mom_Text_Rustboro:
    .string "TEST-TEST....\n"
    .string "We're in RUSTBORO!$"
Mom_Text_Slateport:
    .string "TEST-TEST....\n"
    .string "We're in SLATEPORT!$"
Mom_Text_Mauville:
    .string "TEST-TEST....\n"
    .string "We're in MAUVILLE!$"
Mom_Text_Lilycove:
    .string "TEST-TEST....\n"
    .string "We're in LILYCOVE!$"
Mom_Text_Dewford:
    .string "TEST-TEST....\n"
    .string "We're in DEWFORD!$"
Mom_Text_Lavaridge:
    .string "TEST-TEST....\n"
    .string "We're in LAVARIDGE!$"
Mom_Text_MtPyre:
    .string "TEST-TEST....\n"
    .string "We're in MT. PYRE!$"
Mom_Text_Sootopolis:
    .string "TEST-TEST....\n"
    .string "We're in SOOTOPOLIS!$"