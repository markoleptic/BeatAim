// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InputCoreTypes.h"
#include "Framework/Views/ITypedTableView.h"
#include "Framework/Views/TableViewTypeTraits.h"
#include "Input/DragAndDrop.h"
#include "Input/Events.h"
#include "Input/Reply.h"
#include "Layout/Geometry.h"
#include "Layout/Margin.h"
#include "Misc/Attribute.h"
#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateColor.h"
#include "Styling/SlateTypes.h"
#include "Types/SlateStructs.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/SWidget.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Views/ITableRow.h"
#include "Widgets/Views/SExpanderArrow.h"
#include "Widgets/Views/SHeaderRow.h"
#include "Widgets/Views/STableRow.h"
#include "Widgets/Views/STableViewBase.h"
#if WITH_ACCESSIBILITY
#include "GenericPlatform/Accessibility/GenericAccessibleInterfaces.h"
#include "Widgets/Accessibility/SlateAccessibleMessageHandler.h"
#include "Widgets/Accessibility/SlateAccessibleWidgetCache.h"
#include "Widgets/Accessibility/SlateCoreAccessibleWidgets.h"
#endif

template <typename ItemType>
class SListView;

DECLARE_DELEGATE_OneParam(FOnTableRowDragEnter, FDragDropEvent const&);
DECLARE_DELEGATE_OneParam(FOnTableRowDragLeave, FDragDropEvent const&);
DECLARE_DELEGATE_RetVal_OneParam(FReply, FOnTableRowDrop, FDragDropEvent const&);


/**
 * The ListView is populated by Selectable widgets.\n
 * A Selectable widget is a way of the ListView containing it (OwnerTable) and holds arbitrary Content (Content).\n
 * A Selectable works with its corresponding ListView to provide selection functionality.
 */
template <typename ItemType>
class SBSTableRow : public ITableRow, public SBorder
{
	static_assert(TIsValidListItem<ItemType>::Value,
		"Item type T must be UObjectBase*, TObjectPtr<>, TWeakObjectPtr<>, TSharedRef<>, or TSharedPtr<>.");

public:
	/** Delegate signature for querying whether this FDragDropEvent will be handled by the drop target of type
	 *  ItemType. */
	DECLARE_DELEGATE_RetVal_ThreeParams(TOptional<EItemDropZone>, FOnCanAcceptDrop, const FDragDropEvent&,
		EItemDropZone, ItemType);
	/** Delegate signature for handling the drop of FDragDropEvent onto target of type ItemType. */
	DECLARE_DELEGATE_RetVal_ThreeParams(FReply, FOnAcceptDrop, const FDragDropEvent&, EItemDropZone, ItemType);
	/** Delegate signature for painting drop indicators. */
	DECLARE_DELEGATE_RetVal_EightParams(int32, FOnPaintDropIndicator, EItemDropZone, const FPaintArgs&,
		const FGeometry&, const FSlateRect&, FSlateWindowElementList&, int32, const FWidgetStyle&, bool);

public:
	SLATE_BEGIN_ARGS(SBSTableRow< ItemType >) :
			_Style(&FCoreStyle::Get().GetWidgetStyle<FTableRowStyle>("TableView.Row")),
			_ExpanderStyleSet(&FCoreStyle::Get()), _Padding(FMargin(0)), _ShowSelection(true), _ShowWires(false),
			_bAllowPreselectedItemActivation(false), _SignalSelectionMode(ETableRowSignalSelectionMode::Deferred),
			_Content()
		{
		}

		SLATE_STYLE_ARGUMENT(FTableRowStyle, Style)
		SLATE_ARGUMENT(const ISlateStyle*, ExpanderStyleSet)

		// High Level DragAndDrop

		/** Handle this event to determine whether a drag and drop operation can be executed on top of the target row
		 *  widget. Most commonly, this is used for previewing re-ordering and re-organization operations in lists or
		 *  trees, e.g. A user is dragging one item into a different spot in the list or tree. This delegate will be
		 *  called to figure out if we should give visual feedback on whether an item will  successfully drop into the
		 *  list. */
		SLATE_EVENT(FOnCanAcceptDrop, OnCanAcceptDrop)

		/** Perform a drop operation onto the target row widget.
		 *  Most commonly used for executing a re-ordering and re-organization operations in lists or trees.
		 *  e.g. A user was dragging one item into a different spot in the list; they just dropped it.
		 *  This is our chance to handle the drop by reordering items and calling for a list refresh. */
		SLATE_EVENT(FOnAcceptDrop, OnAcceptDrop)

		/** Used for painting drop indicators. */
		SLATE_EVENT(FOnPaintDropIndicator, OnPaintDropIndicator)

		// Low level DragAndDrop
		SLATE_EVENT(FOnDragDetected, OnDragDetected)
		SLATE_EVENT(FOnTableRowDragEnter, OnDragEnter)
		SLATE_EVENT(FOnTableRowDragLeave, OnDragLeave)
		SLATE_EVENT(FOnTableRowDrop, OnDrop)

		SLATE_ATTRIBUTE(FMargin, Padding)

		SLATE_ARGUMENT(bool, ShowSelection)
		SLATE_ARGUMENT(bool, ShowWires)
		SLATE_ARGUMENT(bool, bAllowPreselectedItemActivation)
		SLATE_ARGUMENT(int32, MaxNumSelectedItems)
		SLATE_ARGUMENT(bool, CanSelectNone)

		/**
		 * The Signal Selection mode affect when the owner table gets notified that the selection has changed.
		 * This only affect the selection with the left mouse button!
		 * When Deferred, the owner table will get notified when the button is released or when a drag started.
		 * When Instantaneous, the owner table is notified as soon as the selection changed.
		 */
		SLATE_ARGUMENT(ETableRowSignalSelectionMode, SignalSelectionMode)

		SLATE_DEFAULT_SLOT(typename SBSTableRow<ItemType>::FArguments, Content)

	SLATE_END_ARGS()

	/**
	 * Construct this widget.
	 * @param InArgs The declaration data for this widget
	 */
	void Construct(const typename SBSTableRow<ItemType>::FArguments& InArgs,
		const TSharedRef<STableViewBase>& InOwnerTableView)
	{
		/** Note: Please initialize any state in ConstructInternal, not here. This is because SBSTableRow derivatives
		 *  call ConstructInternal directly to avoid constructing children. **/

		ConstructInternal(InArgs, InOwnerTableView);

		ConstructChildren(InOwnerTableView->TableViewMode, InArgs._Padding, InArgs._Content.Widget);
	}

	virtual void ConstructChildren(ETableViewMode::Type InOwnerTableMode, const TAttribute<FMargin>& InPadding,
		const TSharedRef<SWidget>& InContent)
	{
		this->Content = InContent;
		InnerContentSlot = nullptr;

		if (InOwnerTableMode == ETableViewMode::List || InOwnerTableMode == ETableViewMode::Tile)
		{
			// We just need to hold on to this row's content.
			this->ChildSlot.Padding(InPadding)[InContent];

			InnerContentSlot = &ChildSlot.AsSlot();
		}
		else
		{
			// -- Row is for TreeView --
			SHorizontalBox::FSlot* InnerContentSlotNativePtr = nullptr;

			// Rows in a TreeView need an expander button and some indentation
			this->ChildSlot[SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Fill)
			[
				SAssignNew(ExpanderArrowWidget, SExpanderArrow, SharedThis(this))
				.StyleSet(ExpanderStyleSet)
				.ShouldDrawWires(bShowWires)
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1)
			.Expose(InnerContentSlotNativePtr)
			.Padding(InPadding)
			[
				InContent
			]];

			InnerContentSlot = InnerContentSlotNativePtr;
		}
	}

#if WITH_ACCESSIBILITY

protected:
	friend class FSlateAccessibleTableRow;
	/**
	* An accessible implementation of SBSTableRow exposed to platform accessibility APIs.
	* For subclasses of SBSTableRow, inherit from this class and override any functions
	* to give the desired behavior.
	*/
	class FSlateAccessibleTableRow : public FSlateAccessibleWidget, public IAccessibleTableRow
	{
	public:
		FSlateAccessibleTableRow(TWeakPtr<SWidget> InWidget, EAccessibleWidgetType InWidgetType) :
			FSlateAccessibleWidget(InWidget, InWidgetType)
		{
		}

		// ~IAccessibleWidget
		virtual IAccessibleTableRow* AsTableRow() override
		{
			return this;
		}

		// ~IAccessibleTableRow
		virtual void Select() override
		{
			if (Widget.IsValid())
			{
				TSharedPtr<SBSTableRow<ItemType>> TableRow = StaticCastSharedPtr<SBSTableRow<ItemType>>(Widget.Pin());
				if (TableRow->OwnerTablePtr.IsValid())
				{
					TSharedRef<ITypedTableView<ItemType>> OwnerTable = TableRow->OwnerTablePtr.Pin().ToSharedRef();
					const bool bIsActive = OwnerTable->AsWidget()->HasKeyboardFocus();

					if (const ItemType* MyItemPtr = TableRow->GetItemForThis(OwnerTable))
					{
						const ItemType& MyItem = *MyItemPtr;
						const bool bIsSelected = OwnerTable->Private_IsItemSelected(MyItem);
						OwnerTable->Private_ClearSelection();
						OwnerTable->Private_SetItemSelection(MyItem, true, true);
						OwnerTable->Private_SignalSelectionChanged(ESelectInfo::Direct);
					}
				}
			}
		}

		virtual void AddToSelection() override
		{
			// @TODOAccessibility: When multi-selection is supported 
		}

		virtual void RemoveFromSelection() override
		{
			// @TODOAccessibility: When multi-selection is supported 
		}

		virtual bool IsSelected() const override
		{
			if (Widget.IsValid())
			{
				TSharedPtr<SBSTableRow<ItemType>> TableRow = StaticCastSharedPtr<SBSTableRow<ItemType>>(Widget.Pin());
				return TableRow->IsItemSelected();
			}
			return false;
		}

		virtual TSharedPtr<IAccessibleWidget> GetOwningTable() const override
		{
			if (Widget.IsValid())
			{
				TSharedPtr<SBSTableRow<ItemType>> TableRow = StaticCastSharedPtr<SBSTableRow<ItemType>>(Widget.Pin());
				if (TableRow->OwnerTablePtr.IsValid())
				{
					TSharedRef<SWidget> OwningTableWidget = TableRow->OwnerTablePtr.Pin()->AsWidget();
					return FSlateAccessibleWidgetCache::GetAccessibleWidgetChecked(OwningTableWidget);
				}
			}
			return nullptr;
		}

		// ~
	};

public:
	virtual TSharedRef<FSlateAccessibleWidget> CreateAccessibleWidget() override
	{
		// @TODOAccessibility: Add support for tile table rows and tree table rows etc.
		// The widget type passed in should be based on the table type of the owning table.
		EAccessibleWidgetType WidgetType = EAccessibleWidgetType::ListItem;
		return MakeShareable<FSlateAccessibleWidget>(
			new SBSTableRow<ItemType>::FSlateAccessibleTableRow(SharedThis(this), WidgetType));
	}
#endif

	/** Retrieves a brush for rendering a drop indicator for the specified drop zone. */
	const FSlateBrush* GetDropIndicatorBrush(EItemDropZone InItemDropZone) const
	{
		switch (InItemDropZone)
		{
		case EItemDropZone::AboveItem:
			return &Style->DropIndicator_Above;
			break;
		default: case EItemDropZone::OntoItem:
			return &Style->DropIndicator_Onto;
			break;
		case EItemDropZone::BelowItem:
			return &Style->DropIndicator_Below;
			break;
		};
	}

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
		FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle,
		bool bParentEnabled) const override
	{
		TSharedRef<ITypedTableView<ItemType>> OwnerTable = OwnerTablePtr.Pin().ToSharedRef();
		const bool bIsActive = OwnerTable->AsWidget()->HasKeyboardFocus();

		if (const ItemType* MyItemPtr = GetItemForThis(OwnerTable))
		{
			if (bIsActive && OwnerTable->Private_UsesSelectorFocus() && OwnerTable->
				Private_HasSelectorFocus(*MyItemPtr))
			{
				FSlateDrawElement::MakeBox(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(),
					&Style->SelectorFocusedBrush, ESlateDrawEffect::None,
					Style->SelectorFocusedBrush.GetTint(InWidgetStyle) * InWidgetStyle.GetColorAndOpacityTint());
			}
		}

		LayerId = SBorder::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle,
			bParentEnabled);

		if (ItemDropZone.IsSet())
		{
			if (PaintDropIndicatorEvent.IsBound())
			{
				LayerId = PaintDropIndicatorEvent.Execute(ItemDropZone.GetValue(), Args, AllottedGeometry,
					MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
			}
			else
			{
				OnPaintDropIndicator(ItemDropZone.GetValue(), Args, AllottedGeometry, MyCullingRect, OutDrawElements,
					LayerId, InWidgetStyle, bParentEnabled);
			}
		}

		return LayerId;
	}

	virtual int32 OnPaintDropIndicator(EItemDropZone InItemDropZone, const FPaintArgs& Args,
		const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
		int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
	{
		TSharedRef<ITypedTableView<ItemType>> OwnerTable = OwnerTablePtr.Pin().ToSharedRef();

		// Draw feedback for user dropping an item above, below, or onto a row.
		const FSlateBrush* DropIndicatorBrush = GetDropIndicatorBrush(InItemDropZone);

		if (OwnerTable->Private_GetOrientation() == Orient_Vertical)
		{
			FSlateDrawElement::MakeBox(OutDrawElements, LayerId++, AllottedGeometry.ToPaintGeometry(),
				DropIndicatorBrush, ESlateDrawEffect::None,
				DropIndicatorBrush->GetTint(InWidgetStyle) * InWidgetStyle.GetColorAndOpacityTint());
		}
		else
		{
			// Reuse the drop indicator asset for horizontal, by rotating the drawn box 90 degrees.
			const FVector2D LocalSize(AllottedGeometry.GetLocalSize());
			const FVector2D Pivot(LocalSize * 0.5f);
			const FVector2D RotatedLocalSize(LocalSize.Y, LocalSize.X);
			FSlateLayoutTransform RotatedTransform(Pivot - RotatedLocalSize * 0.5f);
			// Make the box centered to the alloted geometry, so that it can be rotated around the center.

			FSlateDrawElement::MakeRotatedBox(OutDrawElements, LayerId++,
				AllottedGeometry.ToPaintGeometry(RotatedLocalSize, RotatedTransform), DropIndicatorBrush,
				ESlateDrawEffect::None, -UE_HALF_PI, // 90 deg CCW
				RotatedLocalSize * 0.5f, // Relative center to the flipped
				FSlateDrawElement::RelativeToElement,
				DropIndicatorBrush->GetTint(InWidgetStyle) * InWidgetStyle.GetColorAndOpacityTint());
		}

		return LayerId;
	}

	/**
	 * Called when a mouse button is double clicked.  Override this in derived classes.
	 * @param  InMyGeometry  Widget geometry.
	 * @param  InMouseEvent  Mouse button event.
	 * @return  Returns whether the event was handled, along with other possible actions.
	 */
	virtual FReply OnMouseButtonDoubleClick(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent) override
	{
		if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
		{
			TSharedRef<ITypedTableView<ItemType>> OwnerTable = OwnerTablePtr.Pin().ToSharedRef();

			// Only one item can be double-clicked
			if (const ItemType* MyItemPtr = GetItemForThis(OwnerTable))
			{
				// If we're configured to route double-click messages to the owner of the table, then
				// do that here.  Otherwise, we'll toggle expansion.
				const bool bWasHandled = OwnerTable->Private_OnItemDoubleClicked(*MyItemPtr);
				if (!bWasHandled)
				{
					ToggleExpansion();
				}

				return FReply::Handled();
			}
		}

		return FReply::Unhandled();
	}

	/**
	 * See SWidget::OnMouseButtonDown
	 * @param MyGeometry The Geometry of the widget receiving the event.
	 * @param MouseEvent Information about the input event.
	 * @return Whether the event was handled along with possible requests for the system to take action.
	 */
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		TSharedRef<ITypedTableView<ItemType>> OwnerTable = OwnerTablePtr.Pin().ToSharedRef();
		bChangedSelectionOnMouseDown = false;
		bDragWasDetected = false;

		if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
		{
			const ESelectionMode::Type SelectionMode = GetSelectionMode();
			if (SelectionMode != ESelectionMode::None)
			{
				if (const ItemType* MyItemPtr = GetItemForThis(OwnerTable))
				{
					const ItemType& MyItem = *MyItemPtr;
					const int32 NumSelectedItems = OwnerTable->GetSelectedItems().Num();
					const bool bIsSelected = OwnerTable->Private_IsItemSelected(MyItem);
					const bool bCanSelectMoreItems = (GetMaxNumSelectedItems() == -1) || NumSelectedItems <
						GetMaxNumSelectedItems();
					const bool bCanUnselectMoreItems = CanSelectNone() || NumSelectedItems > 1;
					bool bSignalSelectionChanged = false;

					if (SelectionMode == ESelectionMode::Multi)
					{
						if (MouseEvent.IsControlDown())
						{
							if (bIsSelected && bCanUnselectMoreItems)
							{
								OwnerTable->Private_SetItemSelection(MyItem, false, true);
								bChangedSelectionOnMouseDown = true;
								bSignalSelectionChanged = true;
							}
							else if (!bIsSelected && bCanSelectMoreItems)
							{
								OwnerTable->Private_SetItemSelection(MyItem, true, true);
								bChangedSelectionOnMouseDown = true;
								bSignalSelectionChanged = true;
							}
						}
						else if (MouseEvent.IsShiftDown())
						{
							OwnerTable->Private_SelectRangeFromCurrentTo(MyItem);
							bChangedSelectionOnMouseDown = true;
							bSignalSelectionChanged = true;
						}
					}

					if (!bChangedSelectionOnMouseDown && bCanUnselectMoreItems && bIsSelected && NumSelectedItems == 1)
					{
						OwnerTable->Private_SetItemSelection(MyItem, !bIsSelected, true);
						bChangedSelectionOnMouseDown = true;
						bSignalSelectionChanged = true;
					}

					if (bSignalSelectionChanged)
					{
						if (SignalSelectionMode == ETableRowSignalSelectionMode::Instantaneous)
						{
							OwnerTable->Private_SignalSelectionChanged(ESelectInfo::OnMouseClick);
						}
					}

					if ((bAllowPreselectedItemActivation || !bIsSelected) && !bChangedSelectionOnMouseDown)
					{
						OwnerTable->Private_ClearSelection();
						OwnerTable->Private_SetItemSelection(MyItem, true, true);
						bChangedSelectionOnMouseDown = true;
						if (SignalSelectionMode == ETableRowSignalSelectionMode::Instantaneous)
						{
							OwnerTable->Private_SignalSelectionChanged(ESelectInfo::OnMouseClick);
						}
					}

					return FReply::Handled().DetectDrag(SharedThis(this), EKeys::LeftMouseButton).SetUserFocus(
						OwnerTable->AsWidget(), EFocusCause::Mouse).CaptureMouse(SharedThis(this));
				}
			}
		}

		return FReply::Unhandled();
	}

	/**
	 * See SWidget::OnMouseButtonUp
	 * @param MyGeometry The Geometry of the widget receiving the event.
	 * @param MouseEvent Information about the input event.
	 * @return Whether the event was handled along with possible requests for the system to take action.
	 */
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		TSharedRef<ITypedTableView<ItemType>> OwnerTable = OwnerTablePtr.Pin().ToSharedRef();

		// Requires #include "Widgets/Views/SListView.h" in your header (not done in SBSTableRow.h to avoid circular
		// reference).
		TSharedRef<STableViewBase> OwnerTableViewBase = StaticCastSharedRef<SListView<ItemType>>(OwnerTable);

		if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
		{
			FReply Reply = FReply::Unhandled().ReleaseMouseCapture();

			if (bChangedSelectionOnMouseDown)
			{
				Reply = FReply::Handled().ReleaseMouseCapture();
			}

			const bool bIsUnderMouse = MyGeometry.IsUnderLocation(MouseEvent.GetScreenSpacePosition());
			if (HasMouseCapture())
			{
				if (bIsUnderMouse && !bDragWasDetected)
				{
					switch (GetSelectionMode())
					{
					case ESelectionMode::SingleToggle:
						{
							if (!bChangedSelectionOnMouseDown)
							{
								OwnerTable->Private_ClearSelection();
								OwnerTable->Private_SignalSelectionChanged(ESelectInfo::OnMouseClick);
							}

							Reply = FReply::Handled().ReleaseMouseCapture();
						}
						break;

					case ESelectionMode::Multi:
						{
							if (!bChangedSelectionOnMouseDown && !MouseEvent.IsControlDown() && !MouseEvent.
								IsShiftDown())
							{
								if (const ItemType* MyItemPtr = GetItemForThis(OwnerTable))
								{
									const bool bIsSelected = OwnerTable->Private_IsItemSelected(*MyItemPtr);
									if (bIsSelected && OwnerTable->Private_GetNumSelectedItems() > 1)
									{
										// We are mousing up on a previous selected item;
										// deselect everything but this item.

										OwnerTable->Private_ClearSelection();
										OwnerTable->Private_SetItemSelection(*MyItemPtr, true, true);
										OwnerTable->Private_SignalSelectionChanged(ESelectInfo::OnMouseClick);

										Reply = FReply::Handled().ReleaseMouseCapture();
									}
								}
							}
						}
						break;
					}
				}

				if (const ItemType* MyItemPtr = GetItemForThis(OwnerTable))
				{
					if (OwnerTable->Private_OnItemClicked(*MyItemPtr))
					{
						Reply = FReply::Handled().ReleaseMouseCapture();
					}
				}

				if (bChangedSelectionOnMouseDown && !bDragWasDetected && (SignalSelectionMode ==
					ETableRowSignalSelectionMode::Deferred))
				{
					OwnerTable->Private_SignalSelectionChanged(ESelectInfo::OnMouseClick);
				}

				return Reply;
			}
		}
		else if (MouseEvent.GetEffectingButton() == EKeys::RightMouseButton && !OwnerTableViewBase->
			IsRightClickScrolling())
		{
			// Handle selection of items when releasing the right mouse button, but only if the user isn't actively
			// scrolling the view by holding down the right mouse button.

			switch (GetSelectionMode())
			{
			case ESelectionMode::Single:
			case ESelectionMode::SingleToggle:
			case ESelectionMode::Multi:
				{
					// Only one item can be selected at a time
					if (const ItemType* MyItemPtr = GetItemForThis(OwnerTable))
					{
						const bool bIsSelected = OwnerTable->Private_IsItemSelected(*MyItemPtr);

						// Select the item under the cursor
						if (!bIsSelected)
						{
							OwnerTable->Private_ClearSelection();
							OwnerTable->Private_SetItemSelection(*MyItemPtr, true, true);
							OwnerTable->Private_SignalSelectionChanged(ESelectInfo::OnMouseClick);
						}

						OwnerTable->Private_OnItemRightClicked(*MyItemPtr, MouseEvent);

						return FReply::Handled();
					}
				}
				break;
			}
		}

		return FReply::Unhandled();
	}

	virtual FReply OnTouchStarted(const FGeometry& MyGeometry, const FPointerEvent& InTouchEvent) override
	{
		bProcessingSelectionTouch = true;

		return FReply::Handled()
			// Drag detect because if this tap turns into a drag, we stop processing
			// the selection touch.
			.DetectDrag(SharedThis(this), EKeys::LeftMouseButton);
	}

	virtual FReply OnTouchEnded(const FGeometry& MyGeometry, const FPointerEvent& InTouchEvent) override
	{
		FReply Reply = FReply::Unhandled();

		if (bProcessingSelectionTouch)
		{
			bProcessingSelectionTouch = false;
			const TSharedRef<ITypedTableView<ItemType>> OwnerTable = OwnerTablePtr.Pin().ToSharedRef();
			if (const ItemType* MyItemPtr = GetItemForThis(OwnerTable))
			{
				ESelectionMode::Type SelectionMode = GetSelectionMode();
				if (SelectionMode != ESelectionMode::None)
				{
					const bool bIsSelected = OwnerTable->Private_IsItemSelected(*MyItemPtr);
					if (!bIsSelected)
					{
						if (SelectionMode != ESelectionMode::Multi)
						{
							OwnerTable->Private_ClearSelection();
						}
						OwnerTable->Private_SetItemSelection(*MyItemPtr, true, true);
						OwnerTable->Private_SignalSelectionChanged(ESelectInfo::OnMouseClick);

						Reply = FReply::Handled();
					}
					else if (SelectionMode == ESelectionMode::SingleToggle || SelectionMode == ESelectionMode::Multi)
					{
						OwnerTable->Private_SetItemSelection(*MyItemPtr, true, true);
						OwnerTable->Private_SignalSelectionChanged(ESelectInfo::OnMouseClick);

						Reply = FReply::Handled();
					}
				}

				if (OwnerTable->Private_OnItemClicked(*MyItemPtr))
				{
					Reply = FReply::Handled();
				}
			}
		}

		return Reply;
	}

	virtual FReply OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		if (bProcessingSelectionTouch)
		{
			// With touch input, dragging scrolls the list while selection requires a tap.
			// If we are processing a touch and it turned into a drag; pass it on to the 
			bProcessingSelectionTouch = false;
			return FReply::Handled().CaptureMouse(OwnerTablePtr.Pin()->AsWidget());
		}
		else if (HasMouseCapture())
		{
			// Avoid changing the selection on the mouse up if there was a drag
			bDragWasDetected = true;

			if (bChangedSelectionOnMouseDown && SignalSelectionMode == ETableRowSignalSelectionMode::Deferred)
			{
				TSharedPtr<ITypedTableView<ItemType>> OwnerTable = OwnerTablePtr.Pin();
				OwnerTable->Private_SignalSelectionChanged(ESelectInfo::OnMouseClick);
			}
		}

		if (OnDragDetected_Handler.IsBound())
		{
			return OnDragDetected_Handler.Execute(MyGeometry, MouseEvent);
		}
		else
		{
			return FReply::Unhandled();
		}
	}

	virtual void OnDragEnter(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override
	{
		if (OnDragEnter_Handler.IsBound())
		{
			OnDragEnter_Handler.Execute(DragDropEvent);
		}
	}

	virtual void OnDragLeave(const FDragDropEvent& DragDropEvent) override
	{
		ItemDropZone = TOptional<EItemDropZone>();

		if (OnDragLeave_Handler.IsBound())
		{
			OnDragLeave_Handler.Execute(DragDropEvent);
		}
	}

	/** @return the zone (above, onto, below) based on where the user is hovering over within the row. */
	EItemDropZone ZoneFromPointerPosition(FVector2D LocalPointerPos, FVector2D LocalSize, EOrientation Orientation)
	{
		const FVector2D::FReal PointerPos = Orientation == EOrientation::Orient_Horizontal
			? LocalPointerPos.X
			: LocalPointerPos.Y;
		const FVector2D::FReal Size = Orientation == EOrientation::Orient_Horizontal ? LocalSize.X : LocalSize.Y;

		const FVector2D::FReal ZoneBoundarySu = FMath::Clamp(Size * 0.25f, 3.0f, 10.0f);
		if (PointerPos < ZoneBoundarySu)
		{
			return EItemDropZone::AboveItem;
		}
		else if (PointerPos > Size - ZoneBoundarySu)
		{
			return EItemDropZone::BelowItem;
		}
		else
		{
			return EItemDropZone::OntoItem;
		}
	}

	virtual FReply OnDragOver(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override
	{
		if (OnCanAcceptDrop.IsBound())
		{
			const TSharedRef<ITypedTableView<ItemType>> OwnerTable = OwnerTablePtr.Pin().ToSharedRef();
			const FVector2D LocalPointerPos = MyGeometry.AbsoluteToLocal(DragDropEvent.GetScreenSpacePosition());
			const EItemDropZone ItemHoverZone = ZoneFromPointerPosition(LocalPointerPos, MyGeometry.GetLocalSize(),
				OwnerTable->Private_GetOrientation());

			ItemDropZone = [ItemHoverZone, DragDropEvent, this]()
			{
				TSharedRef<ITypedTableView<ItemType>> OwnerTable = OwnerTablePtr.Pin().ToSharedRef();
				if (const ItemType* MyItemPtr = GetItemForThis(OwnerTable))
				{
					return OnCanAcceptDrop.Execute(DragDropEvent, ItemHoverZone, *MyItemPtr);
				}

				return TOptional<EItemDropZone>();
			}();

			return FReply::Handled();
		}
		else
		{
			return FReply::Unhandled();
		}
	}

	virtual FReply OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override
	{
		const FReply Reply = [&]()
		{
			if (OnAcceptDrop.IsBound())
			{
				const TSharedRef<ITypedTableView<ItemType>> OwnerTable = OwnerTablePtr.Pin().ToSharedRef();

				// A drop finishes the drag/drop operation, so we are no longer providing any feedback.
				ItemDropZone = TOptional<EItemDropZone>();

				// Find item associated with this widget.
				if (const ItemType* MyItemPtr = GetItemForThis(OwnerTable))
				{
					// Which physical drop zone is the drop about to be performed onto?
					const FVector2D LocalPointerPos = MyGeometry.
						AbsoluteToLocal(DragDropEvent.GetScreenSpacePosition());
					const EItemDropZone HoveredZone = ZoneFromPointerPosition(LocalPointerPos,
						MyGeometry.GetLocalSize(), OwnerTable->Private_GetOrientation());

					// The row gets final say over which zone to drop onto regardless of physical location.
					const TOptional<EItemDropZone> ReportedZone = OnCanAcceptDrop.IsBound()
						? OnCanAcceptDrop.Execute(DragDropEvent, HoveredZone, *MyItemPtr)
						: HoveredZone;

					if (ReportedZone.IsSet())
					{
						FReply DropReply = OnAcceptDrop.Execute(DragDropEvent, ReportedZone.GetValue(), *MyItemPtr);
						if (DropReply.IsEventHandled() && ReportedZone.GetValue() == EItemDropZone::OntoItem)
						{
							// Expand the drop target just in case, so that what we dropped is visible.
							OwnerTable->Private_SetItemExpansion(*MyItemPtr, true);
						}

						return DropReply;
					}
				}
			}

			return FReply::Unhandled();
		}();

		// @todo slate : Made obsolete by OnAcceptDrop. Get rid of this.
		if (!Reply.IsEventHandled() && OnDrop_Handler.IsBound())
		{
			return OnDrop_Handler.Execute(DragDropEvent);
		}

		return Reply;
	}

	virtual void InitializeRow() override
	{
	}

	virtual void ResetRow() override
	{
	}

	virtual void SetIndexInList(int32 InIndexInList) override
	{
		IndexInList = InIndexInList;
	}

	virtual int32 GetIndexInList() override
	{
		return IndexInList;
	}

	virtual bool IsItemExpanded() const override
	{
		TSharedRef<ITypedTableView<ItemType>> OwnerTable = OwnerTablePtr.Pin().ToSharedRef();
		if (const ItemType* MyItemPtr = GetItemForThis(OwnerTable))
		{
			return OwnerTable->Private_IsItemExpanded(*MyItemPtr);
		}

		return false;
	}

	virtual void ToggleExpansion() override
	{
		TSharedRef<ITypedTableView<ItemType>> OwnerTable = OwnerTablePtr.Pin().ToSharedRef();

		const bool bItemHasChildren = OwnerTable->Private_DoesItemHaveChildren(IndexInList);
		// Nothing to expand if row being clicked on doesn't have children
		if (bItemHasChildren)
		{
			if (const ItemType* MyItemPtr = GetItemForThis(OwnerTable))
			{
				const bool bIsItemExpanded = bItemHasChildren && OwnerTable->Private_IsItemExpanded(*MyItemPtr);
				OwnerTable->Private_SetItemExpansion(*MyItemPtr, !bIsItemExpanded);
			}
		}
	}

	virtual bool IsItemSelected() const override
	{
		TSharedRef<ITypedTableView<ItemType>> OwnerTable = OwnerTablePtr.Pin().ToSharedRef();
		if (const ItemType* MyItemPtr = GetItemForThis(OwnerTable))
		{
			return OwnerTable->Private_IsItemSelected(*MyItemPtr);
		}

		return false;
	}

	virtual int32 GetIndentLevel() const override
	{
		return OwnerTablePtr.Pin()->Private_GetNestingDepth(IndexInList);
	}

	virtual int32 DoesItemHaveChildren() const override
	{
		return OwnerTablePtr.Pin()->Private_DoesItemHaveChildren(IndexInList);
	}

	virtual TBitArray<> GetWiresNeededByDepth() const override
	{
		return OwnerTablePtr.Pin()->Private_GetWiresNeededByDepth(IndexInList);
	}

	virtual bool IsLastChild() const override
	{
		return OwnerTablePtr.Pin()->Private_IsLastChild(IndexInList);
	}

	virtual TSharedRef<SWidget> AsWidget() override
	{
		return SharedThis(this);
	}

	/** Set the entire content of this row, replacing any extra UI (such as the expander arrows for tree views) that
	 *  was added by ConstructChildren. */
	virtual void SetRowContent(TSharedRef<SWidget> InContent)
	{
		this->Content = InContent;
		InnerContentSlot = nullptr;
		SBorder::SetContent(InContent);
	}

	/** Set the inner content of this row, preserving any extra UI (such as the expander arrows for tree views) that
	 *  was added by ConstructChildren. */
	virtual void SetContent(TSharedRef<SWidget> InContent) override
	{
		this->Content = InContent;

		if (InnerContentSlot)
		{
			InnerContentSlot->AttachWidget(InContent);
		}
		else
		{
			SBorder::SetContent(InContent);
		}
	}

	/** Get the inner content of this row. */
	virtual TSharedPtr<SWidget> GetContent() override
	{
		if (this->Content.IsValid())
		{
			return this->Content.Pin();
		}
		else
		{
			return TSharedPtr<SWidget>();
		}
	}

	virtual void Private_OnExpanderArrowShiftClicked() override
	{
		TSharedRef<ITypedTableView<ItemType>> OwnerTable = OwnerTablePtr.Pin().ToSharedRef();

		const bool bItemHasChildren = OwnerTable->Private_DoesItemHaveChildren(IndexInList);
		// Nothing to expand if row being clicked on doesn't have children
		if (bItemHasChildren)
		{
			if (const ItemType* MyItemPtr = GetItemForThis(OwnerTable))
			{
				const bool IsItemExpanded = bItemHasChildren && OwnerTable->Private_IsItemExpanded(*MyItemPtr);
				OwnerTable->Private_OnExpanderArrowShiftClicked(*MyItemPtr, !IsItemExpanded);
			}
		}
	}

	/** @return The border to be drawn around this list item. */
	virtual const FSlateBrush* GetBorder() const
	{
		TSharedRef<ITypedTableView<ItemType>> OwnerTable = OwnerTablePtr.Pin().ToSharedRef();

		const bool bIsActive = OwnerTable->AsWidget()->HasKeyboardFocus();

		const bool bItemHasChildren = OwnerTable->Private_DoesItemHaveChildren(IndexInList);

		static FName GenericWhiteBoxBrush("GenericWhiteBox");

		// @todo: Slate Style - make this part of the widget style
		const FSlateBrush* WhiteBox = FCoreStyle::Get().GetBrush(GenericWhiteBoxBrush);

		if (const ItemType* MyItemPtr = GetItemForThis(OwnerTable))
		{
			const bool bIsSelected = OwnerTable->Private_IsItemSelected(*MyItemPtr);
			const bool bIsHighlighted = OwnerTable->Private_IsItemHighlighted(*MyItemPtr);

			const bool bAllowSelection = GetSelectionMode() != ESelectionMode::None;
			const bool bEvenEntryIndex = (IndexInList % 2 == 0);

			if (bIsSelected && bShowSelection)
			{
				if (bIsActive)
				{
					return IsHovered() ? &Style->ActiveHoveredBrush : &Style->ActiveBrush;
				}
				else
				{
					return IsHovered() ? &Style->InactiveHoveredBrush : &Style->InactiveBrush;
				}
			}
			else if (!bIsSelected && bIsHighlighted)
			{
				if (bIsActive)
				{
					return IsHovered()
						? (bEvenEntryIndex
							? &Style->EvenRowBackgroundHoveredBrush
							: &Style->OddRowBackgroundHoveredBrush)
						: &Style->ActiveHighlightedBrush;
				}
				else
				{
					return IsHovered()
						? (bEvenEntryIndex
							? &Style->EvenRowBackgroundHoveredBrush
							: &Style->OddRowBackgroundHoveredBrush)
						: &Style->InactiveHighlightedBrush;
				}
			}
			else if (bItemHasChildren && Style->bUseParentRowBrush && GetIndentLevel() == 0)
			{
				return IsHovered() ? &Style->ParentRowBackgroundHoveredBrush : &Style->ParentRowBackgroundBrush;
			}
			else
			{
				// Add a slightly lighter background for even rows
				if (bEvenEntryIndex)
				{
					return (IsHovered() && bAllowSelection)
						? &Style->EvenRowBackgroundHoveredBrush
						: &Style->EvenRowBackgroundBrush;
				}
				else
				{
					return (IsHovered() && bAllowSelection)
						? &Style->OddRowBackgroundHoveredBrush
						: &Style->OddRowBackgroundBrush;
				}
			}
		}

		return nullptr;
	}

	/** 
	 * Callback to determine if the row is selected singularly and has keyboard focus or not.
	 * @return true if selected by owning widget.
	 */
	bool IsSelectedExclusively() const
	{
		TSharedRef<ITypedTableView<ItemType>> OwnerTable = OwnerTablePtr.Pin().ToSharedRef();

		if (!OwnerTable->AsWidget()->HasKeyboardFocus() || OwnerTable->Private_GetNumSelectedItems() > 1)
		{
			return false;
		}

		if (const ItemType* MyItemPtr = GetItemForThis(OwnerTable))
		{
			return OwnerTable->Private_IsItemSelected(*MyItemPtr);
		}

		return false;
	}

	/**
	 * Callback to determine if the row is selected or not.
	 * @return true if selected by owning widget.
	 */
	bool IsSelected() const
	{
		TSharedRef<ITypedTableView<ItemType>> OwnerTable = OwnerTablePtr.Pin().ToSharedRef();

		if (const ItemType* MyItemPtr = GetItemForThis(OwnerTable))
		{
			return OwnerTable->Private_IsItemSelected(*MyItemPtr);
		}

		return false;
	}

	/** By default, this function does nothing, it should be implemented by derived class. */
	virtual FVector2D GetRowSizeForColumn(const FName& InColumnName) const override
	{
		return FVector2D::ZeroVector;
	}

	void SetExpanderArrowVisibility(const EVisibility InExpanderArrowVisibility)
	{
		if (ExpanderArrowWidget)
		{
			ExpanderArrowWidget->SetVisibility(InExpanderArrowVisibility);
		}
	}

	/** Protected constructor; SWidgets should only be instantiated via declarative syntax. */
	SBSTableRow() : IndexInList(0), bShowSelection(true), SignalSelectionMode(ETableRowSignalSelectionMode::Deferred)
	{
#if WITH_ACCESSIBILITY
		// As the contents of table rows could be anything,
		// Ideally, somebody would assign a custom label to each table row with non-accessible content.
		// However, that's not always feasible, so we want the screen reader to read out the concatenated contents of
		// children.
		// E.g. If ItemType == FString, then the screen reader can just read out the contents of the text box.
		AccessibleBehavior = EAccessibleBehavior::Summary;
		bCanChildrenBeAccessible = true;
#endif
	}

protected:
	/**
	 * An internal method to construct and setup this row widget (purposely avoids child construction). 
	 * Split out from Construct() so that sub-classes can invoke super construction without invoking 
	 * ConstructChildren() (sub-classes may want to constuct their own children in their own special way).
	 * @param InArgs Declaration data for this widget.
	 * @param InOwnerTableView The table that this row belongs to.
	 */
	void ConstructInternal(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView)
	{
		bProcessingSelectionTouch = false;

		check(InArgs._Style);
		Style = InArgs._Style;

		check(InArgs._ExpanderStyleSet);
		ExpanderStyleSet = InArgs._ExpanderStyleSet;

		SetBorderImage(TAttribute<const FSlateBrush*>(this, &SBSTableRow::GetBorder));

		this->SetForegroundColor(TAttribute<FSlateColor>(this, &SBSTableRow::GetForegroundBasedOnSelection));

		this->OnCanAcceptDrop = InArgs._OnCanAcceptDrop;
		this->OnAcceptDrop = InArgs._OnAcceptDrop;

		this->OnDragDetected_Handler = InArgs._OnDragDetected;
		this->OnDragEnter_Handler = InArgs._OnDragEnter;
		this->OnDragLeave_Handler = InArgs._OnDragLeave;
		this->OnDrop_Handler = InArgs._OnDrop;

		this->SetOwnerTableView(InOwnerTableView);

		this->bShowSelection = InArgs._ShowSelection;

		this->SignalSelectionMode = InArgs._SignalSelectionMode;

		this->bShowWires = InArgs._ShowWires;

		this->bAllowPreselectedItemActivation = InArgs._bAllowPreselectedItemActivation;

		this->MaxNumSelectedItems = InArgs._MaxNumSelectedItems;

		this->bCanSelectNone = InArgs._CanSelectNone;
	}

	void SetOwnerTableView(TSharedPtr<STableViewBase> OwnerTableView)
	{
		// We want to cast to a ITypedTableView.
		// We cast to a SListView<ItemType> because C++ doesn't know that
		// being a STableView implies being a ITypedTableView.
		// See SListView.
		this->OwnerTablePtr = StaticCastSharedPtr<SListView<ItemType>>(OwnerTableView);
	}

	FSlateColor GetForegroundBasedOnSelection() const
	{
		const TSharedPtr<ITypedTableView<ItemType>> OwnerTable = OwnerTablePtr.Pin();
		const FSlateColor& NonSelectedForeground = Style->TextColor;
		const FSlateColor& SelectedForeground = Style->SelectedTextColor;

		if (!bShowSelection || !OwnerTable.IsValid())
		{
			return NonSelectedForeground;
		}

		if (const ItemType* MyItemPtr = GetItemForThis(OwnerTable.ToSharedRef()))
		{
			const bool bIsSelected = OwnerTable->Private_IsItemSelected(*MyItemPtr);

			return bIsSelected ? SelectedForeground : NonSelectedForeground;
		}

		return NonSelectedForeground;
	}

	virtual ESelectionMode::Type GetSelectionMode() const override
	{
		const TSharedPtr<ITypedTableView<ItemType>> OwnerTable = OwnerTablePtr.Pin();
		return OwnerTable->Private_GetSelectionMode();
	}

	const ItemType* GetItemForThis(const TSharedRef<ITypedTableView<ItemType>>& OwnerTable) const
	{
		const ItemType* MyItemPtr = OwnerTable->Private_ItemFromWidget(this);
		if (MyItemPtr)
		{
			return MyItemPtr;
		}
		else
		{
			checkf(OwnerTable->Private_IsPendingRefresh(),
				TEXT( "We were unable to find the item for this widget.  If it was removed from the source collection, "
					"the list should be pending a refresh." ));
		}

		return nullptr;
	}

	int32 GetMaxNumSelectedItems() const
	{
		return MaxNumSelectedItems;
	}

	bool CanSelectNone() const
	{
		return bCanSelectNone;
	}

protected:
	/** The list that owns this Selectable. */
	TWeakPtr<ITypedTableView<ItemType>> OwnerTablePtr;

	/** Index of the corresponding data item in the list. */
	int32 IndexInList;

	/** Whether to visually show that this row is selected. */
	bool bShowSelection;

	/** When should we signal that selection changed for a left click. */
	ETableRowSignalSelectionMode SignalSelectionMode;

	/** Style used to draw this table row. */
	const FTableRowStyle* Style;

	/** The slate style to use with the expander. */
	const ISlateStyle* ExpanderStyleSet;

	/** A pointer to the expander arrow on the row (if it exists). */
	TSharedPtr<SExpanderArrow> ExpanderArrowWidget;

	/** @see SBSTableRow's OnCanAcceptDrop event. */
	FOnCanAcceptDrop OnCanAcceptDrop;

	/** @see SBSTableRow's OnAcceptDrop event. */
	FOnAcceptDrop OnAcceptDrop;

	/** Optional delegate for painting drop indicators. */
	FOnPaintDropIndicator PaintDropIndicatorEvent;

	/** Are we currently dragging/dropping over this item? */
	TOptional<EItemDropZone> ItemDropZone;

	/** Delegate triggered when a user starts to drag a list item. */
	FOnDragDetected OnDragDetected_Handler;

	/** Delegate triggered when a user's drag enters the bounds of this list item. */
	FOnTableRowDragEnter OnDragEnter_Handler;

	/** Delegate triggered when a user's drag leaves the bounds of this list item. */
	FOnTableRowDragLeave OnDragLeave_Handler;

	/** Delegate triggered when a user's drag is dropped in the bounds of this list item. */
	FOnTableRowDrop OnDrop_Handler;

	/** The slot that contains the inner content for this row. If this is set, SetContent populates this slot with the
	 *  new content rather than replace the content wholesale. */
	FSlotBase* InnerContentSlot;

	/** The widget in the content slot for this row. */
	TWeakPtr<SWidget> Content;

	bool bChangedSelectionOnMouseDown;

	bool bDragWasDetected;

	/** Did the current a touch interaction start in this item? */
	bool bProcessingSelectionTouch;

	/** When activating an item via mouse button, we generally don't allow pre-selected items to be activated. */
	bool bAllowPreselectedItemActivation;

private:
	bool bShowWires;

	bool bCanSelectNone;

	int32 MaxNumSelectedItems;
};
