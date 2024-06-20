// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TargetManager.h"
#include "GameFramework/Actor.h"
#include "GameModes/CustomGameModePreviewWidget.h"
#include "Utilities/BSWidgetInterface.h"
#include "TargetManagerPreview.generated.h"

UCLASS()
class BEATSHOT_API ATargetManagerPreview : public ATargetManager
{
	GENERATED_BODY()

public:
	ATargetManagerPreview();

	/** Initializes the BoxBounds widget. */
	void InitBoxBoundsWidget(const TObjectPtr<UCustomGameModePreviewWidget> InGameModePreviewWidget);

	/** Sets the values of bSimulatePlayerDestroying and DestroyChance. */
	void SetSimulatePlayerDestroyingTargets(const bool bInSimulatePlayerDestroyingTargets,
		const float InDestroyChance = 1.f);

	/** Whether to tell spawned targets to artificially destroy themselves early, simulating a player destroying it. */
	bool bSimulatePlayerDestroyingTargets = false;

	/** The chance that the target should simulate a player destroying it. */
	float DestroyChance = 1.f;

protected:
	/** Generic spawn function that all game modes use to spawn a target. Initializes the target, binds to its
	 * delegates, sets the InSpawnArea's Guid, and adds the target to ManagedTargets. */
	virtual ATarget* SpawnTarget(const FTargetSpawnParams& Params) override;

	/** Updates the SpawnVolume and all directional boxes to match the current SpawnBox. */
	virtual void UpdateSpawnVolume(const float Factor) const override;

	/** Calls DeactivateTarget and Executes any deactivation responses to the target being deactivated. */
	virtual void DeactivateTarget(ATarget* InTarget, const bool bExpired, const bool bOutOfHealth) const override;

	UPROPERTY()
	TObjectPtr<UCustomGameModePreviewWidget> GameModePreviewWidget;

	/** Text to show when the max allowed floor distance has not been exceeded. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="TargetManagerPreview")
	FText FloorDistanceText = IBSWidgetInterface::GetWidgetTextFromKey("GM_FloorDistance");

	/** Text to show when the max allowed floor distance has been exceeded. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="TargetManagerPreview")
	FText FloorDistanceExceededText = IBSWidgetInterface::GetWidgetTextFromKey("GM_FloorDistance_Overflow");

	/** Whether the height of the total spawn area is exceeding the max allowed floor distance. */
	mutable bool bIsExceedingMaxFloorDistance = false;

	/** The max allowed floor distance. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="TargetManagerPreview")
	float MaxAllowedFloorDistance = 300.f;

	/** The amount of overflow floor distance. */
	mutable float ClampedOverflowAmount = 0.f;
};
