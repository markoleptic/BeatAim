// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "MenuOptions/MenuOptionWidget.h"
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
#include "Utilities/TooltipImage.h"


void UMenuOptionWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	SetIndentLevel(IndentLevel);
	SetShowTooltipImage(bShowTooltipImage);
	SetShowCheckBoxLock(bShowCheckBoxLock);
	SetDescriptionText(DescriptionText);
	SetTooltipText(DescriptionTooltipText);

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
	SetShowTooltipImage(bShowTooltipImage);
	SetShowCheckBoxLock(bShowCheckBoxLock);
	SetDescriptionText(DescriptionText);
	SetTooltipText(DescriptionTooltipText);

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

void UMenuOptionWidget::SetIndentLevel(const int32 Value)
{
	IndentLevel = Value;
}

void UMenuOptionWidget::SetShowTooltipImage(const bool bShow)
{
	bShowTooltipImage = bShow;

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

void UMenuOptionWidget::SetTooltipText(const FText& InText)
{
	DescriptionTooltipText = InText;
}

UTooltipImage* UMenuOptionWidget::GetTooltipImage() const
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

FUpdateTooltipState& UMenuOptionWidget::AddWarningTooltipData(const FTooltipData& InTooltipData)
{
	const int32 Index = WarningTooltipData.Add(InTooltipData);
	return WarningTooltipData[Index].UpdateTooltipState;
}

FUpdateDynamicTooltipState& UMenuOptionWidget::AddDynamicWarningTooltipData(const FTooltipData& InTooltipData,
	const FString& FallbackStringTableKey, const float InMin, const int32 InPrecision)
{
	const int32 Index = WarningTooltipData.Add(InTooltipData);
	WarningTooltipData[Index].SetDynamicData(InMin, FallbackStringTableKey, InPrecision);
	return WarningTooltipData[Index].UpdateDynamicTooltipState;
}

void UMenuOptionWidget::ConstructTooltipWarningImageIfNeeded(FTooltipData& InTooltipData)
{
	if (InTooltipData.TooltipImage.IsValid())
	{
		return;
	}

	UTooltipImage* NewTooltipImage;
	switch (InTooltipData.TooltipType)
	{
	case ETooltipImageType::Caution:
		NewTooltipImage = CreateWidget<UTooltipImage>(this, MenuOptionStyle->TooltipCautionImageClass);
		break;
	case ETooltipImageType::Warning:
		NewTooltipImage = CreateWidget<UTooltipImage>(this, MenuOptionStyle->TooltipWarningImageClass);
		break;
	case ETooltipImageType::Default: default:
		NewTooltipImage = CreateWidget<UTooltipImage>(this, MenuOptionStyle->TooltipWarningImageClass);
		break;
	}
	UHorizontalBoxSlot* HorizontalBoxSlot = TooltipBox->AddChildToHorizontalBox(NewTooltipImage);
	HorizontalBoxSlot->SetHorizontalAlignment(HAlign_Right);
	HorizontalBoxSlot->SetPadding(MenuOptionStyle->Padding_TooltipWarning);
	HorizontalBoxSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));

	InTooltipData.TooltipImage = NewTooltipImage;
}

void UMenuOptionWidget::UpdateWarningTooltips()
{
	for (FTooltipData& Data : GetTooltipWarningData())
	{
		if (Data.UpdateTooltipState.IsBound())
		{
			Data.SetShouldShowTooltipImage(Data.UpdateTooltipState.Execute());
		}
		else
		{
			Data.SetShouldShowTooltipImage(false);
		}
	}
}

void UMenuOptionWidget::UpdateDynamicWarningTooltips()
{
	for (FTooltipData& Data : GetTooltipWarningData())
	{
		if (Data.IsDynamic() && Data.UpdateDynamicTooltipState.IsBound())
		{
			const FDynamicTooltipState State = Data.UpdateDynamicTooltipState.Execute();
			Data.UpdateDynamicTooltipText(State);
		}
	}
}

void UMenuOptionWidget::UpdateAllWarningTooltips()
{
	UpdateWarningTooltips();
	UpdateDynamicWarningTooltips();
}

int32 UMenuOptionWidget::GetNumberOfWarnings()
{
	int32 Num = 0;
	for (FTooltipData& Data : WarningTooltipData)
	{
		if (Data.TooltipType == ETooltipImageType::Warning && Data.TooltipImage.IsValid() && Data.TooltipImage->
			IsVisible())
		{
			Num++;
		}
	}
	return Num;
}

int32 UMenuOptionWidget::GetNumberOfCautions()
{
	int32 Num = 0;
	for (FTooltipData& Data : WarningTooltipData)
	{
		if (Data.TooltipType == ETooltipImageType::Caution && Data.TooltipImage.IsValid() && Data.TooltipImage->
			IsVisible())
		{
			Num++;
		}
	}
	return Num;
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
