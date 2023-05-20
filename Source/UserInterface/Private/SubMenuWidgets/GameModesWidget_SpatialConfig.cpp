// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

// ReSharper disable CppMemberFunctionMayBeConst
#include "SubMenuWidgets/GameModesWidget_SpatialConfig.h"
#include "GlobalConstants.h"
#include "BSWidgetInterface.h"
#include "Components/CheckBox.h"
#include "Components/ComboBoxString.h"
#include "Components/EditableTextBox.h"
#include "Components/Slider.h"
#include "WidgetComponents/BSHorizontalBox.h"

using namespace Constants;

void UGameModesWidget_SpatialConfig::NativeConstruct()
{
	Super::NativeConstruct();
	
	AddToTooltipData(QMark_HeadshotHeight, FText::FromStringTable("/Game/StringTables/ST_Tooltips.ST_Tooltips", "HeadshotHeight"));
	AddToTooltipData(QMark_FloorDistance, FText::FromStringTable("/Game/StringTables/ST_Tooltips.ST_Tooltips", "FloorDistance"));
	AddToTooltipData(QMark_ForwardSpread, FText::FromStringTable("/Game/StringTables/ST_Tooltips.ST_Tooltips", "ForwardSpread"));
	AddToTooltipData(QMark_MinDistance, FText::FromStringTable("/Game/StringTables/ST_Tooltips.ST_Tooltips", "MinDistance"));
	AddToTooltipData(QMark_BoundsScalingMethod, FText::FromStringTable("/Game/StringTables/ST_Tooltips.ST_Tooltips", "BoundsScalingMethod"));
	
	CheckBox_HeadShotOnly->OnCheckStateChanged.AddDynamic(this, &UGameModesWidget_SpatialConfig::OnCheckStateChanged_HeadShotOnly);
	CheckBox_ForwardSpread->OnCheckStateChanged.AddDynamic(this, &UGameModesWidget_SpatialConfig::OnCheckStateChanged_MoveTargetsForward);
	Slider_MinTargetDistance->OnValueChanged.AddDynamic(this, &UGameModesWidget_SpatialConfig::OnSliderChanged_MinTargetDistance);
	Value_MinTargetDistance->OnTextCommitted.AddDynamic(this, &UGameModesWidget_SpatialConfig::OnTextCommitted_MinTargetDistance);
	Slider_HorizontalSpread->OnValueChanged.AddDynamic(this, &UGameModesWidget_SpatialConfig::OnSliderChanged_HorizontalSpread);
	Value_HorizontalSpread->OnTextCommitted.AddDynamic(this, &UGameModesWidget_SpatialConfig::OnTextCommitted_HorizontalSpread);
	Slider_VerticalSpread->OnValueChanged.AddDynamic(this, &UGameModesWidget_SpatialConfig::OnSliderChanged_VerticalSpread);
	Value_VerticalSpread->OnTextCommitted.AddDynamic(this, &UGameModesWidget_SpatialConfig::OnTextCommitted_VerticalSpread);
	Slider_ForwardSpread->OnValueChanged.AddDynamic(this, &UGameModesWidget_SpatialConfig::OnSliderChanged_ForwardSpread);
	Value_ForwardSpread->OnTextCommitted.AddDynamic(this, &UGameModesWidget_SpatialConfig::OnTextCommitted_ForwardSpread);
	Slider_FloorDistance->OnValueChanged.AddDynamic(this, &UGameModesWidget_SpatialConfig::OnSliderChanged_FloorDistance);
	Value_FloorDistance->OnTextCommitted.AddDynamic(this, &UGameModesWidget_SpatialConfig::OnTextCommitted_FloorDistance);

	for (const EBoundsScalingMethod& Method : TEnumRange<EBoundsScalingMethod>())
	{
		ComboBox_BoundsScalingMethod->AddOption(UEnum::GetDisplayValueAsText(Method).ToString());
	}
}

void UGameModesWidget_SpatialConfig::InitSettingCategoryWidget()
{
	Super::InitSettingCategoryWidget();
}

void UGameModesWidget_SpatialConfig::InitializeTargetSpread(const FBS_SpatialConfig& SpatialConfig, const EBaseGameMode& BaseGameMode)
{
	// Lock vertical and horizontal spread if HeadShot height only, otherwise unlock them
	if (SpatialConfig.bUseHeadshotHeight)
	{
		Slider_VerticalSpread->SetLocked(true);
		Value_VerticalSpread->SetIsReadOnly(true);
		Slider_FloorDistance->SetLocked(true);
		Value_FloorDistance->SetIsReadOnly(true);
	}
	else
	{
		Slider_FloorDistance->SetLocked(false);
		Value_FloorDistance->SetIsReadOnly(false);
		Slider_VerticalSpread->SetLocked(false);
		Value_VerticalSpread->SetIsReadOnly(false);
	}

	// Hide forward spread box if move targets forward is not enabled, otherwise show
	if (SpatialConfig.bMoveTargetsForward)
	{
		BSBox_ForwardSpread->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		BSBox_ForwardSpread->SetVisibility(ESlateVisibility::Collapsed);
	}

	switch(BaseGameMode)
	{
	case EBaseGameMode::BeatGrid:
		Slider_HorizontalSpread->SetLocked(true);
		Value_HorizontalSpread->SetIsReadOnly(true);
		Slider_VerticalSpread->SetLocked(true);
		Value_VerticalSpread->SetIsReadOnly(true);
		BSBox_MinTargetDistance->SetVisibility(ESlateVisibility::Collapsed);
		BSBox_SpreadType->SetVisibility(ESlateVisibility::Collapsed);
		break;
	case EBaseGameMode::BeatTrack:
		BSBox_MinTargetDistance->SetVisibility(ESlateVisibility::Collapsed);
		BSBox_SpreadType->SetVisibility(ESlateVisibility::Collapsed);
		break;
	case EBaseGameMode::SingleBeat:
	case EBaseGameMode::MultiBeat:
	default:
		BSBox_MinTargetDistance->SetVisibility(ESlateVisibility::Visible);
		BSBox_SpreadType->SetVisibility(ESlateVisibility::Visible);
		Slider_HorizontalSpread->SetLocked(false);
		Value_HorizontalSpread->SetIsReadOnly(false);
		Slider_VerticalSpread->SetLocked(false);
		Value_VerticalSpread->SetIsReadOnly(false);
		break;
	}

	CheckBox_HeadShotOnly->SetIsChecked(SpatialConfig.bUseHeadshotHeight);
	Slider_FloorDistance->SetValue(SpatialConfig.FloorDistance);
	Value_FloorDistance->SetText(FText::AsNumber(SpatialConfig.FloorDistance));
	Slider_MinTargetDistance->SetValue(SpatialConfig.MinDistanceBetweenTargets);
	Value_MinTargetDistance->SetText(FText::AsNumber(SpatialConfig.MinDistanceBetweenTargets));
	ComboBox_BoundsScalingMethod->SetSelectedOption(UEnum::GetDisplayValueAsText(SpatialConfig.BoundsScalingMethod).ToString());
	Slider_HorizontalSpread->SetValue(SpatialConfig.BoxBounds.Y);
	Value_HorizontalSpread->SetText(FText::AsNumber(SpatialConfig.BoxBounds.Y));
	Slider_VerticalSpread->SetValue(SpatialConfig.BoxBounds.Z);
	Value_VerticalSpread->SetText(FText::AsNumber(SpatialConfig.BoxBounds.Z));
	CheckBox_ForwardSpread->SetIsChecked(SpatialConfig.bMoveTargetsForward);
	Slider_ForwardSpread->SetValue(SpatialConfig.MoveForwardDistance);
	Value_ForwardSpread->SetText(FText::AsNumber(SpatialConfig.MoveForwardDistance));

	UpdateBrushColors();
}

FBS_SpatialConfig UGameModesWidget_SpatialConfig::GetSpatialConfig() const
{
	FBS_SpatialConfig SpatialConfig;
	SpatialConfig.bUseHeadshotHeight = CheckBox_HeadShotOnly->IsChecked();
	SpatialConfig.FloorDistance = FMath::GridSnap(FMath::Clamp(Slider_FloorDistance->GetValue(), MinValue_FloorDistance, MaxValue_FloorDistance), SnapSize_FloorDistance);
	SpatialConfig.MinDistanceBetweenTargets = FMath::GridSnap(FMath::Clamp(Slider_MinTargetDistance->GetValue(), MinValue_MinTargetDistance, MaxValue_MinTargetDistance), SnapSize_MinTargetDistance);
	SpatialConfig.BoundsScalingMethod = GetBoundsScalingMethod();
	SpatialConfig.BoxBounds = FVector(0, FMath::GridSnap(FMath::Clamp(Slider_HorizontalSpread->GetValue(), MinValue_HorizontalSpread, MaxValue_HorizontalSpread), SnapSize_HorizontalSpread),
									 FMath::GridSnap(FMath::Clamp(Slider_VerticalSpread->GetValue(), MinValue_VerticalSpread, MaxValue_VerticalSpread), SnapSize_VerticalSpread));
	SpatialConfig.bMoveTargetsForward = CheckBox_ForwardSpread->IsChecked();
	SpatialConfig.MoveForwardDistance = FMath::GridSnap(FMath::Clamp(Slider_ForwardSpread->GetValue(), MinValue_ForwardSpread, MaxValue_ForwardSpread), SnapSize_HorizontalSpread);
	return SpatialConfig;
}

void UGameModesWidget_SpatialConfig::OnTextCommitted_MinTargetDistance(const FText& NewMinTargetDistance, ETextCommit::Type CommitType)
{
	OnEditableTextBoxChanged(NewMinTargetDistance, Value_MinTargetDistance, Slider_MinTargetDistance, SnapSize_MinTargetDistance,MinValue_MinTargetDistance, MaxValue_MinTargetDistance);
}

void UGameModesWidget_SpatialConfig::OnTextCommitted_HorizontalSpread(const FText& NewHorizontalSpread, ETextCommit::Type CommitType)
{
	OnEditableTextBoxChanged(NewHorizontalSpread, Value_HorizontalSpread, Slider_HorizontalSpread, SnapSize_HorizontalSpread, MinValue_HorizontalSpread, MaxValue_HorizontalSpread);
}

void UGameModesWidget_SpatialConfig::OnTextCommitted_VerticalSpread(const FText& NewVerticalSpread, ETextCommit::Type CommitType)
{
	OnEditableTextBoxChanged(NewVerticalSpread, Value_VerticalSpread, Slider_VerticalSpread, SnapSize_VerticalSpread, MinValue_VerticalSpread, MaxValue_VerticalSpread);
}

void UGameModesWidget_SpatialConfig::OnTextCommitted_ForwardSpread(const FText& NewForwardSpread, ETextCommit::Type CommitType)
{
	OnEditableTextBoxChanged(NewForwardSpread, Value_ForwardSpread, Slider_ForwardSpread, SnapSize_HorizontalSpread, MinValue_ForwardSpread, MaxValue_ForwardSpread);
}

void UGameModesWidget_SpatialConfig::OnTextCommitted_FloorDistance(const FText& NewFloorDistance, ETextCommit::Type CommitType)
{
	OnEditableTextBoxChanged(NewFloorDistance, Value_FloorDistance, Slider_FloorDistance, SnapSize_FloorDistance, MinValue_FloorDistance, MaxValue_FloorDistance);
}

void UGameModesWidget_SpatialConfig::OnSliderChanged_MinTargetDistance(const float NewMinTargetDistance)
{
	OnSliderChanged(NewMinTargetDistance, Value_MinTargetDistance, SnapSize_MinTargetDistance);
}

void UGameModesWidget_SpatialConfig::OnSliderChanged_HorizontalSpread(const float NewHorizontalSpread)
{
	OnSliderChanged(NewHorizontalSpread, Value_HorizontalSpread, SnapSize_HorizontalSpread);
}

void UGameModesWidget_SpatialConfig::OnSliderChanged_VerticalSpread(const float NewVerticalSpread)
{
	OnSliderChanged(NewVerticalSpread, Value_VerticalSpread, SnapSize_VerticalSpread);
}

void UGameModesWidget_SpatialConfig::OnSliderChanged_ForwardSpread(const float NewForwardSpread)
{
	OnSliderChanged(NewForwardSpread, Value_ForwardSpread, SnapSize_HorizontalSpread);
}

void UGameModesWidget_SpatialConfig::OnSliderChanged_FloorDistance(const float NewFloorDistance)
{
	OnSliderChanged(NewFloorDistance, Value_FloorDistance, SnapSize_FloorDistance);
}

EBoundsScalingMethod UGameModesWidget_SpatialConfig::GetBoundsScalingMethod() const
{
	const FString SelectedSpread = ComboBox_BoundsScalingMethod->GetSelectedOption();
	if (SelectedSpread.IsEmpty())
	{
		return EBoundsScalingMethod::None;
	}
	return GetEnumFromString<EBoundsScalingMethod>(SelectedSpread, EBoundsScalingMethod::None);
}

void UGameModesWidget_SpatialConfig::OnCheckStateChanged_HeadShotOnly(const bool bHeadshotOnly)
{
	if (bHeadshotOnly)
	{
		Slider_VerticalSpread->SetValue(0);
		Value_VerticalSpread->SetText(FText::AsNumber(0));
		Slider_VerticalSpread->SetLocked(true);
		Value_VerticalSpread->SetIsReadOnly(true);
		
		Slider_FloorDistance->SetValue(DistanceFromFloor);
		Value_FloorDistance->SetText(FText::AsNumber(DistanceFromFloor));
		Slider_FloorDistance->SetLocked(true);
		Value_FloorDistance->SetIsReadOnly(true);
		return;
	}
	
	Slider_FloorDistance->SetLocked(false);
	Value_FloorDistance->SetIsReadOnly(false);
	Slider_VerticalSpread->SetLocked(false);
	Value_VerticalSpread->SetIsReadOnly(false);
	OnEditableTextBoxChanged(FText::AsNumber(MaxValue_VerticalSpread), Value_VerticalSpread, Slider_VerticalSpread, SnapSize_VerticalSpread, MinValue_VerticalSpread, MaxValue_VerticalSpread);
	OnSliderChanged(MaxValue_VerticalSpread, Value_VerticalSpread, SnapSize_VerticalSpread);
}

void UGameModesWidget_SpatialConfig::OnCheckStateChanged_MoveTargetsForward(const bool bUseForwardSpread)
{
	if (bUseForwardSpread)
	{
		BSBox_ForwardSpread->SetVisibility(ESlateVisibility::Visible);
		return;
	}
	BSBox_ForwardSpread->SetVisibility(ESlateVisibility::Collapsed);
}
