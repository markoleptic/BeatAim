// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "Overlays/CombatTextWidget.h"
#include "Components/TextBlock.h"

void UCombatTextWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UCombatTextWidget::SetText(const FText& InText) const
{
	TextBlock->SetText(InText);
}

void UCombatTextWidget::ShowCombatText()
{
	BindToAnimationFinished(FadeOut, FadeOutDelegate);
	PlayAnimationForward(FadeOut);
}

void UCombatTextWidget::ShowCombatTextBottom()
{
	BindToAnimationFinished(FadeOutBottom, FadeOutDelegate);
	PlayAnimationForward(FadeOutBottom);
}
