// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SaveGamePlayerSettings.h"
#include "BSPlayerSettingsInterface.h"
#include "SubmenuWidgets/SettingsWidgets/BSSettingCategoryWidget.h"
#include "SettingsMenuWidget_Game.generated.h"

class USliderTextBoxOptionWidget;
class UCheckBoxOptionWidget;
class UColorSelectOptionWidget;
class UCheckBox;
class UEditableTextBox;
class UBSButton;
class UColorSelectWidget;
class USavedTextWidget;

/** Settings category widget holding game settings */
UCLASS()
class USERINTERFACE_API USettingsMenuWidget_Game : public UBSSettingCategoryWidget, public IBSPlayerSettingsInterface
{
	GENERATED_BODY()

	virtual void NativeConstruct() override;

public:
	/** Returns OnPlayerSettingsChangedDelegate_Game, the delegate that is broadcast when this class saves Game settings */
	FOnPlayerSettingsChanged_Game& GetPublicGameSettingsChangedDelegate()
	{
		return OnPlayerSettingsChangedDelegate_Game;
	}

protected:
	/** Fills all widgets with values from NewSettings */
	void InitializeGameSettings(const FPlayerSettings_Game& PlayerSettings_Game);

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UColorSelectOptionWidget* MenuOption_StartTargetColor;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UColorSelectOptionWidget* MenuOption_PeakTargetColor;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UColorSelectOptionWidget* MenuOption_EndTargetColor;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCheckBoxOptionWidget* MenuOption_UseSeparateOutlineColor;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UColorSelectOptionWidget* MenuOption_TargetOutlineColor;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UColorSelectOptionWidget* MenuOption_InactiveColor;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UColorSelectOptionWidget* MenuOption_TakingTrackingDamageColor;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UColorSelectOptionWidget* MenuOption_NotTakingTrackingDamageColor;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCheckBoxOptionWidget* MenuOption_ShowStreakCombatText;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCheckBoxOptionWidget* MenuOption_Recoil;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCheckBoxOptionWidget* MenuOption_AutomaticFire;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCheckBoxOptionWidget* MenuOption_ShowBulletDecals;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCheckBoxOptionWidget* MenuOption_ShowBulletTracers;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCheckBoxOptionWidget* MenuOption_ShowMuzzleFlash;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCheckBoxOptionWidget* MenuOption_ShowMesh;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCheckBoxOptionWidget* MenuOption_ShowHitTimingWidget;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USliderTextBoxOptionWidget* MenuOption_CombatTextFrequency;

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

	void OnSliderTextBoxValueChanged(USliderTextBoxOptionWidget* Widget, const float Value);

	void OnButtonClicked_BSButton(const UBSButton* Button);

	void OnButtonClicked_Save();
	void OnButtonClicked_Reset();
	void OnButtonClicked_Revert();

	/** The Player Settings at the time of opening the menu */
	FPlayerSettings_Game InitialGameSettings;

	/** The Player Settings that are changed during setting menu interaction */
	FPlayerSettings_Game NewGameSettings;
};
