// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "GameModes/CustomGameModePreviewWidget.h"

#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/SizeBox.h"
#include "Components/Spacer.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBoxSlot.h"
#include "GameModes/BoxBoundsWidget.h"
#include "GameModes/TargetWidget.h"


void UCustomGameModePreviewWidget::SetBoxBounds_Current(const FVector2d& InBounds, const float VerticalOffset) const
{
	BoxBounds_Current->SetBoxBounds(InBounds);
	BoxBounds_Current->SetBoxBoundsPosition(VerticalOffset);
}

void UCustomGameModePreviewWidget::SetBoxBounds_Max(const FVector2d& InBounds, const float VerticalOffset) const
{
	BoxBounds_Max->SetBoxBounds(InBounds);
	BoxBounds_Max->SetBoxBoundsPosition(VerticalOffset);
}

void UCustomGameModePreviewWidget::SetBoxBounds_Min(const FVector2d& InBounds, const float VerticalOffset) const
{
	BoxBounds_Min->SetBoxBounds(InBounds);
	BoxBounds_Min->SetBoxBoundsPosition(VerticalOffset);
}

void UCustomGameModePreviewWidget::SetBoxBoundsVisibility_Max(const ESlateVisibility& InVisibility) const
{
	BoxBounds_Max->SetVisibility(InVisibility);
}

void UCustomGameModePreviewWidget::SetBoxBoundsVisibility_Min(const ESlateVisibility& InVisibility) const
{
	BoxBounds_Min->SetVisibility(InVisibility);
}

void UCustomGameModePreviewWidget::SetStaticBoundsHeight(const float InHeight) const
{
	StaticBounds->SetHeightOverride(InHeight);
}

void UCustomGameModePreviewWidget::SetFloorDistanceHeight(const float InHeight) const
{
	FloorDistance->SetHeightOverride(InHeight);
}

void UCustomGameModePreviewWidget::SetText_FloorDistance(const FText& InText) const
{
	TextBlock_FloorDistance->SetText(InText);
}

void UCustomGameModePreviewWidget::SetOverlayPadding(const FMargin InMargin) const
{
	if (UVerticalBoxSlot* BoxSlot = Cast<UVerticalBoxSlot>(Overlay->Slot))
	{
		TopSpacer->SetSize(FVector2d(0, InMargin.Top));
		FMargin Before = BoxSlot->GetPadding();
		Before.Left = InMargin.Left;
		Before.Right = InMargin.Right;
		BoxSlot->SetPadding(Before);
	}
	if (UVerticalBoxSlot* BoxSlot = Cast<UVerticalBoxSlot>(Overlay_Bottom->Slot))
	{
		FMargin Before = BoxSlot->GetPadding();
		Before.Left = InMargin.Left;
		Before.Right = InMargin.Right;
		BoxSlot->SetPadding(Before);
	}
}

float UCustomGameModePreviewWidget::GetSpacerOffset() const
{
	if (TopSpacer)
	{
		return TopSpacer->GetSize().Y;
	}
	return 0.f;
}

UTargetWidget* UCustomGameModePreviewWidget::ConstructTargetWidget()
{
	UTargetWidget* TargetWidget = CreateWidget<UTargetWidget>(this, TargetWidgetClass);
	UOverlaySlot* OverlaySlot = Overlay->AddChildToOverlay(TargetWidget);
	OverlaySlot->SetHorizontalAlignment(HAlign_Center);
	OverlaySlot->SetVerticalAlignment(VAlign_Center);
	return TargetWidget;
}

void UCustomGameModePreviewWidget::ToggleGameModePreview(const bool bEnable) const
{
	if (bEnable)
	{
		if (BoxBounds_Current)
		{
			BoxBounds_Current->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
		if (BoxBounds_Max)
		{
			BoxBounds_Max->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
		if (BoxBounds_Min)
		{
			BoxBounds_Min->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
		if (TextBlock_GameModePreviewAvailability)
		{
			TextBlock_GameModePreviewAvailability->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
	else
	{
		if (BoxBounds_Current)
		{
			BoxBounds_Current->SetVisibility(ESlateVisibility::Collapsed);
		}
		if (BoxBounds_Max)
		{
			BoxBounds_Max->SetVisibility(ESlateVisibility::Collapsed);
		}
		if (BoxBounds_Min)
		{
			BoxBounds_Min->SetVisibility(ESlateVisibility::Collapsed);
		}
		if (TextBlock_GameModePreviewAvailability)
		{
			TextBlock_GameModePreviewAvailability->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
	}
}
