﻿// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Blueprint/UserWidget.h"
#include "WidgetComponents/Tooltips/TooltipImage.h"
#include "MenuOptionWidget.generated.h"

class UMenuOptionStyle;
class UGameModeCategoryTagWidget;
class UCheckBox;
class UTextBlock;
class UEditableTextBox;
class USpacer;
class UHorizontalBox;
class USlider;
class UBSHorizontalBox;
class UMenuOptionWidget;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnLockStateChanged, UMenuOptionWidget*, const bool);

/** Base class for a Menu Option Widget, which is basically just a description, tooltip, and some value(s) that can be changed */
UCLASS()
class USERINTERFACE_API UMenuOptionWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void SetStyling();

public:
	/** Sets the left hand side indent, with each level increasing by 50 */
	void SetIndentLevel(const int32 Value);

	/** Toggles showing the TooltipImage */
	void SetShowTooltipImage(const bool bShow);

	/** Toggles showing the CheckBoxLock */
	void SetShowCheckBoxLock(const bool bShow);

	/** Sets the Description Text */
	void SetDescriptionText(const FText& InText);

	/** Sets the TooltipImage Text */
	void SetTooltipText(const FText& InText);

	/** Returns the TooltipImage */
	UTooltipImage* GetTooltipImage() const;

	/** Returns the TooltipImage Text */
	FText GetTooltipImageText() const { return DescriptionTooltipText; }

	/** Returns true if locked */
	bool GetIsLocked() const;

	/** Sets the locked state */
	void SetIsLocked(const bool bLocked) const;

	/** Returns value of bShowTooltipImage */
	bool ShouldShowTooltip() const { return bShowTooltipImage; }

	/** Broadcasts the new state of the lock and the index */
	FOnLockStateChanged OnLockStateChanged;

	/** Adds a Warning Tooltip to TooltipData array. Returns update delegate */
	FUpdateTooltipState& AddWarningTooltipData(const FTooltipData& InTooltipData);

	/** Adds a Dynamic Warning Tooltip to TooltipData array. Returns update delegate */
	FUpdateDynamicTooltipState& AddDynamicWarningTooltipData(const FTooltipData& InTooltipData,
		const FString& FallbackStringTableKey, const float InMin, const int32 InPrecision = 0);

	/** Calls UpdateWarningTooltips and UpdateDynamicWarningTooltips, which update the TooltipData by executing delegates on each FTooltipData struct */
	void UpdateAllWarningTooltips();

	/** Creates a TooltipImage if the TooltipImage in InTooltipData is not valid. Adds the widget to TooltipBox */
	void ConstructTooltipWarningImageIfNeeded(FTooltipData& InTooltipData);

	/** Returns by reference TooltipData array. This contains tooltip info for all Caution or Warning Tooltips */
	TArray<FTooltipData>& GetTooltipWarningData() { return WarningTooltipData; }

	/** Returns number of visible Warning Tooltips */
	int32 GetNumberOfWarnings();

	/** Returns number of visible Caution Tooltips */
	int32 GetNumberOfCautions();

	/** Returns Game mode category tags associated with this menu option */
	void GetGameModeCategoryTags(FGameplayTagContainer& OutTags) const
	{
		return OutTags.AppendTags(GameModeCategoryTags);
	}

	/** Adds the widget to Box_TagWidgets */
	void AddGameModeCategoryTagWidgets(TArray<UGameModeCategoryTagWidget*>& InGameModeCategoryTagWidgets);

protected:
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UHorizontalBox* Box_TagWidgets;

	/** Executes UpdateTooltipState on each Warning Tooltip in TooltipData array. Calls SetShouldShowTooltipImage on result */
	void UpdateWarningTooltips();

	/** Executes UpdateDynamicTooltipState on each Warning Tooltip in TooltipData array. Calls SetShouldShowTooltipImage on result */
	void UpdateDynamicWarningTooltips();

	UFUNCTION()
	void OnCheckBox_LockStateChanged(const bool bChecked);

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UBSHorizontalBox* BSBox;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTooltipImage* DescriptionTooltip;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UHorizontalBox* TooltipBox;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UHorizontalBox* Box_Left;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UHorizontalBox* Box_TagsAndTooltips;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	USpacer* Indent_Left;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* TextBlock_Description;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	UCheckBox* CheckBox_Lock;

	UPROPERTY(EditDefaultsOnly, Category="MenuOptionWidget|Style")
	TSubclassOf<UMenuOptionStyle> MenuOptionStyleClass;

	UPROPERTY()
	const UMenuOptionStyle* MenuOptionStyle;

	/** Text that describes the values this widget controls */
	UPROPERTY(EditInstanceOnly, Category="MenuOptionWidget")
	FText DescriptionText = FText();

	UPROPERTY(EditInstanceOnly, Category="MenuOptionWidget")
	int32 IndentLevel = 0;

	UPROPERTY(EditInstanceOnly, Category="MenuOptionWidget")
	bool bShowCheckBoxLock = false;

	UPROPERTY(EditInstanceOnly, Category="MenuOptionWidget|Tooltip")
	bool bShowTooltipImage = true;

	/** Text to show on the tooltip */
	UPROPERTY(EditInstanceOnly, Category="MenuOptionWidget|Tooltip")
	FText DescriptionTooltipText = FText();

	/** The categories this menu option represents */
	UPROPERTY(EditInstanceOnly, Category="MenuOptionWidget|GameModeCategoryTags")
	FGameplayTagContainer GameModeCategoryTags;

	/** Contains tooltip info for all Caution or Warning Tooltips. Size of array never changes after NativeConstruct has been called. */
	TArray<FTooltipData> WarningTooltipData;
};
