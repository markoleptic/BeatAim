// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "Transitions/LoadingScreenWidget.h"
#include "Animation/WidgetAnimation.h"

void ULoadingScreenWidget::NativeConstruct()
{
	Super::NativeConstruct();

	OnFadeInFinished.BindDynamic(this, &ThisClass::OnFadeInFinishedCallback);
	OnFadeOutFinished.BindDynamic(this, &ThisClass::OnFadeOutFinishedCallback);
	BindToAnimationFinished(FadeOutAnim, OnFadeOutFinished);
	PlayAnimation(SpinLoop, 0, 20, EUMGSequencePlayMode::Forward, 1, false);
	FadeIn(0.75);
}

void ULoadingScreenWidget::OnFadeOutFinishedCallback()
{
	UE_LOG(LogTemp, Warning, TEXT("OnFadeOutFinishedCallback"));
	if (OnLoadingScreenFadeOutComplete.IsBound())
	{
		UE_LOG(LogTemp, Warning, TEXT("OnLoadingScreenFadeOutComplete is bound"));
		OnLoadingScreenFadeOutComplete.Execute();
	}
}

void ULoadingScreenWidget::OnFadeInFinishedCallback()
{
	UE_LOG(LogTemp, Warning, TEXT("OnFadeInFinishedCallback - playing FadeOut"));
	FadeOut(LoadingScreenTime);
}

void ULoadingScreenWidget::FadeIn(const float LoadingScreenWidgetFadeOutTime)
{
	UE_LOG(LogTemp, Warning, TEXT("playing FadeIn"));
	PlayAnimationForward(FadeOutAnim, 1 / LoadingScreenWidgetFadeOutTime);
}

void ULoadingScreenWidget::FadeOut(const float LoadingScreenWidgetFadeOutTime)
{
	if (IsAnimationPlaying(FadeInAnim))
	{
		UE_LOG(LogTemp, Warning, TEXT("FadeOut binding to fadeinfinished"));
		BindToAnimationFinished(FadeInAnim, OnFadeInFinished);
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("playing FadeOut"));
	PlayAnimationForward(FadeOutAnim, 1 / LoadingScreenWidgetFadeOutTime);
}
