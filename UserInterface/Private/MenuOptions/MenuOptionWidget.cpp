// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "MenuOptions/MenuOptionWidget.h"
#include "BSGameModeConfig/BSGameModeValidator.h"
#include "Components/BorderSlot.h"
#include "Components/CheckBox.h"
#include "Components/EditableTextBox.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Spacer.h"
#include "Components/TextBlock.h"
#include "Styles/MenuOptionStyle.h"
#include "Utilities/BSWidgetInterface.h"
#include "Utilities/GameModeCategoryTagWidget.h"
#include "Utilities/TooltipData.h"
#include "Utilities/TooltipIcon.h"


void UMenuOptionWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	SetIndentLevel(IndentLevel);
	SetShowTooltipIcon(bShowDescriptionTooltipIcon);
	SetShowCheckBoxLock(bShowCheckBoxLock);
	SetDescriptionText(DescriptionText);
	SetDescriptionTooltipText(DescriptionTooltipText);

	SetStyling();
}

void UMenuOptionWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (CheckBox_Lock)
	{
		CheckBox_Lock->OnCheckStateChanged.AddUniqueDynamic(this, &ThisClass::OnCheckBox_LockStateChanged);
	}
	SetIndentLevel(IndentLevel);
	SetShowTooltipIcon(bShowDescriptionTooltipIcon);
	SetShowCheckBoxLock(bShowCheckBoxLock);
	SetDescriptionText(DescriptionText);
	SetDescriptionTooltipText(DescriptionTooltipText);

	SetStyling();
}

void UMenuOptionWidget::SetStyling()
{
	MenuOptionStyle = IBSWidgetInterface::GetStyleCDO(MenuOptionStyleClass);

	if (!MenuOptionStyle)
	{
		return;
	}

	if (Box_Left)
	{
		if (UBorderSlot* BorderSlot = Cast<UBorderSlot>(Box_Left->Slot))
		{
			BorderSlot->SetPadding(MenuOptionStyle->Padding_LeftBox);
		}
	}
	if (Box_Right)
	{
		if (UBorderSlot* BorderSlot = Cast<UBorderSlot>(Box_Right->Slot))
		{
			BorderSlot->SetPadding(MenuOptionStyle->Padding_RightBox);
		}
	}
	if (Box_TagsAndTooltips)
	{
		UHorizontalBoxSlot* HorizontalBoxSlot = Cast<UHorizontalBoxSlot>(Box_TagsAndTooltips->Slot);
		if (HorizontalBoxSlot)
		{
			HorizontalBoxSlot->SetPadding(MenuOptionStyle->Padding_TagsAndTooltips);
		}
	}
	if (TextBlock_Description)
	{
		TextBlock_Description->SetFont(MenuOptionStyle->Font_DescriptionText);
		UHorizontalBoxSlot* HorizontalBoxSlot = Cast<UHorizontalBoxSlot>(TextBlock_Description->Slot);
		if (HorizontalBoxSlot)
		{
			HorizontalBoxSlot->SetPadding(MenuOptionStyle->Padding_DescriptionText);
		}
	}
	if (Indent_Left)
	{
		Indent_Left->SetSize(FVector2d(IndentLevel * MenuOptionStyle->IndentAmount, 0.f));
	}
}

void UMenuOptionWidget::SetMenuOptionEnabledState(const EMenuOptionEnabledState EnabledState)
{
	switch (EnabledState)
	{
	case EMenuOptionEnabledState::Enabled:
		SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		Box_Right->SetIsEnabled(true);
		Box_Left->SetIsEnabled(true);
		break;
	case EMenuOptionEnabledState::DependentMissing:
		SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		Box_Right->SetIsEnabled(false);
		Box_Left->SetIsEnabled(false);
		break;
	case EMenuOptionEnabledState::Disabled:
		SetVisibility(ESlateVisibility::Collapsed);
		break;
	}
}

UWidget* UMenuOptionWidget::SetSubMenuOptionEnabledState(const TSubclassOf<UWidget>& SubWidget,
	EMenuOptionEnabledState State)
{
	return nullptr;
}

void UMenuOptionWidget::SetIndentLevel(const int32 Value)
{
	IndentLevel = Value;
}

void UMenuOptionWidget::SetShowTooltipIcon(const bool bShow)
{
	bShowDescriptionTooltipIcon = bShow;

	if (!DescriptionTooltip)
	{
		return;
	}

	if (bShow)
	{
		DescriptionTooltip->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
	else
	{
		DescriptionTooltip->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UMenuOptionWidget::SetShowCheckBoxLock(const bool bShow)
{
	bShowCheckBoxLock = bShow;
	if (!CheckBox_Lock)
	{
		return;
	}
	if (bShow)
	{
		CheckBox_Lock->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
	else
	{
		CheckBox_Lock->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UMenuOptionWidget::SetDescriptionText(const FText& InText)
{
	DescriptionText = InText;
	if (TextBlock_Description)
	{
		TextBlock_Description->SetText(InText);
	}
}

void UMenuOptionWidget::SetDescriptionTooltipText(const FText& InText)
{
	DescriptionTooltipText = InText;
}

UTooltipIcon* UMenuOptionWidget::GetDescriptionTooltipIcon() const
{
	return DescriptionTooltip;
}

bool UMenuOptionWidget::GetIsLocked() const
{
	if (CheckBox_Lock)
	{
		return CheckBox_Lock->IsChecked();
	}
	UE_LOG(LogTemp, Warning, TEXT("Tried to access a CheckBox_Lock that wasn't found in a MenuOptionWidget."));
	return false;
}

void UMenuOptionWidget::SetIsLocked(const bool bLocked) const
{
	if (CheckBox_Lock)
	{
		CheckBox_Lock->SetIsChecked(bLocked);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Tried to access a CheckBox_Lock that wasn't found in a MenuOptionWidget."));
	}
}

void UMenuOptionWidget::OnCheckBox_LockStateChanged(const bool bChecked)
{
	if (OnLockStateChanged.IsBound())
	{
		OnLockStateChanged.Broadcast(this, bChecked);
	}
}

UTooltipIcon* UMenuOptionWidget::AddTooltipIcon(const FUniqueValidationCheckData& Data)
{
	ETooltipIconType Type = ETooltipIconType::Default;
	switch (Data.WarningType)
	{
	case EGameModeWarningType::None:
		break;
	case EGameModeWarningType::Caution:
		Type = ETooltipIconType::Caution;
		break;
	case EGameModeWarningType::Warning:
		Type = ETooltipIconType::Warning;
		break;
	case EGameModeWarningType::Error:
		break;
	}
	UTooltipIcon* TooltipIcon = UTooltipIcon::CreateTooltipIcon(this, Type);
	UHorizontalBoxSlot* HorizontalBoxSlot = TooltipBox->AddChildToHorizontalBox(TooltipIcon);
	HorizontalBoxSlot->SetHorizontalAlignment(HAlign_Right);
	HorizontalBoxSlot->SetPadding(MenuOptionStyle->Padding_DynamicTooltipIcons);
	HorizontalBoxSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
	if (!Data.DynamicStringTableKey.IsEmpty())
	{
		TooltipIcon->GetTooltipData().CreateFormattedText(Data.TooltipText);
	}
	else
	{
		TooltipIcon->GetTooltipData().SetTooltipText(Data.TooltipText);
	}

	DynamicTooltipIcons.Add(GetTypeHash(Data), TooltipIcon);
	return TooltipIcon;
}

void UMenuOptionWidget::GetGameModeCategoryTags(FGameplayTagContainer& OutTags) const
{
	OutTags.AppendTags(GameModeCategoryTags);
}

void UMenuOptionWidget::AddGameModeCategoryTagWidgets(TArray<UGameModeCategoryTagWidget*>& InGameModeCategoryTagWidgets)
{
	InGameModeCategoryTagWidgets.Sort(
		[&](const UGameModeCategoryTagWidget& Widget, const UGameModeCategoryTagWidget& Widget2)
		{
			return Widget.GetGameModeCategoryText().ToString() < Widget2.GetGameModeCategoryText().ToString();
		});
	for (UGameModeCategoryTagWidget* Widget : InGameModeCategoryTagWidgets)
	{
		UHorizontalBoxSlot* BoxSlot = Box_TagWidgets->AddChildToHorizontalBox(Cast<UWidget>(Widget));
		BoxSlot->SetPadding(MenuOptionStyle->Padding_TagWidget);
		BoxSlot->SetHorizontalAlignment(MenuOptionStyle->HorizontalAlignment_TagWidget);
		BoxSlot->SetVerticalAlignment(MenuOptionStyle->VerticalAlignment_TagWidget);
	}
}

void UMenuOptionWidget::UpdateDynamicTooltipIcon(const bool bValidated, const FUniqueValidationCheckData& Data,
	const TArray<int32>& CalculatedValues)
{
	const TObjectPtr<UTooltipIcon>* TooltipIconPtr = DynamicTooltipIcons.Find(Data.Hash);
	TObjectPtr<UTooltipIcon> TooltipIcon = TooltipIconPtr ? TooltipIconPtr->Get() : nullptr;

	if (bValidated)
	{
		if (TooltipIcon)
		{
			DynamicTooltipIcons.Remove(Data.Hash);
			TooltipIcon->RemoveFromParent();
		}
	}
	else
	{
		if (!TooltipIcon)
		{
			TooltipIcon = AddTooltipIcon(Data);
		}
		if (TooltipIcon->GetTooltipData().HasFormattedText())
		{
			TooltipIcon->GetTooltipData().SetFormattedTooltipText(CalculatedValues);
		}
	}
}

int32 UMenuOptionWidget::GetNumberOfDynamicTooltipIcons(const ETooltipIconType Type)
{
	int32 Count = 0;
	for (const auto& [Hash, TooltipIcon] : DynamicTooltipIcons)
	{
		if (TooltipIcon->GetType() == Type)
		{
			Count++;
		}
	}
	return Count;
}
