#ifndef APOCALYPSE_PLAYER_ID_H

typedef enum
{
	Player_One,
	Player_Two,
	Player_Count
} player_id;

typedef enum
{
	RelativePlayer_Self,
	RelativePlayer_Opp,
	RelativePlayer_Count
} relative_player_id;

#define APOCALYPSE_PLAYER_ID_H
#endif