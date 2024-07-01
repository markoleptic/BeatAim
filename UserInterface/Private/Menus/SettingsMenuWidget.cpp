// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "Menus/SettingsMenuWidget.h"
#include "CommonWidgetCarousel.h"
#include "Settings/AudioAnalyzerSettingsWidget.h"
#include "Settings/CrossHairSettingsWidget.h"
#include "Settings/GameSettingsWidget.h"
#include "Settings/InputSettingsWidget.h"
#include "Utilities/BSCarouselNavBar.h"

void USettingsMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	AudioAnalyzer_Widget->OnRestartButtonClicked.BindUFunction(this, "OnRestartButtonClicked_AudioAnalyzer");

	Carousel->OnCurrentPageIndexChanged.AddUniqueDynamic(this, &ThisClass::OnCarouselWidgetIndexChanged);
	Carousel->SetActiveWidgetIndex(0);
	CarouselNavBar->SetLinkedCarousel(Carousel);

	if (bIsPauseMenuChild)
	{
		AudioAnalyzer_Widget->InitPauseMenuChild();
	}
}

void USettingsMenuWidget::OnRestartButtonClicked_AudioAnalyzer() const
{
	if (!OnRestartButtonClicked.ExecuteIfBound())
	{
		UE_LOG(LogTemp, Display, TEXT("OnRestartButtonClicked not bound."));
	}
}

void USettingsMenuWidget::OnCarouselWidgetIndexChanged(UCommonWidgetCarousel* InCarousel, const int32 NewIndex)
{
}

FOnPlayerSettingsChanged_Game& USettingsMenuWidget::GetGameDelegate() const
{
	return Game_Widget->GetPublicGameSettingsChangedDelegate();
}

FOnPlayerSettingsChanged_CrossHair& USettingsMenuWidget::GetCrossHairDelegate() const
{
	return CrossHair_Widget->GetPublicCrossHairSettingsChangedDelegate();
}

FOnPlayerSettingsChanged_AudioAnalyzer& USettingsMenuWidget::GetAudioAnalyzerDelegate() const
{
	return AudioAnalyzer_Widget->GetPublicAudioAnalyzerSettingsChangedDelegate();
}

FOnPlayerSettingsChanged_User& USettingsMenuWidget::GetUserDelegate() const
{
	return Input_Widget->GetPublicUserSettingsChangedDelegate();
}
