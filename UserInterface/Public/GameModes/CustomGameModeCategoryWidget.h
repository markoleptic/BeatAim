// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CustomGameModeWidget.h"
#include "EnumTagMap.h"
#include "Utilities/BSSettingCategoryWidget.h"
#include "CustomGameModeCategoryWidget.generated.h"

enum class EMenuOptionEnabledState : uint8;
struct FBSConfig;
class UToggleableSingleRangeInputWidget;
class UDualRangeInputWidget;
class UGameModeCategoryTagMap;
class UMenuOptionWidget;
class UTextInputWidget;
class UCheckBoxWidget;
class UComboBoxWidget;
class USingleRangeInputWidget;
class UCustomGameModeCategoryWidget;


/** Base class for Custom Game Mode Category widgets. */
UCLASS(Abstract)
class USERINTERFACE_API UCustomGameModeCategoryWidget : public UBSSettingCategoryWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

public:
	/** Sets BSConfig, sets the pointer to next widget in linked list, and calls UpdateOptionsFromConfig. */
	virtual void InitComponent(TSharedPtr<FBSConfig> InConfig, const int32 InIndex);

	/** Sets all custom game mode option values using the BSConfig pointer. Only changes the values if different.
	 *  Only called during transitions. */
	virtual void UpdateOptionsFromConfig();

	/** Broadcast when a caution/warning tooltip needed to be added, removed, or updated. Helps synchronize
	 *  caution/warnings across different components. */
	TMulticastDelegate<void()> RequestComponentUpdate;

	/** Broadcast when a widget wants to refresh the preview after a change to the config. */
	TMulticastDelegate<void()> RequestGameModePreviewUpdate;

	/** Returns whether Init has been called. */
	bool IsInitialized() const { return bIsInitialized; }

	/** Checks all custom game mode options for validity by calling UpdateWarningTooltips and broadcasts
	 *  RequestComponentUpdate if any are not valid. Should be called anytime an option is changed. */
	virtual void UpdateAllOptionsValid();

	/** Returns the struct containing info about the number of caution and warnings current present. */
	FCustomGameModeCategoryInfo* GetCustomGameModeCategoryInfo() { return &CustomGameModeCategoryInfo; }

	/** Returns the index set during initialization. */
	int32 GetIndex() const { return Index; }

	/** Returns true if the widget should be indexed on the carousel. */
	bool ShouldIndexOnCarousel() const { return bIndexOnCarousel; }

protected:
	/** Adds a GameModeCategoryTagWidget for each matching GameplayTag on the Menu Option widget. */
	void AddGameModeCategoryTagWidgets(UMenuOptionWidget* MenuOptionWidget);

	/** Updates the slider and editable text box values if different from Value. */
	static bool UpdateValueIfDifferent(const USingleRangeInputWidget* Widget, const float Value);

	/** Updates the combo box selection if NewOption is different from existing. */
	static bool UpdateValueIfDifferent(const UComboBoxWidget* Widget, const FString& NewOption);

	/** Updates the combo box selection if NewOptions is different from existing. */
	static bool UpdateValueIfDifferent(const UComboBoxWidget* Widget, const TArray<FString>& NewOptions);

	/** Updates the checkbox checked state if bIsChecked is different from existing. */
	static bool UpdateValueIfDifferent(const UCheckBoxWidget* Widget, const bool bIsChecked);

	/** Updates the editable text box if NewText is different from existing. */
	static bool UpdateValueIfDifferent(const UTextInputWidget* Widget, const FText& NewText);

	/** Updates the Min and Max slider and editable text box values if different from Min and Max. Also updates the
	 *  checked state. */
	static bool UpdateValuesIfDifferent(const UDualRangeInputWidget* Widget, const bool bIsChecked, const float Min,
		const float Max);

	/** Updates the slider and editable text box values if different from Value,
	 *  but only if the checkbox is unchecked. */
	static bool UpdateValuesIfDifferent(const UToggleableSingleRangeInputWidget* Widget, const bool bIsChecked,
		const float Value);

	/** Iterates through all MenuOptionWidgets, calling UpdateAllWarningTooltips on each. Iterates through each
	 *  widget's TooltipWarningData, checking if any changed from the update. If so, the tooltip is updated.
	 *  Calls UpdateCustomGameModeCategoryInfo when finished. Returns false if any tooltips required an update. */
	bool UpdateWarningTooltips();

	/** Iterates through all MenuOptionWidgets to sum the total of Warning and Caution tooltips visible.
	 *  Updates CustomGameModeCategoryInfo struct. */
	void UpdateCustomGameModeCategoryInfo();

	float GetMinRequiredHorizontalSpread() const;
	float GetMinRequiredVerticalSpread() const;
	float GetMaxTargetDiameter() const;
	int32 GetMaxAllowedNumHorizontalTargets() const;
	int32 GetMaxAllowedNumVerticalTargets() const;
	float GetMaxAllowedHorizontalSpacing() const;
	float GetMaxAllowedVerticalSpacing() const;
	float GetMaxAllowedTargetScale() const;

	/** Set's the widget's enabled state and adds a tooltip for the entire widget if a Key is provided,
	 *  otherwise the tooltip will be cleared. */
	void SetMenuOptionEnabledStateAndAddTooltip(UMenuOptionWidget* Widget, const EMenuOptionEnabledState State,
		const FString& Key = FString());

	/** Set's the sub widget's enabled state and adds a tooltip for just the sub widget if a Key is provided,
	*   otherwise the tooltip will be cleared. */
	void SetSubMenuOptionEnabledStateAndAddTooltip(UMenuOptionWidget* Widget, const TSubclassOf<UWidget> SubWidgetClass,
		const EMenuOptionEnabledState State, const FString& Key = FString());

	/** Shared pointer to the game mode config inside GameModesWidget. */
	TSharedPtr<FBSConfig> BSConfig;

	/** Pointer to next widget in linked list. Used for CreatorView. */
	UPROPERTY()
	TWeakObjectPtr<UCustomGameModeCategoryWidget> Next;

	/** Whether Init has been called. */
	bool bIsInitialized = false;

	/** Index used for parent widgets. */
	int32 Index = -1;

	/** Struct containing info about NumWarning & NumCaution tooltips. */
	FCustomGameModeCategoryInfo CustomGameModeCategoryInfo;

	UPROPERTY()
	TArray<TWeakObjectPtr<UMenuOptionWidget>> MenuOptionWidgets;

	/** Whether to index this widget as part of the carousel. */
	UPROPERTY(EditAnywhere, Category="CustomGameModesWidgetComponent")
	bool bIndexOnCarousel = true;

	UPROPERTY(EditDefaultsOnly, Category="CustomGameModesWidgetComponent")
	TObjectPtr<UEnumTagMap> EnumTagMap;

	UPROPERTY(EditDefaultsOnly, Category="CustomGameModesWidgetComponent")
	TObjectPtr<UGameModeCategoryTagMap> GameModeCategoryTagMap;

	/** Returns the string display name of the enum, or empty string if not found. Requires EnumTagMap.  */
	template <typename T>
	FString GetStringFromEnum_FromTagMap(const T& InEnum);

	/** Returns an array of string display names corresponding to the InEnumArray. Requires EnumTagMap . */
	template <typename T>
	TArray<FString> GetStringArrayFromEnumArray_FromTagMap(const TArray<T>& InEnumArray);

	/** Returns the enum value based on the string display name. Requires EnumTagMap. */
	template <typename T>
	T GetEnumFromString_FromTagMap(const FString& InString);

	/** Returns an array of enum values based on the string display names. Requires EnumTagMap .*/
	template <typename T>
	TArray<T> GetEnumArrayFromStringArray_FromTagMap(const TArray<FString>& InStringArray);
};

template <typename T>
FString UCustomGameModeCategoryWidget::GetStringFromEnum_FromTagMap(const T& InEnum)
{
	if (EnumTagMap)
	{
		FString Found = EnumTagMap->GetStringFromEnumTagPair<T>(InEnum);
		if (!Found.IsEmpty())
		{
			return Found;
		}
	}
	return GetStringFromEnum(InEnum);
}

template <typename T>
TArray<FString> UCustomGameModeCategoryWidget::GetStringArrayFromEnumArray_FromTagMap(const TArray<T>& InEnumArray)
{
	TArray<FString> OutArray;
	for (const T& InEnum : InEnumArray)
	{
		OutArray.Add(GetStringFromEnum_FromTagMap<T>(InEnum));
	}
	return OutArray;
}

template <typename T>
T UCustomGameModeCategoryWidget::GetEnumFromString_FromTagMap(const FString& InString)
{
	if (EnumTagMap)
	{
		return EnumTagMap->FindEnumFromString<T>(InString);
	}
	return GetEnumFromString<T>(InString);
}

template <typename T>
TArray<T> UCustomGameModeCategoryWidget::GetEnumArrayFromStringArray_FromTagMap(const TArray<FString>& InStringArray)
{
	TArray<T> OutArray;
	if (InStringArray.IsEmpty())
	{
		return OutArray;
	}
	for (const FString& InString : InStringArray)
	{
		OutArray.Add(GetEnumFromString_FromTagMap<T>(InString));
	}
	return OutArray;
}
