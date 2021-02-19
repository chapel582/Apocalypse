#ifndef APOCALYPSE_CARD_GAME_H

#define MAX_CARDS_PER_SET 7
#define RESOURCE_TO_TIME 5.0f
#define TURN_TIMER_INCREASE 10.0f
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
	CardSet_Tableau,
	CardSet_Stack,
	CardSet_Count
} card_set_type;

struct deck_card
{
	card_definition* Definition;
	bool InDeck;
};

struct deck
{
	deck_card Cards[MAX_CARDS_IN_DECK];
	uint32_t CardCount;
	// NOTE: these arrays are handles into the Cards Array
	uint32_t InDeck[MAX_CARDS_IN_DECK];
	uint32_t InDeckCount;
};

struct card
{
	uint64_t LastFrame; // NOTE: last frame that leader updated this card on
	uint32_t CardId; // NOTE: distinct from definition ID
	card_definition* Definition;
	player_resources PlayDelta[Player_Count];
	player_resources TapDelta[Player_Count];
	
	player_resources TurnStartPlayDelta[Player_Count];
	player_resources TurnStartTapDelta[Player_Count];

	rectangle Rectangle;
	card_set_type SetType;
	float TimeLeft;
	vector4 Color;
	// TODO: we're getting several bools here...time to move to flags?
	bool Active;
	bool Visible;
	bool HoveredOver;
	
	uint8_t MissedUpdates;

	player_id Owner;
	int32_t TapsAvailable;
	int32_t TimesTapped;
	int16_t Attack;
	int16_t TurnStartAttack;
	int16_t Health;
	int16_t TurnStartHealth;
	tableau_effect_tags TableauTags;
	stack_effect_tags StackTags;
};

struct card_set
{
	card* Cards[MAX_CARDS_PER_SET];
	int CardCount;
	float ScreenWidth;
	float CardWidth;
	float YPos;
	card_set_type Type;
};

struct card_stack_entry
{
	card* Card;
	union
	{
		struct
		{
			player_id PlayerTarget;
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

struct card_game_state
{
	ui_context UiContext;

	uint32_t NextCardId;
	player_id CurrentTurn;
	int16_t LastWholeSecond;
	float TurnTimer;
	bool ShouldUpdateBaseline;
	float BaselineNextTurnTimer;
	float NextTurnTimer;
	player_resources* PlayerResources;
	card* Cards;
	card* SelectedCard;
	uint32_t MaxCards;
	deck* Decks;
	card_set* Hands;
	card_set* Tableaus;
	vector2 InfoCardCenter;
	vector2 InfoCardXBound;
	vector2 InfoCardYBound;
	vector2 ScreenDimInWorld;
	alert Alert;
	card_definitions* Definitions;
	float CardWidth;
	float CardHeight;

	float PlayerLife[Player_Count];
	rectangle PlayerLifeRects[Player_Count];

	player_id StackTurn;
	bool StackBuilding;
	card_stack_entry Stack[256];
	uint32_t StackSize;
	float StackYStart;
	vector2 StackEntryInfoDim;
	scroll_bar StackScrollBar;

	// NOTE: server is assumed to be leader (has final say on game state)
	bool IsLeader;
	bool NetworkGame;
	platform_socket ListenSocket;
	platform_socket ConnectSocket;
	packet_reader_data PacketReader;

	// NOTE: last frame received from master
	uint64_t LastFrame;
};

#pragma pack(push, 1)
struct switch_leader_packet
{
	packet_header Header;
};

struct card_update_payload
{
	uint32_t CardId;
	uint32_t DefId;
	player_id Owner;
	card_set_type SetType;
	player_resources PlayDelta[Player_Count];
	player_resources TapDelta[Player_Count];
	
	player_resources TurnStartPlayDelta[Player_Count];
	player_resources TurnStartTapDelta[Player_Count];

	float TimeLeft;
	int32_t TapsAvailable;
	int32_t TimesTapped;
	int16_t Attack;
	int16_t TurnStartAttack;
	int16_t Health;
	int16_t TurnStartHealth;
	tableau_effect_tags TableauTags;
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
	float TurnTimer;
	float NextTurnTimer;
	player_id StackTurn;
	bool StackBuilding;
	player_resources PlayerResources[Player_Count];
	float PlayerLife[Player_Count];
};
struct state_update_packet
{
	packet_header Header;
	state_update_payload Payload;
};

struct deck_update_payload
{
	player_id Owner;
	uint32_t InDeckCount;
	uint32_t Offset;
};
struct deck_update_packet
{
	packet_header Header;
	deck_update_payload Payload;
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
	platform_socket* ConnectSocket
);

#define APOCALYPSE_CARD_GAME_H
#endif