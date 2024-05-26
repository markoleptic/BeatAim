// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CustomGameModeCategoryWidget.h"
#include "CustomGameModeStartWidget.generated.h"

class UTextInputWidget;
class UCheckBoxWidget;
class UComboBoxWidget;

USTRUCT()
struct USERINTERFACE_API FStartWidgetProperties
{
	GENERATED_BODY()

	bool bUseTemplateChecked;
	bool bIsPreset;
	bool bIsCustom;
	FString GameModeName;
	FString Difficulty;
	FString NewCustomGameModeName;

	FStartWidgetProperties() : bUseTemplateChecked(false), bIsPreset(false), bIsCustom(false)
	{
	}

	FStartWidgetProperties(const bool bInUseTemplate, const bool bInIsPreset, const bool bInIsCustom,
		const FString& InGameModeName, const FString& InDifficulty, const FString& InNewCustomGameModeName):
		bUseTemplateChecked(bInUseTemplate), bIsPreset(bInIsPreset), bIsCustom(bInIsCustom),
		GameModeName(InGameModeName), Difficulty(InDifficulty), NewCustomGameModeName(InNewCustomGameModeName)
	{
	}

	FORCEINLINE bool operator==(const FStartWidgetProperties& Other) const
	{
		return bUseTemplateChecked == Other.bUseTemplateChecked && bIsPreset == Other.bIsPreset && bIsCustom == Other.
			bIsCustom && GameModeName.Equals(Other.NewCustomGameModeName, ESearchCase::CaseSensitive) && Difficulty.
			Equals(Other.Difficulty) && NewCustomGameModeName.Equals(Other.NewCustomGameModeName,
				ESearchCase::CaseSensitive);
	}
};

UCLASS()
class USERINTERFACE_API UCustomGameModeStartWidget : public UCustomGameModeCategoryWidget
{
	GENERATED_BODY()

public:
	UCustomGameModeStartWidget();

	/** @return reference to static properties that define a custom game mode start widget. */
	static FStartWidgetProperties& GetProperties();

	/** Updates widgets using static StartWidgetProperties. */
	void RefreshProperties() const;

	/** Clears all game mode combo box options and repopulates. */
	void RefreshGameModes(const TArray<FBSConfig>& CustomGameModes) const;

	/** Executed when a property is changed by a user. */
	static TDelegate<void(FStartWidgetProperties&)> OnStartWidgetPropertyChanged;

protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnCheckStateChanged_UseTemplate(const bool bChecked);
	UFUNCTION()
	void OnTextChanged_CustomGameModeName(const FText& Text);
	UFUNCTION()
	void OnSelectionChanged_GameModeTemplates(const TArray<FString>& Selected, const ESelectInfo::Type SelectionType);
	UFUNCTION()
	void OnSelectionChanged_GameModeDifficulty(const TArray<FString>& Selected, const ESelectInfo::Type SelectionType);

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCheckBoxWidget* CheckBoxOption_UseTemplate;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UComboBoxWidget* ComboBoxOption_GameModeTemplates;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UComboBoxWidget* ComboBoxOption_GameModeDifficulty;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextInputWidget* EditableTextBoxOption_CustomGameModeName;

	static FStartWidgetProperties StartWidgetProperties;
};
