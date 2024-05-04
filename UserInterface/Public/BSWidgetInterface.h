// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Components/PanelWidget.h"
#include "EnumTagMap.h"
#include "WidgetComponents/Tooltips/TooltipImage.h"
#include "BSWidgetInterface.generated.h"

class UBSComboBoxEntry;
class UBSComboBoxString;
class UButton;
class UTooltipImage;
class UTooltipWidget;
class UEditableTextBox;
class USlider;

DECLARE_MULTICAST_DELEGATE(FOnExitAnimationCompleted);

/** Enum representing a setting button */
UENUM(BlueprintType)
enum class ESettingButtonType : uint8
{
	None UMETA(DisplayName="None"),
	Save UMETA(DisplayName="Save"),
	Reset UMETA(DisplayName="Reset"),
	Revert UMETA(DisplayName="Revert"),
	SaveAndRestart UMETA(DisplayName="Save And Restart"),
};

ENUM_RANGE_BY_FIRST_AND_LAST(ESettingButtonType, ESettingButtonType::Save, ESettingButtonType::SaveAndRestart);

/** Interface for commonly used objects and functions such as adding tooltips and syncing Sliders & TextBoxes */
UINTERFACE()
class UBSWidgetInterface : public UInterface
{
	GENERATED_BODY()
};

/** Interface for commonly used objects and functions such as adding tooltips and syncing Sliders & TextBoxes */
class USERINTERFACE_API IBSWidgetInterface
{
	GENERATED_BODY()

public:
	/** Clamps NewTextValue, updates associated Slider value while rounding to the GridSnapSize */
	static float OnEditableTextBoxChanged(const FText& NewTextValue, UEditableTextBox* TextBoxToChange,
		USlider* SliderToChange, const float GridSnapSize, const float Min, const float Max);

	/** Updates associated TextBoxToChange with result of rounding to the GridSnapSize */
	static float OnSliderChanged(const float NewValue, UEditableTextBox* TextBoxToChange, const float GridSnapSize);

	static void SetSliderAndEditableTextBoxValues(const float NewValue, UEditableTextBox* TextBoxToChange,
		USlider* SliderToChange, const float GridSnapSize, const float Min, const float Max);

	/** Override this function and then call SetupTooltip */
	virtual UTooltipWidget* ConstructTooltipWidget() = 0;

	/** Returns TooltipWidget */
	virtual UTooltipWidget* GetTooltipWidget() const { return nullptr; }

	/** Add tooltip text and bind the OnTooltipImageHovered function to a given TooltipImage */
	void SetupTooltip(UTooltipImage* TooltipImage, const FText& TooltipText, const bool bInAllowTextWrap = false);

	/** Add InTooltipData and bind the OnTooltipImageHovered function to a given TooltipImage */
	void SetupTooltip(const FTooltipData& InTooltipData);

	/** All Tooltip Images are bound to this function */
	UFUNCTION()
	virtual void OnTooltipImageHovered(UTooltipImage* TooltipImage, const FTooltipData& InTooltipData);

	/** Override this function to use OnGenerateWidgetEvent and OnSelectionChanged_GenerateMultiSelectionItem */
	virtual UBSComboBoxEntry* ConstructComboBoxEntryWidget() { return nullptr; }

	/** Returns the widget used for a ComboBox entry. Must override ConstructComboBoxEntryWidget */
	UFUNCTION()
	virtual UWidget* OnGenerateWidgetEvent(const UBSComboBoxString* ComboBoxString, FString Method);

	/** Returns the widget used for a selected ComboBox entry. Must override ConstructComboBoxEntryWidget */
	UFUNCTION()
	virtual UWidget* OnSelectionChanged_GenerateMultiSelectionItem(const UBSComboBoxString* ComboBoxString,
		const TArray<FString>& SelectedOptions);

	/** Returns the String Table key for a specific ComboBox, not the cleanest code but it works */
	virtual FString GetStringTableKeyFromComboBox(const UBSComboBoxString* ComboBoxString, const FString& EnumString);

	/** Simple template function to get default class object from subclass */
	template <typename T>
	static const T* GetStyleCDO(const TSubclassOf<T> InSubclass)
	{
		if (InSubclass)
		{
			if (const T* Style = Cast<T>(InSubclass->ClassDefaultObject))
			{
				return Style;
			}
		}
		return nullptr;
	}

	/** Returns the enum value corresponding to the string, or if not found, the the enum byte value of 0 for T */
	template <typename T>
	static T GetEnumFromString(const FString& InString)
	{
		const FString NewString = InString.Replace(*FString(" "), *FString("")).Replace(*FString("-"), *FString(""));
		const UEnum* const EnumClass = StaticEnum<T>();
		if (!EnumClass)
		{
			return static_cast<T>(0);
		}
		const int64 Value = EnumClass->GetValueByNameString(NewString);
		if (Value == INDEX_NONE)
		{
			UE_LOG(LogTemp, Warning, TEXT("Couldn't find Enum from String: %s"), *InString);
		}
		return Value == INDEX_NONE ? static_cast<T>(0) : static_cast<T>(Value);
	}

	/** Returns the string display name of the enum, or empty string if not found */
	template <typename T>
	static FString GetStringFromEnum(const T& InEnum)
	{
		const UEnum* const EnumClass = StaticEnum<T>();
		if (!EnumClass)
		{
			return FString();
		}
		return EnumClass->GetDisplayValueAsText(InEnum).ToString();
	}

	/** Returns an array of strings corresponding to each enum value using GetStringFromEnum */
	template <typename T>
	static TArray<FString> GetStringArrayFromEnumArray(const TArray<T>& InEnumArray)
	{
		TArray<FString> OutArray;
		for (const T& InEnum : InEnumArray)
		{
			OutArray.Add(GetStringFromEnum(InEnum));
		}
		return OutArray;
	}

	/** Returns an array of enums corresponding to each string using GetEnumFromString */
	template <typename T>
	static TArray<T> GetEnumArrayFromStringArray(const TArray<FString>& InStringArray)
	{
		TArray<T> OutArray;
		if (InStringArray.IsEmpty())
		{
			return OutArray;
		}
		for (const FString& InString : InStringArray)
		{
			OutArray.Add(GetEnumFromString<T>(InString));
		}
		return OutArray;
	}

	/** Returns the String table key for an enum provided that the string table key format is EnumName_EnumValue */
	template <typename T>
	static FString GetStringTableKeyNameFromEnum(const T& InEnum)
	{
		FString EnumName;
		FString EnumValue;
		const FString EnumString = UEnum::GetValueAsString(InEnum);
		if (EnumString.Contains("::"))
		{
			EnumString.Split("::", &EnumName, &EnumValue);
			return EnumName.RightChop(1) + "_" + EnumValue;
		}
		return EnumString;
	}

	/** Returns the String Table entry for a tooltip, provided a valid key */
	static FText GetTooltipTextFromKey(const FString& InKey)
	{
		const FText StringTableEntry = FText::FromStringTable("/Game/StringTables/ST_Tooltips.ST_Tooltips", InKey);
		if (InKey.IsEmpty() || StringTableEntry.IsEmpty())
		{
			UE_LOG(LogTemp, Warning, TEXT("Couldn't find String Table entry for key: %s"), *InKey);
		}
		return StringTableEntry;
	}

	/** Returns the String Table entry from the ST_Widgets string table, provided a valid key */
	static FText GetWidgetTextFromKey(const FString& InKey)
	{
		const FText StringTableEntry = FText::FromStringTable("/Game/StringTables/ST_Widgets.ST_Widgets", InKey);
		if (InKey.IsEmpty() || StringTableEntry.IsEmpty())
		{
			UE_LOG(LogTemp, Warning, TEXT("Couldn't find ST_Widgets entry for key: %s"), *InKey);
		}
		return StringTableEntry;
	}

	/** Returns an array of pointers of type T, recursively searching for all instances of T inside the PanelWidget */
	template <typename T>
	static TArray<T*> DescendWidget(UPanelWidget* PanelWidget)
	{
		TArray<T*> Boxes;
		// Check the PanelWidget first
		if (Cast<T>(PanelWidget))
		{
			Boxes.Add(Cast<T>(PanelWidget));
		}

		// Search through children
		for (UWidget* Child : PanelWidget->GetAllChildren())
		{
			// Child panel widget will get checked in first line of function
			if (UPanelWidget* ChildPanelWidget = Cast<UPanelWidget>(Child))
			{
				// Call function on any child's children
				Boxes.Append(DescendWidget<T>(ChildPanelWidget));
			}
		}
		return Boxes;
	}

	/** Returns a pointer of type T, recursively searching only until it finds the first instances of T inside the PanelWidget */
	template <typename T>
	static T* DescendWidgetReturnFirst(UPanelWidget* PanelWidget)
	{
		// Check the top level panel widget first
		if (Cast<T>(PanelWidget))
		{
			return Cast<T>(PanelWidget);
		}

		// Search through children
		for (UWidget* Child : PanelWidget->GetAllChildren())
		{
			// Child panel widget will get checked in first line of function
			if (UPanelWidget* ChildPanelWidget = Cast<UPanelWidget>(Child))
			{
				// Call function on any child's children
				if (T* FoundBorder = DescendWidgetReturnFirst<T>(ChildPanelWidget))
				{
					return FoundBorder;
				}
			}
		}
		return nullptr;
	}
};
