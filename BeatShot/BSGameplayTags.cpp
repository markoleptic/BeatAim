// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#include "BSGameplayTags.h"


namespace BSGameplayTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability, "Ability", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_Fire, "Ability.Fire", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_Inspect, "Ability.Inspect", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_KnifeAttack, "Ability.KnifeAttack", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_BackStab, "Ability.BackStab", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_Track, "Ability.Track", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_Jump, "Ability.Jump", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_Sprint, "Ability.Sprint", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_Crouch, "Ability.Crouch", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_Interact, "Ability.Interact", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ShiftInteract, "Ability.ShiftInteract", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_InputBlocked, "Ability.InputBlocked", "");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AnimEffect, "AnimEffect", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AnimEffect_Footstep, "AnimEffect.Footstep", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AnimEffect_Footstep_Jog, "AnimEffect.Footstep.Jog", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AnimEffect_Footstep_Land, "AnimEffect.Footstep.Land", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(AnimEffect_Footstep_Walk, "AnimEffect.Footstep.Walk", "");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Cheat, "Cheat", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Cheat_AimBot, "Cheat.AimBot", "");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Data, "Data", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Data_Cooldown, "Data.Cooldown", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Data_Damage, "Data.Damage", "");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Equipped, "Equipped", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Equipped_None, "Equipped.None", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Equipped_Gun, "Equipped.Gun", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Equipped_Knife, "Equipped.Knife", "");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Event, "Event", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Event_Montage, "Event.Montage", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Event_Montage_EndAbility, "Event.Montage.EndAbility", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Event_Montage_SpawnProjectile, "Event.Montage.SpawnProjectile", "");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameModeCategory, "GameModeCategory", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameModeCategory_Spawning, "GameModeCategory.Spawning", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameModeCategory_Activation, "GameModeCategory.Activation", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameModeCategory_Deactivation, "GameModeCategory.Deactivation", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameModeCategory_Destruction, "GameModeCategory.Destruction", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameModeCategory_SpawnArea, "GameModeCategory.SpawnArea", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameModeCategory_Scale, "GameModeCategory.Scale", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameModeCategory_Movement, "GameModeCategory.Movement", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameModeCategory_Position, "GameModeCategory.Position", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameModeCategory_Velocity, "GameModeCategory.Velocity", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameModeCategory_Direction, "GameModeCategory.Direction", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameModeCategory_Health, "GameModeCategory.Health", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameModeCategory_Damage, "GameModeCategory.Damage", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameModeCategory_Time, "GameModeCategory.Time", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameModeCategory_Dynamic, "GameModeCategory.Dynamic", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameModeCategory_Target, "GameModeCategory.Target", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameModeCategory_AI, "GameModeCategory.AI", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameModeCategory_Grid, "GameModeCategory.Grid", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameModeCategory_Effects, "GameModeCategory.Effects", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameModeCategory_Static, "GameModeCategory.Static", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameModeCategory_MultiSelect, "GameModeCategory.MultiSelect", "");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayCue_Sprint, "GameplayCue.Sprint", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayCue_MuzzleFlash, "GameplayCue.MuzzleFlash", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayCue_FireGun_Impact, "GameplayCue.FireGun.Impact", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayCue_TrackGun_Hit, "GameplayCue.TrackGun.Hit", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayCue_TrackGun_Miss, "GameplayCue.TrackGun.Miss", "");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input, "Input", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_Move, "Input.Move", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_Move_Forward, "Input.Move.Forward", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_Move_Backward, "Input.Move.Backward", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_Move_Left, "Input.Move.Left", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_Move_Right, "Input.Move.Right", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_Look, "Input.Look", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_Crouch, "Input.Crouch", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_Fire, "Input.Fire", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_Sprint, "Input.Sprint", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_Walk, "Input.Walk", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_Jump, "Input.Jump", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_Interact, "Input.Interact", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_ShiftInteract, "Input.ShiftInteract", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_Inspect, "Input.Inspect", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_KnifeAttack, "Input.KnifeAttack", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_BackStab, "Input.BackStab", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_EquipmentSlot, "Input.EquipmentSlot", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_EquipmentSlot_1, "Input.EquipmentSlot.1", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_EquipmentSlot_2, "Input.EquipmentSlot.2", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_EquipmentSlot_3, "Input.EquipmentSlot.3", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_EquipmentSlot_LastEquipped, "Input.EquipmentSlot.LastEquipped", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_Pause, "Input.Pause", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Input_LeftClick, "Input.LeftClick", "");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State, "State", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Crouching, "State.Crouching", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Firing, "State.Firing", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Recoiling, "State.Recoiling", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Tracking, "State.Tracking", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Inspecting, "State.Inspecting", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_KnifeAttacking, "State.KnifeAttacking", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_BackStabbing, "State.BackStabbing", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Jumping, "State.Jumping", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Moving, "State.Moving", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Sprinting, "State.Sprinting", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_PlayingBSGameMode, "State.PlayingBSGameMode", "");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Weapon_AutomaticFire, "State.Weapon.AutomaticFire", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Weapon_ShowDecals, "State.Weapon.ShowDecals", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Weapon_ShowTracers, "State.Weapon.ShowTracers", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Weapon_ShowMuzzleFlash, "State.Weapon.ShowMuzzleFlash", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Weapon_ShowMesh, "State.Weapon.ShowMesh", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Weapon_Recoil, "State.Weapon.Recoil", "");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(SurfaceType, "SurfaceType", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(SurfaceType_Character, "SurfaceType.Character", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(SurfaceType_Concrete, "SurfaceType.Concrete", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(SurfaceType_Default, "SurfaceType.Default", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(SurfaceType_Dirt, "SurfaceType.Dirt", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(SurfaceType_Glass, "SurfaceType.Glass", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(SurfaceType_Grass, "SurfaceType.Grass", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(SurfaceType_Gravel, "SurfaceType.Gravel", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(SurfaceType_Ice, "SurfaceType.Ice", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(SurfaceType_Metal, "SurfaceType.Metal", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(SurfaceType_Mud, "SurfaceType.Mud", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(SurfaceType_Sand, "SurfaceType.Sand", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(SurfaceType_Snow, "SurfaceType.Snow", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(SurfaceType_Water, "SurfaceType.Water", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(SurfaceType_Wood, "SurfaceType.Wood", "");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Target, "Target", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Target_State, "Target.State", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Target_State_PreGameModeStart, "Target.State.PreGameModeStart", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Target_State_Immune, "Target.State.Immune", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Target_State_Immune_TrackingDamage, "Target.State.Immune.Tracking", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Target_State_Immune_HitDamage, "Target.State.Immune.FireGun", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Target_State_Grid, "Target.State.Grid", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Target_State_Single, "Target.State.Single", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Target_State_Multi, "Target.State.Multi", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Target_State_Tracking, "Target.State.Tracking", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Target_State_Charged, "Target.State.Charged", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Target_ResetHealth, "Target.ResetHealth", "");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Target_TreatAsExternalDamage, "Target.TreatAsExternalDamage", "");
}
