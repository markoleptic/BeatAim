// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "Utilities/ComboBox/BSComboBoxEntry.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Styles/MenuOptionStyle.h"
#include "Utilities/BSWidgetInterface.h"
#include "Utilities/TooltipIcon.h"

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

void UBSComboBoxEntry::SetTooltipIconVisibility(const bool bIsVisible) const
{
	if (bIsVisible && !bAlwaysHideTooltipIcon)
	{
		TooltipIcon->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
	else
	{
		TooltipIcon->SetVisibility(ESlateVisibility::Collapsed);
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
