@ Stub macros for pokesequel-imported Johto map scripts that use macro
@ instructions not present in pokeemerald. These emit script "end" (.byte 2)
@ so that any callsite returns immediately at runtime without crashing.
@
@ When a real implementation is added in pokeemerald, REMOVE the stub here
@ to avoid silent shadowing.

	@ pokesequel "applymovement2" — variant of applymovement (different opcode)
	.macro applymovement2 localId:req, movements:req
	.byte 2 @ stub: end
	.endm

	@ pokesequel "givenamedmon" — gives a named/nicknamed mon (parameter is index)
	.macro givenamedmon idx:req
	.byte 2 @ stub: end
	.endm

	@ pokesequel "removenamedmon" — removes a previously-given named mon
	.macro removenamedmon idx:req
	.byte 2 @ stub: end
	.endm

	@ Removes the 5 starter / event mons (Gate_NationalPark)
	.macro remove5mons
	.byte 2 @ stub: end
	.endm

	@ Item-choice prompt (IcePath_B3F)
	.macro chooseitem idx:req
	.byte 2 @ stub: end
	.endm

	@ Wild-battle setter with shiny override (LakeOfRage)
	.macro setwildbattleshiny species:req, level:req
	.byte 2 @ stub: end
	.endm

	@ Generic mon removal (LakeOfRage_House2)
	.macro removegenericmon idx:req
	.byte 2 @ stub: end
	.endm

	@ Give battle points (NewBarkTown)
	.macro givebp amount:req
	.byte 2 @ stub: end
	.endm

	@ Buffer the Pokédex category of a mon into a string var (NewBarkTown_Lab)
	.macro buffermoncategory strVar:req, species:req
	.byte 2 @ stub: end
	.endm

	@ Returns randomizer-on flag in VAR_RESULT (Route30_MrPokemonsHouse)
	.macro checkrandomizer
	.byte 2 @ stub: end
	.endm

	@ Gives an odd egg of the specified index (Route34_DayCare)
	.macro giveoddegg idx:req
	.byte 2 @ stub: end
	.endm

	@ Baoba phone-check on a Safari Zone mon (SafariZoneGate_SafariZoneEntrance)
	.macro baobacheckmon idx:req
	.byte 2 @ stub: end
	.endm
