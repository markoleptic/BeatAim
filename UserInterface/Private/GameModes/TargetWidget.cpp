// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "GameModes/TargetWidget.h"
#include "BSConstants.h"
#include "Components/Image.h"
#include "Components/OverlaySlot.h"

void UTargetWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (TargetImage && TargetImage->GetDynamicMaterial())
	{
		TargetImageMaterial = TargetImage->GetDynamicMaterial();
	}
}

void UTargetWidget::SetTargetScale(const FVector& NewScale) const
{
	if (TargetImage)
	{
		TargetImage->SetDesiredSizeOverride(FVector2d(Constants::SphereTargetDiameter * NewScale.X,
			Constants::SphereTargetDiameter * NewScale.Y));
	}
}

void UTargetWidget::SetTargetColor(const FLinearColor& Color) const
{
	if (TargetImageMaterial)
	{
		TargetImageMaterial->SetVectorParameterValue(FName("TargetColor"), Color);
	}
}

void UTargetWidget::SetTargetPosition(const FVector2d& InPosition) const
{
	if (TargetImage)
	{
		const FMargin NewPadding(InPosition.X, 0, 0, InPosition.Y);
		if (UOverlaySlot* OverlaySlot = Cast<UOverlaySlot>(Slot))
		{
			OverlaySlot->SetPadding(NewPadding);
		}
	}
}
