#include "apocalypse_platform.h"
#include "apocalypse_memory_arena.h"
#include "apocalypse_card_definitions.h"

#include "string.h"

card_definitions* DefineCards(memory_arena* MemoryArena)
{
	card_definitions* Definitions = PushStruct(MemoryArena, card_definitions);
	Definitions->Array = PushArray(
		MemoryArena, MAX_CARDS_IN_GAME, card_definition
	);
	
	uint32_t CardId = 0;
	card_definition* Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 2);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Self], 1, -1, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Self], 1, 1, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Opp], 1, 1, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Opp], 1, 1, 0, 0, 0
	);
	SetName(Definition, "ACard", sizeof("ACard"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Self], 1, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Self], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Opp], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Opp], 0, 0, 0, 0, 0
	);
	SetName(Definition, "AAard", sizeof("AAard"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Self], 0, 1, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Self], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Opp], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Opp], 0, 0, 0, 0, 0
	);
	SetName(Definition, "BCard", sizeof("BCard"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Self], 0, 0, 1, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Self], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Opp], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Opp], 0, 0, 0, 0, 0
	);
	SetName(Definition, "BBCard", sizeof("BBCard"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Self], 0, 0, 0, -1, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Self], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Opp], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Opp], 0, 0, 0, 0, 0
	);
	SetName(Definition, "AAACard", sizeof("AAACard"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Self], 0, 0, 0, 0, 1
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Self], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Opp], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Opp], 0, 0, 0, 0, 0
	);
	SetName(Definition, "AAACard", sizeof("aCard"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Self], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Self], 1, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Opp], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Opp], 0, 0, 0, 0, 0
	);
	SetName(Definition, "ABard", sizeof("ABard"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Self], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Self], 0, 1, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Opp], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Opp], 0, 0, 0, 0, 0
	);
	SetName(Definition, "aBard", sizeof("aBard"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Self], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Self], 0, 0, 1, 0, 0
	);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Opp], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Opp], 0, 0, 0, 0, 0
	);
	SetName(Definition, "qukcooo", sizeof("qukcooo"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Self], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Self], 0, 0, 0, 1, 0
	);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Opp], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Opp], 0, 0, 0, 0, 0
	);
	SetName(Definition, "wom wom", sizeof("wom wom"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Self], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Self], 0, 0, 0, 0, 1
	);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Opp], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Opp], 0, 0, 0, 0, 0
	);
	SetName(Definition, "Long", sizeof("Long"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Self], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Self], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Opp], 1, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Opp], 0, 0, 0, 0, 0
	);
	SetName(Definition, "Longer", sizeof("Longer"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Self], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Self], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Opp], 0, 1, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Opp], 0, 0, 0, 0, 0
	);
	SetName(Definition, "TestAgain2", sizeof("TestAgain2"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Self], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Self], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Opp], 0, 0, 1, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Opp], 0, 0, 0, 0, 0
	);
	SetName(Definition, "TestAgain3", sizeof("TestAgain3"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Self], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Self], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Opp], 0, 0, 0, 1, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Opp], 0, 0, 0, 0, 0
	);
	SetName(Definition, "TestAgain4", sizeof("TestAgain4"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Self], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Self], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Opp], 0, 0, 0, 0, 1
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Opp], 0, 0, 0, 0, 0
	);
	SetName(Definition, "TestAgain5", sizeof("TestAgain5"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Self], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Self], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Opp], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Opp], 1, 0, 0, 0, 0
	);
	SetName(Definition, "TestAgain6", sizeof("TestAgain6"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Self], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Self], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Opp], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Opp], 0, 1, 0, 0, 0
	);
	SetName(Definition, "Break up", sizeof("Break up"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Self], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Self], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Opp], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Opp], 0, 0, 1, 0, 0
	);
	SetName(Definition, "TestAgain7", sizeof("TestAgain7"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Self], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Self], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Opp], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Opp], 0, 0, 0, 1, 0
	);
	SetName(Definition, "TestAgain8", sizeof("TestAgain8"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Self], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Self], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Opp], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Opp], 0, 0, 0, 1, 0
	);
	SetName(Definition, "TestAgain9", sizeof("TestAgain9"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Self], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Self], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Opp], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Opp], 0, 0, 0, 1, 0
	);
	SetName(Definition, "Test10", sizeof("Test10"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Self], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Self], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Opp], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Opp], 0, 0, 0, 1, 0
	);
	SetName(Definition, "Test11", sizeof("Test11"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Self], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Self], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Opp], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Opp], 0, 0, 0, 1, 0
	);
	SetName(Definition, "TestWee", sizeof("TestWee"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Self], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Self], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Opp], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Opp], 0, 0, 0, 1, 0
	);
	SetName(Definition, "Test12", sizeof("Test12"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Self], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Self], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Opp], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Opp], 0, 0, 0, 1, 0
	);
	SetName(Definition, "Test13", sizeof("Test13"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Self], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Self], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Opp], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Opp], 0, 0, 0, 1, 0
	);
	SetName(Definition, "TestSlimSlam", sizeof("TestSlimSlam"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Self], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Self], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Opp], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Opp], 0, 0, 0, 1, 0
	);
	SetName(Definition, "Test14", sizeof("Test14"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Self], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Self], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Opp], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Opp], 0, 0, 0, 1, 0
	);
	SetName(Definition, "Test15", sizeof("Test15"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Self], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Self], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Opp], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Opp], 0, 0, 0, 1, 0
	);
	SetName(Definition, "Test16", sizeof("Test16"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Self], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Self], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Opp], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Opp], 0, 0, 0, 1, 0
	);
	SetName(Definition, "Test17", sizeof("Test17"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Self], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Self], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Opp], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Opp], 0, 0, 0, 1, 0
	);
	SetName(Definition, "Description Test", sizeof("Description Test"));
	SetDescription(Definition, "Hit 'em hard", sizeof("Hit 'em hard"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 0, 1);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Self], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Self], 1, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Opp], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Opp], 1, 0, 0, 0, 0
	);
	SetName(Definition, "Red Land", sizeof("Red Land"));
	SetTag(&Definition->TableauTags, TableauEffect_Land);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 0, 1);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Self], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Self], 0, 1, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Opp], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Opp], 0, 1, 0, 0, 0
	);
	SetName(Definition, "Green Land", sizeof("Green Land"));
	SetTag(&Definition->TableauTags, TableauEffect_Land);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 0, 1);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Self], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Self], 0, 0, 1, 0, 0
	);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Opp], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Opp], 0, 0, 1, 0, 0
	);
	SetName(Definition, "Blue Land", sizeof("Blue Land"));
	SetTag(&Definition->TableauTags, TableauEffect_Land);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 0, 1);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Self], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Self], 0, 0, 0, 1, 0
	);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Opp], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Opp], 0, 0, 0, 2, 0
	);
	SetName(Definition, "White Land", sizeof("White Land"));
	SetTag(&Definition->TableauTags, TableauEffect_Land);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 0, 1);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Self], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Self], 0, 0, 0, 0, 2
	);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Opp], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Opp], 0, 0, 0, 0, 1
	);
	SetName(Definition, "Black Land", sizeof("Black Land"));
	SetTag(&Definition->TableauTags, TableauEffect_Land);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 10, 1);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Self], 0, -1, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Self], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Opp], 0, 1, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Opp], 0, 0, 0, 0, 0
	);
	SetName(Definition, "Self Weaker", sizeof("Self Weaker"));
	SetTag(&Definition->TableauTags, TableauEffect_SelfWeaken);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 10, 1);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Self], 0, -1, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Self], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Opp], 0, 1, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Opp], 0, 0, 0, 0, 0
	);
	SetName(Definition, "Opp Strengthen", sizeof("Opp Strengthen"));
	SetTag(&Definition->TableauTags, TableauEffect_OppStrengthen);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 10, 10);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Self], 0, 0, 0, 0, -1
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Self], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Opp], 0, 0, 0, 0, 1
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Opp], 0, 0, 0, 0, 0
	);
	SetName(Definition, "Life Loss", sizeof("Life Loss"));
	SetTag(&Definition->TableauTags, TableauEffect_SelfLifeLoss);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 10, 1);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Self], 0, 0, 0, 0, -1
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Self], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Opp], 0, 0, 0, 0, 1
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Opp], 0, 0, 0, 0, 0
	);
	SetName(Definition, "Life Gain", sizeof("Life Gain"));
	SetTag(&Definition->TableauTags, TableauEffect_OppLifeGain);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 10, 1);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Self], 0, 0, 0, 0, -1
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Self], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Opp], 0, 0, 0, 0, 1
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Opp], 0, 0, 0, 0, 0
	);
	SetName(Definition, "Cost Increase", sizeof("Cost Increase"));
	SetTag(&Definition->TableauTags, TableauEffect_CostIncrease);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 10, 1);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Self], 0, 0, 0, 0, -1
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Self], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Opp], 0, 0, 0, 0, 1
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Opp], 0, 0, 0, 0, 0
	);
	SetName(Definition, "Give Increase", sizeof("Give Increase"));
	SetTag(&Definition->TableauTags, TableauEffect_GiveIncrease);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 10, 1);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Self], 0, 0, 0, 0, -1
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Self], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Opp], 0, 0, 0, 0, 1
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Opp], 0, 0, 0, 0, 0
	);
	SetName(Definition, "Self Burn", sizeof("Self Burn"));
	SetTag(&Definition->TableauTags, TableauEffect_SelfBurn);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 10, 1);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Self], 0, 0, 0, 0, -1
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Self], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Opp], 0, 0, 0, 0, 1
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Opp], 0, 0, 0, 0, 0
	);
	SetName(Definition, "Opp Burn", sizeof("Opp Burn"));
	SetTag(&Definition->TableauTags, TableauEffect_OppBurn);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 10, 1);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Self], 0, 0, 0, 0, -1
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Self], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Opp], 0, 0, 0, 0, 1
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Opp], 0, 0, 0, 0, 0
	);
	SetName(Definition, "Draw Extra", sizeof("Draw Extra"));
	SetTag(&Definition->TableauTags, TableauEffect_DrawExtra);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 10, 1);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Self], 0, 0, 0, 0, -1
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Self], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Opp], 0, 0, 0, 0, 1
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Opp], 0, 0, 0, 0, 0
	);
	SetName(Definition, "Draw Opp Extra", sizeof("Draw Opp Extra"));
	SetTag(&Definition->TableauTags, TableauEffect_DrawOppExtra);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 10, 1);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Self], 0, 0, 0, 0, -1
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Self], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Opp], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Opp], 0, 0, 0, 0, 1
	);
	SetName(Definition, "Give Time", sizeof("Give Time"));
	SetTag(&Definition->TableauTags, TableauEffect_GiveTime);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 10, 1);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Self], 0, 0, 0, 0, -1
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Self], 0, 0, 0, 0, 1
	);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Opp], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Opp], 0, 0, 0, 0, 1
	);
	SetName(Definition, "Get Time", sizeof("Get Time"));
	SetTag(&Definition->TableauTags, TableauEffect_GetTime);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 10, 1);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Self], 0, 0, 0, 0, -1
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Self], 0, 0, 0, 0, 1
	);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Opp], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Opp], 0, 0, 0, 0, 1
	);
	SetName(Definition, "Get Time", sizeof("Get Time"));
	SetTag(&Definition->TableauTags, TableauEffect_GetTime);

	// NOTE: it might be fun if this card was expensive to make the decision 
	// CONT: tougher
	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 10, 1);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Self], 0, -1, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Self], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Opp], 0, 1, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Opp], 0, 0, 0, 0, 0
	);
	SetName(Definition, "Self Hand Weaken", sizeof("Self Hand Weaken"));
	SetTag(&Definition->TableauTags, TableauEffect_SelfHandWeaken);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 5, 1);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Self], 0, -1, -1, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Self], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Opp], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Opp], 0, 0, 0, 0, 0
	);
	SetName(Definition, "Weak Give Little", sizeof("Weak Give Little"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 5, 1);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Self], 0, -1, -1, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Self], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Opp], 0, 5, 5, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Opp], 0, 0, 0, 0, 0
	);
	SetName(Definition, "Strong Give Much", sizeof("Strong Give Much"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 10, 1);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Self], 0, -1, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Self], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Opp], 0, 1, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Opp], 0, 0, 0, 0, 0
	);
	SetName(Definition, "Time Growth", sizeof("Time Growth"));
	SetTag(&Definition->TableauTags, TableauEffect_TimeGrowth);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 0, 1);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Self], 0, -1, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Self], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Opp], 0, 1, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Opp], 0, 0, 0, 0, 0
	);
	SetName(Definition, "Hurt Opp", sizeof("Hurt Opp"));
	SetTag(&Definition->StackTags, StackEffect_HurtOpp);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 0, 1);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Self], 0, -1, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Self], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Opp], 0, 1, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Opp], 0, 0, 0, 0, 0
	);
	SetName(Definition, "Disable Next", sizeof("Disable Next"));
	SetTag(&Definition->StackTags, StackEffect_DisableNext);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 5, 1);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Self], 0, -1, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Self], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->PlayDelta[RelativePlayer_Opp], 0, 0, 0, 0, 0
	);
	InitPlayerResource(
		&Definition->TapDelta[RelativePlayer_Opp], 1, 1, 1, 1, 1
	);
	SetName(Definition, "Attack Timer", sizeof("Attack Timer"));
	SetTag(&Definition->TableauTags, TableauEffect_AttackTimer);

	ASSERT(CardId < MAX_CARDS_IN_GAME);
	Definitions->NumCards = CardId;
	
	return Definitions;
}