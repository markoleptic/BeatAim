// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BSPlayerSettingsInterface.h"
#include "SaveGames/SaveGamePlayerSettings.h"
#include "Utilities/BSSettingCategoryWidget.h"
#include "GameSettingsWidget.generated.h"

class USingleRangeInputWidget;
class UCheckBoxWidget;
class UColorSelectOptionWidget;
class UCheckBox;
class UEditableTextBox;
class UBSButton;
class UColorSelectWidget;
class USavedTextWidget;

/** Settings category widget holding game settings */
UCLASS()
class USERINTERFACE_API UGameSettingsWidget : public UBSSettingCategoryWidget, public IBSPlayerSettingsInterface
{
	GENERATED_BODY()

	virtual void NativeConstruct() override;

public:
	/** Returns OnPlayerSettingsChangedDelegate_Game, the delegate that is broadcast when this class saves Game
	 *  settings. */
	FOnPlayerSettingsChanged_Game& GetPublicGameSettingsChangedDelegate()
	{
		return OnPlayerSettingsChangedDelegate_Game;
	}

protected:
	/** Fills all widgets with values from NewSettings. */
	void InitializeGameSettings(const FPlayerSettings_Game& PlayerSettings_Game);

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UColorSelectWidget* MenuOption_StartTargetColor;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UColorSelectWidget* MenuOption_PeakTargetColor;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UColorSelectWidget* MenuOption_EndTargetColor;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCheckBoxWidget* MenuOption_UseSeparateOutlineColor;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UColorSelectWidget* MenuOption_TargetOutlineColor;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UColorSelectWidget* MenuOption_InactiveColor;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UColorSelectWidget* MenuOption_TakingTrackingDamageColor;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UColorSelectWidget* MenuOption_NotTakingTrackingDamageColor;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCheckBoxWidget* MenuOption_ShowStreakCombatText;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCheckBoxWidget* MenuOption_Recoil;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCheckBoxWidget* MenuOption_AutomaticFire;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCheckBoxWidget* MenuOption_ShowBulletDecals;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCheckBoxWidget* MenuOption_ShowBulletTracers;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCheckBoxWidget* MenuOption_ShowMuzzleFlash;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCheckBoxWidget* MenuOption_ShowMesh;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCheckBoxWidget* MenuOption_ShowHitTimingWidget;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USingleRangeInputWidget* MenuOption_CombatTextFrequency;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USavedTextWidget* SavedTextWidget;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Saving")
	UBSButton* Button_Save;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Saving")
	UBSButton* Button_Revert;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Saving")
	UBSButton* Button_Reset;

	UFUNCTION()
	void OnColorChanged_StartTarget(const FLinearColor& NewColor);
	UFUNCTION()
	void OnOnColorChanged_PeakTarget(const FLinearColor& NewColor);
	UFUNCTION()
	void OnOnColorChanged_EndTarget(const FLinearColor& NewColor);
	UFUNCTION()
	void OnColorChanged_TargetOutline(const FLinearColor& NewColor);
	UFUNCTION()
	void OnColorChanged_BeatGridInactive(const FLinearColor& NewColor);
	UFUNCTION()
	void OnColorChanged_TakingTrackingDamageColor(const FLinearColor& NewColor);
	UFUNCTION()
	void OnColorChanged_NotTakingTrackingDamageColor(const FLinearColor& NewColor);

	UFUNCTION()
	void OnCheckStateChanged_UseSeparateOutlineColor(const bool bIsChecked);
	UFUNCTION()
	void OnCheckStateChanged_ShowCombatText(const bool bIsChecked);
	UFUNCTION()
	void OnCheckStateChanged_Recoil(const bool bIsChecked);
	UFUNCTION()
	void OnCheckStateChanged_AutomaticFire(const bool bIsChecked);
	UFUNCTION()
	void OnCheckStateChanged_ShowBulletDecals(const bool bIsChecked);
	UFUNCTION()
	void OnCheckStateChanged_ShowBulletTracers(const bool bIsChecked);
	UFUNCTION()
	void OnCheckStateChanged_ShowMuzzleFlash(const bool bIsChecked);
	UFUNCTION()
	void OnCheckStateChanged_ShowMesh(const bool bIsChecked);
	UFUNCTION()
	void OnCheckStateChanged_ShowHitTimingWidget(const bool bIsChecked);

	void OnSliderTextBoxValueChanged(USingleRangeInputWidget* Widget, const float Value);

	void OnButtonClicked_BSButton(const UBSButton* Button);

	void OnButtonClicked_Save();
	void OnButtonClicked_Reset();
	void OnButtonClicked_Revert();

	/** The Player Settings at the time of opening the menu. */
	FPlayerSettings_Game InitialGameSettings;

	/** The Player Settings that are changed during setting menu interaction. */
	FPlayerSettings_Game NewGameSettings;
};
