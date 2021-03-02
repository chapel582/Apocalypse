#include "apocalypse_info_card.h"
#include "apocalypse_string.h"
#include "apocalypse_player_id.h"

#define ATTACK_HEALTH_MAX_LENGTH 8

void PushInfoCard(
	render_group* RenderGroup,
	assets* Assets,
	vector2 InfoCardCenter,
	vector2 InfoCardXBound,
	vector2 InfoCardYBound,
	vector4 Color,
	memory_arena* FrameArena,
	char* Name,
	int16_t Attack,
	int16_t Health,
	int16_t SelfPlayDelta,
	int16_t OppPlayDelta,
	char* Description,
	int32_t TapsRemaining
)
{
	PushSizedBitmap(
		RenderGroup,
		Assets,
		BitmapHandle_TestCard2,
		InfoCardCenter,
		InfoCardXBound,
		InfoCardYBound,
		Color
	);

	uint32_t MaxCharacters = (
		CARD_NAME_SIZE + 
		2 * ATTACK_HEALTH_MAX_LENGTH + 
		4 * MAX_RESOURCE_STRING_SIZE
	);
	char* ResourceString = PushArray(FrameArena, MaxCharacters, char);
	string_appender StringAppender = MakeStringAppender(
		ResourceString, MaxCharacters 
	);
	AppendToString(&StringAppender, "%s\n", Name);
	AppendToString(&StringAppender, "Attack: %d\n", Attack);
	AppendToString(&StringAppender, "Health: %d\n", Health);
	AppendToString(&StringAppender, "SelfPlayDelta: %d\n", SelfPlayDelta);
	AppendToString(&StringAppender, "OppPlayDelta: %d\n", OppPlayDelta);

	if(TapsRemaining >= 0)
	{
		AppendToString(&StringAppender, "TapsLeft: %d\n", TapsRemaining);
	}

	AppendToString(&StringAppender, "%s\n", Description);

	vector2 TopLeft = (
		InfoCardCenter - 0.5f * InfoCardXBound + 0.5f * InfoCardYBound
	);
	PushTextTopLeft(
		RenderGroup,
		Assets,
		FontHandle_TestFont,
		ResourceString,
		MaxCharacters,
		20.0f,
		TopLeft,
		Vector4(0.0f, 0.0f, 0.0f, 1.0f),
		FrameArena
	);
}

inline void PushInfoCard(
	render_group* RenderGroup,
	assets* Assets,
	vector2 InfoCardCenter,
	vector2 InfoCardXBound,
	vector2 InfoCardYBound,
	vector4 Color,
	memory_arena* FrameArena,
	card_definition* Definition,
	int32_t TapsRemaining
)
{
	PushInfoCard(
		RenderGroup,
		Assets,
		InfoCardCenter,
		InfoCardXBound,
		InfoCardYBound,
		Color,
		FrameArena,
		Definition->Name,
		Definition->Attack,
		Definition->Health,
		Definition->SelfPlayDelta,
		Definition->OppPlayDelta,
		Definition->Description,
		TapsRemaining
	);
}