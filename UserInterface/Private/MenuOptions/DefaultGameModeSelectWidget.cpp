// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "MenuOptions/DefaultGameModeSelectWidget.h"

#include "CommonTextBlock.h"

void UDefaultGameModeSelectWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UDefaultGameModeSelectWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
}

void UDefaultGameModeSelectWidget::SetStyling()
{
	Super::SetStyling();
}

void UDefaultGameModeSelectWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
	AltDescriptionText->SetScrollingEnabled(true);
}

void UDefaultGameModeSelectWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
	AltDescriptionText->SetScrollingEnabled(false);
}

void UDefaultGameModeSelectWidget::SetAltDescriptionText(const FText& Text)
{
	AltDescriptionText->SetText(Text);
}
