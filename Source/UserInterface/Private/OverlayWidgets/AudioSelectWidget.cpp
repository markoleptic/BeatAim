// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "OverlayWidgets/AudioSelectWidget.h"
#include "AudioAnalyzerManager.h"
#include "WidgetComponents/TooltipWidget.h"
#include "Components/CheckBox.h"
#include "Components/ComboBoxString.h"
#include "Components/EditableTextBox.h"
#include "Components/VerticalBox.h"
#include "Components/HorizontalBox.h"
#include "GameFramework/GameUserSettings.h"
#include "OverlayWidgets/PopupMessageWidget.h"
#include "WidgetComponents/Buttons/BSButton.h"
#include "Microsoft/COMPointer.h"
#include <shobjidl_core.h>

#define MAX_FILETYPES_STR 4096
#define MAX_FILENAME_STR 65536 // This buffer has to be big enough to contain the names of all the selected files as well as the null characters between them and the null character at the end

UTooltipWidget* UAudioSelectWidget::ConstructTooltipWidget()
{
	return CreateWidget<UTooltipWidget>(this, TooltipWidgetClass);
}

UTooltipWidget* UAudioSelectWidget::GetTooltipWidget() const
{
	return ActiveTooltipWidget;
}

void UAudioSelectWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	ActiveTooltipWidget = ConstructTooltipWidget();
	
	NumberFormattingOptions.MinimumIntegralDigits = 2;
	NumberFormattingOptions.MaximumIntegralDigits = 2;

	Button_Back->OnBSButtonPressed.AddDynamic(this, &ThisClass::OnButtonPressed_BSButton);

	Button_Start->SetIsEnabled(false);
	Button_LoadFile->SetIsEnabled(false);
	Button_CaptureAudio->SetDefaults(static_cast<uint8>(EAudioFormat::Capture), Button_AudioFromFile);
	Button_AudioFromFile->SetDefaults(static_cast<uint8>(EAudioFormat::File), Button_CaptureAudio);
	Button_LoadFile->OnPressedAnimFinished.AddDynamic(this, &ThisClass::OnButtonClicked_LoadFile);

	Button_Start->OnBSButtonPressed.AddDynamic(this, &ThisClass::OnButtonPressed_BSButton);
	Button_LoadFile->OnBSButtonPressed.AddDynamic(this, &ThisClass::OnButtonPressed_BSButton);
	Button_AudioFromFile->OnBSButtonPressed.AddDynamic(this, &ThisClass::OnButtonPressed_BSButton);
	Button_CaptureAudio->OnBSButtonPressed.AddDynamic(this, &ThisClass::OnButtonPressed_BSButton);

	Value_SongTitle->OnTextCommitted.AddUniqueDynamic(this, &UAudioSelectWidget::OnValueChanged_SongTitle);
	Value_Seconds->OnTextCommitted.AddUniqueDynamic(this, &UAudioSelectWidget::OnValueChanged_Seconds);
	Value_Minutes->OnTextCommitted.AddUniqueDynamic(this, &UAudioSelectWidget::OnValueChanged_Minutes);
	ComboBox_InAudioDevices->OnSelectionChanged.AddUniqueDynamic(this, &UAudioSelectWidget::OnSelectionChanged_InAudioDevice);
	ComboBox_OutAudioDevices->OnSelectionChanged.AddUniqueDynamic(this, &UAudioSelectWidget::OnSelectionChanged_OutAudioDevice);
	ComboBox_SongTitle->OnSelectionChanged.AddUniqueDynamic(this, &UAudioSelectWidget::OnSelectionChanged_SongTitle);
	Checkbox_PlaybackAudio->OnCheckStateChanged.AddUniqueDynamic(this, &UAudioSelectWidget::OnCheckStateChanged_PlaybackAudio);

	SetupTooltip(QMark_PlaybackAudio, GetTooltipTextFromKey("PlaybackAudio"));

	UAudioAnalyzerManager* Manager = NewObject<UAudioAnalyzerManager>(this);
	TArray<FString> OutAudioDeviceList;
	TArray<FString> InAudioDeviceList;
	Manager->GetOutputAudioDevices(OutAudioDeviceList);
	Manager->GetInputAudioDevices(InAudioDeviceList);

	for (const FString& AudioDevice : OutAudioDeviceList)
	{
		ComboBox_OutAudioDevices->AddOption(AudioDevice);
	}
	for (const FString& AudioDevice : InAudioDeviceList)
	{
		ComboBox_InAudioDevices->AddOption(AudioDevice);
	}
	PopulateSongOptionComboBox();
	Box_AudioDevice->SetVisibility(ESlateVisibility::Collapsed);
	Box_SongTitleLength->SetVisibility(ESlateVisibility::Collapsed);
	OnValueChanged_Seconds(FText::AsNumber(0), ETextCommit::Type::Default);
	OnValueChanged_Minutes(FText::AsNumber(0), ETextCommit::Type::Default);
}

void UAudioSelectWidget::FadeIn()
{
	PlayAnimationForward(FadeInAnim);
}

void UAudioSelectWidget::FadeOut()
{
	FadeOutDelegate.BindDynamic(this, &UAudioSelectWidget::OnFadeOutFinish);
	PlayAnimationForward(FadeOutAnim);
}

void UAudioSelectWidget::OnFadeOutFinish()
{
	RemoveFromParent();
}

void UAudioSelectWidget::OnButtonPressed_BSButton(const UBSButton* Button)
{
	if (Button == Button_Back)
	{
		FadeOut();
		return;
	}
	if (Button == Button_Start)
	{
		OnButtonClicked_Start();
		return;
	}
	
	if (!Button->HasSetDefaults())
	{
		return;
	}
	switch (static_cast<EAudioFormat>(Button->GetEnumValue()))
	{
	case EAudioFormat::File:
		OnButtonClicked_AudioFromFile();
		break;
	case EAudioFormat::Capture:
		OnButtonClicked_CaptureAudio();
		break;
	default:
		break;
	}
}

void UAudioSelectWidget::OnButtonClicked_AudioFromFile()
{
	AudioConfig.AudioFormat = EAudioFormat::File;

	Button_Start->SetIsEnabled(false);
	Button_LoadFile->SetIsEnabled(true);

	ComboBox_InAudioDevices->ClearSelection();
	ComboBox_OutAudioDevices->ClearSelection();

	Box_AudioDevice->SetVisibility(ESlateVisibility::Collapsed);
	Box_SongTitleLength->SetVisibility(ESlateVisibility::Collapsed);
}

void UAudioSelectWidget::OnButtonClicked_CaptureAudio()
{
	AudioConfig.AudioFormat = EAudioFormat::Capture;

	Button_Start->SetIsEnabled(false);
	Button_LoadFile->SetIsEnabled(false);

	ComboBox_InAudioDevices->ClearSelection();
	ComboBox_OutAudioDevices->ClearSelection();

	Box_AudioDevice->SetVisibility(ESlateVisibility::Visible);
	Box_SongTitleLength->SetVisibility(ESlateVisibility::Collapsed);

	const FPlayerSettings_AudioAnalyzer PlayerSettings = LoadPlayerSettings().AudioAnalyzer;
	ComboBox_InAudioDevices->SetSelectedOption(PlayerSettings.LastSelectedInputAudioDevice);
	ComboBox_OutAudioDevices->SetSelectedOption(PlayerSettings.LastSelectedOutputAudioDevice);
}

void UAudioSelectWidget::OnButtonClicked_Start()
{
	FPlayerSettings_AudioAnalyzer PlayerSettings = LoadPlayerSettings().AudioAnalyzer;
	if (!ComboBox_InAudioDevices->GetSelectedOption().IsEmpty())
	{
		PlayerSettings.LastSelectedInputAudioDevice = ComboBox_InAudioDevices->GetSelectedOption();
	}
	if (!ComboBox_OutAudioDevices->GetSelectedOption().IsEmpty())
	{
		PlayerSettings.LastSelectedOutputAudioDevice = ComboBox_OutAudioDevices->GetSelectedOption();
	}
	SavePlayerSettings(PlayerSettings);
	if (!OnStartButtonClickedDelegate.ExecuteIfBound(AudioConfig))
	{
		UE_LOG(LogTemp, Display, TEXT("OnStartButtonClickedDelegate not bound."));
	}
}

void UAudioSelectWidget::OnButtonClicked_LoadFile()
{
	TArray<FString> FileNames;
	const bool bSuccess = OpenFileDialog(FileNames);

	/*if (bWasInFullScreenMode)
	{
		UGameUserSettings* GameUserSettings = UGameUserSettings::GetGameUserSettings();
		GameUserSettings->SetFullscreenMode(EWindowMode::Fullscreen);
		GameUserSettings->ApplySettings(false);
		bWasInFullScreenMode = false;
	}*/
	
	if (!bSuccess || FileNames.IsEmpty() || FileNames[0].IsEmpty())
	{
		ShowSongPathErrorMessage();
		return;
	}
	AudioConfig.SongPath = FileNames[0];

	UAudioAnalyzerManager* Manager = NewObject<UAudioAnalyzerManager>(this);
	if (!Manager->InitPlayerAudio(AudioConfig.SongPath))
	{
		ShowSongPathErrorMessage();
		UE_LOG(LogTemp, Display, TEXT("Init Tracker Error"));
		return;
	}
	/* set Song length and song title in BSConfig if using song file */
	FString Filename, Extension, MetaType, Title, Artist, Album, Year, Genre;
	Manager->GetMetadata(Filename, Extension, MetaType, Title, Artist, Album, Year, Genre);
	if (Title.IsEmpty())
	{
		if (ComboBox_SongTitle->FindOptionIndex(Filename) == -1)
		{
			ComboBox_SongTitle->AddOption(Filename);
		}
		ComboBox_SongTitle->SetSelectedOption(Filename);
		Value_SongTitle->SetText(FText::FromString(Filename));
		AudioConfig.SongTitle = Filename;
	}
	else
	{
		if (ComboBox_SongTitle->FindOptionIndex(Title) == -1)
		{
			ComboBox_SongTitle->AddOption(Title);
		}
		ComboBox_SongTitle->SetSelectedOption(Title);
		Value_SongTitle->SetText(FText::FromString(Title));
		AudioConfig.SongTitle = Title;
	}
	AudioConfig.SongLength = Manager->GetTotalDuration();
	Box_SongTitleLength->SetVisibility(ESlateVisibility::Visible);
	Box_SongTitle->SetVisibility(ESlateVisibility::Visible);
	Box_SongLength->SetVisibility(ESlateVisibility::Collapsed);
	Button_Start->SetIsEnabled(true);
}

void UAudioSelectWidget::OnValueChanged_SongTitle(const FText& NewSongTitle, ETextCommit::Type CommitType)
{
	if (NewSongTitle.IsEmptyOrWhitespace())
	{
		AudioConfig.SongTitle = "";
	}
	else
	{
		AudioConfig.SongTitle = NewSongTitle.ToString();
	}
}

void UAudioSelectWidget::OnValueChanged_Minutes(const FText& NewMinutes, ETextCommit::Type CommitType)
{
	const int32 ClampedValue = FMath::Clamp(FCString::Atoi(*NewMinutes.ToString()), 0, 99);
	Value_Minutes->SetText(FText::AsNumber(ClampedValue, &NumberFormattingOptions));
	AudioConfig.SongLength = ClampedValue * 60 + FMath::Clamp(FCString::Atoi(*Value_Seconds->GetText().ToString()), 0, 60);
}

void UAudioSelectWidget::OnValueChanged_Seconds(const FText& NewSeconds, ETextCommit::Type CommitType)
{
	const int32 ClampedValue = FMath::Clamp(FCString::Atoi(*NewSeconds.ToString()), 0, 60);
	Value_Seconds->SetText(FText::AsNumber(ClampedValue, &NumberFormattingOptions));
	AudioConfig.SongLength = FMath::Clamp(FCString::Atoi(*Value_Minutes->GetText().ToString()), 0, 99) * 60 + ClampedValue;
}

void UAudioSelectWidget::OnSelectionChanged_InAudioDevice(const FString SelectedInAudioDevice, const ESelectInfo::Type SelectionType)
{
	AudioConfig.InAudioDevice = SelectedInAudioDevice;
	if (ComboBox_OutAudioDevices->GetSelectedIndex() != -1 && ComboBox_InAudioDevices->GetSelectedIndex() != -1)
	{
		Box_SongTitleLength->SetVisibility(ESlateVisibility::Visible);
		Box_SongTitle->SetVisibility(ESlateVisibility::Visible);
		Box_SongLength->SetVisibility(ESlateVisibility::Visible);
		Button_Start->SetIsEnabled(true);
	}
}

void UAudioSelectWidget::OnSelectionChanged_OutAudioDevice(const FString SelectedOutAudioDevice, const ESelectInfo::Type SelectionType)
{
	AudioConfig.OutAudioDevice = SelectedOutAudioDevice;
	if (ComboBox_OutAudioDevices->GetSelectedIndex() != -1 && ComboBox_InAudioDevices->GetSelectedIndex() != -1)
	{
		Box_SongTitleLength->SetVisibility(ESlateVisibility::Visible);
		Box_SongTitle->SetVisibility(ESlateVisibility::Visible);
		Box_SongLength->SetVisibility(ESlateVisibility::Visible);
		Button_Start->SetIsEnabled(true);
	}
}

void UAudioSelectWidget::OnSelectionChanged_SongTitle(const FString SelectedSongTitle, const ESelectInfo::Type SelectionType)
{
	AudioConfig.SongTitle = SelectedSongTitle;
}

void UAudioSelectWidget::OnCheckStateChanged_PlaybackAudio(const bool bIsChecked)
{
	AudioConfig.bPlaybackAudio = bIsChecked;
}

void UAudioSelectWidget::PopulateSongOptionComboBox()
{
	for (FPlayerScore SavedScoreObj : LoadPlayerScores())
	{
		if (ComboBox_SongTitle->FindOptionIndex(SavedScoreObj.SongTitle) == -1)
		{
			ComboBox_SongTitle->AddOption(SavedScoreObj.SongTitle);
		}
	}
}

void UAudioSelectWidget::OpenSongFileDialog_Implementation(TArray<FString>& OutFileNames)
{
	/** Cheap fix to make sure open file dialog is always on top of the game */
	if (UGameUserSettings* GameUserSettings = UGameUserSettings::GetGameUserSettings(); GameUserSettings->GetFullscreenMode() == EWindowMode::Fullscreen)
	{
		bWasInFullScreenMode = true;
		GameUserSettings->SetFullscreenMode(EWindowMode::WindowedFullscreen);
		GameUserSettings->ApplySettings(false);
	}
}

bool UAudioSelectWidget::OpenFileDialog(TArray<FString>& OutFileNames)
{
	const FString DialogTitle = "Choose a Song in .mp3 or .ogg Format";
	FString DefaultPath;
	FString DefaultFile;
	const FString FileTypes = ".mp3,.ogg";
	int OutFilterIndex=0;
	return FileDialogShared(false, GEngine->GameViewport->GetWindow()->GetNativeWindow()->GetOSWindowHandle(), DialogTitle, DefaultPath, DefaultFile, FileTypes, 0, OutFileNames, OutFilterIndex);
}

bool UAudioSelectWidget::FileDialogShared(bool bSave, const void* ParentWindowHandle, const FString& DialogTitle, const FString& DefaultPath, const FString& DefaultFile, const FString& FileTypes, uint32 Flags, TArray<FString>& OutFilenames, int32& OutFilterIndex)
{

#if PLATFORM_WINDOWS
	WCHAR Filename[MAX_FILENAME_STR];
	FCString::Strcpy(Filename, MAX_FILENAME_STR, *(DefaultFile.Replace(TEXT("/"), TEXT("\\"))));

	// Convert the forward slashes in the path name to backslashes, otherwise it'll be ignored as invalid and use whatever is cached in the registry
	WCHAR Pathname[MAX_FILENAME_STR];
	FCString::Strcpy(Pathname, MAX_FILENAME_STR, *(FPaths::ConvertRelativePathToFull(DefaultPath).Replace(TEXT("/"), TEXT("\\"))));

	// Convert the "|" delimited list of filetypes to NULL delimited then add a second NULL character to indicate the end of the list
	WCHAR FileTypeStr[MAX_FILETYPES_STR];
	WCHAR* FileTypesPtr = NULL;
	const int32 FileTypesLen = FileTypes.Len();

	// Nicely formatted file types for lookup later and suitable to append to filenames without extensions
	TArray<FString> CleanExtensionList;

	// The strings must be in pairs for windows.
	// It is formatted as follows: Pair1String1|Pair1String2|Pair2String1|Pair2String2
	// where the second string in the pair is the extension.  To get the clean extensions we only care about the second string in the pair
	TArray<FString> UnformattedExtensions;
	FileTypes.ParseIntoArray(UnformattedExtensions, TEXT("|"), true);
	for (int32 ExtensionIndex = 1; ExtensionIndex < UnformattedExtensions.Num(); ExtensionIndex += 2)
	{
		const FString& Extension = UnformattedExtensions[ExtensionIndex];
		// Assume the user typed in an extension or doesnt want one when using the *.* extension. We can't determine what extension they wan't in that case
		if (Extension != TEXT("*.*"))
		{
			// Add to the clean extension list, first removing the * wildcard from the extension
			int32 WildCardIndex = Extension.Find(TEXT("*"));
			CleanExtensionList.Add(WildCardIndex != INDEX_NONE ? Extension.RightChop(WildCardIndex + 1) : Extension);
		}
	}

	if (FileTypesLen > 0 && FileTypesLen - 1 < MAX_FILETYPES_STR)
	{
		FileTypesPtr = FileTypeStr;
		FCString::Strcpy(FileTypeStr, MAX_FILETYPES_STR, *FileTypes);

		TCHAR* Pos = FileTypeStr;
		while (Pos[0] != 0)
		{
			if (Pos[0] == '|')
			{
				Pos[0] = 0;
			}

			Pos++;
		}

		// Add two trailing NULL characters to indicate the end of the list
		FileTypeStr[FileTypesLen] = 0;
		FileTypeStr[FileTypesLen + 1] = 0;
	}

	OPENFILENAME ofn;
	FMemory::Memzero(&ofn, sizeof(OPENFILENAME));

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = (HWND)ParentWindowHandle;
	ofn.lpstrFilter = FileTypesPtr;
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = Filename;
	ofn.nMaxFile = MAX_FILENAME_STR;
	ofn.lpstrInitialDir = Pathname;
	ofn.lpstrTitle = *DialogTitle;
	if (FileTypesLen > 0)
	{
		ofn.lpstrDefExt = &FileTypeStr[0];
	}

	ofn.Flags = OFN_HIDEREADONLY | OFN_ENABLESIZING | OFN_EXPLORER;

	if (bSave)
	{
		ofn.Flags |= OFN_CREATEPROMPT | OFN_OVERWRITEPROMPT | OFN_NOVALIDATE;
	}
	else
	{
		ofn.Flags |= OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	}

	bool bSuccess;
	if (bSave)
	{
		bSuccess = !!::GetSaveFileName(&ofn);
	}
	else
	{
		bSuccess = !!::GetOpenFileName(&ofn);
	}

	if (bSuccess)
	{
		// GetOpenFileName/GetSaveFileName changes the CWD on success. Change it back immediately.
		//FPlatformProcess::SetCurrentWorkingDirectoryToBaseDir();
		
		new(OutFilenames) FString(Filename);
		
		// The index of the filter in OPENFILENAME starts at 1.
		OutFilterIndex = ofn.nFilterIndex - 1;

		// Get the extension to add to the filename (if one doesnt already exist)
		FString Extension = CleanExtensionList.IsValidIndex(OutFilterIndex) ? CleanExtensionList[OutFilterIndex] : TEXT("");

		// Make sure all filenames gathered have their paths normalized and proper extensions added
		for (auto OutFilenameIt = OutFilenames.CreateIterator(); OutFilenameIt; ++OutFilenameIt)
		{
			FString& OutFilename = *OutFilenameIt;

			OutFilename = IFileManager::Get().ConvertToRelativePath(*OutFilename);

			if (FPaths::GetExtension(OutFilename).IsEmpty() && !Extension.IsEmpty())
			{
				// filename does not have an extension. Add an extension based on the filter that the user chose in the dialog
				OutFilename += Extension;
			}

			FPaths::NormalizeFilename(OutFilename);
		}
	}
	else
	{
		uint32 Error = ::CommDlgExtendedError();
		if (Error != ERROR_SUCCESS)
		{
			//UE_LOG(LogDesktopPlatform, Warning, TEXT("Error reading results of file dialog. Error: 0x%04X"), Error);
		}
	}

	return bSuccess;
#endif

	return false;
}

void UAudioSelectWidget::ShowSongPathErrorMessage()
{
	PopupMessageWidget = CreateWidget<UPopupMessageWidget>(GetWorld(), PopupMessageClass);
	PopupMessageWidget->InitPopup(FText::FromStringTable("/Game/StringTables/ST_Widgets.ST_Widgets", "ASW_SongPathErrorTitle"),
	                              FText::FromStringTable("/Game/StringTables/ST_Widgets.ST_Widgets", "ASW_SongPathErrorMessage"),
	                              FText::FromStringTable("/Game/StringTables/ST_Widgets.ST_Widgets", "ASW_SongPathErrorButton"));
	PopupMessageWidget->OnButton1Pressed.AddDynamic(this, &UAudioSelectWidget::HideSongPathErrorMessage);
	PopupMessageWidget->AddToViewport();
	PopupMessageWidget->FadeIn();
}

void UAudioSelectWidget::HideSongPathErrorMessage()
{
	PopupMessageWidget->FadeOut();
	FadeOut();
}
