// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

// ReSharper disable CppUE4CodingStandardNamingViolationWarning
#pragma once

#include "CoreMinimal.h"
#include "BSPlayerSettingsInterface.h"
#include "Blueprint/UserWidget.h"
#include "SettingsMenuWidget.generated.h"

class UInputSettingsWidget;
class UGameSettingsWidget;
class UAudioAnalyzerSettingsWidget;
class UCrossHairSettingsWidget;
class UBSCarouselNavBar;
class UCommonWidgetCarousel;
class UBSButton;
class UMenuButton;
class UVerticalBox;
class UVideoAndSoundSettingsWidget;

/** Widget that holds all settings category widgets. */
UCLASS()
class USERINTERFACE_API USettingsMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Whether this instance of SettingsMenuWidget belongs to Pause Menu or not. */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "SettingsMenuWidget", meta = (ExposeOnSpawn="true"))
	bool bIsPauseMenuChild;

	/** Broadcast to owning widget that user has clicked restart button. */
	TDelegate<void()> OnRestartButtonClicked;

	/** Returns the Game widget's OnPlayerSettingsChangedDelegate_Game, which is broadcast when the widget
	 *  changes Game Settings. */
	FOnPlayerSettingsChanged_Game& GetGameDelegate() const;

	/** Returns the CrossHair Settings widget's OnPlayerSettingsChangedDelegate_Game, which is broadcast when the
	 *  widget changes Game Settings. */
	FOnPlayerSettingsChanged_CrossHair& GetCrossHairDelegate() const;

	/** Returns the Audio Analyzer Settings widget's OnPlayerSettingsChangedDelegate_AudioAnalyzer, which is broadcast
	 *  when the widget changes Audio Analyzer Settings. */
	FOnPlayerSettingsChanged_AudioAnalyzer& GetAudioAnalyzerDelegate() const;

	/** Returns the Input Settings widget's OnPlayerSettingsChangedDelegate_User, which is broadcast when the widget
	 *  changes User Settings. */
	FOnPlayerSettingsChanged_User& GetUserDelegate() const;

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
	UGameSettingsWidget* Game_Widget;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UVideoAndSoundSettingsWidget* VideoAndSound_Widget;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCrossHairSettingsWidget* CrossHair_Widget;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UAudioAnalyzerSettingsWidget* AudioAnalyzer_Widget;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UInputSettingsWidget* Input_Widget;
};
