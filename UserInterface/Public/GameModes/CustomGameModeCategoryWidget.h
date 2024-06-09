// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Mappings/EnumTagMap.h"
#include "Utilities/BSSettingCategoryWidget.h"
#include "CustomGameModeCategoryWidget.generated.h"

enum class ETooltipIconType : uint8;
struct FValidationCheckKeyFuncs;
struct FValidationCheckResult;
struct FUniqueValidationCheckData;
enum class EGameModeCategory : uint8;
enum class EMenuOptionEnabledState : uint8;
struct FBSConfig;
struct FValidationResult;
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
	/** Sets BSConfig, sets the pointer to next widget in linked list. */
	virtual void InitComponent(const TSharedPtr<FBSConfig>& InConfig);

	/** Sets all custom game mode option values using BSConfig. */
	virtual void UpdateOptionsFromConfig();

	/** Creates or finds tooltip icons to display warnings and errors to the user.
	 *  @param CheckResults The validation check results to iterate through.
	 */
	virtual void HandlePropertyValidation(TSet<FValidationCheckResult, FValidationCheckKeyFuncs>& CheckResults);

	/** @return the game mode category the widget represents. */
	EGameModeCategory GetGameModeCategory() const { return GameModeCategory; }

	/** Executed when a property is changed by the user. */
	TDelegate<void(const TSet<uint32>&)> OnPropertyChanged;

	/** @return the number tooltip icons of the specified type. */
	int32 GetNumberOfDynamicTooltipIcons(ETooltipIconType Type);

protected:
	/** Adds a GameModeCategoryTagWidget for each matching GameplayTag on the Menu Option widget. */
	void AddGameModeCategoryTagWidgets(UMenuOptionWidget* MenuOptionWidget);

	/** Associates a BSConfig property with a MenuOptionWidget.
	 *  @param PropertyHash the property hashed using its unique properties (FProperty).
	 *  @param MenuOptionWidget the widget to associate the property with.
	 */
	void AssociatePropertyWithMenuOption(uint32 PropertyHash, UMenuOptionWidget* MenuOptionWidget);

	/** Set's the widget's enabled state, optionally adding a tooltip to the entire widget.
	 *  @param Widget the MenuOptionWidget to modify and optionally add a tooltip to.
	 *  @param State the state to set the widget to.
	 *  @param Key Adds a tooltip for the entire widget if provided, otherwise the tooltip will be cleared.
	 */
	void SetMenuOptionEnabledStateAndAddTooltip(UMenuOptionWidget* Widget, EMenuOptionEnabledState State,
		const FString& Key = FString());

	/** Set's the enabled state of a specific widget type in a menu option widget, optionally adding a tooltip to the
	 *  entire widget.
	 *  @param Widget the MenuOptionWidget to query for the sub-widget.
	 *  @param SubWidgetClass the specific widget type class to enable/disable.
	 *  @param State the state to set the sub-widget to.
	 *  @param Key Adds a tooltip to the queried sub-widget if provided, otherwise the tooltip will be cleared.
	 */
	void SetSubMenuOptionEnabledStateAndAddTooltip(UMenuOptionWidget* Widget,
		const TSubclassOf<UWidget>& SubWidgetClass, EMenuOptionEnabledState State, const FString& Key = FString());

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

	/** Shared pointer to the game mode config inside GameModesWidget. */
	TSharedPtr<FBSConfig> BSConfig;

	UPROPERTY()
	TArray<TWeakObjectPtr<UMenuOptionWidget>> MenuOptionWidgets;

	/** Whether to index this widget as part of the carousel. */
	UPROPERTY(EditAnywhere, Category="CustomGameModesWidgetComponent")
	bool bIndexOnCarousel = true;

	UPROPERTY(EditDefaultsOnly, Category="CustomGameModesWidgetComponent")
	TObjectPtr<UEnumTagMap> EnumTagMap;

	UPROPERTY(EditDefaultsOnly, Category="CustomGameModesWidgetComponent")
	TObjectPtr<UGameModeCategoryTagMap> GameModeCategoryTagMap;

	EGameModeCategory GameModeCategory;

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

private:
	/** Stores menu options associated with properties in BSConfig. */
	TMap<uint32, TWeakObjectPtr<UMenuOptionWidget>> PropertyMenuOptionWidgetMap;
};

template <typename T>
FString UCustomGameModeCategoryWidget::GetStringFromEnum_FromTagMap(const T& InEnum)
{
	if (EnumTagMap)
	{
		FString Found = EnumTagMap->FindStringFromEnum<T>(InEnum);
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
