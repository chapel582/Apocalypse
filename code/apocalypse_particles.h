#ifndef APOCALYPSE_PARTICLES_H

typedef enum
{
	ParticleState_Inactive,
	ParticleState_FadingIn,
	ParticleState_FadingOut
} particle_state;

struct particle
{
	particle_state State;
	vector2 Pos;
	vector2 Velocity; // NOTE: movement per frame
	// TODO: acceleration?
	vector2 Dim;
	vector4 Color;
	vector4 DColor; // NOTE: change in color
	vector4 ColorTarget; // NOTE: when done fading in or fading out
};

struct particle_system
{
	bitmap_handle ParticleBitmap;

	uint32_t IntSpawnsPerFrame;
	float FracSpawnsPerFrame;
	float RemainderSum;

	vector2 DimMin;
	vector2 DimMax;
	vector2 SpawnMin;
	vector2 SpawnMax;
	float EmissionAngleMin;
	float EmissionAngleMax;
	float EmissionSpeedMin;
	float EmissionSpeedMax;
	vector4 InitColorMin;
	vector4 InitColorMax;
	vector4 DColorMin; // NOTE: DColor.A should always be negative
	vector4 DColorMax;
	float FadeInRate; // NOTE: 0.25f takes four frames to fade in, 1.0f takes one 

	uint32_t ParticleCount;
	particle* Particles;
};

#define APOCALYPSE_PARTICLES_H
#endif