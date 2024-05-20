// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once


#include "CoreMinimal.h"
#include "BSPlayerScoreInterface.h"
#include "BSPlayerSettingsInterface.h"
#include "HttpRequestInterface.h"
#include "SaveGames/SaveGamePlayerSettings.h"
#include "Target/ReinforcementLearningComponent.h"
#include "BSPlayerController.generated.h"

class UBSGameUserSettings;
class AFloatingTextActor;
class UQTableWidget;
class ABSCharacterBase;
class ULoginWidget;
class UFrameCounterWidget;
class UScreenFadeWidget;
class UPostGameMenuWidget;
class UCrossHairWidget;
class UPlayerHUD;
class UMainMenuWidget;
class UPauseMenuWidget;
class UCountdownWidget;
class ABSPlayerState;
class UBSAbilitySystemComponent;

DECLARE_DELEGATE(FOnScreenFadeToBlackFinish);

/** Base PlayerController class for this game. Responsible for adding any main widget from the UserInterface module
 *  to the viewport (MainMenuWidget, PauseMenu, PostGameModeMenu), and several other overlay widgets. */
UCLASS()
class BEATSHOT_API ABSPlayerController : public APlayerController, public IHttpRequestInterface,
                                         public IBSPlayerSettingsInterface, public IBSPlayerScoreInterface
{
	GENERATED_BODY()

	virtual void BeginPlay() override;
	virtual void PreProcessInput(const float DeltaTime, const bool bGamePaused) override;
	virtual void PostProcessInput(const float DeltaTime, const bool bGamePaused) override;

	/** Returns the player state. */
	UFUNCTION(BlueprintCallable, Category = "BSPlayerController")
	ABSPlayerState* GetBSPlayerState() const;

	/** Returns the ability system component. */
	UFUNCTION(BlueprintCallable, Category = "BSPlayerController")
	UBSAbilitySystemComponent* GetBSAbilitySystemComponent() const;

public:
	/** Returns the possessed character. */
	ABSCharacterBase* GetBSCharacter() const;

	const FPlayerSettings& GetPlayerSettings() const { return PlayerSettings; }

	/** Sets the enabled state of the pawn. */
	void SetPlayerEnabledState(const bool bPlayerEnabled);

	/** Shows the main menu widget, creating it if it does not exist. */
	void ShowMainMenu();
	/** Hides and destroys the main menu widget, if it exists. */
	void HideMainMenu();

	/** Shows the pause menu widget, creating it if it does not exist. */
	void ShowPauseMenu();
	/** Hides and destroys the pause menu widget, if it exists. */
	void HidePauseMenu();

	/** Shows the crosshair widget, creating it if it does not exist. */
	void ShowCrossHair();
	/** Hides and destroys the crosshair widget, if it exists. */
	void HideCrossHair();

	/** Shows the PlayerHUD widget, creating it if it does not exist. */
	void ShowPlayerHUD();
	/** Hides and destroys the PlayerHUD widget, if it exists. */
	void HidePlayerHUD();

	/** Updates the PlayerHUD widget. */
	void UpdatePlayerHUD(const FPlayerScore& PlayerScore, const float TimeOffsetNormalized, const float TimeOffsetRaw);

	/** Shows the countdown widget, creating it if it does not exist. */
	void ShowCountdown();
	/** Hides and destroys the countdown widget, if it exists. */
	void HideCountdown();

	/** Shows the post game mode menu widget, creating it if it does not exist. */
	void ShowPostGameMenu();
	/** Hides and destroys the post game mode menu widget, if it exists. */
	void HidePostGameMenu();

	/** Shows the FPS counter widget, creating it if it does not exist. */
	void ShowFPSCounter();
	/** Hides and destroys the FPS counter widget, if it exists. */
	void HideFPSCounter();

	/** Creates the ScreenFadeWidget and binds to its OnScreenFadeToBlackFinish delegate. */
	void CreateScreenFadeWidget(const float StartOpacity);
	/** Calls CreateScreenFadeWidget if necessary and plays the fade to black animation. */
	void FadeScreenToBlack();
	/** Calls CreateScreenFadeWidget if necessary and plays the fade from black animation. */
	void FadeScreenFromBlack();

	/** Shows the interact info widget, creating it if it does not exist. */
	void ShowInteractInfo();
	/** Hides and destroys the interact info widget, if it exists. */
	void HideInteractInfo();

	/** Shows the RLAgent widget, creating it if it does not exist. Binds the provided delegate to the
	 *  widget's UpdatePanel function. */
	void ShowQTableWidget(FOnQTableUpdate& OnQTableUpdate, const int32 Rows, const int32 Columns,
		const TArray<float>& QTable);
	/** Hides and destroys the RLAgent widget, if it exists. */
	void HideQTableWidget();

	/** Spawns the floating text actor (combat text) with text indicating the streak of targets destroyed. */
	void ShowCombatText(const int32 Streak, const FTransform& Transform);
	/** Spawns the floating text actor (combat text) with text indicating the accuracy based on time offset. */
	void ShowAccuracyText(const float TimeOffset, const FTransform& Transform);

	/** Called by Game Instance after saving scores to database, or before if there was a problem. */
	void OnPostScoresResponseReceived(const FString& StringTableKey = FString());

	/** Called by Character when receiving input from IA_Pause, or by exiting the PostGameMenu. */
	void HandlePause();

	/** Called by Character when receiving input from IA_LeftClick. */
	void HandleLeftClick();

	// Server only
	virtual void OnPossess(APawn* InPawn) override;

	// Server only
	virtual void OnRep_PlayerState() override;

	/** Login the user by authenticating using GetAuthTicketForWebApi. */
	void LoginUser();

	/** Executed when the player requests to try to log in through Steam after a failed attempt. */
	void InitiateSteamLogin();

	/** Delegate that executes when the screen fade widget completes its animation.  */
	FOnScreenFadeToBlackFinish OnScreenFadeToBlackFinish;

	/** The length of time to fade in MainMenu music. */
	UPROPERTY(EditDefaultsOnly, Category = "BSPlayerController|Loading Screen")
	float MainMenuMusicFadeInDuration = 2.f;

	/** The time the screen fade widget plays its animations for. Also applies to fading out MainMenu music. */
	float ScreenFadeWidgetAnimationDuration = 0.75f;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "BSPlayerController|Classes")
	TSubclassOf<UMainMenuWidget> MainMenuClass;

	UPROPERTY(EditDefaultsOnly, Category = "BSPlayerController|Classes")
	TSubclassOf<UCrossHairWidget> CrossHairClass;

	UPROPERTY(EditDefaultsOnly, Category = "BSPlayerController|Classes")
	TSubclassOf<UPlayerHUD> PlayerHUDClass;

	UPROPERTY(EditDefaultsOnly, Category = "BSPlayerController|Classes")
	TSubclassOf<UPauseMenuWidget> PauseMenuClass;

	UPROPERTY(EditDefaultsOnly, Category = "BSPlayerController|Classes")
	TSubclassOf<UCountdownWidget> CountdownClass;

	UPROPERTY(EditDefaultsOnly, Category = "BSPlayerController|Classes")
	TSubclassOf<UPostGameMenuWidget> PostGameMenuWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "BSPlayerController|Classes")
	TSubclassOf<UFrameCounterWidget> FPSCounterClass;

	UPROPERTY(EditDefaultsOnly, Category = "BSPlayerController|Classes")
	TSubclassOf<UScreenFadeWidget> ScreenFadeClass;

	UPROPERTY(EditDefaultsOnly, Category = "BSPlayerController|Classes")
	TSubclassOf<UUserWidget> InteractInfoWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "BSPlayerController|Classes")
	TSubclassOf<UQTableWidget> QTableWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "BSPlayerController|Classes")
	TSubclassOf<AFloatingTextActor> FloatingTextActorClass;

private:
	/** Callback function for screen fade widget's OnFadeFromBlackFinish delegate. */
	void OnFadeScreenFromBlackFinish();

	/** Callback functions for when settings change. */
	virtual void OnPlayerSettingsChanged(const FPlayerSettings_Game& NewGameSettings) override;
	virtual void OnPlayerSettingsChanged(const FPlayerSettings_User& NewUserSettings) override;
	virtual void OnPlayerSettingsChanged(const FPlayerSettings_AudioAnalyzer& NewAudioAnalyzerSettings) override;
	virtual void OnPlayerSettingsChanged(const FPlayerSettings_CrossHair& NewCrossHairSettings) override;
	void HandleGameUserSettingsChanged(const UBSGameUserSettings* InGameUserSettings);

	/** Calls ResetAuthTicket from SteamAPI if both MainMenuWidget and BeatShot API requests were completed. */
	void TryResetAuthTicketHandle(const uint32 Handle);

	UPROPERTY()
	TObjectPtr<UMainMenuWidget> MainMenuWidget;
	UPROPERTY()
	TObjectPtr<UCrossHairWidget> CrossHairWidget;
	UPROPERTY()
	TObjectPtr<UPlayerHUD> PlayerHUDWidget;
	UPROPERTY()
	TObjectPtr<UPauseMenuWidget> PauseMenuWidget;
	UPROPERTY()
	TObjectPtr<UCountdownWidget> CountdownWidget;
	UPROPERTY()
	TObjectPtr<UPostGameMenuWidget> PostGameMenuWidget;
	UPROPERTY()
	TObjectPtr<UFrameCounterWidget> FPSCounterWidget;
	UPROPERTY()
	TObjectPtr<UScreenFadeWidget> ScreenFadeWidget;
	UPROPERTY()
	TObjectPtr<UUserWidget> InteractInfoWidget;
	UPROPERTY()
	TObjectPtr<UQTableWidget> QTableWidget;

	/** Whether the user successfully received a steam auth ticket BeatShot api response. */
	bool bIsLoggedIn = false;

	/** Number of completed steam auth ticket usages. */
	uint8 NumAuthTicketFinishes = 0;

	/** Z Order for the screen fade widget. */
	const int32 ZOrderFadeScreen = 20;

	/** Z Order for the FPS counter widget. */
	const int32 ZOrderFPSCounter = 19;

	UPROPERTY()
	FPlayerSettings PlayerSettings;

	UPROPERTY()
	const UBSGameUserSettings* GameUserSettings;
};
