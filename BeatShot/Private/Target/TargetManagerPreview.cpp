// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "Target/TargetManagerPreview.h"
#include "Target/SpawnAreaManagerComponent.h"
#include "Target/TargetPreview.h"


ATargetManagerPreview::ATargetManagerPreview()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ATargetManagerPreview::InitBoxBoundsWidget(const TObjectPtr<UCustomGameModePreviewWidget> InGameModePreviewWidget)
{
	GameModePreviewWidget = InGameModePreviewWidget;
}

void ATargetManagerPreview::SetSimulatePlayerDestroyingTargets(const bool bInSimulatePlayerDestroyingTargets,
	const float InDestroyChance)
{
	bSimulatePlayerDestroyingTargets = bInSimulatePlayerDestroyingTargets;
	DestroyChance = InDestroyChance;
}

ATarget* ATargetManagerPreview::SpawnTarget(const FTargetSpawnParams& Params)
{
	ATarget* Target = Super::SpawnTarget(Params);
	if (Target)
	{
		if (ATargetPreview* TargetPreview = Cast<ATargetPreview>(Target))
		{
			if (UTargetWidget* TargetWidget = GameModePreviewWidget->ConstructTargetWidget())
			{
				const float Height = StaticExtents.Z + (BSConfig->TargetConfig.FloorDistance - ClampedOverflowAmount) *
					0.5f + GameModePreviewWidget->GetSpacerOffset() * 0.5f;
				TargetPreview->InitTargetWidget(TargetWidget, TargetPreview->GetActorLocation(), Height);
				TargetPreview->SetSimulatePlayerDestroying(bSimulatePlayerDestroyingTargets, DestroyChance);
			}
		}
	}
	return Target;
}

void ATargetManagerPreview::UpdateSpawnVolume(const float Factor) const
{
	Super::UpdateSpawnVolume(Factor);

	if (!BSConfig || !GameModePreviewWidget)
	{
		return;
	}

	if (BSConfig->TargetConfig.FloorDistance > MaxAllowedFloorDistance)
	{
		ClampedOverflowAmount = MaxAllowedFloorDistance - BSConfig->TargetConfig.FloorDistance;
		if (!bIsExceedingMaxFloorDistance)
		{
			GameModePreviewWidget->SetText_FloorDistance(FloorDistanceExceededText);
		}
		bIsExceedingMaxFloorDistance = true;
	}
	else
	{
		ClampedOverflowAmount = 0.f;
		if (bIsExceedingMaxFloorDistance)
		{
			GameModePreviewWidget->SetText_FloorDistance(FloorDistanceText);
		}
		bIsExceedingMaxFloorDistance = false;
	}

	GameModePreviewWidget->SetFloorDistanceHeight(FMath::Clamp(BSConfig->TargetConfig.FloorDistance, 110.f,
		MaxAllowedFloorDistance));

	const float MaxTargetRadius = GetMaxTargetRadius(BSConfig->TargetConfig);
	GameModePreviewWidget->SetOverlayPadding(FMargin(MaxTargetRadius, MaxTargetRadius, MaxTargetRadius, 0.f));

	const FVector Origin = GetSpawnBoxOrigin();
	const FVector Extents = GetSpawnBoxExtents();

	// Set the Current box bounds widget size and position
	const float CurrentY = Extents.Y * 2.f;
	const float CurrentZ = Extents.Z * 2.f;
	float Height = Origin.Z - Extents.Z + ClampedOverflowAmount;

	GameModePreviewWidget->SetBoxBounds_Current(FVector2d(CurrentY, CurrentZ), Height);

	if (BSConfig->TargetConfig.BoundsScalingPolicy == EBoundsScalingPolicy::Dynamic)
	{
		const FVector StartExtents = BSConfig->DynamicSpawnAreaScaling.GetStartExtents();
		const FIntVector3 Inc = SpawnAreaManager->GetSpawnAreaDimensions();

		// Set the Min/Start box bounds widget size and position
		const float StartZ = BSConfig->TargetConfig.TargetDistributionPolicy ==
			ETargetDistributionPolicy::HeadshotHeightOnly
			? Constants::HeadshotHeight_VerticalSpread
			: FMath::GridSnap(StartExtents.Z, Inc.Z) * 2.f;
		const float StartY = FMath::GridSnap(StartExtents.Y, Inc.Y) * 2.f;
		Height = Origin.Z - (StartZ / 2.f) + ClampedOverflowAmount;
		GameModePreviewWidget->SetBoxBounds_Min(FVector2d(StartY, StartZ), Height);

		// Set the Max/End box bounds widget size and position
		const float EndZ = BSConfig->TargetConfig.TargetDistributionPolicy ==
			ETargetDistributionPolicy::HeadshotHeightOnly
			? Constants::HeadshotHeight_VerticalSpread
			: FMath::GridSnap(StaticExtents.Z, Inc.Z) * 2.f;
		const float EndY = FMath::GridSnap(StaticExtents.Y, Inc.Y) * 2.f;
		Height = Origin.Z - (EndZ / 2.f) + ClampedOverflowAmount;
		GameModePreviewWidget->SetBoxBounds_Max(FVector2d(EndY, EndZ), Height);

		// Adjust visibility and Preview Widget height
		GameModePreviewWidget->SetBoxBoundsVisibility_Min(ESlateVisibility::SelfHitTestInvisible);
		GameModePreviewWidget->SetBoxBoundsVisibility_Max(ESlateVisibility::SelfHitTestInvisible);
		GameModePreviewWidget->SetStaticBoundsHeight(FMath::Max(StartZ, EndZ));
	}
	else
	{
		GameModePreviewWidget->SetBoxBoundsVisibility_Min(ESlateVisibility::Collapsed);
		GameModePreviewWidget->SetBoxBoundsVisibility_Max(ESlateVisibility::Collapsed);
		GameModePreviewWidget->SetStaticBoundsHeight(CurrentZ);
	}
}

void ATargetManagerPreview::DeactivateTarget(ATarget* InTarget, const bool bExpired, const bool bOutOfHealth) const
{
	Super::DeactivateTarget(InTarget, bExpired, bOutOfHealth);

	// Hide target
	if (InTarget && BSConfig->TargetConfig.TargetDeactivationResponses.
	                          Contains(ETargetDeactivationResponse::HideTarget))
	{
		if (const ATargetPreview* TargetPreview = Cast<ATargetPreview>(InTarget))
		{
			TargetPreview->SetTargetWidgetOpacity(0.2f);
		}
	}
}
