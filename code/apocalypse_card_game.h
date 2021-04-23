#ifndef APOCALYPSE_CARD_GAME_H

#define DEFAULT_HAND_SIZE 5
#define MAX_CARDS_PER_SET 64
#define MAX_CARDS_PER_DATA_SET 256
#define DEFAULT_NEXT_TURN_TIMER 40.0f
#define MISSED_UPDATES_BEFORE_DESTRUCTION 5 // NOTE: measured in frames

#include "apocalypse_socket.h"
#include "apocalypse_platform.h"
#include "apocalypse_vector.h"
#include "apocalypse_rectangle.h"
#include "apocalypse_memory_arena.h"
#include "apocalypse_bitmap.h"
#include "apocalypse_render_group.h"
#include "apocalypse_assets.h"
#include "apocalypse_audio.h"
#include "apocalypse_particles.h"
#include "apocalypse_card_definitions.h"
#include "apocalypse_player_id.h"
#include "apocalypse_deck_storage.h"
#include "apocalypse_alert.h"
#include "apocalypse_ui.h"
#include "apocalypse_scroll.h"

typedef enum
{
	CardSet_Hand,
	CardSet_Grid,
	CardSet_Stack,
	CardSet_Count
} card_set_type;

typedef enum
{
	CardDataSet_Draw,
	CardDataSet_Discard,
	CardDataSet_Lost,
	CardDataSet_Count
} card_data_set_type;

struct deck
{
	card_definition* Cards[MAX_CARDS_IN_DECK];
	uint32_t CardCount;
};

struct card // TODO: maybe rename to active or visible card?
{
	uint64_t LastFrame; // NOTE: last frame that leader updated this card on
	uint32_t CardId; // NOTE: distinct from definition ID
	card_definition* Definition;

	rectangle Rectangle;
	card_set_type SetType;
	vector4 Color;
	// TODO: we're getting several bools here...time to move to flags?
	bool Active;
	bool Visible;
	bool HoveredOver;
	
	uint8_t MissedUpdates;

	player_id Owner;
	int32_t TapsAvailable;
	int32_t TimesTapped;
	int16_t SelfPlayDelta;
	int16_t TurnStartSelfPlayDelta;
	int16_t OppPlayDelta;
	int16_t TurnStartOppPlayDelta;
	int16_t Attack;
	int16_t TurnStartAttack;
	int16_t Health;
	int16_t TurnStartHealth;
	uint8_t Movement;
	uint8_t Row;
	uint8_t Col;
	grid_effect_tags GridTags;
	stack_effect_tags StackTags;
};

struct card_set
{
	card* Cards[MAX_CARDS_PER_SET];
	uint32_t CardCount;
	float ScreenWidth;
	float CardWidth;
	float YPos;
	card_set_type Type;
};

struct card_data
{
	uint32_t CardId;
	card_definition* Definition;
	
	player_id Owner;
	int32_t TapsAvailable;
	int16_t SelfPlayDelta;
	int16_t OppPlayDelta;
	int16_t Attack;
	int16_t Health;
	uint8_t Movement;
	grid_effect_tags GridTags;
	stack_effect_tags StackTags;

	rectangle Rectangle;
};

struct card_data_set
{
	card_data Cards[MAX_CARDS_PER_DATA_SET];
	uint32_t CardCount;
	card_data_set_type Type;
	card_data* ShowInfoCard;
};

struct card_stack_entry
{
	card* Card;
	union
	{
		struct
		{
			bool PlayerTargetSet;
			player_id PlayerTarget;
		};
		struct
		{
			card* CardTarget;
		};
	};
};

typedef enum
{
	CardGameEvent_TimeUpdate,
	CardGameEvent_TurnChange
} card_game_event_type;

struct card_game_event_header
{
	card_game_event_type EventType;
	uint32_t SequenceNum;
	uint32_t DataSize;
};

typedef enum
{
	SyncState_Undefined,
	SyncState_Complete,
	SyncState_Send,
	SyncState_Read
} sync_state;

typedef enum
{
	TargetType_Card,
	TargetType_Player
	// NOTE: if this gets too large or inflexible, use a tag system
} target_type;

struct target_type_tags
{
	// NOTE: wrapped in a struct for easy transtion to having even more tags
	uint32_t Tags;
};

inline void SetTag(target_type_tags* Tags, target_type ToAdd)
{
	ASSERT(ToAdd < ((8 * sizeof(target_type_tags)) - 1));
	Tags->Tags |= (1 << ToAdd);
}

inline bool HasTag(target_type_tags* Tags, target_type Check)
{
	ASSERT(Check < ((8 * sizeof(target_type_tags)) - 1));
	return (Tags->Tags & (1 << Check)) > 0;
}

struct target
{
	target_type Type;
	union
	{
		struct
		{
			card* CardTarget;
		};
		struct
		{
			player_id PlayerTarget;
		};
	};
};

struct grid_cell;
struct grid_cell
{
	rectangle Rectangle;
	card* Occupant;
	grid_cell* ShortestPathPrev;
	uint8_t MovesTaken;
};

struct grid
{
	grid_cell* Cells;
	uint8_t RowCount;
	uint8_t ColCount;
	uint32_t FromRow;
	uint32_t FromCol;
	uint32_t Movement;
};

inline grid_cell* GetGridCell(grid* Grid, uint32_t Row, uint32_t Col)
{
	return Grid->Cells + Row * Grid->ColCount + Col;
}

struct card_game_state
{
	ui_context UiContext;

	uint32_t Seed;
	sync_state SyncState;

	uint32_t NextCardId;
	player_id CurrentTurn;
	int16_t LastWholeSecond;
	float TurnTimer;
	bool ShouldUpdateBaseline;
	card* Cards;
	card* SelectedCard;
	// TODO: should probably put this in an arena
	target Targets[64];
	target_type_tags ValidTargets;
	uint8_t TargetsSet;
	uint8_t TargetsNeeded; // NOTE: when a card needs targets, set this 

	uint32_t MaxCards;
	deck* Decks;
	card_data_set DrawSets[Player_Count];
	card_data_set DiscardSets[Player_Count];
	card_set* Hands;
	vector2 InfoCardCenter;
	vector2 InfoCardXBound;
	vector2 InfoCardYBound;
	vector2 ScreenDimInWorld;
	alert Alert;
	card_definitions* Definitions;
	float CardWidth;
	float CardHeight;
	float ResourceLeftPadding;
	float NextTurnTimerHeight;
	float ResourceTextHeight;

	vector2 NextTurnTimerPos[Player_Count];

	float PlayerLife[Player_Count];
	rectangle PlayerLifeRects[Player_Count];
	float NextTurnTimer[Player_Count];

	rectangle DrawRects[Player_Count];
	rectangle DiscardRects[Player_Count];

	player_id StackTurn;
	bool StackBuilding;
	// TODO: probably should put this in an arena
	card_stack_entry Stack[256];
	uint32_t StackSize;
	float StackYStart;
	vector2 StackEntryInfoDim;
	scroll_bar StackScrollBar;

	card_data_set* ViewingCardDataSet;

	// NOTE: server is assumed to be leader (has final say on game state)
	bool IsLeader;
	bool NetworkGame;
	platform_socket ListenSocket;
	platform_socket ConnectSocket;
	packet_reader_data PacketReader;

	// TODO: probably should put this in an arena
	grid Grid;

	// NOTE: last frame received from master
	uint64_t LastFrame;
};

#pragma pack(push, 1)
struct sync_done_packet
{
	packet_header Header;
};

struct switch_leader_packet
{
	packet_header Header;
};

struct set_leader_payload
{
	bool IsLeader;
};
struct set_leader_packet
{
	packet_header Header;
	set_leader_payload Payload;
};

struct card_update_payload
{
	uint32_t CardId;
	uint32_t DefId;
	player_id Owner;
	card_set_type SetType;
	
	int32_t TapsAvailable;
	int32_t TimesTapped;
	int16_t SelfPlayDelta;
	int16_t TurnStartSelfPlayDelta;
	int16_t OppPlayDelta;
	int16_t TurnStartOppPlayDelta;
	int16_t Attack;
	int16_t TurnStartAttack;
	int16_t Health;
	int16_t TurnStartHealth;
	grid_effect_tags GridTags;
	stack_effect_tags StackTags;
};
struct card_update_packet
{
	packet_header Header;
	card_update_payload Payload;
};

struct remove_card_payload
{
	uint32_t CardId;
};
struct remove_card_packet
{
	packet_header Header;
	remove_card_payload Payload;
};

struct state_update_payload
{
	player_id CurrentTurn; // NOTE: current turn is relative to leader
	player_id StackTurn;

	bool StackBuilding;

	float TurnTimer;
	float PlayerLife[Player_Count];
	float NextTurnTimer[Player_Count];
};
struct state_update_packet
{
	packet_header Header;
	state_update_payload Payload;
};

struct card_data_update
{
	uint32_t CardId;
	uint32_t DefId;
	
	int32_t TapsAvailable;
	int16_t SelfPlayDelta;
	int16_t OppPlayDelta;
	int16_t Attack;
	int16_t Health;
	grid_effect_tags GridTags;
	stack_effect_tags StackTags;
};

struct card_data_set_update_payload
{
	player_id Owner;
	card_data_set_type Type;
	uint32_t CardCount;
	// NOTE: followed by CardCount card_data_update structures
};
struct card_data_set_update_packet
{
	packet_header Header;
	card_data_set_update_payload Payload;
};
#pragma pack(pop)

struct card_game_args
{
	loaded_deck P1Deck;
	loaded_deck P2Deck;
	platform_socket ListenSocket;
	platform_socket ConnectSocket;
	bool IsLeader;
	bool NetworkGame;
	bool NewCardGame;
	uint32_t Seed;
	bool SeedSet;
};
void StartCardGamePrep(
	game_state* GameState, char* P1DeckName, char* P2DeckName
);
void StartCardGamePrep(
	game_state* GameState,
	loaded_deck P1Deck,
	loaded_deck P2Deck,
	bool IsLeader,
	platform_socket* ListenSocket,
	platform_socket* ConnectSocket
);
void ResumeCardGamePrep(
	game_state* GameState,
	bool IsLeader,
	platform_socket* ListenSocket,
	platform_socket* ConnectSocket,
	uint32_t Seed,
	bool SeedSet
);

#define APOCALYPSE_CARD_GAME_H
#endif