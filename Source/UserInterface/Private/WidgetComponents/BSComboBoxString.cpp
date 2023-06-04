﻿// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "WidgetComponents/BSComboBoxString.h"
#include "WidgetComponents/BSComboBoxEntry.h"
#include "WidgetComponents/SBSComboBox.h"
#include "WidgetComponents/TooltipWidget.h"
#include "Widgets/SNullWidget.h"
#include "UObject/EditorObjectVersion.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/Font.h"
#include "Styling/UMGCoreStyle.h"
#include "WidgetComponents/TooltipImage.h"

#define LOCTEXT_NAMESPACE "UMG"

static FComboBoxStyle* DefaultComboBoxStyle = nullptr;
static FTableRowStyle* DefaultComboBoxRowStyle = nullptr;

#if WITH_EDITOR
static FComboBoxStyle* EditorComboBoxStyle = nullptr;
static FTableRowStyle* EditorComboBoxRowStyle = nullptr;
#endif 


void UBSComboBoxString::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
	for (TSharedPtr<FString> SharedStr : CurrentlySelectedOptionPointers)
	{
		SharedStr.Reset();
	}
	MyComboBox.Reset();
	ComboBoxContent.Reset();
}

void UBSComboBoxString::PostInitProperties()
{
	Super::PostInitProperties();

	// Initialize the set of options from the default set only once.
	for (const FString& DefaultOption : DefaultOptions)
	{
		AddOption(DefaultOption);
	}
}

void UBSComboBoxString::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	Ar.UsingCustomVersion(FEditorObjectVersion::GUID);
}

void UBSComboBoxString::PostLoad()
{
	Super::PostLoad();

	// Initialize the set of options from the default set only once.
	for (const FString& DefaultOption : DefaultOptions)
	{
		AddOption(DefaultOption);
	}

	if (GetLinkerCustomVersion(FEditorObjectVersion::GUID) < FEditorObjectVersion::ComboBoxControllerSupportUpdate)
	{
		EnableGamepadNavigationMode = false;
	}
}

void UBSComboBoxString::InitializeComboBoxEntry(const UBSComboBoxEntry* Entry, const FText& EntryText, const bool bShowTooltipImage, const FText& TooltipText) const
{
	Entry->SetEntryText(EntryText);
	if (UTooltipImage* TooltipImage = Entry->GetTooltipImage())
	{
		TooltipImage->OnTooltipHovered.AddDynamic(this, &ThisClass::OnTooltipImageHovered);
		TooltipImage->SetupTooltipImage(TooltipText);
	}
}

const FText UBSComboBoxString::GetPaletteCategory()
{
	return Super::GetPaletteCategory();
}

UBSComboBoxString::UBSComboBoxString()
{
	if (DefaultComboBoxStyle == nullptr)
	{
		DefaultComboBoxStyle = new FComboBoxStyle(FUMGCoreStyle::Get().GetWidgetStyle<FComboBoxStyle>("ComboBox"));

		// Unlink UMG default colors.
		DefaultComboBoxStyle->UnlinkColors();
	}

	if (DefaultComboBoxRowStyle == nullptr)
	{
		DefaultComboBoxRowStyle = new FTableRowStyle(FUMGCoreStyle::Get().GetWidgetStyle<FTableRowStyle>("TableView.Row"));

		// Unlink UMG default colors.
		DefaultComboBoxRowStyle->UnlinkColors();
	}

	WidgetStyle = *DefaultComboBoxStyle;
	ItemStyle = *DefaultComboBoxRowStyle;

#if WITH_EDITOR 
	if (EditorComboBoxStyle == nullptr)
	{
		EditorComboBoxStyle = new FComboBoxStyle(FCoreStyle::Get().GetWidgetStyle<FComboBoxStyle>("EditorUtilityComboBox"));

		// Unlink UMG Editor colors from the editor settings colors.
		EditorComboBoxStyle->UnlinkColors();
	}

	if (EditorComboBoxRowStyle == nullptr)
	{
		EditorComboBoxRowStyle = new FTableRowStyle(FCoreStyle::Get().GetWidgetStyle<FTableRowStyle>("TableView.Row"));

		// Unlink UMG Editor colors from the editor settings colors.
		EditorComboBoxRowStyle->UnlinkColors();
	}

	if (IsEditorWidget())
	{
		WidgetStyle = *EditorComboBoxStyle;
		ItemStyle = *EditorComboBoxRowStyle;

		// The CDO isn't an editor widget and thus won't use the editor style, call post edit change to mark difference from CDO
		PostEditChange();
	}
#endif // WITH_EDITOR

	WidgetStyle.UnlinkColors();
	ItemStyle.UnlinkColors();

	ForegroundColor = ItemStyle.TextColor;
	bIsFocusable = true;

	ContentPadding = FMargin(4.0, 2.0);
	MaxListHeight = 450.0f;
	HasDownArrow = true;
	EnableGamepadNavigationMode = true;
	// We don't want to try and load fonts on the server.
	if ( !IsRunningDedicatedServer() )
	{
		static ConstructorHelpers::FObjectFinder<UFont> RobotoFontObj(*UWidget::GetDefaultFontName());
		Font = FSlateFontInfo(RobotoFontObj.Object, 16, FName("Bold"));
	}
}

UTooltipWidget* UBSComboBoxString::ConstructTooltipWidget()
{
	return CreateWidget<UTooltipWidget>(this, TooltipWidgetClass);
}

UTooltipWidget* UBSComboBoxString::GetTooltipWidget() const
{
	return ActiveTooltipWidget;
}

void UBSComboBoxString::UpdateOrGenerateWidget(TSharedPtr<FString> Item)
{
	ComboBoxContent->SetContent(HandleGenerateWidget(Item));
}

TSharedRef<SWidget> UBSComboBoxString::HandleGenerateWidget(TSharedPtr<FString> Item) const
{
	const FString StringItem = Item.IsValid() ? *Item : FString();
	
	// Call the user's delegate to see if they want to generate a custom widget bound to the data source.
	if (!IsDesignTime() && OnGenerateWidgetEventDelegate.IsBound())
	{
		if (UWidget* Widget = OnGenerateWidgetEventDelegate.Execute(this, StringItem))
		{
			if (const UBSComboBoxEntry* Entry = Cast<UBSComboBoxEntry>(Widget))
			{
				// Show the tooltip image if creating a combo box entry widget
				Entry->SetTooltipImageVisibility(true);
			}
			return Widget->TakeWidget();
		}
	}

	// If a row wasn't generated just create the default one, a simple text block of the item's name.
	return SNew(STextBlock)
		.Text(FText::FromString(StringItem))
		.Font(Font);
}

void UBSComboBoxString::HandleSelectionChanged(const TArray<TSharedPtr<FString>>& Items, const ESelectInfo::Type SelectionType)
{
	CurrentlySelectedOptionPointers = Items;

	// When the selection changes we always generate another widget to represent the content area of the combobox.
	if (ComboBoxContent && MyComboBox)
	{
		ComboBoxContent->SetContent(HandleSelectionChangedGenerateWidget(CurrentlySelectedOptionPointers));
	}
	
	if (!IsDesignTime())
	{
		TArray<FString> Temp;
		for (const TSharedPtr<FString>& Item : Items)
		{
			if (Item != nullptr)
			{
				Temp.Add(*Item);
				// UE_LOG(LogTemp, Display, TEXT("Item: %s"), **Item.Get());
			}
		}
		OnSelectionChanged.Broadcast(Temp, SelectionType);
	}
	// UE_LOG(LogTemp, Display, TEXT("GetOptionCount %d"), GetOptionCount());
	// UE_LOG(LogTemp, Display, TEXT("GetSelectedOptionCount %d"), GetSelectedOptionCount());
}

TSharedRef<SWidget> UBSComboBoxString::HandleSelectionChangedGenerateWidget(TConstArrayView<SBSComboBox<TSharedPtr<FString>>::NullableOptionType> Items) const
{
	// Call the user's delegate to see if they want to generate a custom widget bound to the data source.
	if (!IsDesignTime() && OnSelectionChanged_GenerateWidgetForMultiSelection.IsBound())
	{
		TArray<FString> Copy;
		for (const TSharedPtr<FString>& Item : Items)
		{
			if (Item.Get())
			{
				Copy.Add(*Item);
			}
		}
		if (UWidget* Widget = OnSelectionChanged_GenerateWidgetForMultiSelection.Execute(this, Copy))
		{
			if (const UBSComboBoxEntry* Entry = Cast<UBSComboBoxEntry>(Widget))
			{
				// Hide the tooltip image if creating the top "selection entry" widget
				Entry->SetTooltipImageVisibility(false);
			}
			return Widget->TakeWidget();
		}
	}
	FString DefaultListOfItems;
	for (int i = 0; i < Items.Num(); i++)
	{
		if (Items[i].Get())
		{
			DefaultListOfItems.Append(*Items[i]);
			if (i < Items.Num() - 1)
			{
				DefaultListOfItems.Append(", ");
			}
		}
	}
	// If a row wasn't generated just create the default one, a simple text block of the item's name.
	return SNew(STextBlock)
		.Text(FText::FromString(DefaultListOfItems))
		.Font(Font);
}

void UBSComboBoxString::HandleOpening()
{
	OnOpening.Broadcast();
}

TSharedRef<SWidget> UBSComboBoxString::RebuildWidget()
{
	ActiveTooltipWidget = ConstructTooltipWidget();
	
	int32 InitialIndex = FindOptionIndex(GetSelectedOption());
	
	TSharedPtr<FString> CurrentOptionPtr;
	
	if ( InitialIndex != -1 )
	{
		CurrentOptionPtr = Options[InitialIndex];
	}
	
	MyComboBox =
		SNew(SBSComboBox<TSharedPtr<FString>>)
		.ComboBoxStyle(&WidgetStyle)
		.ItemStyle(&ItemStyle)
		.ForegroundColor(ForegroundColor)
		.OptionsSource(&Options)
		.InitiallySelectedItems(CurrentlySelectedOptionPointers)
		.ContentPadding(ContentPadding)
		.MaxListHeight(MaxListHeight)
		.HasDownArrow(HasDownArrow)
		.EnableGamepadNavigationMode(EnableGamepadNavigationMode)
		.OnGenerateWidget(BIND_UOBJECT_DELEGATE(SBSComboBox<TSharedPtr<FString>>::FOnGenerateWidget, HandleGenerateWidget))
		.OnMultiSelectionChanged(BIND_UOBJECT_DELEGATE(FOnMultiSelectionChanged, HandleSelectionChanged))
		.OnComboBoxOpening(BIND_UOBJECT_DELEGATE(FOnComboBoxOpening, HandleOpening))
		.IsFocusable(bIsFocusable)
		.CloseComboBoxOnSelectionChanged(bCloseComboBoxOnSelectionChanged)
		.SelectionMode(GetSelectionModeType(SelectionMode))
		.MaxNumSelectedItems(MaxNumberOfSelections)
		.CanSelectNone(bCanSelectNone)
		[
			SAssignNew(ComboBoxContent, SBox)
		];

	if ( InitialIndex != -1 )
	{
		// Generate the widget for the initially selected widget if needed
		UpdateOrGenerateWidget(CurrentOptionPtr);
	}

	return MyComboBox.ToSharedRef();
}

void UBSComboBoxString::AddOption(const FString& Option)
{
	Options.Add(MakeShareable(new FString(Option)));
	RefreshOptions();
}

bool UBSComboBoxString::RemoveOption(const FString& Option)
{
	const int32 OptionIndex = FindOptionIndex(Option);
	if ( OptionIndex != -1 )
	{
		if ( CurrentlySelectedOptionPointers.Contains(Options[OptionIndex]))
		{
			ClearSelection();
		}
		Options.RemoveAt(OptionIndex);
		RefreshOptions();
		return true;
	}
	return false;
}

int32 UBSComboBoxString::FindOptionIndex(const FString& Option) const
{
	for (int32 OptionIndex = 0; OptionIndex < Options.Num(); OptionIndex++)
	{
		if (Options.IsValidIndex(OptionIndex) && *Options[OptionIndex] == Option)
		{
			return OptionIndex;
		}
	}
	return -1;
}

FString UBSComboBoxString::FindOptionAtIndex(const int32 Index) const
{
	if (Options.IsValidIndex(Index))
	{
		return *Options[Index];
	}
	return FString();
}

int32 UBSComboBoxString::GetSelectedIndex() const
{
	if (GetSelectedOptionCount() > 0)
	{
		for (int32 OptionIndex = 0; OptionIndex < Options.Num(); ++OptionIndex)
		{
			if (CurrentlySelectedOptionPointers.IsValidIndex(OptionIndex))
			{
				return OptionIndex;
			}
		}
	}

	return -1;
}

FString UBSComboBoxString::GetSelectedOption() const
{
	return FindOptionAtIndex(GetSelectedIndex());
}

TArray<int32> UBSComboBoxString::GetSelectedIndices() const
{
	TArray<int32> ReturnArray = TArray<int32>();
	for (const TSharedPtr<FString>& OptionPointer : CurrentlySelectedOptionPointers)
	{
		if (OptionPointer.Get())
		{
			for (int32 OptionIndex = 0; OptionIndex < Options.Num(); ++OptionIndex)
			{
				if (Options[OptionIndex] == OptionPointer)
				{
					ReturnArray.AddUnique(OptionIndex);
				}
			}
		}
	}
	return ReturnArray;
}

TArray<FString> UBSComboBoxString::GetSelectedOptions() const
{
	TArray<FString> ReturnArray = TArray<FString>();
	for (const TSharedPtr<FString>& OptionPointer : CurrentlySelectedOptionPointers)
	{
		if (OptionPointer.Get())
		{
			ReturnArray.AddUnique(*OptionPointer);
		}
	}
	return ReturnArray;
}

int32 UBSComboBoxString::GetOptionCount() const
{
	return Options.Num();
}

int32 UBSComboBoxString::GetSelectedOptionCount() const
{
	return CurrentlySelectedOptionPointers.Num();
}

void UBSComboBoxString::SetSelectedIndex(const int32 InIndex)
{
	if (Options.IsValidIndex(InIndex))
	{
		if (ComboBoxContent)
		{
			if (!CurrentlySelectedOptionPointers.Contains(Options[InIndex]))
			{
				CurrentlySelectedOptionPointers.Add(Options[InIndex]);
			}
			
			ComboBoxContent->SetContent(HandleSelectionChangedGenerateWidget(CurrentlySelectedOptionPointers));
			
			if (MyComboBox)
			{
				MyComboBox->SetItemSelection(Options[InIndex], true);
			}
		}
		else
		{
			HandleSelectionChanged(Options, ESelectInfo::Direct);
		}
	}
}

void UBSComboBoxString::SetSelectedOption(const FString InOption)
{
	SetSelectedIndex(FindOptionIndex(InOption));
}

void UBSComboBoxString::SetSelectedIndices(const TArray<int32> InIndices)
{
	if (ComboBoxContent.IsValid())
	{
		// Refresh currently selected options
		CurrentlySelectedOptionPointers.Empty();
		for (const int32 Index : InIndices)
		{
			if (!Options.IsValidIndex(Index))
			{
				continue;
			}
			if (!CurrentlySelectedOptionPointers.Contains(Options[Index]))
			{
				CurrentlySelectedOptionPointers.Add(Options[Index]);
			}
		}
		ComboBoxContent->SetContent(HandleSelectionChangedGenerateWidget(CurrentlySelectedOptionPointers));
		if (MyComboBox)
		{
			MyComboBox->SetItemSelection(CurrentlySelectedOptionPointers, true);
		}
	}
	else
	{
		HandleSelectionChanged(Options, ESelectInfo::Direct);
	}
}

void UBSComboBoxString::SetSelectedOptions(TArray<FString> InOptions)
{
	TArray<int32> OptionIndices;
	for (const FString& String : InOptions)
	{
		if (const int32 Index = FindOptionIndex(String); Options.IsValidIndex(Index))
		{
			OptionIndices.AddUnique(Index);
		}
	}
	SetSelectedIndices(OptionIndices);
}

bool UBSComboBoxString::IsOpen() const
{
	return MyComboBox.IsValid() && MyComboBox->IsOpen();
}

void UBSComboBoxString::RefreshOptions()
{
	if (MyComboBox.IsValid())
	{
		MyComboBox->RefreshOptions();
	}
}

void UBSComboBoxString::ClearOptions()
{
	ClearSelection();
	Options.Empty();
	if ( MyComboBox.IsValid() )
	{
		MyComboBox->RefreshOptions();
	}
}

void UBSComboBoxString::ClearSelection()
{
	for (TSharedPtr<FString> String : CurrentlySelectedOptionPointers)
	{
		String.Reset();
	}
	if (MyComboBox.IsValid())
	{
		MyComboBox->ClearSelection();
	}

	if (ComboBoxContent.IsValid())
	{
		ComboBoxContent->SetContent(SNullWidget::NullWidget);
	}
}

#undef LOCTEXT_NAMESPACE