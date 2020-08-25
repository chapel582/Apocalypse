#include "apocalypse_info_card.h"
#include "apocalypse_string.h"
#include "apocalypse_player_resources.h"
#include "apocalypse_player_id.h"

void AppendResourceStringToInfoCard(
	player_resources* Deltas,
	string_appender* StringAppender,
	char* SelfOrOpp,
	char* DeltaType
)
{
	bool NonZeroDelta = false;
	for(
		int DeltaIndex = 0;
		DeltaIndex < PlayerResource_Count;
		DeltaIndex++
	)
	{
		uint32_t Delta = Deltas->Resources[DeltaIndex];
		if(Delta != 0)
		{
			if(!NonZeroDelta)
			{
				AppendToString(
					StringAppender, "%s: %s\n", SelfOrOpp, DeltaType
				);
				NonZeroDelta = true;
			}
			AppendToString(
				StringAppender,
				"%s: %d\n",
				GetResourceInitial((player_resource_type) DeltaIndex),
				Delta
			);
		}
	}
}

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
	player_resources* PlayDelta,
	player_resources* TapDelta,
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

	AppendResourceStringToInfoCard(
		PlayDelta + Player_One, &StringAppender, "Self", "Play"
	);
	AppendResourceStringToInfoCard(
		TapDelta + Player_One, &StringAppender, "Self", "Tap"
	);

	AppendResourceStringToInfoCard(
		PlayDelta + Player_Two, &StringAppender, "Opp", "Play"
	);
	AppendResourceStringToInfoCard(
		TapDelta + Player_Two, &StringAppender, "Opp", "Tap"
	);

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
		Definition->PlayDelta,
		Definition->TapDelta,
		Definition->Description,
		TapsRemaining
	);
}