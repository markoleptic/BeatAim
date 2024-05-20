// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ScreenFadeWidget.generated.h"

class UImage;

/** Widget used any time a screen transition needs to take place, used by PlayerController. */
UCLASS()
class USERINTERFACE_API UScreenFadeWidget : public UUserWidget
{
	GENERATED_BODY()

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

public:
	TMulticastDelegate<void()> OnFadeFromBlackFinish;
	TMulticastDelegate<void()> OnFadeToBlackFinish;

	void SetStartOpacity(const float Value);

	UFUNCTION(BlueprintCallable)
	void FadeFromBlack(const float FadeOutTime);
	UFUNCTION(BlueprintCallable)
	void FadeToBlack(const float FadeOutTime);

private:
	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	UImage* BlackScreen;

	UPROPERTY(EditDefaultsOnly, Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* FadeOutBlackScreen;
	UPROPERTY(EditDefaultsOnly, Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* FadeInBlackScreen;

	FWidgetAnimationDynamicEvent FadeFromBlackFinishDelegate;
	FWidgetAnimationDynamicEvent FadeToBlackFinishDelegate;

	UFUNCTION()
	void FadeFromBlackAnimFinished();
	UFUNCTION()
	void FadeToBlackAnimFinished();
};
