// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

// ReSharper disable CppMemberFunctionMayBeConst
#include "SubMenuWidgets/SettingsWidgets/SettingsMenuWidget_VideoAndSound.h"

#include "BSGameUserSettings.h"
#include "WidgetComponents/Buttons/BSButton.h"
#include "WidgetComponents/Boxes/BSComboBoxString.h"
#include "WidgetComponents/SavedTextWidget.h"
#include "WidgetComponents/MenuOptionWidgets/CheckBoxOptionWidget.h"
#include "WidgetComponents/MenuOptionWidgets/ComboBoxOptionWidget.h"
#include "WidgetComponents/MenuOptionWidgets/EditableTextBoxOptionWidget.h"
#include "WidgetComponents/MenuOptionWidgets/SliderTextBoxOptionWidget.h"
#include "WidgetComponents/MenuOptionWidgets/VideoSettingOptionWidget.h"
#include "BSWidgetInterface.h"
#include "StreamlineLibraryDLSSG.h"
#include "StreamlineLibraryReflex.h"
#include "Components/CheckBox.h"
#include "Components/EditableTextBox.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetStringLibrary.h"
#include "OverlayWidgets/PopupWidgets/PopupMessageWidget.h"


ENUM_RANGE_BY_FIRST_AND_LAST(UDLSSSupport, UDLSSSupport::Supported,
	UDLSSSupport::NotSupportedIncompatibleAPICaptureToolActive);

ENUM_RANGE_BY_FIRST_AND_LAST(UDLSSMode, UDLSSMode::Off, UDLSSMode::UltraPerformance);

ENUM_RANGE_BY_FIRST_AND_LAST(UNISMode, UNISMode::Off, UNISMode::Custom);

ENUM_RANGE_BY_FIRST_AND_LAST(UStreamlineReflexMode, UStreamlineReflexMode::Disabled,
	UStreamlineReflexMode::EnabledPlusBoost);

ENUM_RANGE_BY_FIRST_AND_LAST(UStreamlineDLSSGMode, UStreamlineDLSSGMode::Off, UStreamlineDLSSGMode::On);

ENUM_RANGE_BY_FIRST_AND_LAST(EWindowMode::Type, EWindowMode::Type::Fullscreen, EWindowMode::Type::WindowedFullscreen);

using namespace Constants;

void USettingsMenuWidget_VideoAndSound::NativeConstruct()
{
	Super::NativeConstruct();

	VideoSettingOptionWidget_AA->Init(EVideoSettingType::AntiAliasing);
	VideoSettingOptionWidget_GI->Init(EVideoSettingType::GlobalIllumination);
	VideoSettingOptionWidget_PP->Init(EVideoSettingType::PostProcessing);
	VideoSettingOptionWidget_RQ->Init(EVideoSettingType::Reflection);
	VideoSettingOptionWidget_TQ->Init(EVideoSettingType::Texture);
	VideoSettingOptionWidget_SGQ->Init(EVideoSettingType::Shading);
	VideoSettingOptionWidget_SWQ->Init(EVideoSettingType::Shadow);
	VideoSettingOptionWidget_VD->Init(EVideoSettingType::ViewDistance);
	VideoSettingOptionWidget_VEQ->Init(EVideoSettingType::VisualEffect);

	SliderTextBoxOption_GlobalSound->SetValues(MinValue_Volume, MaxValue_Volume, SnapSize_Volume);
	SliderTextBoxOption_MenuSound->SetValues(MinValue_Volume, MaxValue_Volume, SnapSize_Volume);
	SliderTextBoxOption_MusicSound->SetValues(MinValue_Volume, MaxValue_Volume, SnapSize_Volume);
	SliderTextBoxOption_DLSS_Sharpness->SetValues(MinValue_DLSSSharpness, MaxValue_DLSSSharpness,
		SnapSize_DLSSSharpness);
	SliderTextBoxOption_NIS_Sharpness->SetValues(MinValue_NISSharpness, MaxValue_NISSharpness, SnapSize_NISSharpness);
	SliderTextBoxOption_ResolutionScale->SetValues(0.f, 1.f, 0.001f);
	SliderTextBoxOption_HDRNits->SetValues(1000.f, 2000.f, 1.f);
	SliderTextBoxOption_Brightness->SetValues(MinValue_Brightness, MaxValue_Brightness, SnapSize_Brightness);

	VideoSettingOptionWidget_AA->OnVideoSettingQualityButtonPressed.AddUObject(this,
		&ThisClass::OnVideoSettingOptionWidget_ButtonPressed);
	VideoSettingOptionWidget_GI->OnVideoSettingQualityButtonPressed.AddUObject(this,
		&ThisClass::OnVideoSettingOptionWidget_ButtonPressed);
	VideoSettingOptionWidget_PP->OnVideoSettingQualityButtonPressed.AddUObject(this,
		&ThisClass::OnVideoSettingOptionWidget_ButtonPressed);
	VideoSettingOptionWidget_RQ->OnVideoSettingQualityButtonPressed.AddUObject(this,
		&ThisClass::OnVideoSettingOptionWidget_ButtonPressed);
	VideoSettingOptionWidget_TQ->OnVideoSettingQualityButtonPressed.AddUObject(this,
		&ThisClass::OnVideoSettingOptionWidget_ButtonPressed);
	VideoSettingOptionWidget_SGQ->OnVideoSettingQualityButtonPressed.AddUObject(this,
		&ThisClass::OnVideoSettingOptionWidget_ButtonPressed);
	VideoSettingOptionWidget_SWQ->OnVideoSettingQualityButtonPressed.AddUObject(this,
		&ThisClass::OnVideoSettingOptionWidget_ButtonPressed);
	VideoSettingOptionWidget_VD->OnVideoSettingQualityButtonPressed.AddUObject(this,
		&ThisClass::OnVideoSettingOptionWidget_ButtonPressed);
	VideoSettingOptionWidget_VEQ->OnVideoSettingQualityButtonPressed.AddUObject(this,
		&ThisClass::OnVideoSettingOptionWidget_ButtonPressed);

	SliderTextBoxOption_GlobalSound->OnSliderTextBoxValueChanged.AddUObject(this,
		&ThisClass::OnSliderTextBoxValueChanged);
	SliderTextBoxOption_MenuSound->OnSliderTextBoxValueChanged.
	                               AddUObject(this, &ThisClass::OnSliderTextBoxValueChanged);
	SliderTextBoxOption_MusicSound->OnSliderTextBoxValueChanged.AddUObject(this,
		&ThisClass::OnSliderTextBoxValueChanged);
	SliderTextBoxOption_DLSS_Sharpness->OnSliderTextBoxValueChanged.AddUObject(this,
		&ThisClass::OnSliderTextBoxValueChanged);
	SliderTextBoxOption_NIS_Sharpness->OnSliderTextBoxValueChanged.AddUObject(this,
		&ThisClass::OnSliderTextBoxValueChanged);
	SliderTextBoxOption_ResolutionScale->OnSliderTextBoxValueChanged.AddUObject(this,
		&ThisClass::OnSliderTextBoxValueChanged);
	SliderTextBoxOption_HDRNits->OnSliderTextBoxValueChanged.AddUObject(this, &ThisClass::OnSliderTextBoxValueChanged);
	SliderTextBoxOption_Brightness->OnSliderTextBoxValueChanged.AddUObject(this,
		&ThisClass::OnSliderTextBoxValueChanged);

	ComboBoxOption_WindowMode->ComboBox->OnSelectionChanged.AddDynamic(this, &ThisClass::OnSelectionChanged_WindowMode);
	ComboBoxOption_Resolution->ComboBox->OnSelectionChanged.AddDynamic(this, &ThisClass::OnSelectionChanged_Resolution);
	ComboBoxOption_DLSS->ComboBox->OnSelectionChanged.AddDynamic(this, &ThisClass::OnSelectionChanged_DLSS_EnabledMode);
	ComboBoxOption_DLSS_FrameGeneration->ComboBox->OnSelectionChanged.AddDynamic(this,
		&ThisClass::OnSelectionChanged_FrameGeneration);
	ComboBoxOption_DLSS_SuperResolution->ComboBox->OnSelectionChanged.AddDynamic(this,
		&ThisClass::OnSelectionChanged_SuperResolution);
	ComboBoxOption_NIS->ComboBox->OnSelectionChanged.AddDynamic(this, &ThisClass::OnSelectionChanged_NIS_EnabledMode);
	ComboBoxOption_NIS_Mode->ComboBox->OnSelectionChanged.AddDynamic(this, &ThisClass::OnSelectionChanged_NIS_Mode);
	ComboBoxOption_Reflex->ComboBox->OnSelectionChanged.AddDynamic(this, &ThisClass::OnSelectionChanged_Reflex);
	ComboBoxOption_AntiAliasingMethod->ComboBox->OnSelectionChanged.AddDynamic(this,
		&ThisClass::OnSelectionChanged_AntiAliasingMethod);

	ComboBoxOption_WindowMode->ComboBox->OnGenerateWidgetEventDelegate.BindDynamic(this,
		&ThisClass::OnGenerateWidgetEvent);
	ComboBoxOption_Resolution->ComboBox->OnGenerateWidgetEventDelegate.BindDynamic(this,
		&ThisClass::OnGenerateWidgetEvent);
	ComboBoxOption_DLSS->ComboBox->OnGenerateWidgetEventDelegate.BindDynamic(this, &ThisClass::OnGenerateWidgetEvent);
	ComboBoxOption_DLSS_FrameGeneration->ComboBox->OnGenerateWidgetEventDelegate.BindDynamic(this,
		&ThisClass::OnGenerateWidgetEvent);
	ComboBoxOption_DLSS_SuperResolution->ComboBox->OnGenerateWidgetEventDelegate.BindDynamic(this,
		&ThisClass::OnGenerateWidgetEvent);
	ComboBoxOption_NIS->ComboBox->OnGenerateWidgetEventDelegate.BindDynamic(this, &ThisClass::OnGenerateWidgetEvent);
	ComboBoxOption_NIS_Mode->ComboBox->OnGenerateWidgetEventDelegate.BindDynamic(this,
		&ThisClass::OnGenerateWidgetEvent);
	ComboBoxOption_Reflex->ComboBox->OnGenerateWidgetEventDelegate.BindDynamic(this, &ThisClass::OnGenerateWidgetEvent);
	ComboBoxOption_AntiAliasingMethod->ComboBox->OnGenerateWidgetEventDelegate.BindDynamic(this,
		&ThisClass::OnGenerateWidgetEvent);

	ComboBoxOption_WindowMode->ComboBox->OnSelectionChanged_GenerateWidgetForMultiSelection.BindDynamic(this,
		&ThisClass::OnSelectionChanged_GenerateMultiSelectionItem);
	ComboBoxOption_Resolution->ComboBox->OnSelectionChanged_GenerateWidgetForMultiSelection.BindDynamic(this,
		&ThisClass::OnSelectionChanged_GenerateMultiSelectionItem);
	ComboBoxOption_DLSS->ComboBox->OnSelectionChanged_GenerateWidgetForMultiSelection.BindDynamic(this,
		&ThisClass::OnSelectionChanged_GenerateMultiSelectionItem);
	ComboBoxOption_DLSS_FrameGeneration->ComboBox->OnSelectionChanged_GenerateWidgetForMultiSelection.BindDynamic(this,
		&ThisClass::OnSelectionChanged_GenerateMultiSelectionItem);
	ComboBoxOption_DLSS_SuperResolution->ComboBox->OnSelectionChanged_GenerateWidgetForMultiSelection.BindDynamic(this,
		&ThisClass::OnSelectionChanged_GenerateMultiSelectionItem);
	ComboBoxOption_NIS->ComboBox->OnSelectionChanged_GenerateWidgetForMultiSelection.BindDynamic(this,
		&ThisClass::OnSelectionChanged_GenerateMultiSelectionItem);
	ComboBoxOption_NIS_Mode->ComboBox->OnSelectionChanged_GenerateWidgetForMultiSelection.BindDynamic(this,
		&ThisClass::OnSelectionChanged_GenerateMultiSelectionItem);
	ComboBoxOption_Reflex->ComboBox->OnSelectionChanged_GenerateWidgetForMultiSelection.BindDynamic(this,
		&ThisClass::OnSelectionChanged_GenerateMultiSelectionItem);
	ComboBoxOption_AntiAliasingMethod->ComboBox->OnSelectionChanged_GenerateWidgetForMultiSelection.BindDynamic(this,
		&ThisClass::OnSelectionChanged_GenerateMultiSelectionItem);

	SetupTooltip(ComboBoxOption_DLSS->GetTooltipImage(), ComboBoxOption_DLSS->GetTooltipImageText());
	SetupTooltip(ComboBoxOption_DLSS_FrameGeneration->GetTooltipImage(),
		ComboBoxOption_DLSS_FrameGeneration->GetTooltipImageText());
	SetupTooltip(ComboBoxOption_DLSS_SuperResolution->GetTooltipImage(),
		ComboBoxOption_DLSS_SuperResolution->GetTooltipImageText());
	SetupTooltip(ComboBoxOption_NIS->GetTooltipImage(), ComboBoxOption_NIS->GetTooltipImageText());
	SetupTooltip(ComboBoxOption_Reflex->GetTooltipImage(), ComboBoxOption_Reflex->GetTooltipImageText());
	SetupTooltip(SliderTextBoxOption_Brightness->GetTooltipImage(),
		SliderTextBoxOption_Brightness->GetTooltipImageText());
	SetupTooltip(CheckBoxOption_HDREnabled->GetTooltipImage(), CheckBoxOption_HDREnabled->GetTooltipImageText());
	SetupTooltip(SliderTextBoxOption_ResolutionScale->GetTooltipImage(),
		SliderTextBoxOption_ResolutionScale->GetTooltipImageText());

	CheckBoxOption_HDREnabled->CheckBox->OnCheckStateChanged.AddDynamic(this,
		&ThisClass::OnCheckStateChanged_HDREnabled);
	CheckBoxOption_FPSCounter->CheckBox->OnCheckStateChanged.AddDynamic(this,
		&ThisClass::OnCheckStateChanged_FPSCounter);
	CheckBoxOption_VSync->CheckBox->OnCheckStateChanged.AddDynamic(this, &ThisClass::OnCheckStateChanged_VSyncEnabled);

	EditableTextBoxOption_FPSLimitMenu->EditableTextBox->OnTextCommitted.AddDynamic(this,
		&ThisClass::OnTextCommitted_FPSLimitMenu);
	EditableTextBoxOption_FPSLimitGame->EditableTextBox->OnTextCommitted.AddDynamic(this,
		&ThisClass::OnTextCommitted_FPSLimitGame);
	EditableTextBoxOption_FPSLimitBackground->EditableTextBox->OnTextCommitted.AddDynamic(this,
		&ThisClass::OnTextCommitted_FPSLimitBackground);
	EditableTextBoxOption_FPSLimitGame->EditableTextBox->SetJustification(ETextJustify::Type::Center);
	EditableTextBoxOption_FPSLimitMenu->EditableTextBox->SetJustification(ETextJustify::Type::Center);
	EditableTextBoxOption_FPSLimitBackground->EditableTextBox->SetJustification(ETextJustify::Type::Center);

	Button_Reset->OnBSButtonPressed.AddDynamic(this, &ThisClass::OnBSButtonPressed_SaveReset);
	Button_Save->OnBSButtonPressed.AddDynamic(this, &ThisClass::OnBSButtonPressed_SaveReset);

	Button_Reset->SetDefaults(static_cast<uint8>(ESettingButtonType::Reset));
	Button_Save->SetDefaults(static_cast<uint8>(ESettingButtonType::Save));
	TArray<FString> Options;

	// Window Modes
	WindowModeMap.Add("Fullscreen", EWindowMode::Type::Fullscreen);
	WindowModeMap.Add("Windowed", EWindowMode::Type::Windowed);
	WindowModeMap.Add("Windowed Fullscreen", EWindowMode::Type::WindowedFullscreen);
	WindowModeMap.GetKeys(Options);
	ComboBoxOption_WindowMode->SortAndAddOptions(Options);
	Options.Empty();

	UBSGameUserSettings* GameUserSettings = UBSGameUserSettings::Get();

	// DLSS On/Off
	Options.Add(GetStringFromEnum(EDLSSEnabledMode::Off));
	if (GameUserSettings->IsDLSSSupported())
	{
		Options.Add(GetStringFromEnum(EDLSSEnabledMode::On));
	}
	ComboBoxOption_DLSS->SortAndAddOptions(Options);
	Options.Empty();

	// Frame Generation
	for (const UStreamlineDLSSGMode Mode : UStreamlineLibraryDLSSG::GetSupportedDLSSGModes())
	{
		Options.Add(GetStringFromEnum(Mode));
	}
	ComboBoxOption_DLSS_FrameGeneration->SortAndAddOptions(Options);
	Options.Empty();

	// Super Resolution (DLSS Modes)
	Options.Add(GetStringFromEnum(UDLSSMode::Off));
	Options.Add(GetStringFromEnum(UDLSSMode::Auto));
	for (const UDLSSMode Mode : UDLSSLibrary::GetSupportedDLSSModes())
	{
		if (Mode != UDLSSMode::Off && Mode != UDLSSMode::Auto)
		{
			Options.Add(GetStringFromEnum(Mode));
		}
	}
	ComboBoxOption_DLSS_SuperResolution->SortAndAddOptions(Options);
	// Make DLAA the last option
	if (ComboBoxOption_DLSS_SuperResolution->ComboBox->GetIndexOfOption(GetStringFromEnum(UDLSSMode::DLAA)) !=
		INDEX_NONE)
	{
		ComboBoxOption_DLSS_SuperResolution->ComboBox->RemoveOption(GetStringFromEnum(UDLSSMode::DLAA));
		ComboBoxOption_DLSS_SuperResolution->ComboBox->AddOption(GetStringFromEnum(UDLSSMode::DLAA));
	}
	Options.Empty();

	// NIS On/Off
	Options.Add(GetStringFromEnum(ENISEnabledMode::Off));
	if (GameUserSettings->IsNISSupported())
	{
		Options.Add(GetStringFromEnum(ENISEnabledMode::On));
	}
	ComboBoxOption_NIS->SortAndAddOptions(Options);
	Options.Empty();

	// NIS Modes
	for (const UNISMode Mode : UNISLibrary::GetSupportedNISModes())
	{
		if (Mode != UNISMode::Custom)
		{
			Options.Add(GetStringFromEnum(Mode));
		}
	}
	ComboBoxOption_NIS_Mode->SortAndAddOptions(Options);
	Options.Empty();

	// Reflex
	Options.Add(GetStringFromEnum(UStreamlineReflexMode::Disabled));
	if (UStreamlineLibraryReflex::IsReflexSupported())
	{
		Options.Add(GetStringFromEnum(UStreamlineReflexMode::Enabled));
		Options.Add(GetStringFromEnum(UStreamlineReflexMode::EnabledPlusBoost));
	}
	ComboBoxOption_Reflex->SortAndAddOptions(Options);
	Options.Empty();

	AntiAliasingMethodMap.Add("None", AAM_None);
	AntiAliasingMethodMap.Add("Fast Approximate Anti-Aliasing (FXAA)", AAM_FXAA);
	AntiAliasingMethodMap.Add("Temporal Anti-Aliasing (TAA)", AAM_TemporalAA);
	AntiAliasingMethodMap.Add("Temporal Super-Resolution (TSR)", AAM_TSR);
	AntiAliasingMethodMap.GetKeys(Options);
	ComboBoxOption_AntiAliasingMethod->SortAndAddOptions(Options);
	Options.Empty();

	UpdateBrushColors();
	InitializeVideoAndSoundSettings();
}

void USettingsMenuWidget_VideoAndSound::InitializeVideoAndSoundSettings()
{
	UBSGameUserSettings* GameUserSettings = UBSGameUserSettings::Get();
	float CurrentScaleNormalized;
	float CurrentScale;
	float MinScale;
	float MaxScale;

	GameUserSettings->GetResolutionScaleInformationEx(CurrentScaleNormalized, CurrentScale, MinScale, MaxScale);
	SliderTextBoxOption_ResolutionScale->SetValues(MinScale / 100.f, MaxScale / 100.f, 0.001f);
	SliderTextBoxOption_GlobalSound->SetValue(GameUserSettings->GetOverallVolume());
	SliderTextBoxOption_MenuSound->SetValue(GameUserSettings->GetMenuVolume());
	SliderTextBoxOption_MusicSound->SetValue(GameUserSettings->GetMusicVolume());
	SliderTextBoxOption_DLSS_Sharpness->SetValue(GameUserSettings->GetDLSSSharpness());
	SliderTextBoxOption_NIS_Sharpness->SetValue(GameUserSettings->GetNISSharpness());
	SliderTextBoxOption_ResolutionScale->SetValue(CurrentScale / 100.f);

	EditableTextBoxOption_FPSLimitMenu->EditableTextBox->SetText(
		FText::AsNumber(GameUserSettings->GetFrameRateLimitMenu()));
	EditableTextBoxOption_FPSLimitGame->EditableTextBox->SetText(
		FText::AsNumber(GameUserSettings->GetFrameRateLimitGame()));
	EditableTextBoxOption_FPSLimitBackground->EditableTextBox->SetText(
		FText::AsNumber(GameUserSettings->GetFrameRateLimitBackground()));

	CheckBoxOption_FPSCounter->CheckBox->SetIsChecked(GameUserSettings->GetShowFPSCounter());

	VideoSettingOptionWidget_AA->SetActiveButton(GameUserSettings->GetAntiAliasingQuality());
	VideoSettingOptionWidget_GI->SetActiveButton(GameUserSettings->GetGlobalIlluminationQuality());
	VideoSettingOptionWidget_PP->SetActiveButton(GameUserSettings->GetPostProcessingQuality());
	VideoSettingOptionWidget_RQ->SetActiveButton(GameUserSettings->GetReflectionQuality());
	VideoSettingOptionWidget_TQ->SetActiveButton(GameUserSettings->GetTextureQuality());
	VideoSettingOptionWidget_SGQ->SetActiveButton(GameUserSettings->GetShadingQuality());
	VideoSettingOptionWidget_SWQ->SetActiveButton(GameUserSettings->GetShadowQuality());
	VideoSettingOptionWidget_VD->SetActiveButton(GameUserSettings->GetViewDistanceQuality());
	VideoSettingOptionWidget_VEQ->SetActiveButton(GameUserSettings->GetVisualEffectQuality());

	const int32 AntiAliasingValue = GameUserSettings->GetAntiAliasingMethod();
	for (const TPair<FString, EAntiAliasingMethod>& Pair : AntiAliasingMethodMap)
	{
		if (static_cast<int32>(Pair.Value) == AntiAliasingValue)
		{
			ComboBoxOption_AntiAliasingMethod->ComboBox->SetSelectedOption(Pair.Key);
			break;
		}
	}

	const EWindowMode::Type WindowMode = GameUserSettings->GetFullscreenMode();
	for (const TPair<FString, EWindowMode::Type>& Pair : WindowModeMap)
	{
		if (Pair.Value == WindowMode)
		{
			ComboBoxOption_WindowMode->ComboBox->SetSelectedOption(Pair.Key);
			break;
		}
	}

	PopulateResolutionComboBox();

	ComboBoxOption_DLSS->ComboBox->SetSelectedOption(GetStringFromEnum(GameUserSettings->GetDLSSEnabledMode()));
	ComboBoxOption_DLSS_FrameGeneration->ComboBox->SetSelectedOption(
		GetStringFromEnum(GameUserSettings->GetFrameGenerationEnabledMode()));
	ComboBoxOption_DLSS_SuperResolution->ComboBox->SetSelectedOption(
		GetStringFromEnum(GameUserSettings->GetDLSSMode()));
	ComboBoxOption_NIS->ComboBox->SetSelectedOption(GetStringFromEnum(GameUserSettings->GetNISEnabledMode()));
	ComboBoxOption_NIS_Mode->ComboBox->SetSelectedOption(GetStringFromEnum(GameUserSettings->GetNISMode()));
	ComboBoxOption_Reflex->ComboBox->SetSelectedOption(GetStringFromEnum(GameUserSettings->GetStreamlineReflexMode()));

	const bool bSupportsHDR = GameUserSettings->SupportsHDRDisplayOutput();
	const bool bHDREnabled = GameUserSettings->IsHDREnabled();

	CheckBoxOption_HDREnabled->CheckBox->SetIsChecked(bSupportsHDR && bHDREnabled);
	SliderTextBoxOption_HDRNits->SetValue(GameUserSettings->GetCurrentHDRDisplayNits());

	CheckBoxOption_HDREnabled->CheckBox->SetIsEnabled(bSupportsHDR);
	SliderTextBoxOption_HDRNits->SetSliderAndTextBoxEnabledStates(bSupportsHDR && bHDREnabled);

	SliderTextBoxOption_Brightness->SetValue(GameUserSettings->GetDisplayGamma());
	HandleDLSSEnabledChanged(GameUserSettings->GetDLSSEnabledMode());
}

FPlayerSettings_VideoAndSound USettingsMenuWidget_VideoAndSound::GetVideoAndSoundSettings() const
{
	FPlayerSettings_VideoAndSound ReturnSettings;

	ReturnSettings.GlobalVolume = SliderTextBoxOption_GlobalSound->GetSliderValueSnapped();
	ReturnSettings.MenuVolume = SliderTextBoxOption_MenuSound->GetSliderValueSnapped();
	ReturnSettings.MusicVolume = SliderTextBoxOption_MusicSound->GetSliderValueSnapped();
	ReturnSettings.DLSSEnabledMode = GetEnumFromString<EDLSSEnabledMode>(
		ComboBoxOption_DLSS->ComboBox->GetSelectedOption());
	ReturnSettings.FrameGenerationEnabledMode = GetEnumFromString<UStreamlineDLSSGMode>(
		ComboBoxOption_DLSS_FrameGeneration->ComboBox->GetSelectedOption());
	ReturnSettings.DLSSMode = GetEnumFromString<UDLSSMode>(
		ComboBoxOption_DLSS_SuperResolution->ComboBox->GetSelectedOption());
	ReturnSettings.DLSSSharpness = SliderTextBoxOption_DLSS_Sharpness->GetSliderValueSnapped();
	ReturnSettings.NISEnabledMode = GetEnumFromString<ENISEnabledMode>(
		ComboBoxOption_NIS->ComboBox->GetSelectedOption());
	ReturnSettings.NISMode = GetEnumFromString<UNISMode>(ComboBoxOption_NIS_Mode->ComboBox->GetSelectedOption());
	ReturnSettings.NISSharpness = SliderTextBoxOption_NIS_Sharpness->GetSliderValueSnapped();
	ReturnSettings.StreamlineReflexMode = GetEnumFromString<UStreamlineReflexMode>(
		ComboBoxOption_Reflex->ComboBox->GetSelectedOption());
	ReturnSettings.bShowFPSCounter = CheckBoxOption_FPSCounter->CheckBox->IsChecked();
	ReturnSettings.FrameRateLimitGame = FMath::GridSnap<int32>(
		FMath::Clamp(FCString::Atof(*EditableTextBoxOption_FPSLimitGame->EditableTextBox->GetText().ToString()),
			MinValue_FrameRateLimit, MaxValue_FrameRateLimit), SnapSize_FrameRateLimit);
	ReturnSettings.FrameRateLimitMenu = FMath::GridSnap<int32>(
		FMath::Clamp(FCString::Atof(*EditableTextBoxOption_FPSLimitMenu->EditableTextBox->GetText().ToString()),
			MinValue_FrameRateLimit, MaxValue_FrameRateLimit), SnapSize_FrameRateLimit);
	ReturnSettings.Brightness = SliderTextBoxOption_Brightness->GetSliderValueSnapped();
	return ReturnSettings;
}

void USettingsMenuWidget_VideoAndSound::OnCheckStateChanged_VSyncEnabled(const bool bIsChecked)
{
	UBSGameUserSettings::Get()->SetVSyncEnabled(bIsChecked);
}

void USettingsMenuWidget_VideoAndSound::OnCheckStateChanged_HDREnabled(const bool bIsChecked)
{
	UBSGameUserSettings* GameUserSettings = UBSGameUserSettings::Get();
	SliderTextBoxOption_HDRNits->SetSliderAndTextBoxEnabledStates(bIsChecked);
	if (GameUserSettings->SupportsHDRDisplayOutput())
	{
		const float Clamped = FMath::Clamp(SliderTextBoxOption_HDRNits->GetSliderValueSnapped(), 1000, 2000);
		GameUserSettings->EnableHDRDisplayOutput(bIsChecked, Clamped);
	}
}

void USettingsMenuWidget_VideoAndSound::OnSelectionChanged_WindowMode(const TArray<FString>& SelectedOptions,
	ESelectInfo::Type SelectionType)
{
	if (SelectionType == ESelectInfo::Type::Direct || SelectedOptions.Num() != 1)
	{
		return;
	}
	const FString SelectedOption = SelectedOptions[0];
	UBSGameUserSettings* GameUserSettings = UBSGameUserSettings::Get();
	LastConfirmedResolution = GameUserSettings->GetLastConfirmedScreenResolution();
	LastConfirmedWindowMode = GameUserSettings->GetLastConfirmedFullscreenMode();

	const EWindowMode::Type* Found = WindowModeMap.Find(SelectedOption);
	if (!Found) return;

	switch (*Found)
	{
	case EWindowMode::Fullscreen:
		GameUserSettings->SetFullscreenMode(EWindowMode::Type::Fullscreen);
		break;
	case EWindowMode::WindowedFullscreen:
		GameUserSettings->SetFullscreenMode(EWindowMode::Type::WindowedFullscreen);
		break;
	case EWindowMode::Windowed:
		GameUserSettings->SetFullscreenMode(EWindowMode::Type::Windowed);
		break;
	case EWindowMode::NumWindowModes:
		break;
	default: ;
	}

	GameUserSettings->ApplyResolutionSettings(false);
	ShowConfirmVideoSettingsMessage();
}

void USettingsMenuWidget_VideoAndSound::OnSelectionChanged_Resolution(const TArray<FString>& SelectedOptions,
	ESelectInfo::Type SelectionType)
{
	if (SelectionType == ESelectInfo::Type::Direct || SelectedOptions.Num() != 1)
	{
		return;
	}
	const FString SelectedOption = SelectedOptions[0];
	FString LeftS;
	FString RightS;
	SelectedOption.Split("x", &LeftS, &RightS);
	LeftS = UKismetStringLibrary::Replace(LeftS, ",", "");
	RightS = UKismetStringLibrary::Replace(RightS, ",", "");

	const FIntPoint NewResolution = FIntPoint(FCString::Atoi(*LeftS), FCString::Atoi(*RightS));

	UBSGameUserSettings* GameUserSettings = UBSGameUserSettings::Get();
	LastConfirmedResolution = GameUserSettings->GetLastConfirmedScreenResolution();
	LastConfirmedWindowMode = GameUserSettings->GetLastConfirmedFullscreenMode();

	GameUserSettings->SetScreenResolution(NewResolution);
	GameUserSettings->ApplyResolutionSettings(false);

	ShowConfirmVideoSettingsMessage();
}

void USettingsMenuWidget_VideoAndSound::OnSelectionChanged_DLSS_EnabledMode(const TArray<FString>& SelectedOptions,
	ESelectInfo::Type SelectionType)
{
	if (SelectedOptions.Num() != 1 || SelectionType == ESelectInfo::Type::Direct)
	{
		return;
	}

	UBSGameUserSettings* GameUserSettings = UBSGameUserSettings::Get();
	const EDLSSEnabledMode Mode = GetSelectedDLSSEnabledMode();
	GameUserSettings->SetDLSSEnabledMode(Mode);

	HandleDLSSEnabledChanged(Mode);
	HandleDLSSDependencies(Mode);
}

void USettingsMenuWidget_VideoAndSound::OnSelectionChanged_FrameGeneration(const TArray<FString>& SelectedOptions,
	ESelectInfo::Type SelectionType)
{
	if (SelectedOptions.Num() != 1 || SelectionType == ESelectInfo::Type::Direct)
	{
		return;
	}

	UBSGameUserSettings* GameUserSettings = UBSGameUserSettings::Get();
	GameUserSettings->SetFrameGenerationEnabledMode(GetEnumFromString<UStreamlineDLSSGMode>(SelectedOptions.Top()));
}

void USettingsMenuWidget_VideoAndSound::OnSelectionChanged_SuperResolution(const TArray<FString>& SelectedOptions,
	ESelectInfo::Type SelectionType)
{
	if (SelectedOptions.Num() != 1 || SelectionType == ESelectInfo::Type::Direct)
	{
		return;
	}
	// TODO: This might need to change
	UBSGameUserSettings* GameUserSettings = UBSGameUserSettings::Get();
	GameUserSettings->SetDLSSMode(GetSelectedDLSSMode());
}

void USettingsMenuWidget_VideoAndSound::OnSelectionChanged_NIS_EnabledMode(const TArray<FString>& SelectedOptions,
	ESelectInfo::Type SelectionType)
{
	if (SelectedOptions.Num() != 1 || SelectionType == ESelectInfo::Type::Direct)
	{
		return;
	}

	const ENISEnabledMode NISEnabledMode = GetSelectedNISEnabledMode();
	const EDLSSEnabledMode DLSSEnabledMode = GetSelectedDLSSEnabledMode();

	if (NISEnabledMode == ENISEnabledMode::On)
	{
		ComboBoxOption_NIS_Mode->SetIsEnabled(true);
		SliderTextBoxOption_NIS_Sharpness->SetSliderAndTextBoxEnabledStates(true);
	}
	else
	{
		ComboBoxOption_NIS_Mode->SetIsEnabled(false);
		SliderTextBoxOption_NIS_Sharpness->SetSliderAndTextBoxEnabledStates(false);
	}

	if (NISEnabledMode == ENISEnabledMode::On || DLSSEnabledMode == EDLSSEnabledMode::On)
	{
		SliderTextBoxOption_ResolutionScale->SetSliderAndTextBoxEnabledStates(false);
	}
	else
	{
		SliderTextBoxOption_ResolutionScale->SetSliderAndTextBoxEnabledStates(true);
	}
}

void USettingsMenuWidget_VideoAndSound::OnSelectionChanged_NIS_Mode(const TArray<FString>& SelectedOptions,
	ESelectInfo::Type SelectionType)
{
	if (SelectedOptions.Num() != 1 || SelectionType == ESelectInfo::Type::Direct)
	{
		return;
	}

	UBSGameUserSettings* GameUserSettings = UBSGameUserSettings::Get();
	GameUserSettings->SetNISMode(GetSelectedNISMode());
}

void USettingsMenuWidget_VideoAndSound::OnSelectionChanged_Reflex(const TArray<FString>& SelectedOptions,
	ESelectInfo::Type SelectionType)
{
	if (SelectedOptions.Num() != 1 || SelectionType == ESelectInfo::Type::Direct)
	{
		return;
	}

	UBSGameUserSettings* GameUserSettings = UBSGameUserSettings::Get();
	GameUserSettings->SetStreamlineReflexMode(GetSelectedReflexMode());
}

void USettingsMenuWidget_VideoAndSound::OnSelectionChanged_AntiAliasingMethod(const TArray<FString>& SelectedOptions,
	ESelectInfo::Type SelectionType)
{
	if (SelectionType == ESelectInfo::Type::Direct || SelectedOptions.Num() != 1)
	{
		return;
	}

	if (const EAntiAliasingMethod* Found = AntiAliasingMethodMap.Find(SelectedOptions[0]))
	{
		UBSGameUserSettings* GameUserSettings = UBSGameUserSettings::Get();
		GameUserSettings->SetAntiAliasingMethod(*Found);
	}
}

void USettingsMenuWidget_VideoAndSound::OnSliderTextBoxValueChanged(USliderTextBoxOptionWidget* Widget,
	const float Value)
{
	UBSGameUserSettings* GameUserSettings = UBSGameUserSettings::Get();
	if (Widget == SliderTextBoxOption_HDRNits)
	{
		if (GameUserSettings->SupportsHDRDisplayOutput())
		{
			GameUserSettings->EnableHDRDisplayOutput(CheckBoxOption_HDREnabled->CheckBox->IsChecked(),
				SliderTextBoxOption_HDRNits->GetSliderValueSnapped());
		}
	}
	else if (Widget == SliderTextBoxOption_Brightness)
	{
		GameUserSettings->SetDisplayGamma(Value);
	}
	else if (Widget == SliderTextBoxOption_GlobalSound)
	{
		GameUserSettings->SetOverallVolume(Value);
	}
	else if (Widget == SliderTextBoxOption_MenuSound)
	{
		GameUserSettings->SetMenuVolume(Value);
	}
	else if (Widget == SliderTextBoxOption_MusicSound)
	{
		GameUserSettings->SetMusicVolume(Value);
	}
	else if (Widget == SliderTextBoxOption_DLSS_Sharpness)
	{
		GameUserSettings->SetDLSSSharpness(Value);
	}
	else if (Widget == SliderTextBoxOption_NIS_Sharpness)
	{
		GameUserSettings->SetNISSharpness(Value);
	}
	else if (Widget == SliderTextBoxOption_ResolutionScale)
	{
		GameUserSettings->SetResolutionScaleChecked(Value);
	}
}

void USettingsMenuWidget_VideoAndSound::PopulateResolutionComboBox()
{
	UBSGameUserSettings* Settings = UBSGameUserSettings::Get();
	const FIntPoint CurrentResolution = Settings->GetScreenResolution();
	TArray<FIntPoint> Resolutions;
	FIntPoint MaxResolution = FIntPoint(0, 0);
	bool bIsWindowedFullscreen = false;

	switch (Settings->GetFullscreenMode())
	{
	case EWindowMode::Type::Fullscreen:
		{
			UKismetSystemLibrary::GetSupportedFullscreenResolutions(Resolutions);
			break;
		}
	case EWindowMode::Type::WindowedFullscreen:
		{
			UKismetSystemLibrary::GetSupportedFullscreenResolutions(Resolutions);
			bIsWindowedFullscreen = true;
			break;
		}
	case EWindowMode::Type::Windowed:
		{
			UKismetSystemLibrary::GetConvenientWindowedResolutions(Resolutions);
			break;
		}
	case EWindowMode::Type::NumWindowModes:
		{
			UKismetSystemLibrary::GetSupportedFullscreenResolutions(Resolutions);
			break;
		}
	}

	TArray<FString> Options;
	FString SelectedOption = "";

	for (const FIntPoint Resolution : Resolutions)
	{
		if (!bIsWindowedFullscreen)
		{
			FString ResString = FString::FormatAsNumber(Resolution.X) + "x" + FString::FormatAsNumber(Resolution.Y);
			Options.AddUnique(ResString);
			if (Resolution == CurrentResolution)
			{
				SelectedOption = ResString;
			}
		}
		else
		{
			if (Resolution.X > MaxResolution.X)
			{
				MaxResolution = Resolution;
			}
		}
	}

	const FString ResString = FString::FormatAsNumber(MaxResolution.X) + "x" + FString::FormatAsNumber(MaxResolution.Y);
	if (bIsWindowedFullscreen)
	{
		Options.AddUnique(ResString);
	}

	ComboBoxOption_Resolution->ComboBox->ClearOptions();
	ComboBoxOption_Resolution->SortAndAddOptions(Options);

	if (bIsWindowedFullscreen)
	{
		ComboBoxOption_Resolution->ComboBox->SetSelectedOption(ResString);
	}
	else
	{
		ComboBoxOption_Resolution->ComboBox->SetSelectedOption(SelectedOption);
	}
}

void USettingsMenuWidget_VideoAndSound::HandleDLSSEnabledChanged(const EDLSSEnabledMode DLSSEnabledMode)
{
	// Enabling/disabling
	if (DLSSEnabledMode == EDLSSEnabledMode::On)
	{
		// Enable Settings that require DLSS to be on
		ComboBoxOption_DLSS_FrameGeneration->ComboBox->SetIsEnabled(true);
		ComboBoxOption_DLSS_SuperResolution->ComboBox->SetIsEnabled(true);

		SliderTextBoxOption_DLSS_Sharpness->SetSliderAndTextBoxEnabledStates(true);

		// Disable Settings that require DLSS to be off, or are forced to be on
		ComboBoxOption_NIS->ComboBox->SetIsEnabled(false);
		ComboBoxOption_NIS_Mode->ComboBox->SetIsEnabled(false);
		ComboBoxOption_Reflex->ComboBox->SetIsEnabled(false);

		SliderTextBoxOption_NIS_Sharpness->SetValue(0.f);
		SliderTextBoxOption_NIS_Sharpness->SetSliderAndTextBoxEnabledStates(false);

		SliderTextBoxOption_ResolutionScale->SetSliderAndTextBoxEnabledStates(false);

		// Force V Sync disabled
		CheckBoxOption_VSync->CheckBox->SetIsEnabled(false);
		CheckBoxOption_VSync->CheckBox->SetIsChecked(false);
	}
	else
	{
		// Disable Settings that require DLSS to be on
		ComboBoxOption_DLSS_FrameGeneration->ComboBox->SetIsEnabled(false);
		ComboBoxOption_DLSS_SuperResolution->ComboBox->SetIsEnabled(false);

		SliderTextBoxOption_DLSS_Sharpness->SetValue(0.f);
		SliderTextBoxOption_DLSS_Sharpness->SetSliderAndTextBoxEnabledStates(false);

		// Enable Settings that are don't require DLSS to be on
		ComboBoxOption_NIS->ComboBox->SetIsEnabled(true);
		ComboBoxOption_Reflex->ComboBox->SetIsEnabled(true);

		// Enable Resolution Scale if NIS is off
		if (GetSelectedNISEnabledMode() == ENISEnabledMode::Off)
		{
			SliderTextBoxOption_ResolutionScale->SetSliderAndTextBoxEnabledStates(true);
		}
		else
		{
			SliderTextBoxOption_ResolutionScale->SetSliderAndTextBoxEnabledStates(false);
		}

		CheckBoxOption_VSync->CheckBox->SetIsEnabled(true);
	}
}

void USettingsMenuWidget_VideoAndSound::HandleDLSSDependencies(const EDLSSEnabledMode DLSSEnabledMode)
{
	UBSGameUserSettings* GameUserSettings = UBSGameUserSettings::Get();

	ComboBoxOption_DLSS_FrameGeneration->ComboBox->SetSelectedOption(
		GetStringFromEnum(GameUserSettings->GetFrameGenerationEnabledMode()));
	ComboBoxOption_DLSS_SuperResolution->ComboBox->
	                                     SetSelectedOption(GetStringFromEnum(GameUserSettings->GetDLSSMode()));
	ComboBoxOption_NIS->ComboBox->SetSelectedOption(GetStringFromEnum(GameUserSettings->GetNISEnabledMode()));
	ComboBoxOption_NIS_Mode->ComboBox->SetSelectedOption(GetStringFromEnum(GameUserSettings->GetNISMode()));
	ComboBoxOption_Reflex->ComboBox->SetSelectedOption(GetStringFromEnum(GameUserSettings->GetStreamlineReflexMode()));
	CheckBoxOption_VSync->CheckBox->SetIsChecked(GameUserSettings->IsVSyncEnabled());


	float CurrentScaleNormalized;
	float CurrentScale;
	float MinScale;
	float MaxScale;

	GameUserSettings->GetResolutionScaleInformationEx(CurrentScaleNormalized, CurrentScale, MinScale, MaxScale);
	SliderTextBoxOption_ResolutionScale->SetValue(CurrentScale / 100.f);
}

void USettingsMenuWidget_VideoAndSound::OnButtonPressed_Save()
{
	UGameUserSettings::GetGameUserSettings()->ApplySettings(false);
	SavePlayerSettings(GetVideoAndSoundSettings());
	SavedTextWidget->SetSavedText(GetWidgetTextFromKey("SM_Saved_VideoAndSound"));
	SavedTextWidget->PlayFadeInFadeOut();
}

void USettingsMenuWidget_VideoAndSound::OnButtonPressed_Reset()
{
	UBSGameUserSettings::Get()->SetToDefaults();
	UBSGameUserSettings::Get()->ApplySettings(false);

	if (IConsoleVariable* CVarAntiAliasingMethod = IConsoleManager::Get().FindConsoleVariable(
		TEXT("r.AntiAliasingMethod")))
	{
		CVarAntiAliasingMethod->Set(AAM_TSR, ECVF_SetByGameOverride);
	}
	if (GConfig)
	{
		const FString Value = FString::FromInt(AAM_TSR);
		GConfig->SetString(TEXT("/Script/Engine.RendererSettings"), TEXT("r.AntiAliasingMethod"), *Value, GGameIni);
		GConfig->Flush(false, GGameIni);
	}

	InitializeVideoAndSoundSettings();
}

void USettingsMenuWidget_VideoAndSound::OnBSButtonPressed_SaveReset(const UBSButton* Button)
{
	switch (static_cast<ESettingButtonType>(Button->GetEnumValue()))
	{
	case ESettingButtonType::Save:
		OnButtonPressed_Save();
		break;
	case ESettingButtonType::Reset:
		OnButtonPressed_Reset();
		break;
	default:
		break;
	}
}

void USettingsMenuWidget_VideoAndSound::OnVideoSettingOptionWidget_ButtonPressed(
	const EVideoSettingType VideoSettingType, const uint8 Quality)
{
	UBSGameUserSettings* Settings = UBSGameUserSettings::Get();
	switch (VideoSettingType)
	{
	case EVideoSettingType::AntiAliasing:
		Settings->SetAntiAliasingQuality(Quality);
		break;
	case EVideoSettingType::GlobalIllumination:
		Settings->SetGlobalIlluminationQuality(Quality);
		break;
	case EVideoSettingType::PostProcessing:
		Settings->SetPostProcessingQuality(Quality);
		break;
	case EVideoSettingType::Reflection:
		Settings->SetReflectionQuality(Quality);
		break;
	case EVideoSettingType::Shadow:
		Settings->SetShadowQuality(Quality);
		break;
	case EVideoSettingType::Shading:
		Settings->SetShadingQuality(Quality);
		break;
	case EVideoSettingType::Texture:
		Settings->SetTextureQuality(Quality);
		break;
	case EVideoSettingType::ViewDistance:
		Settings->SetViewDistanceQuality(Quality);
		break;
	case EVideoSettingType::VisualEffect:
		Settings->SetVisualEffectQuality(Quality);
		break;
	default: ;
	}
}

void USettingsMenuWidget_VideoAndSound::OnCheckStateChanged_FPSCounter(const bool bIsChecked)
{
	UBSGameUserSettings::Get()->SetShowFPSCounter(bIsChecked);
}

void USettingsMenuWidget_VideoAndSound::OnTextCommitted_FPSLimitMenu(const FText& Text, ETextCommit::Type CommitType)
{
	const float FrameLimit = UKismetMathLibrary::GridSnap_Float(FCString::Atof(*Text.ToString()),
		SnapSize_FrameRateLimit);
	UBSGameUserSettings::Get()->SetFrameRateLimitMenu(FrameLimit);
}

void USettingsMenuWidget_VideoAndSound::OnTextCommitted_FPSLimitGame(const FText& Text, ETextCommit::Type CommitType)
{
	const float FrameLimit = UKismetMathLibrary::GridSnap_Float(FCString::Atof(*Text.ToString()),
		SnapSize_FrameRateLimit);
	UBSGameUserSettings::Get()->SetFrameRateLimitGame(FrameLimit);
}

void USettingsMenuWidget_VideoAndSound::OnTextCommitted_FPSLimitBackground(const FText& Text,
	ETextCommit::Type CommitType)
{
	const float FrameLimit = UKismetMathLibrary::GridSnap_Float(FCString::Atof(*Text.ToString()),
		SnapSize_FrameRateLimit);
	UBSGameUserSettings::Get()->SetFrameRateLimitBackground(FrameLimit);
}

void USettingsMenuWidget_VideoAndSound::ShowConfirmVideoSettingsMessage()
{
	PopupMessageWidget = CreateWidget<UPopupMessageWidget>(this, PopupMessageClass);
	if (PopupMessageWidget)
	{
		TArray<UBSButton*> Buttons = PopupMessageWidget->InitPopup(
			GetWidgetTextFromKey("ConfirmVideoSettingsPopupTitle"),
			GetWidgetTextFromKey("ConfirmVideoSettingsPopupMessage"), 2);
		if (Buttons[0])
		{
			Buttons[0]->SetButtonText(GetWidgetTextFromKey("ConfirmVideoSettingsPopupButton2"));
			Buttons[0]->OnBSButtonButtonPressed_NoParams.AddDynamic(this,
				&USettingsMenuWidget_VideoAndSound::OnButtonPressed_CancelVideoSettings);
		}
		if (Buttons[1])
		{
			Buttons[1]->SetButtonText(GetWidgetTextFromKey("ConfirmVideoSettingsPopupButton1"));
			Buttons[1]->OnBSButtonButtonPressed_NoParams.AddDynamic(this,
				&USettingsMenuWidget_VideoAndSound::OnButtonPressed_ConfirmVideoSettings);
		}
		GetWorld()->GetTimerManager().SetTimer(RevertVideoSettingsTimer_UpdateSecond, this,
			&USettingsMenuWidget_VideoAndSound::RevertVideoSettingsTimerCallback, 1.f, true);
		GetWorld()->GetTimerManager().SetTimer(RevertVideoSettingsTimer, VideoSettingsTimeoutLength, false);
	}
	PopupMessageWidget->AddToViewport();
	PopupMessageWidget->FadeIn();
}

void USettingsMenuWidget_VideoAndSound::OnButtonPressed_ConfirmVideoSettings()
{
	GetWorld()->GetTimerManager().ClearTimer(RevertVideoSettingsTimer_UpdateSecond);
	GetWorld()->GetTimerManager().ClearTimer(RevertVideoSettingsTimer);

	UBSGameUserSettings* GameUserSettings = UBSGameUserSettings::Get();

	GameUserSettings->ConfirmVideoMode();

	// ???
	GameUserSettings->SetResolutionScaleValueEx(100.f);
	SliderTextBoxOption_ResolutionScale->SetValue(1.f);
	GameUserSettings->ApplyResolutionSettings(false);
	GameUserSettings->SaveSettings();

	PopulateResolutionComboBox();

	PopupMessageWidget->FadeOut();

	SavedTextWidget->SetSavedText(GetWidgetTextFromKey("SM_Saved_VideoAndSound"));
	SavedTextWidget->PlayFadeInFadeOut();
}

void USettingsMenuWidget_VideoAndSound::OnButtonPressed_CancelVideoSettings()
{
	GetWorld()->GetTimerManager().ClearTimer(RevertVideoSettingsTimer_UpdateSecond);
	GetWorld()->GetTimerManager().ClearTimer(RevertVideoSettingsTimer);

	UBSGameUserSettings* GameUserSettings = UBSGameUserSettings::Get();

	GameUserSettings->RevertVideoMode();
	GameUserSettings->SetScreenResolution(LastConfirmedResolution);
	GameUserSettings->SetFullscreenMode(LastConfirmedWindowMode);
	GameUserSettings->SetResolutionScaleValueEx(100.f);

	SliderTextBoxOption_ResolutionScale->SetValue(1.f);

	GameUserSettings->ApplyResolutionSettings(false);
	GameUserSettings->ConfirmVideoMode();
	// TODO: Is this necessary?
	GameUserSettings->SaveSettings();

	for (const TPair<FString, EWindowMode::Type>& Pair : WindowModeMap)
	{
		if (LastConfirmedWindowMode == Pair.Value)
		{
			ComboBoxOption_WindowMode->ComboBox->SetSelectedOption(Pair.Key);
			break;
		}
	}
	ComboBoxOption_Resolution->ComboBox->SetSelectedOption(
		FString::FormatAsNumber(LastConfirmedResolution.X) + "x" + FString::FormatAsNumber(LastConfirmedResolution.Y));

	PopupMessageWidget->FadeOut();

	SavedTextWidget->SetSavedText(FText::FromString("Fullscreen Mode and Resolution Reset"));
	SavedTextWidget->PlayFadeInFadeOut();
}

void USettingsMenuWidget_VideoAndSound::RevertVideoSettingsTimerCallback()
{
	const float Elapsed = GetWorld()->GetTimerManager().GetTimerElapsed(RevertVideoSettingsTimer);
	if (Elapsed >= VideoSettingsTimeoutLength || Elapsed == -1.f)
	{
		OnButtonPressed_CancelVideoSettings();
		return;
	}
	TArray<FString> Out;
	GetWidgetTextFromKey("ConfirmVideoSettingsPopupMessage").ToString().ParseIntoArray(Out, TEXT(" "));
	int32 Index = INDEX_NONE;
	for (int i = 0; i < Out.Num(); i++)
	{
		if (UKismetStringLibrary::IsNumeric(Out[i]))
		{
			Index = i;
			break;
		}
	}
	if (Index != INDEX_NONE && PopupMessageWidget)
	{
		Out[Index] = FString::FromInt(roundf(VideoSettingsTimeoutLength - Elapsed));
		PopupMessageWidget->ChangeMessageText(FText::FromString(UKismetStringLibrary::JoinStringArray(Out)));
	}
}

FString USettingsMenuWidget_VideoAndSound::GetComboBoxEntryTooltipStringTableKey_DLSS_FrameGeneration(
	const FString& EnumString)
{
	const UStreamlineDLSSGMode EnumValue = GetEnumFromString<UStreamlineDLSSGMode>(EnumString);
	return GetStringTableKeyNameFromEnum(EnumValue);
}

FString USettingsMenuWidget_VideoAndSound::GetComboBoxEntryTooltipStringTableKey_DLSS_SuperResolution(
	const FString& EnumString)
{
	const UDLSSMode EnumValue = GetEnumFromString<UDLSSMode>(EnumString);
	return GetStringTableKeyNameFromEnum(EnumValue);
}

FString USettingsMenuWidget_VideoAndSound::GetComboBoxEntryTooltipStringTableKey_Reflex(const FString& EnumString)
{
	const UStreamlineReflexMode EnumValue = GetEnumFromString<UStreamlineReflexMode>(EnumString);
	return GetStringTableKeyNameFromEnum(EnumValue);
}

// NVIDIA settings getters

EDLSSEnabledMode USettingsMenuWidget_VideoAndSound::GetSelectedDLSSEnabledMode() const
{
	return GetEnumFromString<EDLSSEnabledMode>(ComboBoxOption_DLSS->ComboBox->GetSelectedOption());
}

UStreamlineDLSSGMode USettingsMenuWidget_VideoAndSound::GetSelectedFrameGenerationMode() const
{
	return GetEnumFromString<UStreamlineDLSSGMode>(ComboBoxOption_DLSS_FrameGeneration->ComboBox->GetSelectedOption());
}

UDLSSMode USettingsMenuWidget_VideoAndSound::GetSelectedDLSSMode() const
{
	return GetEnumFromString<UDLSSMode>(ComboBoxOption_DLSS_SuperResolution->ComboBox->GetSelectedOption());
}

ENISEnabledMode USettingsMenuWidget_VideoAndSound::GetSelectedNISEnabledMode() const
{
	return GetEnumFromString<ENISEnabledMode>(ComboBoxOption_NIS->ComboBox->GetSelectedOption());
}

UNISMode USettingsMenuWidget_VideoAndSound::GetSelectedNISMode() const
{
	return GetEnumFromString<UNISMode>(ComboBoxOption_NIS->ComboBox->GetSelectedOption());
}

UStreamlineReflexMode USettingsMenuWidget_VideoAndSound::GetSelectedReflexMode() const
{
	return GetEnumFromString<UStreamlineReflexMode>(ComboBoxOption_Reflex->ComboBox->GetSelectedOption());
}
