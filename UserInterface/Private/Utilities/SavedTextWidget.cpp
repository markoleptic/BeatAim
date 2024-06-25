// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "Utilities/SavedTextWidget.h"

void USavedTextWidget::PlayFadeInFadeOut(const float PlaybackRate)
{
	if (IsAnimationPlaying(FadeInFadeOut))
	{
		StopAnimation(FadeInFadeOut);
	}
	PlayAnimationForward(FadeInFadeOut, PlaybackRate);
}
