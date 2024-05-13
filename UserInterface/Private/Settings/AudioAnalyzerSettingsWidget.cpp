// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "Settings/AudioAnalyzerSettingsWidget.h"
#include "Components/VerticalBox.h"
#include "Utilities/ComboBox/BSComboBoxString.h"
#include "Utilities/Buttons/BSButton.h"
#include "Utilities/SavedTextWidget.h"
#include "MenuOptions/BandChannelWidget.h"
#include "MenuOptions/BandThresholdWidget.h"
#include "MenuOptions/ComboBoxWidget.h"
#include "MenuOptions/SingleRangeInputWidget.h"
#include "Overlays/PopupMessageWidget.h"

void UAudioAnalyzerSettingsWidget::InitPauseMenuChild()
{
	Button_SaveAndRestart->SetVisibility(ESlateVisibility::Visible);
}

void UAudioAnalyzerSettingsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	Button_Reset->OnBSButtonPressed.AddUObject(this, &ThisClass::OnButtonClicked_BSButton);
	Button_Save->OnBSButtonPressed.AddUObject(this, &ThisClass::OnButtonClicked_BSButton);
	Button_SaveAndRestart->OnBSButtonPressed.AddUObject(this, &ThisClass::OnButtonClicked_BSButton);

	Button_Reset->SetDefaults(static_cast<uint8>(ESettingButtonType::Reset));
	Button_Save->SetDefaults(static_cast<uint8>(ESettingButtonType::Save));
	Button_SaveAndRestart->SetDefaults(static_cast<uint8>(ESettingButtonType::SaveAndRestart));

	SliderTextBoxOption_TimeWindow->SetValues(0, 1.f, 0.01f);
	SliderTextBoxOption_TimeWindow->OnSliderTextBoxValueChanged.AddUObject(this,
		&ThisClass::OnSliderTextBoxValueChanged);

	ComboBoxOption_NumBandChannels->ComboBox->OnSelectionChanged.AddDynamic(this,
		&ThisClass::OnSelectionChanged_NumBandChannels);

	SavedTextWidget->SetSavedText(GetWidgetTextFromKey("SM_Saved_AudioAnalyzer"));

	Button_SaveAndRestart->SetVisibility(ESlateVisibility::Collapsed);

	AASettings = LoadPlayerSettings().AudioAnalyzer;
	NewAASettings = AASettings;
	const FPlayerSettings_AudioAnalyzer DefaultAASettings = FPlayerSettings_AudioAnalyzer();

	for (int i = 0; i < DefaultAASettings.MaxNumBandChannels; i++)
	{
		ComboBoxOption_NumBandChannels->ComboBox->AddOption(FString::FromInt(i + 1));
	}

	UBandChannelWidget* PreviousBandChannel = nullptr;
	UBandThresholdWidget* PreviousBandThreshold = nullptr;

	for (int i = 0; i < DefaultAASettings.MaxNumBandChannels; i++)
	{
		UBandChannelWidget* CurrentBandChannel = CreateWidget<UBandChannelWidget>(this, BandChannelWidgetClass);
		UBandThresholdWidget* CurrentBandThreshold = CreateWidget<UBandThresholdWidget>(this, BandThresholdWidgetClass);
		CurrentBandChannel->OnChannelValueCommitted.BindUFunction(this, "OnChannelValueCommitted");
		Box_BandChannelBounds->AddChild(Cast<UBandChannelWidget>(CurrentBandChannel));
		CurrentBandThreshold->OnThresholdValueCommitted.BindUFunction(this, "OnBandThresholdChanged");
		Box_BandThresholdBounds->AddChild(Cast<UBandThresholdWidget>(CurrentBandThreshold));

		if (i == 0)
		{
			BandChannelWidget = CurrentBandChannel;
			BandThresholdWidget = CurrentBandThreshold;
			PreviousBandChannel = BandChannelWidget;
			PreviousBandThreshold = BandThresholdWidget;
		}
		else
		{
			PreviousBandChannel->Next = CurrentBandChannel;
			PreviousBandThreshold->Next = CurrentBandThreshold;
			PreviousBandChannel = CurrentBandChannel;
			PreviousBandThreshold = CurrentBandThreshold;
		}
	}
	PopulateAASettings();
}

void UAudioAnalyzerSettingsWidget::OnChannelValueCommitted(const UBandChannelWidget* BandChannel, const int32 Index,
	const float NewValue, const bool bIsMinValue)
{
	if (bIsMinValue)
	{
		NewAASettings.BandLimits[Index].X = NewValue;
	}
	else
	{
		NewAASettings.BandLimits[Index].Y = NewValue;
	}
}

void UAudioAnalyzerSettingsWidget::OnBandThresholdChanged(const UBandThresholdWidget* BandThreshold, const int32 Index,
	const float NewValue)
{
	NewAASettings.BandLimitsThreshold[Index] = NewValue;
}

void UAudioAnalyzerSettingsWidget::OnSliderTextBoxValueChanged(USingleRangeInputWidget* Widget, const float Value)
{
	if (Widget == SliderTextBoxOption_TimeWindow)
	{
		NewAASettings.TimeWindow = Value;
	}
}

void UAudioAnalyzerSettingsWidget::OnSelectionChanged_NumBandChannels(const TArray<FString>& SelectedOptions,
	ESelectInfo::Type SelectionType)
{
	if (SelectionType == ESelectInfo::Type::Direct)
	{
		return;
	}

	NewAASettings.NumBandChannels = ComboBoxOption_NumBandChannels->ComboBox->GetSelectedIndex() + 1;
	FPlayerSettings_AudioAnalyzer DefaultAASettings = FPlayerSettings_AudioAnalyzer();
	if (const int ElementsToAdd = NewAASettings.NumBandChannels - NewAASettings.BandLimits.Num(); ElementsToAdd > 0)
	{
		for (int i = 0; i < ElementsToAdd; i++)
		{
			NewAASettings.BandLimitsThreshold.Emplace(2.1);
			NewAASettings.BandLimits.Emplace(FVector2d::ZeroVector);
		}
	}
	else
	{
		NewAASettings.BandLimits.SetNum(NewAASettings.NumBandChannels, true);
		NewAASettings.BandLimitsThreshold.SetNum(NewAASettings.NumBandChannels, true);
	}
	PopulateAASettings();
}

void UAudioAnalyzerSettingsWidget::OnButtonClicked_BSButton(const UBSButton* Button)
{
	switch (static_cast<ESettingButtonType>(Button->GetEnumValue()))
	{
	case ESettingButtonType::Save:
		SortAndCheckOverlap();
		break;
	case ESettingButtonType::Reset:
		ResetAASettings();
		break;
	case ESettingButtonType::SaveAndRestart:
		SortAndCheckOverlap();
		if (!OnRestartButtonClicked.ExecuteIfBound())
		{
			UE_LOG(LogTemp, Display, TEXT("OnRestartButtonClicked not bound."));
		}
		break;
	default:
		break;
	}
}

void UAudioAnalyzerSettingsWidget::PopulateAASettings()
{
	ComboBoxOption_NumBandChannels->ComboBox->SetSelectedIndex(NewAASettings.NumBandChannels - 1);
	BandChannelWidget->SetDefaultValues(NewAASettings.BandLimits[0], 0);
	BandThresholdWidget->SetDefaultValue(NewAASettings.BandLimitsThreshold[0], 0);

	UBandChannelWidget* PreviousBandChannel = nullptr;
	UBandThresholdWidget* PreviousBandThreshold = nullptr;
	FPlayerSettings_AudioAnalyzer DefaultAASettings = FPlayerSettings_AudioAnalyzer();
	for (int i = 0; i < DefaultAASettings.MaxNumBandChannels; i++)
	{
		if (i == 0)
		{
			PreviousBandChannel = BandChannelWidget;
			PreviousBandThreshold = BandThresholdWidget;
		}

		if (NewAASettings.NumBandChannels > i)
		{
			PreviousBandChannel->SetDefaultValues(NewAASettings.BandLimits[i], i);
			PreviousBandThreshold->SetDefaultValue(NewAASettings.BandLimitsThreshold[i], i);
			PreviousBandChannel->SetVisibility(ESlateVisibility::Visible);
			PreviousBandThreshold->SetVisibility(ESlateVisibility::Visible);
		}
		else if (DefaultAASettings.NumBandChannels > i)
		{
			PreviousBandChannel->SetDefaultValues(DefaultAASettings.BandLimits[i], i);
			PreviousBandThreshold->SetDefaultValue(DefaultAASettings.BandLimitsThreshold[i], i);
			PreviousBandChannel->SetVisibility(ESlateVisibility::Collapsed);
			PreviousBandThreshold->SetVisibility(ESlateVisibility::Collapsed);
		}
		else
		{
			PreviousBandChannel->SetDefaultValues(FVector2d(0, 0), i);
			PreviousBandThreshold->SetDefaultValue(DefaultAASettings.BandLimitsThreshold[0], i);
			PreviousBandChannel->SetVisibility(ESlateVisibility::Collapsed);
			PreviousBandThreshold->SetVisibility(ESlateVisibility::Collapsed);
		}
		PreviousBandChannel = PreviousBandChannel->Next;
		PreviousBandThreshold = PreviousBandThreshold->Next;
	}

	SliderTextBoxOption_TimeWindow->SetValue(NewAASettings.TimeWindow);
	UpdateBrushColors();
}

void UAudioAnalyzerSettingsWidget::ResetAASettings()
{
	NewAASettings.ResetToDefault();
	UBandChannelWidget* CurrentBandChannel = BandChannelWidget;
	UBandThresholdWidget* CurrentBandThreshold = BandThresholdWidget;
	CurrentBandChannel->SetDefaultValues(NewAASettings.BandLimits[0], 0);
	CurrentBandThreshold->SetDefaultValue(NewAASettings.BandLimitsThreshold[0], 0);
	FPlayerSettings_AudioAnalyzer DefaultAASettings = FPlayerSettings_AudioAnalyzer();
	for (int i = 1; i < DefaultAASettings.MaxNumBandChannels; i++)
	{
		if (DefaultAASettings.NumBandChannels > i)
		{
			CurrentBandChannel->Next->SetDefaultValues(DefaultAASettings.BandLimits[i], i);
			CurrentBandThreshold->Next->SetDefaultValue(DefaultAASettings.BandLimitsThreshold[i], i);
		}
		else
		{
			CurrentBandChannel->Next->SetDefaultValues(FVector2d(0, 0), i);
			CurrentBandThreshold->Next->SetDefaultValue(DefaultAASettings.BandLimitsThreshold[0], i);
		}
		CurrentBandChannel = CurrentBandChannel->Next;
		CurrentBandThreshold = CurrentBandThreshold->Next;
	}
	PopulateAASettings();
}

void UAudioAnalyzerSettingsWidget::SortAndCheckOverlap()
{
	NewAASettings.BandLimits.RemoveAll([this](const FVector2d& BandLimit)
	{
		if (BandLimit == FVector2d(0, 0))
		{
			return true;
		}
		return false;
	});
	NewAASettings.BandLimits.Sort([](const FVector2d& A, const FVector2d& B)
	{
		if (A.X < B.X)
		{
			return true;
		}
		if (A.X == B.X && A.Y < B.Y)
		{
			return true;
		}
		return false;
	});

	NewAASettings.NumBandChannels = NewAASettings.BandLimits.Num();
	NewAASettings.BandLimitsThreshold.SetNum(NewAASettings.BandLimits.Num());
	PopulateAASettings();

	double LastEndThreshold = NewAASettings.BandLimits[0].Y;
	for (int i = 0; i < NewAASettings.BandLimits.Num(); i++)
	{
		if (NewAASettings.BandLimits[i].X >= NewAASettings.BandLimits[i].Y)
		{
			ShowBandLimitErrorMessage();
			return;
		}
		if (i == 0)
		{
			continue;
		}
		if (NewAASettings.BandLimits[i].X <= LastEndThreshold)
		{
			ShowBandLimitErrorMessage();
			return;
		}
		LastEndThreshold = NewAASettings.BandLimits[i].Y;
	}

	SavePlayerSettings(NewAASettings);
	SavedTextWidget->PlayFadeInFadeOut();
}

void UAudioAnalyzerSettingsWidget::ShowBandLimitErrorMessage()
{
	PopupMessageWidget = CreateWidget<UPopupMessageWidget>(GetWorld(), PopupMessageClass);
	TArray<UBSButton*> Buttons = PopupMessageWidget->InitPopup(GetWidgetTextFromKey("ASW_SongPathErrorTitle"),
		GetWidgetTextFromKey("AA_BandLimitThresholdError"), 1);

	if (Buttons[0])
	{
		Buttons[0]->SetButtonText(GetWidgetTextFromKey("ASW_SongPathErrorButton"));
		Buttons[0]->OnBSButtonPressed.AddLambda([this](const UBSButton* /*Button*/)
		{
			PopupMessageWidget->FadeOut();
		});
	}

	PopupMessageWidget->AddToViewport();
	PopupMessageWidget->FadeIn();
}
