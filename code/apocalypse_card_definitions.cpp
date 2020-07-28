#include "apocalypse_platform.h"
#include "apocalypse_memory_arena.h"
#include "apocalypse_card_definitions.h"

card_definition* DefineCards(memory_arena* MemoryArena)
{
	uint32_t MaxCardsInGame = 200;
	card_definition* Definitions = PushArray(
		MemoryArena, MaxCardsInGame, card_definition
	);
	
	uint32_t CardId = 0;
	card_definition* Definition = &Definitions[CardId];
	InitCard(Definition, CardId++, 1, 1, 2);
	InitPlayerResource(&Definition->PlayDelta[Player_One], 1, -1, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_One], 1, 1, 0, 0, 0);
	InitPlayerResource(&Definition->PlayDelta[Player_Two], 1, 1, 0, 0, 0);
	InitPlayerResource(&Definition->TapDelta[Player_Two], 1, 1, 0, 0, 0);

	ASSERT(CardId < MaxCardsInGame);
	return Definitions;
}