// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "OverlayWidgets/HUDWidgets/HitTimingWidget.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UHitTimingWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Image_LogoTick->GetDynamicMaterial())
	{
		DynamicImageMaterial = Image_LogoTick->GetDynamicMaterial();
	}

	OnTickTransition.BindDynamic(this, &ThisClass::TickTransition);
	Tick_Transition_Timeline.AddInterpFloat(Tick_Current_Curve, OnTickTransition);
	Tick_Transition_Timeline.SetPlayRate(4.f);
}

void UHitTimingWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	Tick_Transition_Timeline.TickTimeline(InDeltaTime);
}

void UHitTimingWidget::Init(const FText& MinTickValue, const FText& MaxTickValue)
{
	Tick_Min->SetText(MinTickValue);
	Tick_Max->SetText(MaxTickValue);
	Tick_Mid->SetText(FText::FromString(FString::FromInt(0) + "ms"));
}

void UHitTimingWidget::UpdateHitTiming(const float TimeOffsetNormalized, const float TimeOffsetRaw)
{
	if (TimeOffsetNormalized > 0.f && DynamicImageMaterial)
	{
		DynamicImageMaterial->SetScalarParameterValue(FName("LastTimeOffset"), LastTimeOffset);
		DynamicImageMaterial->SetScalarParameterValue(FName("TimeOffset"), TimeOffsetNormalized);

		Tick_Current->SetText(MakeCurrentTickText(TimeOffsetRaw));
		Tick_Current_Offset_Start = Tick_Current_Offset_End;
		Tick_Current_Offset_End = -300.f + TimeOffsetNormalized * (300.f - -300.f);
		LastTimeOffset = TimeOffsetNormalized;
		Tick_Transition_Timeline.PlayFromStart();
	}
}

FText UHitTimingWidget::MakeCurrentTickText(const float TimeOffsetRaw)
{
	const FString Offset = FString::FromInt(static_cast<int32>(TimeOffsetRaw * 100.f)) + "ms";
	return FText::FromString(Offset);
}

void UHitTimingWidget::TickTransition(const float Value)
{
	DynamicImageMaterial->SetScalarParameterValue(FName("EaseInOut"), Value);
	const float LerpValue = Tick_Current_Offset_Start + Value * (Tick_Current_Offset_End - Tick_Current_Offset_Start);
	Cast<UHorizontalBoxSlot>(Tick_Current->Slot)->SetPadding(FMargin(LerpValue, 0.f, 0.f, 0.f));
}
