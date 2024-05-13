// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "Utilities/InputMappingWidget.h"

#include "Utilities/BSWidgetInterface.h"
#include "Components/BorderSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/InputKeySelector.h"
#include "Components/TextBlock.h"
#include "Utilities/Buttons/BSButton.h"
#include "Styles/MenuOptionStyle.h"

void UInputMappingWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetStyles();

	InputKeySelectorSlot1->OnIsSelectingKeyChanged.AddDynamic(this, &ThisClass::OnIsSelectingKeyChanged_Slot1);
	InputKeySelectorSlot2->OnIsSelectingKeyChanged.AddDynamic(this, &ThisClass::OnIsSelectingKeyChanged_Slot2);

	InputKeySelectorSlot1->OnKeySelected.AddDynamic(this, &ThisClass::OnKeySelected_Slot1);
	InputKeySelectorSlot2->OnKeySelected.AddDynamic(this, &ThisClass::OnKeySelected_Slot2);
}

void UInputMappingWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
	SetStyles();
}

void UInputMappingWidget::SetStyles()
{
	MenuOptionStyle = IBSWidgetInterface::GetStyleCDO(MenuOptionStyleClass);
	if (MenuOptionStyle)
	{
		TextBlock_KeyDescription->SetFont(MenuOptionStyle->Font_DescriptionText);
		UHorizontalBoxSlot* HorizontalBoxSlot = Cast<UHorizontalBoxSlot>(TextBlock_KeyDescription->Slot);
		if (HorizontalBoxSlot)
		{
			HorizontalBoxSlot->SetPadding(MenuOptionStyle->Padding_DescriptionText);
		}
		if (Box_Left)
		{
			UBorderSlot* BorderSlot = Cast<UBorderSlot>(Box_Left->Slot);
			if (BorderSlot)
			{
				BorderSlot->SetPadding(MenuOptionStyle->Padding_LeftBox);
			}
		}
		FTextBlockStyle TextBlockStyle = InputKeySelectorSlot1->GetTextStyle();
		TextBlockStyle.SetFont(MenuOptionStyle->Font_EditableTextBesideSlider);
		InputKeySelectorSlot1->SetTextStyle(TextBlockStyle);
		InputKeySelectorSlot2->SetTextStyle(TextBlockStyle);
	}
}

void UInputMappingWidget::SetMappingName(const FName& InMappingName, const FText& DisplayName)
{
	MappingName = InMappingName;
	TextBlock_KeyDescription->SetText(DisplayName);
}

void UInputMappingWidget::SetKeyForSlot(const EPlayerMappableKeySlot& InSlot, const FKey& InKey)
{
	if (InSlot == EPlayerMappableKeySlot::First)
	{
		InputKeySelectorSlot1->SetSelectedKey(InKey);
	}
	else if (InSlot == EPlayerMappableKeySlot::Second)
	{
		InputKeySelectorSlot2->SetSelectedKey(InKey);
	}
}

TSet<EPlayerMappableKeySlot> UInputMappingWidget::GetSlotsFromKey(const FKey& InKey) const
{
	TSet<EPlayerMappableKeySlot> Out;
	if (InputKeySelectorSlot1->GetSelectedKey().Key == InKey)
	{
		Out.Add(EPlayerMappableKeySlot::First);
	}
	if (InputKeySelectorSlot2->GetSelectedKey().Key == InKey)
	{
		Out.Add(EPlayerMappableKeySlot::Second);
	}
	return Out;
}

FInputChord UInputMappingWidget::GetKeyFromSlot(const EPlayerMappableKeySlot& InSlot) const
{
	if (InSlot == EPlayerMappableKeySlot::First)
	{
		return InputKeySelectorSlot1->GetSelectedKey();
	}
	if (InSlot == EPlayerMappableKeySlot::Second)
	{
		return InputKeySelectorSlot2->GetSelectedKey();
	}
	return FInputChord();
}

void UInputMappingWidget::OnIsSelectingKeyChanged_Slot1()
{
	OnIsSelectingKey.Broadcast(InputKeySelectorSlot1);
}

void UInputMappingWidget::OnIsSelectingKeyChanged_Slot2()
{
	OnIsSelectingKey.Broadcast(InputKeySelectorSlot2);
}

void UInputMappingWidget::OnKeySelected_Slot1(FInputChord SelectedKey)
{
	OnKeySelected.Broadcast(GetMappingName(), EPlayerMappableKeySlot::First, SelectedKey);
}

void UInputMappingWidget::OnKeySelected_Slot2(FInputChord SelectedKey)
{
	OnKeySelected.Broadcast(GetMappingName(), EPlayerMappableKeySlot::Second, SelectedKey);
}
