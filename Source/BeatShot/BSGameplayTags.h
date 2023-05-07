﻿// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"

class UGameplayTagsManager;

/** Singleton containing native gameplay tags */
struct FBSGameplayTags
{
	static const FBSGameplayTags& Get() { return GameplayTags; }
	static void InitializeTags();

	FGameplayTag GameplayCue_Sprint;
	FGameplayTag GameplayCue_MuzzleFlash;
	FGameplayTag GameplayCue_FireGun_Impact;

	FGameplayTag Data;
	FGameplayTag Data_Damage;

	FGameplayTag Event;
	FGameplayTag Event_Montage;
	FGameplayTag Event_Montage_EndAbility;
	FGameplayTag Event_Montage_SpawnProjectile;

	FGameplayTag Input;
	FGameplayTag Input_Move;
	FGameplayTag Input_Move_Forward;
	FGameplayTag Input_Move_Backward;
	FGameplayTag Input_Move_Left;
	FGameplayTag Input_Move_Right;
	FGameplayTag Input_Look;
	FGameplayTag Input_Crouch;
	FGameplayTag Input_Fire;
	FGameplayTag Input_Walk;
	FGameplayTag Input_Sprint;
	FGameplayTag Input_Jump;
	FGameplayTag Input_Interact;
	FGameplayTag Input_ShiftInteract;
	FGameplayTag Input_Disabled;

	FGameplayTag Ability;
	FGameplayTag Ability_Fire;
	FGameplayTag Ability_Track;
	FGameplayTag Ability_Jump;
	FGameplayTag Ability_Sprint;
	FGameplayTag Ability_Crouch;
	FGameplayTag Ability_Interact;
	FGameplayTag Ability_ShiftInteract;

	FGameplayTag State;
	FGameplayTag State_Crouching;
	FGameplayTag State_Firing;
	FGameplayTag State_Jumping;
	FGameplayTag State_Moving;
	FGameplayTag State_Sprinting;
	FGameplayTag State_PlayingBSGameMode;
	
	FGameplayTag State_Weapon_AutomaticFire;
	FGameplayTag State_Weapon_ShowDecals;
	FGameplayTag State_Weapon_ShowTracers;
	FGameplayTag State_Weapon_Recoil;

	FGameplayTag Target;
	FGameplayTag Target_State;
	FGameplayTag Target_State_PreGameModeStart;
	FGameplayTag Target_State_Damageable;
	FGameplayTag Target_State_Immune;
	FGameplayTag Target_State_Grid;
	FGameplayTag Target_State_Single;
	FGameplayTag Target_State_Multi;
	FGameplayTag Target_State_Tracking;

	FGameplayTag Cheat;
	FGameplayTag Cheat_AimBot;
	

protected:
	void AddAllTags(UGameplayTagsManager& Manager);
	void AddTag(FGameplayTag& OutTag, const ANSICHAR* TagName, const ANSICHAR* TagComment);

private:
	static FBSGameplayTags GameplayTags;
};
