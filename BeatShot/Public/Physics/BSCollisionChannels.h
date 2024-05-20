// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"

/**
 * DefaultEngine.ini [/Script/Engine.CollisionProfile] should match with this list
 **/

#define BS_TraceChannel_Projectile					ECC_GameTraceChannel1

#define BS_TraceChannel_Gun					ECC_GameTraceChannel2

// Trace used by weapons, will hit physics assets instead of capsules.
#define BS_TraceChannel_Weapon					ECC_GameTraceChannel3

// Trace used by weapons, will hit pawn capsules instead of physics assets.
#define BS_TraceChannel_Weapon_Capsule					ECC_GameTraceChannel4

// Trace used by weapons, will trace through multiple pawns rather than stopping on the first hit.
#define Lyra_TraceChannel_Weapon_Multi					ECC_GameTraceChannel5
