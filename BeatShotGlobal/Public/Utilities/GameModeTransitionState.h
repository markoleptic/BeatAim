// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BSGameModeConfig/BSConfig.h"
#include "GameModeTransitionState.generated.h"

/** The transition state describing the start state and end state of a transition. */
UENUM(BlueprintType)
enum class ETransitionState : uint8
{
	StartFromMainMenu UMETA(DisplayName="StartFromMainMenu"),
	StartFromPostGameMenu UMETA(DisplayName="StartFromPostGameMenu"),
	Restart UMETA(DisplayName="Restart"),
	QuitToMainMenu UMETA(DisplayName="QuitToMainMenu"),
	QuitToDesktop UMETA(DisplayName="QuitToDesktop"),
	PlayAgain UMETA(DisplayName="PlayAgain"),
	None UMETA(DisplayName="None")};

ENUM_RANGE_BY_FIRST_AND_LAST(ETransitionState, ETransitionState::StartFromMainMenu, ETransitionState::None);

/** Information about the transition state of the game. */
struct FGameModeTransitionState
{
	/** The game mode transition to perform. */
	ETransitionState TransitionState;

	/** Whether to save current scores if the transition is Restart or Quit. */
	bool bSaveCurrentScores;

	/** The game mode properties, only used if Start or Restart. */
	FBSConfig BSConfig;
};
