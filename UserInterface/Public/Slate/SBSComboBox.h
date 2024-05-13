// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InputCoreTypes.h"
#include "Layout/Margin.h"
#include "Styling/SlateColor.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Input/Events.h"
#include "Input/Reply.h"
#include "Widgets/SWidget.h"
#include "Sound/SlateSound.h"
#include "Styling/SlateTypes.h"
#include "Styling/AppStyle.h"
#include "Framework/SlateDelegates.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Application/SlateUser.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Views/STableViewBase.h"
#include "Framework/Views/TableViewTypeTraits.h"
#include "SBSComboBox.h"
#include "SBSListView.h"
#include "SBSTableRow.h"

#ifdef PlaySound
#undef PlaySound
#endif

#if WITH_ACCESSIBILITY
#include "GenericPlatform/Accessibility/GenericAccessibleInterfaces.h"
#include "Widgets/Accessibility/SlateCoreAccessibleWidgets.h"
#include "Widgets/Accessibility/SlateAccessibleMessageHandler.h"
#endif

DECLARE_DELEGATE(FOnComboBoxOpening)
/** Delegate that executes on selection changed with all current selections */
DECLARE_DELEGATE_TwoParams(FOnMultiSelectionChanged, const TArray<TSharedPtr<FString>>&, const ESelectInfo::Type);

template <typename OptionType>
class SBSComboRow : public SBSTableRow<OptionType>
{
public:
	typedef SBSTableRow<OptionType> SBSTableRowOptionType;

	SLATE_BEGIN_ARGS(SBSComboRow) : _Style(&FAppStyle::Get().GetWidgetStyle<FTableRowStyle>("ComboBox.Row")),
	                                _Content(), _Padding(FMargin(0)), _MaxNumSelectedItems(-1), _CanSelectNone(false)
		{
		}

		SLATE_STYLE_ARGUMENT(FTableRowStyle, Style)
		SLATE_DEFAULT_SLOT(FArguments, Content)
		SLATE_ATTRIBUTE(FMargin, Padding)
		SLATE_ARGUMENT(int32, MaxNumSelectedItems)
		SLATE_ARGUMENT(bool, CanSelectNone)
	SLATE_END_ARGS()

	/** Constructs this widget. */
	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTable)
	{
		SBSTableRow<OptionType>::Construct(
			typename SBSTableRow<OptionType>::FArguments().Style(InArgs._Style).Padding(InArgs._Padding).Content()[
				InArgs._Content.Widget].MaxNumSelectedItems(InArgs._MaxNumSelectedItems).CanSelectNone(
				InArgs._CanSelectNone), InOwnerTable);
	}
};

/** A combo box that shows arbitrary content. */
template <typename OptionType>
class SBSComboBox : public SComboButton
{
public:
	typedef TListTypeTraits<OptionType> ListTypeTraits;
	typedef typename TListTypeTraits<OptionType>::NullableType NullableOptionType;

	/** Type of list used for showing menu options. */
	typedef SBSListView<OptionType> SComboListType;

	/** Delegate type used to generate widgets that represent Options */
	typedef typename TSlateDelegates<OptionType>::FOnGenerateWidget FOnGenerateWidget;

	/** Delegate type used for when a selection has been changed */
	typedef typename TSlateDelegates<NullableOptionType>::FOnSelectionChanged FOnSelectionChanged;

	SLATE_BEGIN_ARGS(SBSComboBox)
			: _Content(), _ComboBoxStyle(&FAppStyle::Get().GetWidgetStyle<FComboBoxStyle>("ComboBox")),
			  _ButtonStyle(nullptr), _ItemStyle(&FAppStyle::Get().GetWidgetStyle<FTableRowStyle>("ComboBox.Row")),
			  _ContentPadding(_ComboBoxStyle->ContentPadding), _ForegroundColor(FSlateColor::UseStyle()),
			  _OptionsSource(), _OnSelectionChanged(), _OnGenerateWidget(), _OnMultiSelectionChanged(),
			  _InitiallySelectedItems(TArray<NullableOptionType>()), _Method(), _MaxListHeight(450.0f),
			  _HasDownArrow(true), _EnableGamepadNavigationMode(false), _IsFocusable(true),
			  _CloseComboBoxOnSelectionChanged(false), _MaxNumSelectedItems(-1), _CanSelectNone(false)
		{
		}

		/** Slot for this button's content (optional) */
		SLATE_DEFAULT_SLOT(FArguments, Content)
		SLATE_STYLE_ARGUMENT(FComboBoxStyle, ComboBoxStyle)

		/** The visual style of the button part of the combo box (overrides ComboBoxStyle) */
		SLATE_STYLE_ARGUMENT(FButtonStyle, ButtonStyle)
		SLATE_STYLE_ARGUMENT(FTableRowStyle, ItemStyle)
		SLATE_ATTRIBUTE(FMargin, ContentPadding)
		SLATE_ATTRIBUTE(FSlateColor, ForegroundColor)
		SLATE_ARGUMENT(const TArray< OptionType >*, OptionsSource)
		SLATE_EVENT(FOnSelectionChanged, OnSelectionChanged)
		SLATE_EVENT(FOnGenerateWidget, OnGenerateWidget)
		SLATE_EVENT(FOnMultiSelectionChanged, OnMultiSelectionChanged)

		/** Called when combo box is opened, before list is actually created */
		SLATE_EVENT(FOnComboBoxOpening, OnComboBoxOpening)

		/** The custom scrollbar to use in the ListView */
		SLATE_ARGUMENT(TSharedPtr<SScrollBar>, CustomScrollbar)

		/** The option that should be selected when the combo box is first created */
		SLATE_ARGUMENT(TArray<NullableOptionType>, InitiallySelectedItems)
		SLATE_ARGUMENT(TOptional<EPopupMethod>, Method)

		/** The max height of the combo box menu */
		SLATE_ARGUMENT(float, MaxListHeight)

		/** The sound to play when the button is pressed (overrides ComboBoxStyle) */
		SLATE_ARGUMENT(TOptional<FSlateSound>, PressedSoundOverride)

		/** The sound to play when the selection changes (overrides ComboBoxStyle) */
		SLATE_ARGUMENT(TOptional<FSlateSound>, SelectionChangeSoundOverride)

		/**
		 * When false, the down arrow is not generated and it is up to the API consumer
		 * to make their own visual hint that this is a drop down.
		 */
		SLATE_ARGUMENT(bool, HasDownArrow)

		/**
		 *  When false, directional keys will change the selection. When true, ComboBox
		 *	must be activated and will only capture arrow input while activated.
		*/
		SLATE_ARGUMENT(bool, EnableGamepadNavigationMode)

		/** When true, allows the combo box to receive keyboard focus */
		SLATE_ARGUMENT(bool, IsFocusable)

		/** True if this combo's menu should be collapsed when our parent receives focus, false (default) otherwise */
		SLATE_ARGUMENT(bool, CollapseMenuOnParentFocus)
		SLATE_ARGUMENT(bool, CloseComboBoxOnSelectionChanged)

		/** When true, allows the combo box to receive keyboard focus */
		SLATE_ARGUMENT(TAttribute<ESelectionMode::Type>, SelectionMode)

		/** Max number of selections, or -1 if unlimited */
		SLATE_ARGUMENT(int32, MaxNumSelectedItems)

		/** Should the user be able to select no options */
		SLATE_ARGUMENT(bool, CanSelectNone)

	SLATE_END_ARGS()

	/**
	 * Construct the widget from a declaration
	 *
	 * @param InArgs   Declaration from which to construct the combo box
	 */
	void Construct(const FArguments& InArgs)
	{
		check(InArgs._ComboBoxStyle);

		ItemStyle = InArgs._ItemStyle;
		MenuRowPadding = InArgs._ComboBoxStyle->MenuRowPadding;

		// Work out which values we should use based on whether we were given an override, or should use the style's version
		const FComboButtonStyle& OurComboButtonStyle = InArgs._ComboBoxStyle->ComboButtonStyle;
		const FButtonStyle* const OurButtonStyle = InArgs._ButtonStyle
			? InArgs._ButtonStyle
			: &OurComboButtonStyle.ButtonStyle;
		PressedSound = InArgs._PressedSoundOverride.Get(InArgs._ComboBoxStyle->PressedSlateSound);
		SelectionChangeSound = InArgs._SelectionChangeSoundOverride.Get(
			InArgs._ComboBoxStyle->SelectionChangeSlateSound);

		this->OnComboBoxOpening = InArgs._OnComboBoxOpening;
		this->OnSelectionChanged = InArgs._OnSelectionChanged;
		this->OnGenerateWidget = InArgs._OnGenerateWidget;
		this->OnMultiSelectionChanged = InArgs._OnMultiSelectionChanged;
		this->EnableGamepadNavigationMode = InArgs._EnableGamepadNavigationMode;
		this->bControllerInputCaptured = false;
		this->bCloseComboBoxOnSelectionChanged = InArgs._CloseComboBoxOnSelectionChanged;
		this->MaxNumSelectedItems = InArgs._MaxNumSelectedItems;
		this->bCanSelectNone = InArgs._CanSelectNone;

		OptionsSource = InArgs._OptionsSource;
		CustomScrollbar = InArgs._CustomScrollbar;


		TSharedRef<SWidget> ComboBoxMenuContent = SNew(SBox).MaxDesiredHeight(InArgs._MaxListHeight)
			[
				SAssignNew(this->ComboListView, SComboListType)
				.ListItemsSource(InArgs._OptionsSource)
				.OnGenerateRow(this, &SBSComboBox<OptionType>::GenerateMenuItemRow)
				.OnSelectionChanged(this, &SBSComboBox<OptionType>::OnSelectionChanged_Internal)
				.OnKeyDownHandler(this, &SBSComboBox<OptionType>::OnKeyDownHandler)
				.SelectionMode(InArgs._SelectionMode)
				.ExternalScrollbar(InArgs._CustomScrollbar)
			];

		// Set up content
		TSharedPtr<SWidget> ButtonContent = InArgs._Content.Widget;
		if (InArgs._Content.Widget == SNullWidget::NullWidget)
		{
			SAssignNew(ButtonContent, STextBlock)
			.Text(NSLOCTEXT("SBSComboBox", "ContentWarning", "No Content Provided"))
			.ColorAndOpacity(FLinearColor::Red);
		}


		SComboButton::Construct(
			SComboButton::FArguments().ComboButtonStyle(&OurComboButtonStyle).ButtonStyle(OurButtonStyle).
			                           Method(InArgs._Method).ButtonContent()[ButtonContent.ToSharedRef()].MenuContent()
			[ComboBoxMenuContent].HasDownArrow(InArgs._HasDownArrow).ContentPadding(InArgs._ContentPadding).
			                      ForegroundColor(InArgs._ForegroundColor).OnMenuOpenChanged(this,
				                      &SBSComboBox<OptionType>::OnMenuOpenChanged).IsFocusable(InArgs._IsFocusable).
			                      CollapseMenuOnParentFocus(InArgs._CollapseMenuOnParentFocus));
		SetMenuContentWidgetToFocus(ComboListView);

		// Need to establish the selected item at point of construction so its available for querying
		// NB: If you need a selection to fire use SetItemSelection rather than setting an Initially SelectedItem
		SelectedItems = InArgs._InitiallySelectedItems;
		for (NullableOptionType InitialOptionType : SelectedItems)
		{
			if (TListTypeTraits<OptionType>::IsPtrValid(InitialOptionType))
			{
				OptionType ValidatedItem = TListTypeTraits<OptionType>::NullableItemTypeConvertToItemType(
					InitialOptionType);
				ComboListView->SetItemSelection(ValidatedItem, true);
				ComboListView->RequestScrollIntoView(ValidatedItem, 0);
			}
		}
	}

	SBSComboBox(): ItemStyle(nullptr), EnableGamepadNavigationMode(false), bControllerInputCaptured(false),
	               bCloseComboBoxOnSelectionChanged(false), OptionsSource(nullptr), MaxNumSelectedItems(-1),
	               bCanSelectNone(false)

	{
#if WITH_ACCESSIBILITY
		AccessibleBehavior = EAccessibleBehavior::Auto;
		bCanChildrenBeAccessible = true;
#endif
	}

#if WITH_ACCESSIBILITY

protected:
	friend class FSlateAccessibleComboBox;
	/**
	* An accessible implementation of SComboBox to expose to platform accessibility APIs.
	* We inherit from IAccessibleProperty as Windows will use the interface to read out 
	* the value associated with the combo box. Convenient place to return the value of the currently selected option. 
	* For subclasses of SComboBox, inherit and override the necessary functions
	*/
	class FSlateAccessibleComboBox : public FSlateAccessibleWidget, public IAccessibleProperty
	{
	public:
		FSlateAccessibleComboBox(TWeakPtr<SWidget> InWidget) : FSlateAccessibleWidget(InWidget,
			EAccessibleWidgetType::ComboBox)
		{
		}

		// IAccessibleWidget
		virtual IAccessibleProperty* AsProperty() override
		{
			return this;
		}

		// ~

		// IAccessibleProperty
		virtual FString GetValue() const override
		{
			if (Widget.IsValid())
			{
				TSharedPtr<SBSComboBox<OptionType>> ComboBox = StaticCastSharedPtr<SBSComboBox<OptionType>>(
					Widget.Pin());
				if (TListTypeTraits<OptionType>::IsPtrValid(ComboBox->SelectedItem))
				{
					OptionType SelectedOption = TListTypeTraits<OptionType>::NullableItemTypeConvertToItemType(
						ComboBox->SelectedItem);
					const TSharedPtr<ITableRow> SelectedTableRow = ComboBox->ComboListView->WidgetFromItem(
						SelectedOption);
					if (SelectedTableRow.IsValid())
					{
						const TSharedRef<SWidget> TableRowWidget = SelectedTableRow->AsWidget();
						return TableRowWidget->GetAccessibleText().ToString();
					}
				}
			}
			return FText::GetEmpty().ToString();
		}

		virtual FVariant GetValueAsVariant() const override
		{
			return FVariant(GetValue());
		}

		// ~
	};

public:
	virtual TSharedRef<FSlateAccessibleWidget> CreateAccessibleWidget() override
	{
		return MakeShareable<FSlateAccessibleWidget>(
			new SBSComboBox<OptionType>::FSlateAccessibleComboBox(SharedThis(this)));
	}

	virtual TOptional<FText> GetDefaultAccessibleText(EAccessibleType AccessibleType) const override
	{
		// current behaviour will red out the  templated type of the combo box which is verbose and unhelpful 
		// This coupled with UIA type will announce Combo Box twice, but it's the best we can do for now if there's no label
		//@TODOAccessibility: Give a better name
		static FString Name(TEXT("BS Combo Box"));
		return FText::FromString(Name);
	}
#endif

	void ClearSelection()
	{
		ComboListView->ClearSelection();
	}

	// Only really called when initializing options in BSComboBoxString
	void SetItemSelection(NullableOptionType InSelectedItem, bool bSelected,
		ESelectInfo::Type SelectInfo = ESelectInfo::Direct)
	{
		if (TListTypeTraits<OptionType>::IsPtrValid(InSelectedItem))
		{
			OptionType InSelected = TListTypeTraits<OptionType>::NullableItemTypeConvertToItemType(InSelectedItem);
			ComboListView->SetItemSelection(InSelected, bSelected, SelectInfo);
			bSelected ? SelectedItems.Add(InSelected) : SelectedItems.Remove(InSelected);
		}
		else
		{
			ClearSelection();
		}
	}

	void SetItemSelection(TConstArrayView<NullableOptionType> InItems, bool bSelected,
		ESelectInfo::Type SelectInfo = ESelectInfo::Direct)
	{
		const TArray<OptionType> ComboListItems = ComboListView->GetSelectedItems();
		if (InItems.Num() > 0)
		{
			for (const OptionType& Item : InItems)
			{
				if (TListTypeTraits<OptionType>::IsPtrValid(Item))
				{
					OptionType InSelected = TListTypeTraits<OptionType>::NullableItemTypeConvertToItemType(Item);
					ComboListView->SetItemSelection(InSelected, bSelected, SelectInfo);
					bSelected ? SelectedItems.Add(InSelected) : SelectedItems.Remove(InSelected);
				}
			}
		}
		else
		{
			ComboListView->ClearSelection();
		}
	}

	TConstArrayView<NullableOptionType> GetSelectedItems()
	{
		return SelectedItems;
	}

	/** 
	 * Requests a list refresh after updating options 
	 * Call SetSelectedItem to update the selected item if required
	 * @see SetSelectedItem
	 */
	void RefreshOptions()
	{
		ComboListView->RequestListRefresh();
	}

protected:
	/** Handle key presses that SListView ignores */
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override
	{
		if (IsInteractable())
		{
			const EUINavigationAction NavAction = FSlateApplication::Get().GetNavigationActionFromKey(InKeyEvent);
			const EUINavigation NavDirection = FSlateApplication::Get().GetNavigationDirectionFromKey(InKeyEvent);
			if (EnableGamepadNavigationMode)
			{
				// The controller's bottom face button must be pressed once to begin manipulating the combobox value.
				// Navigation away from the widget is prevented until the button has been pressed again or focus is lost.
				if (NavAction == EUINavigationAction::Accept)
				{
					if (bControllerInputCaptured == false)
					{
						// Begin capturing controller input and open the ListView
						bControllerInputCaptured = true;
						PlayPressedSound();
						OnComboBoxOpening.ExecuteIfBound();
						return SComboButton::OnButtonClicked();
					}
					else
					{
						// Set selection to the selected item on the list and close
						bControllerInputCaptured = false;

						// Re-select first selected item, just in case it was selected by navigation previously
						const TArray<OptionType> ComboListItems = ComboListView->GetSelectedItems();
						if (ComboListItems.Num() > 0)
						{
							for (const OptionType& ComboListItem : ComboListItems)
							{
								if (TListTypeTraits<OptionType>::IsPtrValid(ComboListItem))
								{
									OptionType InSelected = TListTypeTraits<
										OptionType>::NullableItemTypeConvertToItemType(ComboListItem);
									SetItemSelection(InSelected, ComboListView->IsItemSelected(InSelected));
								}
							}
						}

						// Set focus back to ComboBox
						FReply Reply = FReply::Handled();
						Reply.SetUserFocus(this->AsShared(), EFocusCause::SetDirectly);
						return Reply;
					}
				}
				if (NavAction == EUINavigationAction::Back || InKeyEvent.GetKey() == EKeys::BackSpace)
				{
					const bool bWasInputCaptured = bControllerInputCaptured;

					OnMenuOpenChanged(false);
					if (bWasInputCaptured)
					{
						return FReply::Handled();
					}
				}
				else
				{
					if (bControllerInputCaptured)
					{
						return FReply::Handled();
					}
				}
			}
			else
			{
				if (NavDirection == EUINavigation::Up)
				{
					const TArray<OptionType> ComboListItems = ComboListView->GetSelectedItems();
					if (ComboListItems.Num() > 0)
					{
						for (const OptionType& ComboListItem : ComboListItems)
						{
							if (TListTypeTraits<OptionType>::IsPtrValid(ComboListItem))
							{
								OptionType InSelected = TListTypeTraits<OptionType>::NullableItemTypeConvertToItemType(
									ComboListItem);
								const int32 SelectionIndex = OptionsSource->Find(InSelected);
								SetItemSelection((*OptionsSource)[SelectionIndex + -1],
									ComboListView->IsItemSelected(InSelected));
							}
						}
						return FReply::Handled();
					}
				}
				if (NavDirection == EUINavigation::Down)
				{
					const TArray<OptionType> ComboListItems = ComboListView->GetSelectedItems();
					if (ComboListItems.Num() > 0)
					{
						for (const OptionType& ComboListItem : ComboListItems)
						{
							if (TListTypeTraits<OptionType>::IsPtrValid(ComboListItem))
							{
								OptionType InSelected = TListTypeTraits<OptionType>::NullableItemTypeConvertToItemType(
									ComboListItem);
								const int32 SelectionIndex = OptionsSource->Find(InSelected);
								SetItemSelection((*OptionsSource)[SelectionIndex + 1],
									ComboListView->IsItemSelected(InSelected));
							}
						}
						return FReply::Handled();
					}
				}
				return SComboButton::OnKeyDown(MyGeometry, InKeyEvent);
			}
		}
		return SWidget::OnKeyDown(MyGeometry, InKeyEvent);
	}

	virtual bool SupportsKeyboardFocus() const override
	{
		return bIsFocusable;
	}

	virtual bool IsInteractable() const override
	{
		return IsEnabled();
	}

private:
	/** Generate a row for the InItem in the combo box's list (passed in as OwnerTable). Do this by calling the user-specified OnGenerateWidget */
	TSharedRef<ITableRow> GenerateMenuItemRow(OptionType InItem, const TSharedRef<STableViewBase>& OwnerTable)
	{
		if (OnGenerateWidget.IsBound())
		{
			TSharedRef<SBSComboRow<OptionType>> BSComboRow = SNew(SBSComboRow<OptionType>, OwnerTable)
				.Style(ItemStyle)
				.Padding(MenuRowPadding)
				.MaxNumSelectedItems(MaxNumSelectedItems)
				.CanSelectNone(bCanSelectNone)
				[
					OnGenerateWidget.Execute(InItem)
				];
			//BSComboRow->SetMaxNumSelectedItems(MaxNumSelectedItems);
			return BSComboRow;
		}
		return SNew(SBSComboRow<OptionType>, OwnerTable)
			[
				SNew(STextBlock).Text(NSLOCTEXT("SlateCore", "ComboBoxMissingOnGenerateWidgetMethod",
					"Please provide a .OnGenerateWidget() handler."))
			];
	}

	//** Called if the menu is closed
	void OnMenuOpenChanged(bool bOpen)
	{
		if (bOpen == false)
		{
			bControllerInputCaptured = false;
			const TArray<OptionType> ComboListItems = ComboListView->GetSelectedItems();
			for (const OptionType& ComboListItem : ComboListItems)
			{
				if (TListTypeTraits<OptionType>::IsPtrValid(ComboListItem))
				{
					// Ensure the ListView selection is set back to the last committed selection
					OptionType ActuallySelected = TListTypeTraits<OptionType>::NullableItemTypeConvertToItemType(
						ComboListItem);
					ComboListView->Private_SetItemSelection(ActuallySelected,
						ComboListView->IsItemSelected(ActuallySelected));
					ComboListView->RequestScrollIntoView(ActuallySelected, 0);
				}
			}

			// Set focus back to ComboBox for users focusing the ListView that just closed
			FSlateApplication::Get().ForEachUser([this](FSlateUser& User)
			{
				const TSharedRef<SWidget> ThisRef = this->AsShared();
				if (User.IsWidgetInFocusPath(this->ComboListView))
				{
					User.SetFocus(ThisRef);
				}
			});
		}
	}

	/** Invoked when the selection in the list changes */
	void OnSelectionChanged_Internal(NullableOptionType ProposedSelection, ESelectInfo::Type SelectInfo)
	{
		if (SelectInfo != ESelectInfo::OnNavigation)
		{
			SelectedItems = ComboListView->GetSelectedItems();
			OnMultiSelectionChanged.ExecuteIfBound(SelectedItems, SelectInfo);
			PlaySelectionChangeSound();

			if (bCloseComboBoxOnSelectionChanged)
			{
				this->SetIsOpen(false);
			}
			//UE_LOG(LogTemp, Display, TEXT("SBSComboBox SelectedItems %d"), SelectedItems.Num());
			//UE_LOG(LogTemp, Display, TEXT("SBSListView SelectedItems %d"), ComboListView->GetNumItemsSelected());
		}
	}

	/** Handle clicking on the content menu */
	virtual FReply OnButtonClicked() override
	{
		// if user clicked to close the combo menu
		if (this->IsOpen())
		{
			const TArray<OptionType> ComboListItems = ComboListView->GetSelectedItems();
			for (const OptionType& ComboListItem : ComboListItems)
			{
				if (TListTypeTraits<OptionType>::IsPtrValid(ComboListItem))
				{
					OnSelectionChanged_Internal(ComboListItem, ESelectInfo::Direct);
				}
			}
		}
		else
		{
			PlayPressedSound();
			OnComboBoxOpening.ExecuteIfBound();
		}

		return SComboButton::OnButtonClicked();
	}

	FReply OnKeyDownHandler(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
	{
		if (InKeyEvent.GetKey() == EKeys::Enter)
		{
			const TArray<OptionType> ComboListItems = ComboListView->GetSelectedItems();
			if (ComboListItems.Num() > 0)
			{
				for (const OptionType& ComboListItem : ComboListItems)
				{
					if (TListTypeTraits<OptionType>::IsPtrValid(ComboListItem))
					{
						OnSelectionChanged_Internal(ComboListItem, ESelectInfo::Direct);
					}
				}
				return FReply::Handled();
			}
		}
		return FReply::Unhandled();
	}


	/** Play the pressed sound */
	void PlayPressedSound() const
	{
		FSlateApplication::Get().PlaySound(PressedSound);
	}

	/** Play the selection changed sound */
	void PlaySelectionChangeSound() const
	{
		FSlateApplication::Get().PlaySound(SelectionChangeSound);
	}

	/** The Sound to play when the button is pressed */
	FSlateSound PressedSound;

	/** The Sound to play when the selection is changed */
	FSlateSound SelectionChangeSound;

	/** The item style to use. */
	const FTableRowStyle* ItemStyle;

	/** The padding around each menu row */
	FMargin MenuRowPadding;

private:
	/** Delegate that is invoked when the selected item in the combo box changes */
	FOnSelectionChanged OnSelectionChanged;
	/** Delegate that is invoked when the selected items in the combo box changes */
	FOnMultiSelectionChanged OnMultiSelectionChanged;
	/** The item currently selected in the combo box */
	NullableOptionType SelectedItem;
	/** The item currently selected in the combo box */
	TArray<NullableOptionType> SelectedItems;
	/** The ListView that we pop up; visualized the available options. */
	TSharedPtr<SComboListType> ComboListView;
	/** The Scrollbar used in the ListView. */
	TSharedPtr<SScrollBar> CustomScrollbar;
	/** Delegate to invoke before the combo box is opening. */
	FOnComboBoxOpening OnComboBoxOpening;
	/** Delegate to invoke when we need to visualize an option as a widget. */
	FOnGenerateWidget OnGenerateWidget;
	// Use activate button to toggle ListView when enabled
	bool EnableGamepadNavigationMode;
	// Holds a flag indicating whether a controller/keyboard is manipulating the combobox value. 
	// When true, navigation away from the widget is prevented until a new value has been accepted or canceled. 
	bool bControllerInputCaptured;
	bool bCloseComboBoxOnSelectionChanged;

	const TArray<OptionType>* OptionsSource;

	int32 MaxNumSelectedItems;
	bool bCanSelectNone;
};
