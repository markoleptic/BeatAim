// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "QuitMenuWidget.generated.h"

struct FGameModeTransitionState;
DECLARE_DYNAMIC_DELEGATE(FOnExitQuitMenu);

class UTextBlock;
class UVerticalBox;
class UOverlay;
class UBSButton;
class UWidgetAnimation;
class UScreenFadeWidget;

/** Provides a quit menu to other widgets. */
UCLASS()
class USERINTERFACE_API UQuitMenuWidget : public UUserWidget
{
	GENERATED_BODY()

	virtual void NativeConstruct() override;

public:
	/** Fades in the MenuOverlay, and also fades in the background blur. */
	UFUNCTION()
	void PlayInitialFadeInMenu()
	{
		PlayAnimationReverse(FadeOutMenu);
		PlayAnimationReverse(FadeOutBackgroundBlur);
	}

	/** Fades in the RestartOverlay. */
	UFUNCTION()
	void PlayFadeInRestartMenu()
	{
		PlayAnimationReverse(FadeOutRestartMenu);
		PlayAnimationReverse(FadeOutBackgroundBlur);
	}

	/** Whether or not this instance of QuitMenu belongs to The PostGameMenuWidget or not. */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Default", meta = (ExposeOnSpawn="true"))
	bool bIsPostGameMenuChild;

	/** Broadcasts to parent so it can slide menu button back to starting position. */
	UPROPERTY()
	FOnExitQuitMenu OnExitQuitMenu;

	/** Bound to DefaultGameInstance when constructed in DefaultPlayerController. */
	TMulticastDelegate<void(const FGameModeTransitionState& TransitionState)> OnGameModeStateChanged;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UOverlay* Overlay_QuitMenuSwitcher;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UOverlay* Overlay_Menu;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UOverlay* Overlay_SaveMenu;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UOverlay* Overlay_Restart;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UOverlay* Overlay_SaveInProgress;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UBSButton* Button_QuitMainMenu;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UBSButton* Button_QuitDesktop;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UBSButton* Button_QuitBack;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UBSButton* Button_QuitAndSave;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UBSButton* Button_QuitWithoutSave;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UBSButton* Button_SaveBack;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UBSButton* Button_RestartAndSave;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UBSButton* Button_RestartWithoutSave;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UBSButton* Button_RestartBack;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* TextBlock_SaveMenuTitle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* FadeOutMenu;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* FadeOutSaveMenu;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* FadeOutRestartMenu;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* FadeOutBackgroundBlur;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* FadeOutSaveInProgress;

	/** Delegate used to bind CollapseWidget to FadeOutBackgroundBlur. */
	FWidgetAnimationDynamicEvent FadeOutWidgetDelegate;

private:
	void OnButtonClicked_BSButton(const UBSButton* Button);

	/** Fades in the MenuOverlay. */
	UFUNCTION()
	void PlayFadeInMenu() { PlayAnimationReverse(FadeOutMenu); }

	/** Fades out the MenuOverlay. */
	UFUNCTION()
	void PlayFadeOutMenu() { PlayAnimationForward(FadeOutMenu); }

	/** Fades in the SaveMenuOverlay. */
	UFUNCTION()
	void PlayFadeInSaveMenu() { PlayAnimationReverse(FadeOutSaveMenu); }

	/** Fades out the SaveMenuOverlay. */
	UFUNCTION()
	void PlayFadeOutSaveMenu() { PlayAnimationForward(FadeOutSaveMenu); }

	/** Fades out the RestartOverlay. */
	UFUNCTION()
	void PlayFadeOutRestartMenu() { PlayAnimationForward(FadeOutRestartMenu); }

	/** Fades in the SaveInProgressOverlay. */
	UFUNCTION()
	void PlayFadeInSaveInProgress() { PlayAnimationReverse(FadeOutSaveInProgress); }

	/** Called when either QuitAndSaveButton or Button_QuitWithoutSave is clicked. */
	UFUNCTION()
	void Quit();

	/** Called from Quit() if bGotoMainMenu is true. */
	UFUNCTION()
	void OnQuitToMainMenu();

	/** Called from Quit() if bGotoMainMenu is false. */
	UFUNCTION()
	void OnQuitToDesktop();

	/** Called when either Button_RestartAndSave or Button_RestartWithoutSave is clicked. */
	UFUNCTION()
	void OnRestart();

	/** Plays FadeOutBackgroundBlur, binds FadeOutWidgetDelegate to ShowAudioFormatSelect and executes
	 *  OnExitQuitMenu. */
	UFUNCTION()
	void InitializeExit();

	/** Function that is bound to FadeOutBackgroundBlur to set the visibility of the widget to collapsed. */
	UFUNCTION()
	void CollapseWidget();
	UFUNCTION()
	void SetGotoMainMenuTrue() { bGotoMainMenu = true; }

	UFUNCTION()
	void SetGotoMainMenuFalse() { bGotoMainMenu = false; }

	UFUNCTION()
	void SetShouldSaveScoresTrue() { bShouldSaveScores = true; }

	UFUNCTION()
	void SetShouldSaveScoresFalse() { bShouldSaveScores = false; }

	UFUNCTION()
	void SetSaveMenuTitleMainMenu() { TextBlock_SaveMenuTitle->SetText(FText::FromString("Quit to Main Menu")); }

	UFUNCTION()
	void SetSaveMenuTitleDesktop() { TextBlock_SaveMenuTitle->SetText(FText::FromString("Quit to Desktop")); }

	/** Whether to save scores, used as argument when calling EndGameMode() from DefaultGameMode. */
	bool bShouldSaveScores;

	/** Whether to go to the MainMenuWidget vs exiting to desktop. Used in Quit(). */
	bool bGotoMainMenu;
};
