// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TimelineComponent.h"
#include "HitTimingWidget.generated.h"

class UTextBlock;
class UImage;

UCLASS()
class USERINTERFACE_API UHitTimingWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void UpdateHitTiming(float TimeOffsetNormalized = -1.f, float TimeOffsetRaw = -1.f);

	void Init(const FText& MinTickValue, const FText& MaxTickValue);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	static FText MakeCurrentTickText(const float TimeOffsetRaw);

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UImage* Image_LogoTick;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UImage* Image_Background;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* Tick_Min;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* Tick_Mid;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* Tick_Max;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* Tick_Current;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> DynamicImageMaterial;
	UPROPERTY(BlueprintReadOnly, Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* EaseInNewOffset;

	UPROPERTY(EditDefaultsOnly)
	UCurveFloat* Tick_Current_Curve;
	FTimeline Tick_Transition_Timeline;
	FOnTimelineFloat OnTickTransition;
	UFUNCTION()
	void TickTransition(const float Value);

	float Tick_Current_Offset_Start = 0.f;
	float Tick_Current_Offset_End = 0.f;
	float LastTimeOffset = 0.5f;
};
