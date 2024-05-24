// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CustomGameModeCategoryWidget.h"
#include "CustomGameModeStartWidget.generated.h"

class UTextInputWidget;
class UCheckBoxWidget;
class UComboBoxWidget;
struct FBS_DefiningConfig;

template <typename DelegateType, typename ObjectType, typename FunctionType>
class TSignalBlocker
{
public:
	TSignalBlocker(DelegateType& InDelegate, ObjectType* InObject, FunctionType InFunctionType) : Delegate(InDelegate),
		Object(InObject), Function(InFunctionType)
	{
		bWasDelegateBound = Delegate.IsBound();
		if (bWasDelegateBound)
		{
			Delegate.RemoveDynamic(Object, Function);
		}
	}

	~TSignalBlocker()
	{
		Delegate.AddDynamic(Object, Function);
	}

private:
	DelegateType& Delegate;
	ObjectType* Object;
	FunctionType Function;
	bool bWasDelegateBound;
};


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
		if (bUseTemplateChecked == Other.bUseTemplateChecked)
		{
			if (bIsPreset == Other.bIsPreset && bIsCustom == Other.bIsCustom)
			{
				if (NewCustomGameModeName.Equals(Other.NewCustomGameModeName, ESearchCase::CaseSensitive))
				{
					if (GameModeName.Equals(Other.NewCustomGameModeName, ESearchCase::CaseSensitive))
					{
						if (Difficulty.Equals(Other.Difficulty))
						{
							return true;
						}
					}
				}
			}
		}
		return false;
	}
};

UCLASS()
class USERINTERFACE_API UCustomGameModeStartWidget : public UCustomGameModeCategoryWidget
{
	GENERATED_BODY()

public:
	UCustomGameModeStartWidget();
	/** Broadcast when the user changes the template checkbox, template combo box, or difficulty combo box. */
	//FRequestGameModeTemplateUpdate RequestGameModeTemplateUpdate;

	/** Broadcast when the user changes the EditableTextBoxOption_CustomGameModeName. */
	//FRequestButtonStateUpdate OnCustomGameModeNameChanged;

	/** Returns the value of EditableTextBoxOption_CustomGameModeName. */
	//FString GetNewCustomGameModeName() const;

	/** Returns the options for a start widget since they're not all shared with BSConfig pointer. */
	FStartWidgetProperties GetStartWidgetProperties() const;

	/** Sets the value of EditableTextBoxOption_CustomGameModeName. */
	//void SetNewCustomGameModeName(const FString& InCustomGameModeName) const;

	/** Sets the options for a start widget since they're not all shared with BSConfig pointer. */
	void SetStartWidgetProperties(const FStartWidgetProperties& InProperties);

	/** Updates the Difficulty ComboBox selection if different from BSConfig, or if none selected with Preset.
	 *  Returns true if the selection was changed. */
	//bool UpdateDifficultySelection(const EGameModeDifficulty& Difficulty) const;

	/** Updates the Difficulty ComboBox visibility based on the type of game mode. Returns true if the visibility
	 *  was changed. */
	//bool UpdateDifficultyVisibility() const;

	/** Updates the GameModeTemplate ComboBox visibility based on the type of game mode and the checkbox.
	 *  Returns true if the visibility was changed. */
	//bool UpdateGameModeTemplateVisibility() const;

	/** Clears all GameModeTemplate options and repopulates. */
	void RefreshGameModeTemplateComboBoxOptions(const TArray<FBSConfig>& CustomGameModes) const;

	TDelegate<void(const FStartWidgetProperties&)> OnStartWidgetPropertyChanged;

protected:
	virtual void NativeConstruct() override;
	//virtual void UpdateAllOptionsValid() override;
	//virtual void UpdateOptionsFromConfig() override;

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

	FStartWidgetProperties StartWidgetProperties;
};
