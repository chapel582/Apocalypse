#include "apocalypse_info_card.h"
#include "apocalypse_string.h"
#include "apocalypse_player_id.h"

#define ATTACK_HEALTH_MAX_LENGTH 16

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
	int32_t TapsRemaining,
	bool IsStackCard,
	uint32_t Layer
)
{
	float FontHeight = 0.08f * InfoCardYBound.Y;
	PushSizedBitmap(
		RenderGroup,
		Assets,
		BitmapHandle_TestCard2,
		InfoCardCenter,
		InfoCardXBound,
		InfoCardYBound,
		Color,
		Layer
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

	AppendToString(&StringAppender, "SelfPlayDelta: %d\n", SelfPlayDelta);
	AppendToString(&StringAppender, "OppPlayDelta: %d\n", OppPlayDelta);
	if(!IsStackCard)
	{
		AppendToString(&StringAppender, "Attack: %d\n", Attack);
		AppendToString(&StringAppender, "Health: %d\n", Health);
		if(TapsRemaining >= 0)
		{
			AppendToString(&StringAppender, "TapsLeft: %d", TapsRemaining);
		}
	}

	vector2 TopLeft = (
		InfoCardCenter - 0.5f * InfoCardXBound + 0.5f * InfoCardYBound
	);
	push_text_result PushTextResult = PushTextTopLeft(
		RenderGroup,
		Assets,
		FontHandle_TestFont,
		ResourceString,
		MaxCharacters,
		FontHeight,
		TopLeft,
		Vector4(0.0f, 0.0f, 0.0f, 1.0f),
		FrameArena,
		Layer
	);

	if(Description != NULL)
	{
		if(Description[0] != '\0')
		{
			TopLeft.Y = PushTextResult.NextLineY;
			PushTextTopLeftAutoWrap(
				RenderGroup,
				Assets,
				FontHandle_TestFont,
				Description,
				MaxCharacters,
				FontHeight,
				TopLeft,
				Vector4(0.0f, 0.0f, 0.0f, 1.0f),
				FrameArena,
				InfoCardXBound.X,
				Layer
			);
		}
	}
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
	int32_t TapsRemaining,
	uint32_t Layer
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
		TapsRemaining,
		HasAnyTag(&Definition->StackTags),
		Layer
	);
}