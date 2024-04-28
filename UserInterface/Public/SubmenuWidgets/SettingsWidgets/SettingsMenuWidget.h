// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

// ReSharper disable CppUE4CodingStandardNamingViolationWarning
#pragma once

#include "CoreMinimal.h"
#include "SettingsMenuWidget_AudioAnalyzer.h"
#include "SettingsMenuWidget_CrossHair.h"
#include "SettingsMenuWidget_Game.h"
#include "SettingsMenuWidget_Input.h"
#include "SettingsMenuWidget_VideoAndSound.h"
#include "Blueprint/UserWidget.h"
#include "SettingsMenuWidget.generated.h"

class UBSCarouselNavBar;
class UCommonWidgetCarousel;
class UBSButton;
class UMenuButton;
class UVerticalBox;

/** Widget that holds all settings category widgets */
UCLASS()
class USERINTERFACE_API USettingsMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Whether this instance of SettingsMenuWidget belongs to Pause Menu or not */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "SettingsMenuWidget", meta = (ExposeOnSpawn="true"))
	bool bIsPauseMenuChild;

	/** Broadcast to owning widget that user has clicked restart button */
	FOnRestartButtonClicked OnRestartButtonClicked;

	/** Returns the UGameSettings widget's OnPlayerSettingsChangedDelegate_Game, which is broadcast when the widget changes Game Settings */
	FOnPlayerSettingsChanged_Game& GetGameDelegate() const
	{
		return Game_Widget->GetPublicGameSettingsChangedDelegate();
	}

	/** Returns the UCrossHairSettings widget's OnPlayerSettingsChangedDelegate_Game, which is broadcast when the widget changes Game Settings */
	FOnPlayerSettingsChanged_CrossHair& GetCrossHairDelegate() const
	{
		return CrossHair_Widget->GetPublicCrossHairSettingsChangedDelegate();
	}

	/** Returns the UAASettings widget's OnPlayerSettingsChangedDelegate_AudioAnalyzer, which is broadcast when the widget changes Audio Analyzer Settings */
	FOnPlayerSettingsChanged_AudioAnalyzer& GetAudioAnalyzerDelegate() const
	{
		return AudioAnalyzer_Widget->GetPublicAudioAnalyzerSettingsChangedDelegate();
	}

	/** Returns the UInputSettings widget's OnPlayerSettingsChangedDelegate_User, which is broadcast when the widget changes User Settings */
	FOnPlayerSettingsChanged_User& GetUserDelegate() const
	{
		return Input_Widget->GetPublicUserSettingsChangedDelegate();
	}

protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnRestartButtonClicked_AudioAnalyzer() const;

	UFUNCTION()
	void OnCarouselWidgetIndexChanged(UCommonWidgetCarousel* InCarousel, const int32 NewIndex);

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCommonWidgetCarousel* Carousel;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UBSCarouselNavBar* CarouselNavBar;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USettingsMenuWidget_Game* Game_Widget;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USettingsMenuWidget_VideoAndSound* VideoAndSound_Widget;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USettingsMenuWidget_CrossHair* CrossHair_Widget;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USettingsMenuWidget_AudioAnalyzer* AudioAnalyzer_Widget;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USettingsMenuWidget_Input* Input_Widget;
};
