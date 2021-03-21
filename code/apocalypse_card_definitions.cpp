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
	InitCard(Definition, CardId++, 1, 1, 2, -1, 1);
	SetName(Definition, "ACard", sizeof("ACard"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1, -1, 1);
	SetName(Definition, "AAard", sizeof("AAard"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1, -1, 1);
	SetName(Definition, "BCard", sizeof("BCard"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1, -1, 1);
	SetName(Definition, "BBCard", sizeof("BBCard"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1, -1, 1);
	SetName(Definition, "AAACard", sizeof("AAACard"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1, -1, 1);
	SetName(Definition, "AAACard", sizeof("aCard"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1, -1, 1);
	SetName(Definition, "ABard", sizeof("ABard"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1, -1, 1);
	SetName(Definition, "aBard", sizeof("aBard"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1, -1, 1);
	SetName(Definition, "qukcooo", sizeof("qukcooo"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1, -1, 1);
	SetName(Definition, "wom wom", sizeof("wom wom"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1, -1, 1);
	SetName(Definition, "Long", sizeof("Long"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1, -1, 1);
	SetName(Definition, "Longer", sizeof("Longer"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1, -1, 1);
	SetName(Definition, "TestAgain2", sizeof("TestAgain2"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1, -1, 1);
	SetName(Definition, "TestAgain3", sizeof("TestAgain3"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1, -1, 1);
	SetName(Definition, "TestAgain4", sizeof("TestAgain4"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1, -1, 1);
	SetName(Definition, "TestAgain5", sizeof("TestAgain5"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1, -1, 1);
	SetName(Definition, "TestAgain6", sizeof("TestAgain6"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1, -1, 1);
	SetName(Definition, "Break up", sizeof("Break up"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1, -1, 1);
	SetName(Definition, "TestAgain7", sizeof("TestAgain7"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1, -1, 1);
	SetName(Definition, "TestAgain8", sizeof("TestAgain8"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1, -1, 1);
	SetName(Definition, "TestAgain9", sizeof("TestAgain9"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1, -1, 1);
	SetName(Definition, "Test10", sizeof("Test10"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1, -1, 1);
	SetName(Definition, "Test11", sizeof("Test11"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1, -1, 1);
	SetName(Definition, "TestWee", sizeof("TestWee"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1, -1, 1);
	SetName(Definition, "Test12", sizeof("Test12"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1, -1, 1);
	SetName(Definition, "Test13", sizeof("Test13"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1, -1, 1);
	SetName(Definition, "TestSlimSlam", sizeof("TestSlimSlam"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1, -1, 1);
	SetName(Definition, "Test14", sizeof("Test14"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1, -1, 1);
	SetName(Definition, "Test15", sizeof("Test15"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1, -1, 1);
	SetName(Definition, "Test16", sizeof("Test16"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1, -1, 1);
	SetName(Definition, "Test17", sizeof("Test17"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1, -1, 1);
	SetName(Definition, "Description Test", sizeof("Description Test"));
	SetDescription(Definition, "Hit 'em hard", sizeof("Hit 'em hard"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 10, 10, -10, 10);
	SetName(Definition, "Self Weaker", sizeof("Self Weaker"));
	SetTag(&Definition->TableauTags, TableauEffect_SelfWeaken);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 10, 10, -10, 10);
	SetName(Definition, "Opp Strengthen", sizeof("Opp Strengthen"));
	SetTag(&Definition->TableauTags, TableauEffect_OppStrengthen);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 10, 10, -10, 10);
	SetName(Definition, "Life Loss", sizeof("Life Loss"));
	SetTag(&Definition->TableauTags, TableauEffect_SelfLifeLoss);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 10, 20, -10, 10);
	SetName(Definition, "Life Gain", sizeof("Life Gain"));
	SetTag(&Definition->TableauTags, TableauEffect_OppLifeGain);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 10, 10, -10, 10);
	SetName(Definition, "Cost Increase", sizeof("Cost Increase"));
	SetTag(&Definition->TableauTags, TableauEffect_CostIncrease);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 10, 10, -10, 10);
	SetName(Definition, "Give Increase", sizeof("Give Increase"));
	SetTag(&Definition->TableauTags, TableauEffect_GiveIncrease);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 10, 40, -20, 20);
	SetName(Definition, "Current Boost", sizeof("Current Boost"));
	SetTag(&Definition->TableauTags, TableauEffect_CurrentBoost);

	/*
	SECTION START: Define stack cards
	*/
	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 0, 0, 10, 10);
	SetName(Definition, "Hurt Opp", sizeof("Hurt Opp"));
	SetTag(&Definition->StackTags, StackEffect_HurtOpp);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 0, 0, 10, 10);
	SetName(Definition, "Disable Next", sizeof("Disable Next"));
	SetTag(&Definition->StackTags, StackEffect_DisableNext);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 0, 0, 20, 20);
	SetName(
		Definition,
		"Increase Next Turn Times",
		sizeof("Increase Next Turn Times")
	);
	SetTag(&Definition->StackTags, StackEffect_NoEffect);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 0, 0, -40, 0);
	SetName(
		Definition, "Increase Current Time", sizeof("Increase Current Time")
	);
	SetDescription(
		Definition,
		"Increase current turn time by 20 seconds",
		sizeof("Increase current turn time by 20 seconds")
	);
	SetTag(&Definition->StackTags, StackEffect_IncreaseCurrentTime);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 0, 0, 40, 0);
	SetName(
		Definition,
		"Invest Time",
		sizeof("Invest Time")
	);
	SetDescription(
		Definition,
		"Decrease current turn time by 20 seconds",
		sizeof("Decrease current turn time by 20 seconds")
	);
	SetTag(&Definition->StackTags, StackEffect_DecreaseCurrentTime);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 0, 0, 40, 0);
	SetName(
		Definition,
		"Swap deltas",
		sizeof("Swap deltas")
	);
	SetDescription(
		Definition,
		"Swap the self and opp deltas of any card in either player's hand",
		sizeof("Swap the self and opp deltas of any card in either player's hand")
	);
	SetTag(&Definition->StackTags, StackEffect_SwapDeltas);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 0, 0, 0, 30);
	SetName(
		Definition,
		"Draw Two",
		sizeof("Draw Two")
	);
	SetTag(&Definition->StackTags, StackEffect_DrawTwo);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 0, 0, 20, 20);
	SetName(
		Definition,
		"Discard Random",
		sizeof("Discard Random")
	);
	SetDescription(
		Definition,
		"Discard a random card from each players hand",
		sizeof("Discard a random card from each players hand")
	);
	SetTag(&Definition->StackTags, StackEffect_RandomDiscard);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 0, 0, -10, 20);
	SetName(
		Definition,
		"Pass Remaining",
		sizeof("Pass Remaining")
	);
	SetDescription(
		Definition,
		"End turn. Any unused time from this turn is passed to your next turn.",
		sizeof(
			"End turn. Any unused time from this turn is passed to your next turn."
		)
	);
	SetTag(&Definition->StackTags, StackEffect_PassRemaining);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 0, 0, -10, 20);
	SetName(
		Definition,
		"Get Remaining",
		sizeof("Get Remaining")
	);
	SetDescription(
		Definition,
		"Any time in your next turn is now in the current turn.",
		sizeof(
			"Any time in your next turn is now in the current turn."
		)
	);
	SetTag(&Definition->StackTags, StackEffect_GetRemaining);

	ASSERT(CardId < MAX_CARDS_IN_GAME);
	Definitions->NumCards = CardId;
	
	return Definitions;
}