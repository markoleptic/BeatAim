// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/Widget.h"
#include "Fonts/SlateFontInfo.h"
#include "Layout/Margin.h"
#include "Slate/SBSComboBox.h"
#include "Styling/SlateColor.h"
#include "Styling/SlateTypes.h"
#include "UObject/ObjectMacros.h"
#include "Utilities/BSWidgetInterface.h"
#include "Widgets/SWidget.h"
#include "BSComboBoxString.generated.h"

UENUM(BlueprintType)
enum class ESelectionModeType : uint8
{
	/** Nothing can be selected and there is no hover cue for selection. You can still handle mouse button events
	 *  though. */
	None,
	/** A single item can be selected at once, or no item may be selected. */
	Single,
	/** A single item can be selected at once, or no item may be selected. You can click the item to toggle selection
	 *  on and off. */
	SingleToggle,
	/** Multiple items can be selected at the same time. */
	Multi
};

class UBSComboBoxEntry;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSelectionChangedEvent, FString, SelectedItem, ESelectInfo::Type,
	SelectionType);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnOpeningEvent);

DECLARE_DYNAMIC_DELEGATE_RetVal_TwoParams(UWidget*, FGenerateWidgetForSingleItem, const UBSComboBoxString*,
	BSComboBoxString, FString, Item);

DECLARE_DYNAMIC_DELEGATE_RetVal_TwoParams(UWidget*, FGenerateWidgetForMultiSelection, const UBSComboBoxString*,
	BSComboBoxString, const TArray<FString>&, Items);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMultiSelectionChangedEvent, const TArray<FString>&, ActiveSelections,
	const ESelectInfo::Type, SelectionType);

UCLASS(meta=( DisplayName="BSComboBox (String) C++"))
class USERINTERFACE_API UBSComboBoxString : public UWidget, public IBSWidgetInterface
{
	GENERATED_BODY()

	/** The default list of items to be displayed on the combobox. */
	UPROPERTY(EditAnywhere, Category=Content)
	TArray<FString> DefaultOptions;

	/** The item in the combobox to select by default. */
	UPROPERTY(EditAnywhere, Category=Content)
	FString SelectedOption;

	UPROPERTY(EditDefaultsOnly, Category = "BSComboBoxString|Classes")
	TSubclassOf<UBSComboBoxEntry> ComboboxEntryWidget;

public:
	TSubclassOf<UBSComboBoxEntry> GetComboboxEntryWidget() const { return ComboboxEntryWidget; }

	/** The style. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Style, meta=( DisplayName="Style" ))
	FComboBoxStyle WidgetStyle;

	/** The item row style. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Style)
	FTableRowStyle ItemStyle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Content)
	FMargin ContentPadding;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Content)
	ESelectionModeType SelectionMode = ESelectionModeType::Multi;

	/** Should the user be able to select no options. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Content)
	bool bCanSelectNone = false;

	/** Should the combo box automatically close when a selection is made. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Content)
	bool bCloseComboBoxOnSelectionChanged = false;

	/** The max number of options that a user can select. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Content)
	int32 MaxNumberOfSelections = -1;

	/** When false, directional keys will change the selection. When true, ComboBox 
	 *  must be activated and will only capture arrow input while activated. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Content, AdvancedDisplay)
	bool EnableGamepadNavigationMode;

	/** The max height of the combobox list that opens */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Content, AdvancedDisplay)
	float MaxListHeight;

	/** When false, the down arrow is not generated, and it is up to the API consumer
	 *  to make their own visual hint that this is a drop-down. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Content, AdvancedDisplay)
	bool HasDownArrow;

	/** The default font to use in the combobox, only applies if you're not implementing OnGenerateWidgetEvent
	 *  to factory each new entry. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Style)
	FSlateFontInfo Font;

	/** The foreground color to pass through the hierarchy. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Style, meta=(DesignerRebuild))
	FSlateColor ForegroundColor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Interaction)
	bool bIsFocusable;

	/** Called when the widget is needed for the item. */
	UPROPERTY(EditAnywhere, Category=Events, meta=( IsBindableEvent="True" ))
	FGenerateWidgetForString OnGenerateWidgetEvent;

	/** Called when ayn change in selection occurs in the combobox. */
	UPROPERTY(BlueprintAssignable, Category=Events)
	FOnMultiSelectionChangedEvent OnSelectionChanged;

	/** Called when the combobox is opening */
	UPROPERTY(BlueprintAssignable, Category=Events)
	FOnOpeningEvent OnOpening;

	UFUNCTION(BlueprintCallable, Category = "ComboBox")
	void SetSelectionMode(const ESelectionModeType InSelectionMode) { SelectionMode = InSelectionMode; }

	UFUNCTION(BlueprintCallable, Category = "ComboBox")
	void SetCloseComboBoxOnSelectionChanged(const bool bClose) { bCloseComboBoxOnSelectionChanged = bClose; }

	UFUNCTION(BlueprintCallable, Category = "ComboBox")
	void SetCanSelectNone(const bool bCan);

	/** Adds a new option to the combo box. */
	UFUNCTION(BlueprintCallable, Category = "ComboBox")
	void AddOption(const FString& Option);

	/** Adds a new option to the combo box at the specified index. */
	UFUNCTION(BlueprintCallable, Category = "ComboBox")
	void InsertOptionAt(const FString& Option, const int32 Index);

	/** Removes existing option from the combo box. */
	UFUNCTION(BlueprintCallable, Category = "ComboBox")
	bool RemoveOption(const FString& Option);

	/** Removes existing option from the combo box. */
	UFUNCTION(BlueprintCallable, Category = "ComboBox")
	bool HasOption(const FString& Option);

	/** Returns the index corresponding to the Option, or -1 if not found. */
	UFUNCTION(BlueprintPure, Category = "ComboBox")
	int32 GetIndexOfOption(const FString& Option) const;

	/** Returns string option corresponding to the Index, or empty string if not found. */
	UFUNCTION(BlueprintPure, Category = "ComboBox")
	FString GetOptionAtIndex(int32 Index) const;

	/** Returns the selected index. Use GetSelectedIndices if it is possible for more than one to be selected. */
	UFUNCTION(BlueprintPure, Category = "ComboBox")
	int32 GetSelectedIndex() const;

	/** Returns the selected option. Use GetSelectedOptions if it is possible for more than one to be selected. */
	UFUNCTION(BlueprintPure, Category = "ComboBox")
	FString GetSelectedOption() const;

	/** Returns an array of selected indices. */
	UFUNCTION(BlueprintPure, Category = "ComboBox")
	TArray<int32> GetSelectedIndices() const;

	/** Returns an array of selected string options. */
	UFUNCTION(BlueprintPure, Category = "ComboBox")
	TArray<FString> GetSelectedOptions() const;

	/** Returns the number of options. */
	UFUNCTION(BlueprintPure, Category = "ComboBox")
	int32 GetOptionCount() const;

	/** Returns the number of currently selected options. */
	UFUNCTION(BlueprintPure, Category = "ComboBox")
	int32 GetSelectedOptionCount() const;

	/** Selects the specified index if it exists. */
	UFUNCTION(BlueprintCallable, Category = "ComboBox")
	void SetSelectedIndex(const int32 InIndex, const bool bClearCurrentSelection = true);

	/** Selects the specified option if it exists. DOES NOT clear selected options if multi-selection. */
	UFUNCTION(BlueprintCallable, Category = "ComboBox")
	void SetSelectedOption(const FString InOption, const bool bClearCurrentSelection = true);

	/** Selects the specified indices if they exists. Clears all selected options before setting new options. */
	UFUNCTION(BlueprintCallable, Category = "ComboBox")
	void SetSelectedIndices(const TArray<int32> InIndices);

	/** Selects the specified option(s) if they exist. Clears all selected options before setting new options. */
	UFUNCTION(BlueprintCallable, Category = "ComboBox")
	void SetSelectedOptions(TArray<FString> InOptions);

	/** Returns whether the combo box is open. */
	UFUNCTION(BlueprintPure, Category = "ComboBox", Meta = (ReturnDisplayName = "bOpen"))
	bool IsOpen() const;

	/** Refreshes the list of options. If you added new ones, and want to update the list even if it's currently being
	 *  displayed use this. */
	UFUNCTION(BlueprintCallable, Category = "ComboBox")
	void RefreshOptions();

	/** Clears all options. */
	UFUNCTION(BlueprintCallable, Category = "ComboBox")
	void ClearOptions();

	/** Clears all selected options. */
	UFUNCTION(BlueprintCallable, Category = "ComboBox")
	void ClearSelection();

	/** Executed to allow other widgets to create a combo box row. */
	UPROPERTY(EditAnywhere, Category=Events, meta=( IsBindableEvent="True" ))
	FGenerateWidgetForSingleItem OnGenerateWidgetEventDelegate;

	/** Executed to allow other widgets to create selected (top) combo box row. */
	UPROPERTY(EditAnywhere, Category=Events, meta=( IsBindableEvent="True" ))
	FGenerateWidgetForSingleItem OnSelectionChanged_GenerateWidgetForSingleSelection;

	/** Executed to allow other widgets to create selected (top) combo box row. */
	UPROPERTY(EditAnywhere, Category=Events, meta=( IsBindableEvent="True" ))
	FGenerateWidgetForMultiSelection OnSelectionChanged_GenerateWidgetForMultiSelection;

	//~ Begin UVisual Interface
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	//~ End UVisual Interface

	//~ Begin UObject Interface
	virtual void PostInitProperties() override;
	virtual void Serialize(FArchive& Ar) override;
	virtual void PostLoad() override;
	//~ End UObject Interface

	/** Sets the text for the Entry and tooltip text, and binds to the OnHovered event in the TooltipImage. This can be
	 *  called by classes that bind to OnGenerateWidgetEvent to customize the entry text and tooltip text, and if needed
	 *  further modify the Entry. */
	static void InitializeComboBoxEntry(const UBSComboBoxEntry* Entry, const FText& EntryText,
		const bool bShowTooltipIcon, const FText& TooltipText = FText());

#if WITH_EDITOR
	virtual const FText GetPaletteCategory() override;
#endif

protected:
	UBSComboBoxString();

	/** Refresh ComboBoxContent with the correct widget/data when the selected option changes. */
	void UpdateOrGenerateWidget(TSharedPtr<FString> Item);

	/** Called by slate when it needs to generate a new item for the combobox. */
	virtual TSharedRef<SWidget> HandleGenerateWidget(TSharedPtr<FString> Item) const;

	/** Called by slate when the underlying combobox selection changes. Handles both single select and multi-select. */
	virtual void HandleSelectionChanged(const TArray<TSharedPtr<FString>>& Items,
		const ESelectInfo::Type SelectionType);

	/** Generates a widget for the combobox content that is selected. */
	virtual TSharedRef<SWidget> HandleSelectionChangedGenerateWidget(
		TConstArrayView<SBSComboBox<TSharedPtr<FString>>::NullableOptionType> Items) const;

	/** Called by slate when the underlying combobox is opening. */
	virtual void HandleOpening();

	//~ Begin UWidget Interface
	virtual TSharedRef<SWidget> RebuildWidget() override;
	//~ End UWidget Interface

	static TAttribute<ESelectionMode::Type> GetSelectionModeType(const ESelectionModeType& SelectionModeType)
	{
		switch (SelectionModeType)
		{
		case ESelectionModeType::None:
			return ESelectionMode::Type::None;
		case ESelectionModeType::Single:
			return ESelectionMode::Type::Single;
		case ESelectionModeType::SingleToggle:
			return ESelectionMode::Type::SingleToggle;
		case ESelectionModeType::Multi:
			return ESelectionMode::Type::Multi;
		}
		return ESelectionMode::Type::None;
	}

	/** The true objects bound to the Slate combobox. */
	TArray<TSharedPtr<FString>> Options;

	/** A shared pointer to the underlying slate combobox. */
	TSharedPtr<SBSComboBox<TSharedPtr<FString>>> SlateComboBox;

	/** A shared pointer to a container that holds the combobox content that is selected. */
	TSharedPtr<SBox> ComboBoxContent;

	/** If OnGenerateWidgetEvent is not bound, this will store the default STextBlock generated. */
	TWeakPtr<STextBlock> DefaultComboBoxContent;

	/** An array of shared pointers to the current selected strings. */
	TArray<TSharedPtr<FString>> CurrentlySelectedOptionPointers;
};
