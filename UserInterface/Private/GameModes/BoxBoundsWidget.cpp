// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "GameModes/BoxBoundsWidget.h"
#include "Components/OverlaySlot.h"
#include "Components/SizeBox.h"

void UBoxBoundsWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UBoxBoundsWidget::SetBoxBounds(const FVector2d& InBounds) const
{
	BoxBounds->SetWidthOverride(InBounds.X);
	BoxBounds->SetHeightOverride(InBounds.Y);
}

void UBoxBoundsWidget::SetBoxBoundsPosition(const float VerticalOffset) const
{
	if (UOverlaySlot* OverlaySlot = Cast<UOverlaySlot>(Slot))
	{
		OverlaySlot->SetPadding(FMargin(0.f, 0.f, 0.f, VerticalOffset));
	}
}
