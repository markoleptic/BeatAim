// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once


#include "CoreMinimal.h"
#include "BSGameInstance.h"
#include "HttpRequestInterface.h"
#include "LoadingProcessInterface.h"
#include "SaveLoadInterface.h"
#include "Target/ReinforcementLearningComponent.h"
#include "BSPlayerController.generated.h"

class AFloatingTextActor;
class URLAgentWidget;
class ABSCharacter;
class ULoginWidget;
class UFPSCounterWidget;
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
 *  to the viewport (MainMenu, PauseMenu, PostGameModeMenu), and several other overlay widgets. */
UCLASS()
class BEATSHOT_API ABSPlayerController : public APlayerController, public ISaveLoadInterface,
                                         public IHttpRequestInterface, public ILoadingProcessInterface
{
	GENERATED_BODY()

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "BSPlayerController")
	ABSPlayerState* GetBSPlayerState() const;

	UFUNCTION(BlueprintCallable, Category = "BSPlayerController")
	UBSAbilitySystemComponent* GetBSAbilitySystemComponent() const;

public:
	/** Sets the enabled state of the pawn */
	void SetPlayerEnabledState(const bool bPlayerEnabled);

	UFUNCTION(BlueprintCallable)
	void ShowMainMenu();
	UFUNCTION(BlueprintCallable)
	void HideMainMenu();

	UFUNCTION(BlueprintCallable)
	void ShowPauseMenu();
	UFUNCTION(BlueprintCallable)
	void HidePauseMenu();

	void ShowCrossHair();
	void HideCrossHair();

	void ShowPlayerHUD();
	void HidePlayerHUD();

	void ShowCountdown(const bool bIsRestart);
	void HideCountdown();

	void ShowPostGameMenu();
	void HidePostGameMenu();

	void ShowFPSCounter();
	void HideFPSCounter();

	void CreateScreenFadeWidget(const float StartOpacity);
	void FadeScreenToBlack();

	/** Called when entering a new level and the loading screen is finished, or when a game mode has been restarted inside a level */
	void FadeScreenFromBlack();

	void ShowInteractInfo();
	void HideInteractInfo();

	void ShowRLAgentWidget(FOnQTableUpdate& OnQTableUpdate, const int32 Rows, const int32 Columns,
		const TArray<float>& QTable);
	void HideRLAgentWidget();

	void ShowCombatText(const int32 Streak, const FTransform& Transform);
	void ShowAccuracyText(const float TimeOffset, const FTransform& Transform);

	/** Called by Game Instance after saving scores to database, or before if there was a problem */
	void OnPostScoresResponseReceived(const EPostScoresResponse& Response);

	bool IsPostGameMenuActive() const { return PostGameMenuActive; }

	/** Called by Character when receiving input from IA_Pause, or by exiting the PostGameMenu */
	void HandlePause();

	// Server only
	virtual void OnPossess(APawn* InPawn) override;

	virtual void OnRep_PlayerState() override;

	/** Delegate that executes when the ScreenFadeWidget completes its animation*/
	FOnScreenFadeToBlackFinish OnScreenFadeToBlackFinish;

	ABSCharacter* GetBSCharacter() const;

	/** Login the user by authenticating using GetAuthTicketForWebApi */
	void LoginUser();

	/** Executed when the player requests to try to login through Steam after a failed attempt */
	void InitiateSteamLogin();

	/** ~ILoadingProcessInterface begin */
	virtual void BindToLoadingScreenDelegates(
		FOnLoadingScreenVisibilityChangedDelegate& OnLoadingScreenVisibilityChanged,
		FOnReadyToHideLoadingScreenDelegate& OnReadyToHideLoadingScreen) override;
	/** ~ILoadingProcessInterface end */

protected:
	virtual void PreProcessInput(const float DeltaTime, const bool bGamePaused) override;
	virtual void PostProcessInput(const float DeltaTime, const bool bGamePaused) override;

	UFUNCTION()
	void OnLoadingScreenVisibilityChanged(bool bIsVisible);

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
	TSubclassOf<UFPSCounterWidget> FPSCounterClass;
	UPROPERTY(EditDefaultsOnly, Category = "BSPlayerController|Classes")
	TSubclassOf<UScreenFadeWidget> ScreenFadeClass;
	UPROPERTY(EditDefaultsOnly, Category = "BSPlayerController|Classes")
	TSubclassOf<UUserWidget> InteractInfoWidgetClass;
	UPROPERTY(EditDefaultsOnly, Category = "BSPlayerController|Classes")
	TSubclassOf<URLAgentWidget> RLAgentWidgetClass;
	UPROPERTY(EditDefaultsOnly, Category = "BSPlayerController|Classes")
	TSubclassOf<AFloatingTextActor> FloatingTextActorClass;

private:
	UFUNCTION()
	void OnFadeScreenFromBlackFinish();
	UFUNCTION()
	void OnPlayerSettingsChanged(const FPlayerSettings_VideoAndSound& PlayerSettings);

	void TryResetAuthTicketHandle(const uint32 Handle);

	UPROPERTY()
	UMainMenuWidget* MainMenu;
	UPROPERTY()
	UCrossHairWidget* CrossHair;
	UPROPERTY()
	UPlayerHUD* PlayerHUD;
	UPROPERTY()
	UPauseMenuWidget* PauseMenu;
	UPROPERTY()
	UCountdownWidget* Countdown;
	UPROPERTY()
	UPostGameMenuWidget* PostGameMenuWidget;
	UPROPERTY()
	UFPSCounterWidget* FPSCounter;
	UPROPERTY()
	UScreenFadeWidget* ScreenFadeWidget;
	UPROPERTY()
	UUserWidget* InteractInfoWidget;
	UPROPERTY()
	URLAgentWidget* RLAgentWidget;
	
	bool CountdownActive;
	bool PlayerHUDActive = false;
	bool PostGameMenuActive = false;
	bool bIsLoggedIn = false;
	uint8 NumAuthTicketFinishes = 0;
	const int32 ZOrderFadeScreen = 20;
	const int32 ZOrderFPSCounter = 19;
	float LoadingScreenWidgetFadeOutTime = 0.75f;
};
