// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LoadingScreenWidget.generated.h"

class UImage;

DECLARE_DELEGATE(FOnLoadingScreenFadeOutComplete);

UCLASS(BlueprintType)
class USERINTERFACE_API ULoadingScreenWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION()
	void FadeIn(const float LoadingScreenWidgetFadeOutTime);

	UFUNCTION()
	void FadeOut(const float LoadingScreenWidgetFadeOutTime);

	FOnLoadingScreenFadeOutComplete OnLoadingScreenFadeOutComplete;

protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnFadeOutFinishedCallback();

	UFUNCTION()
	void OnFadeInFinishedCallback();

	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	UImage* Icon;

	FWidgetAnimationDynamicEvent OnFadeOutFinished;
	FWidgetAnimationDynamicEvent OnFadeInFinished;

	UPROPERTY(EditDefaultsOnly, Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* FadeOutAnim;
	UPROPERTY(EditDefaultsOnly, Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* FadeInAnim;
	UPROPERTY(EditDefaultsOnly, Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* SpinLoop;

	float LoadingScreenTime = 0.75f;
};
