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
	InitCard(Definition, CardId++, 1, 20, 300, 0, 0);
	SetName(
		Definition,
		"General",
		sizeof("General")
	);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 10, 10, -10, 10);
	SetName(Definition, "Basic 1", sizeof("Basic 1"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 20, 20, -20, 20);
	SetName(Definition, "Basic 2", sizeof("Basic 2"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 30, 30, -30, 30);
	SetName(Definition, "Basic 3", sizeof("Basic 3"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 40, 40, -40, 40);
	SetName(Definition, "Basic 4", sizeof("Basic 4"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 50, 50, -50, 50);
	SetName(Definition, "Basic 5", sizeof("Basic 5"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 5, 10, -5, 10);
	SetName(Definition, "Basic 6", sizeof("Basic 6"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 15, 20, -15, 20);
	SetName(Definition, "Basic 7", sizeof("Basic 7"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 25, 30, -25, 30);
	SetName(Definition, "Basic 8", sizeof("Basic 8"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 35, 40, -35, 40);
	SetName(Definition, "Basic 9", sizeof("Basic 9"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 45, 50, -45, 50);
	SetName(Definition, "Basic 10", sizeof("Basic 10"));

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 10, 10, -10, 10);
	SetName(Definition, "Self Weaker", sizeof("Self Weaker"));
	SetTag(&Definition->GridTags, GridEffect_SelfWeaken);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 10, 10, -10, 10);
	SetName(Definition, "Opp Strengthen", sizeof("Opp Strengthen"));
	SetTag(&Definition->GridTags, GridEffect_OppStrengthen);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 10, 10, -10, 10);
	SetName(Definition, "Life Loss", sizeof("Life Loss"));
	SetTag(&Definition->GridTags, GridEffect_SelfLifeLoss);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 10, 20, -10, 10);
	SetName(Definition, "Life Gain", sizeof("Life Gain"));
	SetTag(&Definition->GridTags, GridEffect_OppLifeGain);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 10, 10, -10, 10);
	SetName(Definition, "Cost Increase", sizeof("Cost Increase"));
	SetTag(&Definition->GridTags, GridEffect_CostIncrease);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 10, 10, -10, 10);
	SetName(Definition, "Give Increase", sizeof("Give Increase"));
	SetTag(&Definition->GridTags, GridEffect_GiveIncrease);
	SetDescription(
		Definition,
		"Opp delta increases each second",
		sizeof("Opp delta increases each second")
	);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 10, 40, -20, 20);
	SetName(Definition, "Current Boost", sizeof("Current Boost"));
	SetTag(&Definition->GridTags, GridEffect_CurrentBoost);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 40, 40, -20, 0);
	SetName(Definition, "Current Loss", sizeof("Current Loss"));
	SetTag(&Definition->GridTags, GridEffect_CurrentLoss);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 20, 40, -40, 30);
	SetName(Definition, "Taunt 1", sizeof("Taunt 1"));
	SetTag(&Definition->GridTags, GridEffect_Taunt);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 20, 40, -10, 0);
	SetName(Definition, "Self Delta on Attack", sizeof("Self Delta on Attack"));
	SetTag(&Definition->GridTags, GridEffect_SelfDeltaOnAttack);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 20, 40, 0, 10);
	SetName(Definition, "Opp Delta on Attack", sizeof("Opp Delta on Attack"));
	SetTag(&Definition->GridTags, GridEffect_OppDeltaOnAttack);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 20, 20, -20, 0);
	SetName(Definition, "SelfDeltaFromCurrent", sizeof("SelfDeltaFromCurrent"));
	SetTag(&Definition->GridTags, GridEffect_SelfDeltaFromCurrent);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 20, 20, -10, 0);
	SetName(
		Definition,
		"SelfDeltaFromCurrentOnAttack",
		sizeof("SelfDeltaFromCurrentOnAttack")
	);
	SetTag(&Definition->GridTags, GridEffect_SelfDeltaFromCurrent);
	SetTag(&Definition->GridTags, GridEffect_SelfDeltaOnAttack);
	SetDescription(
		Definition,
		"Take from self play from current. Trigger self play delta on attack.",
		sizeof("Take from self play from current. Trigger self play delta on attack.")
	);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 0, 40, -40, 20);
	SetName(
		Definition,
		"GainRemainingAsAttack",
		sizeof("GainRemainingAsAttack")
	);
	SetTag(&Definition->GridTags, GridEffect_GainRemainingAsAttack);
	SetDescription(
		Definition,
		"At the end of your turn, gain half of the time remaining as attack",
		sizeof("At the end of your turn, gain half of the time remaining as attack")
	);

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
		"Increase Next",
		sizeof("Increase Next")
	);
	SetTag(&Definition->StackTags, StackEffect_NoEffect);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 0, 0, -20, -20);
	SetName(
		Definition,
		"Decrease Next",
		sizeof("Decrease Next")
	);
	SetTag(&Definition->StackTags, StackEffect_NoEffect);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 0, 0, -40, 0);
	SetName(
		Definition, "Increase Current", sizeof("Increase Current")
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
	InitCard(Definition, CardId++, 1, 0, 0, 40, 0);
	SetName(
		Definition,
		"Rand OppDelta",
		sizeof("Rand OppDelta")
	);
	SetDescription(
		Definition,
		"Select a player. Randomize the opp delta of hand cards with a uniform distribution ranging from the lowest value to the highest.",
		sizeof("Select a player. Randomize the opp delta of hand cards with a uniform distribution ranging from the lowest value to the highest."
		)
	);
	SetTag(&Definition->StackTags, StackEffect_OppDeltaConfuse);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 0, 0, 40, 0);
	SetName(
		Definition,
		"Rand SelfDelta",
		sizeof("Rand SelfDelta")
	);
	SetDescription(
		Definition,
		"Select a player. Randomize the self delta of hand cards with a uniform distribution ranging from the lowest value to the highest.",
		sizeof("Select a player. Randomize the self delta of hand cards with a uniform distribution ranging from the lowest value to the highest."
		)
	);
	SetTag(&Definition->StackTags, StackEffect_SelfDeltaConfuse);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 0, 0, 0, 40);
	SetName(
		Definition,
		"Draw Two and Give",
		sizeof("Draw Two and Give")
	);
	SetTag(&Definition->StackTags, StackEffect_DrawTwo);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 0, 0, -40, 0);
	SetName(
		Definition,
		"Draw Two and Cost",
		sizeof("Draw Two and Cost")
	);
	SetTag(&Definition->StackTags, StackEffect_DrawTwo);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 0, 0, 30, 30);
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
	InitCard(Definition, CardId++, 1, 0, 0, -20, 0);
	SetName(
		Definition,
		"Discard and Give",
		sizeof("Discard and Give")
	);
	SetDescription(
		Definition,
		"Discard a card and trigger the opp delta for the owner instead.",
		sizeof("Discard a card and trigger the opp delta for the owner instead.")
	);
	SetTag(&Definition->StackTags, StackEffect_DiscardAndGive);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 0, 0, -20, 0);
	SetName(
		Definition,
		"Draw and Cost",
		sizeof("Draw and Cost")
	);
	SetDescription(
		Definition,
		"Choose a player to draw a card. Trigger the self delta of the drawn card.",
		sizeof("Choose a player to draw a card. Trigger the self delta of the drawn card")
	);
	SetTag(&Definition->StackTags, StackEffect_DrawAndCost);

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

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 0, 0, -10, 20);
	SetName(
		Definition,
		"Both Draw",
		sizeof("Both Draw")
	);
	SetDescription(
		Definition,
		"Both players draw a card",
		sizeof("Both players draw a card")
	);
	SetTag(&Definition->StackTags, StackEffect_BothDraw);

	ASSERT(CardId < MAX_CARDS_IN_GAME);
	Definitions->NumCards = CardId;
	
	return Definitions;
}