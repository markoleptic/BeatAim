// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BSWidgetInterface.h"
#include "EnumTagMap.h"
#include "MenuOptionWidget.h"
#include "WidgetComponents/Boxes/BSComboBoxString.h"
#include "ComboBoxOptionWidget.generated.h"

class UGameModeCategoryTagMap;
class UBSComboBoxEntry_Tagged;
class UEnumTagMap;
class UGameModeCategoryTagWidget;
class UBSComboBoxString;

DECLARE_DELEGATE_RetVal_OneParam(FString, FGetComboBoxEntryTooltipStringTableKey, const FString& EnumString);

UCLASS()
class USERINTERFACE_API UComboBoxOptionWidget : public UMenuOptionWidget, public IBSWidgetInterface
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UBSComboBoxString* ComboBox;

	/** Executed when a ComboBoxEntry requests a tooltip description. If an empty string is returned, no tooltip image is shown */
	FGetComboBoxEntryTooltipStringTableKey GetComboBoxEntryTooltipStringTableKey;

	/** Sorts the array alphabetically and adds each option to the ComboBox */
	void SortAndAddOptions(TArray<FString>& InOptions);

	/** Sets the correct EnumTagMapping from the EnumTagMap depending on the enum class */
	template <typename T>
	void SetEnumType();

	/** Combines SetEnumType and SortAndAddOptions */
	template <typename T>
	void SortAddOptionsAndSetEnumType(TArray<FString>& InOptions);

	/** Sets the GameplayTagWidgetMap */
	void SetGameplayTagWidgetMap(const TObjectPtr<UGameModeCategoryTagMap> InMap);

	/** Sets the EnumTagMap */
	void SetEnumTagMap(const TObjectPtr<UEnumTagMap> InMap);

protected:
	virtual void NativeConstruct() override;
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual UTooltipWidget* IBSWidgetInterface::ConstructTooltipWidget() override { return nullptr; }
	virtual UBSComboBoxEntry* ConstructComboBoxEntryWidget() override;
	virtual UWidget* OnGenerateWidgetEvent(const UBSComboBoxString* ComboBoxString, FString Method) override;
	virtual FString
	GetStringTableKeyFromComboBox(const UBSComboBoxString* ComboBoxString, const FString& EnumString) override;

	/** Adds GameModeCategoryTagWidgets to the ComboBox entry if matching tags are found */
	UWidget* AddGameModeCategoryTagWidgets(UBSComboBoxEntry_Tagged* ComboBoxEntry);

	/** Pointer to CustomGameModesWidgetComponent's GameplayTagWidgetMap */
	UPROPERTY()
	TObjectPtr<UGameModeCategoryTagMap> GameModeCategoryTagMap;;

	/** Pointer to CustomGameModesWidgetComponent's EnumTagMap */
	UPROPERTY()
	TObjectPtr<UEnumTagMap> EnumTagMap;

	/** The enum to gameplay tag mapping for this combo box option widget */
	FEnumTagMapping EnumTagMapping;

	/** The type of selection for the combo box */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MenuOptionWidget|ComboBox")
	ESelectionModeType SelectionMode = ESelectionModeType::Single;

	/** Should the user be able to select no options */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MenuOptionWidget|ComboBox")
	bool bCanSelectNone = false;

	/** Should the combo box automatically close when a selection is made */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MenuOptionWidget|ComboBox")
	bool bCloseComboBoxOnSelectionChanged = true;
};

template <typename T>
void UComboBoxOptionWidget::SetEnumType()
{
	if (!EnumTagMap)
	{
		UE_LOG(LogTemp, Warning, TEXT("No EnumTagMap found."));
		return;
	}
	if (const FEnumTagMapping* FoundEnumTagMapping = EnumTagMap->GetEnumTagMapping<T>())
	{
		EnumTagMapping = *FoundEnumTagMapping;
	}
}

template <typename T>
void UComboBoxOptionWidget::SortAddOptionsAndSetEnumType(TArray<FString>& InOptions)
{
	if (!EnumTagMap)
	{
		UE_LOG(LogTemp, Warning, TEXT("No EnumTagMap found."));
		return;
	}
	if (const FEnumTagMapping* FoundEnumTagMapping = EnumTagMap->GetEnumTagMapping<T>())
	{
		EnumTagMapping = *FoundEnumTagMapping;
	}
	SortAndAddOptions(InOptions);
}
