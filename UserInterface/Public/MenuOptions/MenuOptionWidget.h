﻿// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Blueprint/UserWidget.h"
#include "Utilities/TooltipIcon.h"
#include "MenuOptionWidget.generated.h"

struct FUniqueValidationCheckData;
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

/** The custom enabled state of a menu option. */
UENUM(BlueprintType)
enum class EMenuOptionEnabledState : uint8
{
	/** All features enabled. */
	Enabled UMETA(DisplayName="Enabled"),
	/** The option is not interactive because it depends on another menu option selection, but does show a tooltip
	 *  showing why it is not interactive. */
	DependentMissing UMETA(DisplayName="DependentMissing"),
	/** The option is not interactive and displays no tooltip. Same as regular disabled. */
	Disabled UMETA(DisplayName="Disabled"),
};

ENUM_RANGE_BY_FIRST_AND_LAST(EMenuOptionEnabledState, EMenuOptionEnabledState::Enabled,
	EMenuOptionEnabledState::Disabled);

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnLockStateChanged, UMenuOptionWidget*, const bool);

/** Base class for a Menu Option Widget, which is basically just a description, tooltip, and some value(s) that can
 *  be changed. */
UCLASS()
class USERINTERFACE_API UMenuOptionWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void SetStyling();

public:
	/** Sets the enabled state of the menu option. By default, this sets the enabled
	 *  states of the two main horizontal boxes of a MenuOptionWidget, but subclasses may
	 *  choose to modify this behavior.
	 *  @param EnabledState the state to set the widget to.
	 */
	virtual void SetMenuOptionEnabledState(EMenuOptionEnabledState EnabledState);

	/** Sets the enabled state of a specific widget type in a menu option widget.
	 * @param SubWidget the specific widget type to enable/disable
	 * @param State the state to set the sub-widget to.
	 * @return nullptr by default, or the widget containing the sub-widget.
	 */
	virtual UWidget* SetSubMenuOptionEnabledState(const TSubclassOf<UWidget>& SubWidget, EMenuOptionEnabledState State);

	/** @return the custom enabled state of the menu option. */
	EMenuOptionEnabledState GetMenuOptionEnabledState() const { return MenuOptionEnabledState; }

	/** Sets the left hand side indent, with each level increasing by 50. */
	void SetIndentLevel(const int32 Value);

	/** Toggles showing the TooltipImage. */
	void SetShowTooltipIcon(const bool bShow);

	/** Toggles showing the CheckBoxLock. */
	void SetShowCheckBoxLock(const bool bShow);

	/** Sets the Description Text. */
	void SetDescriptionText(const FText& InText);

	/** Sets the tooltip text associated with the tooltip icon. */
	void SetDescriptionTooltipText(const FText& InText);

	/** Returns the tooltip icon. */
	UTooltipIcon* GetDescriptionTooltipIcon() const;

	/** Returns the tooltip text associated with the tooltip icon. */
	FText GetDescriptionTooltipIconText() const { return DescriptionTooltipText; }

	/** Returns true if locked. */
	bool GetIsLocked() const;

	/** Sets the locked state. */
	void SetIsLocked(const bool bLocked) const;

	/** Returns value of bShowTooltipIcon. */
	bool ShouldShowDescriptionTooltipIcon() const { return bShowDescriptionTooltipIcon; }

	/** Broadcasts the new state of the lock and the index. */
	FOnLockStateChanged OnLockStateChanged;

	/** Returns Game mode category tags associated with this menu option. */
	void GetGameModeCategoryTags(FGameplayTagContainer& OutTags) const;

	/** Adds the widget to Box_TagWidgets. */
	void AddGameModeCategoryTagWidgets(TArray<UGameModeCategoryTagWidget*>& InGameModeCategoryTagWidgets);

	/** Updates a tooltip icon that is only shown if a validation check failed.
	 *  @param bValidated whether or not the validation check succeeded
	 *  @param Data data that is unique per property per validation check
	 *  @param CalculatedValues any values calculated during the validation check to populate formatted text with,
	 *  if applicable.
	 */
	void UpdateDynamicTooltipIcon(const bool bValidated, const FUniqueValidationCheckData& Data,
		const TArray<int32>& CalculatedValues);

	/** @return the number tooltip icons of the specified type. */
	int32 GetNumberOfDynamicTooltipIcons(ETooltipIconType Type);

protected:
	UFUNCTION()
	void OnCheckBox_LockStateChanged(const bool bChecked);

	/**
	 *  Creates a new tooltip icon unqiue to the validation check data and stores it.
	 *  @param Data data that is unique per property per validation check
	 *  @return added tooltip icon.
	 */
	UTooltipIcon* AddTooltipIcon(const FUniqueValidationCheckData& Data);

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	UHorizontalBox* Box_TagWidgets;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UBSHorizontalBox* BSBox;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UTooltipIcon* DescriptionTooltip;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UHorizontalBox* TooltipBox;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UHorizontalBox* Box_Left;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UHorizontalBox* Box_Right;

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

	/** Text that describes the values this widget controls. */
	UPROPERTY(EditInstanceOnly, Category="MenuOptionWidget")
	FText DescriptionText = FText();

	UPROPERTY(EditInstanceOnly, Category="MenuOptionWidget")
	int32 IndentLevel = 0;

	UPROPERTY(EditInstanceOnly, Category="MenuOptionWidget")
	bool bShowCheckBoxLock = false;

	UPROPERTY(EditInstanceOnly, Category="MenuOptionWidget|Tooltip")
	bool bShowDescriptionTooltipIcon = true;

	/** Text to show on the tooltip. */
	UPROPERTY(EditInstanceOnly, Category="MenuOptionWidget|Tooltip")
	FText DescriptionTooltipText = FText();

	/** The categories this menu option represents. */
	UPROPERTY(EditInstanceOnly, Category="MenuOptionWidget|GameModeCategoryTags")
	FGameplayTagContainer GameModeCategoryTags;

	/** The custom enabled state of the menu option. */
	EMenuOptionEnabledState MenuOptionEnabledState;

	UPROPERTY()
	TMap<uint32, TObjectPtr<UTooltipIcon>> DynamicTooltipIcons;
};
