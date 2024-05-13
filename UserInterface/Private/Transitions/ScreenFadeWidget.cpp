// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "Transitions/ScreenFadeWidget.h"
#include "Components/Image.h"

void UScreenFadeWidget::NativeConstruct()
{
	Super::NativeConstruct();
	FadeToBlackFinishDelegate.BindDynamic(this, &UScreenFadeWidget::FadeToBlackAnimFinished);
	BindToAnimationFinished(FadeInBlackScreen, FadeToBlackFinishDelegate);
	FadeFromBlackFinishDelegate.BindDynamic(this, &UScreenFadeWidget::FadeFromBlackAnimFinished);
	BindToAnimationFinished(FadeOutBlackScreen, FadeFromBlackFinishDelegate);
}

void UScreenFadeWidget::NativeDestruct()
{
	Super::NativeDestruct();
	FadeToBlackFinishDelegate.Unbind();
	FadeFromBlackFinishDelegate.Unbind();
}

void UScreenFadeWidget::FadeFromBlackAnimFinished()
{
	OnFadeFromBlackFinish.Broadcast();
}

void UScreenFadeWidget::FadeToBlackAnimFinished()
{
	OnFadeToBlackFinish.Broadcast();
}

void UScreenFadeWidget::SetStartOpacity(const float Value)
{
	BlackScreen->SetRenderOpacity(Value);
}

void UScreenFadeWidget::FadeFromBlack(const float FadeOutTime)
{
	PlayAnimationForward(FadeOutBlackScreen, 1 / FadeOutTime);
}

void UScreenFadeWidget::FadeToBlack(const float FadeOutTime)
{
	PlayAnimationForward(FadeInBlackScreen, 1 / FadeOutTime);
}
