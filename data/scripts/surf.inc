EventScript_UseSurf::
	checkpartymove MOVE_SURF
	goto_if_eq VAR_RESULT, PARTY_SIZE, EventScript_EndUseSurf
	bufferpartymonnick STR_VAR_1, VAR_RESULT
	setfieldeffectargument 0, VAR_RESULT
	lockall
	msgbox gText_WantToUseSurf, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, EventScript_ReleaseUseSurf
	msgbox gText_PlayerUsedSurf, MSGBOX_DEFAULT
	dofieldeffect FLDEFF_USE_SURF
EventScript_ReleaseUseSurf::
	releaseall
EventScript_EndUseSurf::
	end

@ With the HM in the Bag there's nothing to ask: a Dragonite uses the move.
@ Walking into the water is enough to get here (see GetHMObstacleScript)
EventScript_UseSurfWithHM::
	lockall
	setfieldeffectargument 0, FIELD_MOVE_USER_DRAGONITE
	dofieldeffect FLDEFF_USE_SURF
	releaseall
	end
