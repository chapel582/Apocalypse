#ifndef APOCALYPSE_PLAYER_RESOURCES_H

typedef enum 
{
	PlayerResource_Red,
	PlayerResource_Green,
	PlayerResource_Blue,
	PlayerResource_White,
	PlayerResource_Black,
	PlayerResource_Count	
} player_resource_type;

struct player_resources
{
	int32_t Resources[PlayerResource_Count];
};

inline bool CanChangeResources(
	player_resources* Target, player_resources* Delta
)
{
	for(int Index = 0; Index < PlayerResource_Count; Index++)
	{
		if(Target->Resources[Index] + Delta->Resources[Index] < 0)
		{
			return false;
		}
	}
	return true;
}

inline void ChangeResources(player_resources* Target, player_resources* Delta)
{
	for(int Index = 0; Index < PlayerResource_Count; Index++)
	{
		Target->Resources[Index] += Delta->Resources[Index];
		if(Target->Resources[Index] <= 0)
		{
			Target->Resources[Index] = 0;
		}
	}
}

inline void SetResource(
	player_resources* Resources, player_resource_type Type, int32_t SetTo
)
{
	Resources->Resources[Type] = SetTo;
}

inline int32_t SumResources(player_resources* Resources)
{
	int32_t Result = 0;
	for(int Index = 0; Index < PlayerResource_Count; Index++)
	{
		Result += Resources->Resources[Index];
	}
	return Result;
}

inline void InitPlayerResource(
	player_resources* Resources,
	int32_t Red,
	int32_t Green,
	int32_t Blue,
	int32_t White,
	int32_t Black
)
{
	SetResource(Resources, PlayerResource_Red, Red);
	SetResource(Resources, PlayerResource_Green, Green);
	SetResource(Resources, PlayerResource_Blue, Blue);
	SetResource(Resources, PlayerResource_White, White);
	SetResource(Resources, PlayerResource_Black, Black);
}

#define APOCALYPSE_PLAYER_RESOURCES_H
#endif