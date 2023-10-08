// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "WidgetComponents/SavedTextWidget.h"

void USavedTextWidget::PlayFadeInFadeOut()
{
	if (IsAnimationPlaying(FadeInFadeOut))
	{
		StopAnimation(FadeInFadeOut);
	}
	PlayAnimationForward(FadeInFadeOut);
}
