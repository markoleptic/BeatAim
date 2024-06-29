// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PauseMenuWidget.generated.h"

class UMenuStyle;
class UQuitMenuWidget;
class UVerticalBox;
class UWidgetSwitcher;
class USettingsMenuWidget;
class UFAQWidget;
class UTextBlock;
class UBSButton;
class UMenuButton;

DECLARE_DELEGATE(FResumeGame);

/** Widget displayed if the Escape key is pressed, or the SteamOverlay is activated. */
UCLASS()
class USERINTERFACE_API UPauseMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Delegate that gets bound to lambda function in DefaultPlayerController. */
	FResumeGame ResumeGame;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UQuitMenuWidget> QuitMenuWidget;

protected:
	virtual void NativeConstruct() override;
	virtual void NativePreConstruct() override;

	UPROPERTY(EditDefaultsOnly, Category = "MainMenuWidget")
	TSubclassOf<UMenuStyle> MenuStyleClass;

	UPROPERTY()
	const UMenuStyle* MenuStyle;

	void SetStyles();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UVerticalBox* Box_PauseScreen;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UVerticalBox* Box_SettingsMenu;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UVerticalBox* Box_FAQ;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	USettingsMenuWidget* SettingsMenuWidget;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UFAQWidget* FAQWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UMenuButton* MenuButton_Resume;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UMenuButton* MenuButton_RestartCurrentMode;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UMenuButton* MenuButton_Settings;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UMenuButton* MenuButton_FAQ;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UMenuButton* MenuButton_Quit;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UWidgetSwitcher* PauseMenuSwitcher;
	/** Animation to fade in the widget. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* FadeInPauseMenu;

private:
	UFUNCTION()
	void FadeInWidget() { PlayAnimationForward(FadeInPauseMenu); }

	UFUNCTION()
	void SetQuitMenuButtonsInActive();

	void OnButtonClicked_BSButton(const UBSButton* Button);

	UFUNCTION()
	void OnButtonClicked_RestartCurrentMode();
};
