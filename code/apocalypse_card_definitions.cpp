#include "apocalypse_platform.h"
#include "apocalypse_memory_arena.h"
#include "apocalypse_card_definitions.h"

#include "string.h"

#define MAX_CARDS_IN_GAME 200
card_definitions* DefineCards(memory_arena* MemoryArena)
{
	card_definitions* Definitions = PushStruct(MemoryArena, card_definitions);
	Definitions->Array = PushArray(
		MemoryArena, MAX_CARDS_IN_GAME, card_definition
	);
	
	uint32_t CardId = 0;
	card_definition* Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 2);
	InitPlayerResource(&Definition->PlayDelta[Player_One], 1, -1, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_One], 1, 1, 0, 0, 0);
	InitPlayerResource(&Definition->PlayDelta[Player_Two], 1, 1, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_Two], 1, 1, 0, 0, 0);
	SetName(Definition, "ACard", sizeof("ACard"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1);
	InitPlayerResource(&Definition->PlayDelta[Player_One], 1, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->PlayDelta[Player_Two], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_Two], 0, 0, 0, 0, 0);
	SetName(Definition, "AAard", sizeof("AAard"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1);
	InitPlayerResource(&Definition->PlayDelta[Player_One], 0, 1, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->PlayDelta[Player_Two], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_Two], 0, 0, 0, 0, 0);
	SetName(Definition, "BCard", sizeof("BCard"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1);
	InitPlayerResource(&Definition->PlayDelta[Player_One], 0, 0, 1, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->PlayDelta[Player_Two], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_Two], 0, 0, 0, 0, 0);
	SetName(Definition, "BBCard", sizeof("BBCard"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1);
	InitPlayerResource(&Definition->PlayDelta[Player_One], 0, 0, 0, 1, 0);
	InitPlayerResource(&Definition->TapDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->PlayDelta[Player_Two], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_Two], 0, 0, 0, 0, 0);
	SetName(Definition, "AAACard", sizeof("AAACard"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1);
	InitPlayerResource(&Definition->PlayDelta[Player_One], 0, 0, 0, 0, 1);
	InitPlayerResource(&Definition->TapDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->PlayDelta[Player_Two], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_Two], 0, 0, 0, 0, 0);
	SetName(Definition, "AAACard", sizeof("aCard"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1);
	InitPlayerResource(&Definition->PlayDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_One], 1, 0, 0, 0, 0);
	InitPlayerResource(&Definition->PlayDelta[Player_Two], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_Two], 0, 0, 0, 0, 0);
	SetName(Definition, "ABard", sizeof("ABard"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1);
	InitPlayerResource(&Definition->PlayDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_One], 0, 1, 0, 0, 0);
	InitPlayerResource(&Definition->PlayDelta[Player_Two], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_Two], 0, 0, 0, 0, 0);
	SetName(Definition, "aBard", sizeof("aBard"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1);
	InitPlayerResource(&Definition->PlayDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_One], 0, 0, 1, 0, 0);
	InitPlayerResource(&Definition->PlayDelta[Player_Two], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_Two], 0, 0, 0, 0, 0);
	SetName(Definition, "qukcooo", sizeof("qukcooo"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1);
	InitPlayerResource(&Definition->PlayDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_One], 0, 0, 0, 1, 0);
	InitPlayerResource(&Definition->PlayDelta[Player_Two], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_Two], 0, 0, 0, 0, 0);
	SetName(Definition, "wom wom", sizeof("wom wom"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1);
	InitPlayerResource(&Definition->PlayDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_One], 0, 0, 0, 0, 1);
	InitPlayerResource(&Definition->PlayDelta[Player_Two], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_Two], 0, 0, 0, 0, 0);
	SetName(Definition, "Long", sizeof("Long"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1);
	InitPlayerResource(&Definition->PlayDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->PlayDelta[Player_Two], 1, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_Two], 0, 0, 0, 0, 0);
	SetName(Definition, "Longer", sizeof("Longer"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1);
	InitPlayerResource(&Definition->PlayDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->PlayDelta[Player_Two], 0, 1, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_Two], 0, 0, 0, 0, 0);
	SetName(Definition, "TestAgain2", sizeof("TestAgain2"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1);
	InitPlayerResource(&Definition->PlayDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->PlayDelta[Player_Two], 0, 0, 1, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_Two], 0, 0, 0, 0, 0);
	SetName(Definition, "TestAgain3", sizeof("TestAgain3"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1);
	InitPlayerResource(&Definition->PlayDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->PlayDelta[Player_Two], 0, 0, 0, 1, 0);
	InitPlayerResource(&Definition->TapDelta[Player_Two], 0, 0, 0, 0, 0);
	SetName(Definition, "TestAgain4", sizeof("TestAgain4"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1);
	InitPlayerResource(&Definition->PlayDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->PlayDelta[Player_Two], 0, 0, 0, 0, 1);
	InitPlayerResource(&Definition->TapDelta[Player_Two], 0, 0, 0, 0, 0);
	SetName(Definition, "TestAgain5", sizeof("TestAgain5"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1);
	InitPlayerResource(&Definition->PlayDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->PlayDelta[Player_Two], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_Two], 1, 0, 0, 0, 0);
	SetName(Definition, "TestAgain6", sizeof("TestAgain6"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1);
	InitPlayerResource(&Definition->PlayDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->PlayDelta[Player_Two], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_Two], 0, 1, 0, 0, 0);
	SetName(Definition, "Break up", sizeof("Break up"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1);
	InitPlayerResource(&Definition->PlayDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->PlayDelta[Player_Two], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_Two], 0, 0, 1, 0, 0);
	SetName(Definition, "TestAgain7", sizeof("TestAgain7"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1);
	InitPlayerResource(&Definition->PlayDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->PlayDelta[Player_Two], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_Two], 0, 0, 0, 1, 0);
	SetName(Definition, "TestAgain8", sizeof("TestAgain8"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1);
	InitPlayerResource(&Definition->PlayDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->PlayDelta[Player_Two], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_Two], 0, 0, 0, 1, 0);
	SetName(Definition, "TestAgain9", sizeof("TestAgain9"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1);
	InitPlayerResource(&Definition->PlayDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->PlayDelta[Player_Two], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_Two], 0, 0, 0, 1, 0);
	SetName(Definition, "Test10", sizeof("Test10"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1);
	InitPlayerResource(&Definition->PlayDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->PlayDelta[Player_Two], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_Two], 0, 0, 0, 1, 0);
	SetName(Definition, "Test11", sizeof("Test11"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1);
	InitPlayerResource(&Definition->PlayDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->PlayDelta[Player_Two], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_Two], 0, 0, 0, 1, 0);
	SetName(Definition, "TestWee", sizeof("TestWee"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1);
	InitPlayerResource(&Definition->PlayDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->PlayDelta[Player_Two], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_Two], 0, 0, 0, 1, 0);
	SetName(Definition, "Test12", sizeof("Test12"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1);
	InitPlayerResource(&Definition->PlayDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->PlayDelta[Player_Two], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_Two], 0, 0, 0, 1, 0);
	SetName(Definition, "Test13", sizeof("Test13"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1);
	InitPlayerResource(&Definition->PlayDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->PlayDelta[Player_Two], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_Two], 0, 0, 0, 1, 0);
	SetName(Definition, "TestSlimSlam", sizeof("TestSlimSlam"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1);
	InitPlayerResource(&Definition->PlayDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->PlayDelta[Player_Two], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_Two], 0, 0, 0, 1, 0);
	SetName(Definition, "Test14", sizeof("Test14"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1);
	InitPlayerResource(&Definition->PlayDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->PlayDelta[Player_Two], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_Two], 0, 0, 0, 1, 0);
	SetName(Definition, "Test15", sizeof("Test15"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1);
	InitPlayerResource(&Definition->PlayDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->PlayDelta[Player_Two], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_Two], 0, 0, 0, 1, 0);
	SetName(Definition, "Test16", sizeof("Test16"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1);
	InitPlayerResource(&Definition->PlayDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->PlayDelta[Player_Two], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_Two], 0, 0, 0, 1, 0);
	SetName(Definition, "Test17", sizeof("Test17"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1);
	InitPlayerResource(&Definition->PlayDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->PlayDelta[Player_Two], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_Two], 0, 0, 0, 1, 0);
	SetName(Definition, "Description Test", sizeof("Description Test"));
	SetDescription(Definition, "Hit 'em hard", sizeof("Hit 'em hard"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 0, 1);
	InitPlayerResource(&Definition->PlayDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_One], 1, 0, 0, 0, 0);
	InitPlayerResource(&Definition->PlayDelta[Player_Two], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_Two], 1, 0, 0, 0, 0);
	SetName(Definition, "Red Land", sizeof("Red Land"));
	SetTag(&Definition->Tags, CardEffect_Land);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 0, 1);
	InitPlayerResource(&Definition->PlayDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_One], 0, 1, 0, 0, 0);
	InitPlayerResource(&Definition->PlayDelta[Player_Two], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_Two], 0, 1, 0, 0, 0);
	SetName(Definition, "Green Land", sizeof("Green Land"));
	SetTag(&Definition->Tags, CardEffect_Land);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 0, 1);
	InitPlayerResource(&Definition->PlayDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_One], 0, 0, 1, 0, 0);
	InitPlayerResource(&Definition->PlayDelta[Player_Two], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_Two], 0, 0, 1, 0, 0);
	SetName(Definition, "Blue Land", sizeof("Blue Land"));
	SetTag(&Definition->Tags, CardEffect_Land);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 0, 1);
	InitPlayerResource(&Definition->PlayDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_One], 0, 0, 0, 1, 0);
	InitPlayerResource(&Definition->PlayDelta[Player_Two], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_Two], 0, 0, 0, 1, 0);
	SetName(Definition, "White Land", sizeof("White Land"));
	SetTag(&Definition->Tags, CardEffect_Land);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 0, 1);
	InitPlayerResource(&Definition->PlayDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_One], 0, 0, 0, 0, 1);
	InitPlayerResource(&Definition->PlayDelta[Player_Two], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_Two], 0, 0, 0, 0, 1);
	SetName(Definition, "Black Land", sizeof("Black Land"));
	SetTag(&Definition->Tags, CardEffect_Land);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 10, 1);
	InitPlayerResource(&Definition->PlayDelta[Player_One], 0, -1, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->PlayDelta[Player_Two], 0, 1, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_Two], 0, 0, 0, 0, 0);
	SetName(Definition, "Self Weaker", sizeof("Self Weaker"));
	SetTag(&Definition->Tags, CardEffect_SelfWeaken);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 10, 1);
	InitPlayerResource(&Definition->PlayDelta[Player_One], 0, -1, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->PlayDelta[Player_Two], 0, 1, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_Two], 0, 0, 0, 0, 0);
	SetName(Definition, "Opp Strengthen", sizeof("Opp Strengthen"));
	SetTag(&Definition->Tags, CardEffect_OppStrengthen);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 10, 10);
	InitPlayerResource(&Definition->PlayDelta[Player_One], 0, 0, 0, 0, -1);
	InitPlayerResource(&Definition->TapDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->PlayDelta[Player_Two], 0, 0, 0, 0, 1);
	InitPlayerResource(&Definition->TapDelta[Player_Two], 0, 0, 0, 0, 0);
	SetName(Definition, "Life Loss", sizeof("Life Loss"));
	SetTag(&Definition->Tags, CardEffect_SelfLifeLoss);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 10, 1);
	InitPlayerResource(&Definition->PlayDelta[Player_One], 0, 0, 0, 0, -1);
	InitPlayerResource(&Definition->TapDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->PlayDelta[Player_Two], 0, 0, 0, 0, 1);
	InitPlayerResource(&Definition->TapDelta[Player_Two], 0, 0, 0, 0, 0);
	SetName(Definition, "Life Gain", sizeof("Life Gain"));
	SetTag(&Definition->Tags, CardEffect_OppLifeGain);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 10, 1);
	InitPlayerResource(&Definition->PlayDelta[Player_One], 0, 0, 0, 0, -1);
	InitPlayerResource(&Definition->TapDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->PlayDelta[Player_Two], 0, 0, 0, 0, 1);
	InitPlayerResource(&Definition->TapDelta[Player_Two], 0, 0, 0, 0, 0);
	SetName(Definition, "Cost Increase", sizeof("Cost Increase"));
	SetTag(&Definition->Tags, CardEffect_CostIncrease);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 10, 1);
	InitPlayerResource(&Definition->PlayDelta[Player_One], 0, 0, 0, 0, -1);
	InitPlayerResource(&Definition->TapDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->PlayDelta[Player_Two], 0, 0, 0, 0, 1);
	InitPlayerResource(&Definition->TapDelta[Player_Two], 0, 0, 0, 0, 0);
	SetName(Definition, "Give Increase", sizeof("Give Increase"));
	SetTag(&Definition->Tags, CardEffect_GiveIncrease);

	ASSERT(CardId < MAX_CARDS_IN_GAME);
	Definitions->NumCards = CardId;
	
	return Definitions;
}