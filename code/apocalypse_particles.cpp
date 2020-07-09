#include "apocalypse_particles.h"
#include "apocalypse_memory_arena.h"
#include "apocalypse_random.h"

#include <math.h>

vector2 RandomizedVector2(vector2 Min, vector2 Max)
{
	return Vector2(RandFloat(Min.X, Max.X), RandFloat(Min.Y, Max.Y));
}

vector4 RandomizedVector4(vector4 Min, vector4 Max)
{
	return Vector4(
		RandFloat(Min.X, Max.X),
		RandFloat(Min.Y, Max.Y),
		RandFloat(Min.Z, Max.Z),
		RandFloat(Min.W, Max.W)
	);
}

particle_system MakeParticleSystem(
	memory_arena* Arena,
	bitmap_handle_e ParticleBitmap,
	float SpawnsPerFrame,
	vector2 DimMin,
	vector2 DimMax,
	vector2 SpawnMin,
	vector2 SpawnMax,
	float EmissionAngleMin,
	float EmissionAngleMax,
	float EmissionSpeedMin,
	float EmissionSpeedMax,
	vector4 InitColorMin,
	vector4 InitColorMax,
	vector4 DColorMin,
	vector4 DColorMax,
	float FadeInRate,
	uint32_t ParticleCount
)
{
	particle_system Result = {};
	Result = {};
	Result.ParticleBitmap = ParticleBitmap;
	
	Result.IntSpawnsPerFrame = (int) SpawnsPerFrame;
	Result.FracSpawnsPerFrame = SpawnsPerFrame - Result.IntSpawnsPerFrame;
	Result.RemainderSum = 0.0f;

	Result.DimMin = DimMin;
	Result.DimMax = DimMax;
	Result.SpawnMin = SpawnMin;
	Result.SpawnMax = SpawnMax;
	Result.EmissionAngleMin = EmissionAngleMin;
	Result.EmissionAngleMax = EmissionAngleMax;
	Result.EmissionSpeedMin = EmissionSpeedMin;
	Result.EmissionSpeedMax = EmissionSpeedMax;
	Result.InitColorMin = InitColorMin;
	Result.InitColorMax = InitColorMax;
	Result.DColorMin = DColorMin;
	Result.DColorMax = DColorMax;
	Result.FadeInRate = FadeInRate;
	Result.ParticleCount = ParticleCount;
	Result.Particles = PushArray(
		Arena, Result.ParticleCount, particle
	);
	// NOTE: this loop can be removed for speed so long as 
	// CONT: ParticleState->Inactive is 0
	particle* Particle = &Result.Particles[0];
	for(uint32_t Index = 0; Index < Result.ParticleCount; Index++)
	{
		Particle->State = ParticleState_Inactive;
		Particle++;
	}
	return Result;
}

void UpdateParticleSystem(particle_system* ParticleSystem)
{
	// SECTION START: spawn new particles
	uint32_t ParticlesToSpawn = ParticleSystem->IntSpawnsPerFrame;
	ParticleSystem->RemainderSum += ParticleSystem->FracSpawnsPerFrame;
	if(ParticleSystem->RemainderSum >= 1.0f)
	{
		ParticleSystem->RemainderSum -= 1.0f;
		ParticlesToSpawn += 1;
	}

	particle* Particle = &ParticleSystem->Particles[0];
	for(uint32_t Index = 0; Index < ParticleSystem->ParticleCount; Index++)
	{
		if(Particle->State == ParticleState_Inactive)
		{
			Particle->State = ParticleState_FadingIn;
			Particle->Pos = RandomizedVector2(
				ParticleSystem->SpawnMin, ParticleSystem->SpawnMax
			);
			float Speed = RandFloat(
				ParticleSystem->EmissionSpeedMin,
				ParticleSystem->EmissionSpeedMax
			);
			float Angle = RandFloat(
				ParticleSystem->EmissionAngleMin,
				ParticleSystem->EmissionAngleMax
			);
			Particle->Velocity = Speed * Vector2(cosf(Angle), sinf(Angle));			

			Particle->Dim = RandomizedVector2(
				ParticleSystem->DimMin, ParticleSystem->DimMax
			);

			Particle->Color = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
			Particle->DColor = RandomizedVector4(
				ParticleSystem->DColorMin, ParticleSystem->DColorMax
			);
			Particle->ColorTarget = RandomizedVector4(
				ParticleSystem->InitColorMin, ParticleSystem->InitColorMax
			);

			ParticlesToSpawn--;
		}
		if(ParticlesToSpawn <= 0)
		{
			break;
		}
		Particle++;
	}
	// SECTION STOP: spawn new particles

	// SECTION START: update old particles
	Particle = &ParticleSystem->Particles[0];
	for(uint32_t Index = 0; Index < ParticleSystem->ParticleCount; Index++)
	{
		switch(Particle->State)
		{
			case(ParticleState_FadingIn):
			{
				Particle->Pos += Particle->Velocity;
				Particle->Color += (
					ParticleSystem->FadeInRate * Particle->ColorTarget
				);
				
				Particle->Color.R = Clamp01(Particle->Color.R); 
				Particle->Color.G = Clamp01(Particle->Color.G); 
				Particle->Color.B = Clamp01(Particle->Color.B); 
				Particle->Color.A = Clamp01(Particle->Color.A); 

				if(Particle->Color.A >= Particle->ColorTarget.A)
				{
					Particle->ColorTarget = Vector4(
						Particle->Color.R,
						Particle->Color.G, 
						Particle->Color.B, 
						0.0f  
					);
					Particle->State = ParticleState_FadingOut;
				}
				break;
			}
			case(ParticleState_FadingOut):
			{
				Particle->Pos += Particle->Velocity;
				Particle->Color += Particle->DColor;

				Particle->Color.R = Clamp01(Particle->Color.R); 
				Particle->Color.G = Clamp01(Particle->Color.G); 
				Particle->Color.B = Clamp01(Particle->Color.B); 
				Particle->Color.A = Clamp01(Particle->Color.A); 

				if(Particle->Color.A <= Particle->ColorTarget.A)
				{
					Particle->State = ParticleState_Inactive;
				}
				break;
			}
			case(ParticleState_Inactive):
			{
				break;
			}	
		}
		Particle++;
	}
	// SECTION STOP: update old particles
}