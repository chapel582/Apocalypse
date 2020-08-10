#include "apocalypse_platform.h"
#include "apocalypse_memory_arena.h"
#include "apocalypse_card_definitions.h"

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

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1);
	InitPlayerResource(&Definition->PlayDelta[Player_One], 1, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->PlayDelta[Player_Two], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_Two], 0, 0, 0, 0, 0);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1);
	InitPlayerResource(&Definition->PlayDelta[Player_One], 0, 1, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->PlayDelta[Player_Two], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_Two], 0, 0, 0, 0, 0);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1);
	InitPlayerResource(&Definition->PlayDelta[Player_One], 0, 0, 1, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->PlayDelta[Player_Two], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_Two], 0, 0, 0, 0, 0);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1);
	InitPlayerResource(&Definition->PlayDelta[Player_One], 0, 0, 0, 1, 0);
	InitPlayerResource(&Definition->TapDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->PlayDelta[Player_Two], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_Two], 0, 0, 0, 0, 0);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1);
	InitPlayerResource(&Definition->PlayDelta[Player_One], 0, 0, 0, 0, 1);
	InitPlayerResource(&Definition->TapDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->PlayDelta[Player_Two], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_Two], 0, 0, 0, 0, 0);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1);
	InitPlayerResource(&Definition->PlayDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_One], 1, 0, 0, 0, 0);
	InitPlayerResource(&Definition->PlayDelta[Player_Two], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_Two], 0, 0, 0, 0, 0);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1);
	InitPlayerResource(&Definition->PlayDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_One], 0, 1, 0, 0, 0);
	InitPlayerResource(&Definition->PlayDelta[Player_Two], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_Two], 0, 0, 0, 0, 0);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1);
	InitPlayerResource(&Definition->PlayDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_One], 0, 0, 1, 0, 0);
	InitPlayerResource(&Definition->PlayDelta[Player_Two], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_Two], 0, 0, 0, 0, 0);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1);
	InitPlayerResource(&Definition->PlayDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_One], 0, 0, 0, 1, 0);
	InitPlayerResource(&Definition->PlayDelta[Player_Two], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_Two], 0, 0, 0, 0, 0);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1);
	InitPlayerResource(&Definition->PlayDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_One], 0, 0, 0, 0, 1);
	InitPlayerResource(&Definition->PlayDelta[Player_Two], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_Two], 0, 0, 0, 0, 0);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1);
	InitPlayerResource(&Definition->PlayDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->PlayDelta[Player_Two], 1, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_Two], 0, 0, 0, 0, 0);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1);
	InitPlayerResource(&Definition->PlayDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->PlayDelta[Player_Two], 0, 1, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_Two], 0, 0, 0, 0, 0);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1);
	InitPlayerResource(&Definition->PlayDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->PlayDelta[Player_Two], 0, 0, 1, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_Two], 0, 0, 0, 0, 0);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1);
	InitPlayerResource(&Definition->PlayDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->PlayDelta[Player_Two], 0, 0, 0, 1, 0);
	InitPlayerResource(&Definition->TapDelta[Player_Two], 0, 0, 0, 0, 0);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1);
	InitPlayerResource(&Definition->PlayDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->PlayDelta[Player_Two], 0, 0, 0, 0, 1);
	InitPlayerResource(&Definition->TapDelta[Player_Two], 0, 0, 0, 0, 0);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1);
	InitPlayerResource(&Definition->PlayDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->PlayDelta[Player_Two], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_Two], 1, 0, 0, 0, 0);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1);
	InitPlayerResource(&Definition->PlayDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->PlayDelta[Player_Two], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_Two], 0, 1, 0, 0, 0);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1);
	InitPlayerResource(&Definition->PlayDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->PlayDelta[Player_Two], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_Two], 0, 0, 1, 0, 0);

	Definition = &Definitions->Array[CardId];
	InitCard(Definition, CardId++, 1, 1, 1);
	InitPlayerResource(&Definition->PlayDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_One], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->PlayDelta[Player_Two], 0, 0, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_Two], 0, 0, 0, 1, 0);

	ASSERT(CardId < MAX_CARDS_IN_GAME);
	Definitions->NumCards = CardId;
	
	return Definitions;
}