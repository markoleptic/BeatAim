﻿// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "WidgetComponents/Boxes/BSComboBoxEntry.h"
#include "BSWidgetInterface.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "WidgetComponents/TooltipImage.h"
#include "Styles/MenuOptionStyle.h"

void UBSComboBoxEntry::NativePreConstruct()
{
	Super::NativePreConstruct();
	SetStyles();
}

void UBSComboBoxEntry::NativeConstruct()
{
	Super::NativeConstruct();
	SetStyles();
}

void UBSComboBoxEntry::SetStyles()
{
	MenuOptionStyle = IBSWidgetInterface::GetStyleCDO(MenuOptionStyleClass);
	if (MenuOptionStyle)
	{
		TextBlock_Entry->SetFont(MenuOptionStyle->Font_DescriptionText);
	}
}

void UBSComboBoxEntry::SetEntryText(const FText& InText) const
{
	TextBlock_Entry->SetText(InText);
}

FText UBSComboBoxEntry::GetEntryText() const
{
	return TextBlock_Entry->GetText();
}

void UBSComboBoxEntry::SetTooltipImageVisibility(const bool bIsVisible) const
{
	if (bIsVisible && !bAlwaysHideTooltipImage)
	{
		TooltipImage->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
	else
	{
		TooltipImage->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UBSComboBoxEntry::SetBackgroundBrushTint(const FLinearColor& Color) const
{
	Background->SetBrushColor(Color);
}

FString UBSComboBoxEntry::GetEntryTextAsString() const
{
	return TextBlock_Entry->GetText().ToString();
}
