// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "SubMenuWidgets/SettingsWidgets/SettingsMenuWidget_Input.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "SaveGamePlayerSettings.h"
#include "Components/EditableTextBox.h"
#include "Components/InputKeySelector.h"
#include "Components/ScrollBox.h"
#include "WidgetComponents/Buttons/BSButton.h"
#include "WidgetComponents/SavedTextWidget.h"
#include "WidgetComponents/MenuOptionWidgets/EditableTextBoxOptionWidget.h"
#include "WidgetComponents/MenuOptionWidgets/SliderTextBoxOptionWidget.h"

void USettingsMenuWidget_Input::NativeConstruct()
{
	Super::NativeConstruct();

	MenuOption_CurrentSensitivity->EditableTextBox->SetIsReadOnly(true);
	MenuOption_CurrentSensitivity->EditableTextBox->WidgetStyle.TextStyle.SetTypefaceFontName(FName("Semi-Bold"));
	MenuOption_CurrentSensitivity->EditableTextBox->SetJustification(ETextJustify::Type::Center);

	MenuOption_NewSensitivity->SetValues(0, 100, 0.001);
	MenuOption_NewSensitivityCsgo->SetValues(0, 100 * Constants::CsgoMultiplier, 0.001);

	MenuOption_NewSensitivity->OnSliderTextBoxValueChanged.AddUObject(this, &ThisClass::OnSliderTextBoxValueChanged);
	MenuOption_NewSensitivityCsgo->OnSliderTextBoxValueChanged.
	                               AddUObject(this, &ThisClass::OnSliderTextBoxValueChanged);

	Button_Reset->OnBSButtonPressed.AddUObject(this, &ThisClass::OnButtonClicked_BSButton);
	Button_Revert->OnBSButtonPressed.AddUObject(this, &ThisClass::OnButtonClicked_BSButton);
	Button_Save->OnBSButtonPressed.AddUObject(this, &ThisClass::OnButtonClicked_BSButton);

	Button_Reset->SetDefaults(static_cast<uint8>(ESettingButtonType::Reset));
	Button_Revert->SetDefaults(static_cast<uint8>(ESettingButtonType::Revert));
	Button_Save->SetDefaults(static_cast<uint8>(ESettingButtonType::Save));

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
		GetOwningPlayer()->GetLocalPlayer());
	check(Subsystem);

	// Init User settings
	Subsystem->InitalizeUserSettings();
	UEnhancedInputUserSettings* EnhancedInputUserSettings = Subsystem->GetUserSettings();
	check(EnhancedInputUserSettings);

	// Register mapping context
	if (BaseMappingContext && !EnhancedInputUserSettings->IsMappingContextRegistered(BaseMappingContext))
	{
		EnhancedInputUserSettings->RegisterInputMappingContext(BaseMappingContext);
	}

	const UEnhancedPlayerMappableKeyProfile* KeyProfile = EnhancedInputUserSettings->GetCurrentKeyProfile();
	InitialPlayerMappingRows = KeyProfile->GetPlayerMappingRows();

	InitializeInputSettings();
}

void USettingsMenuWidget_Input::InitializeInputSettings(const TMap<FName, FKeyMappingRow>& InPlayerMappedRows)
{
	FPlayerSettings_User PlayerSettings_User = LoadPlayerSettings().User;
	Sensitivity = PlayerSettings_User.Sensitivity;

	MenuOption_CurrentSensitivity->EditableTextBox->SetText(FText::AsNumber(Sensitivity));
	MenuOption_NewSensitivity->SetValue(Sensitivity);
	MenuOption_NewSensitivityCsgo->SetValue(Sensitivity * Constants::CsgoMultiplier);

	TMap<FName, FKeyMappingRow> Rows = InPlayerMappedRows;
	if (InPlayerMappedRows.IsEmpty())
	{
		UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
			GetOwningPlayer()->GetLocalPlayer());
		Rows = Subsystem->GetUserSettings()->GetCurrentKeyProfile()->GetPlayerMappingRows();
	}

	TArray<FMapPlayerKeyArgs> LegacyPlayerKeyArgs = PlayerSettings_User.GetLegacyKeybindings();

	for (const TPair<FName, FKeyMappingRow>& Row : Rows)
	{
		if (Row.Value.Mappings.IsEmpty()) continue;
		FName MappingName = Row.Key;
		FText DisplayCategory = Row.Value.Mappings.Array()[0].GetDisplayCategory();
		FText DisplayName = Row.Value.Mappings.Array()[0].GetDisplayName();

		UInputMappingWidget* Widget = InputMappingWidgetMap.FindRef(MappingName);
		if (!Widget)
		{
			Widget = CreateWidget<UInputMappingWidget>(this, InputMappingWidgetClass);
			Widget->SetMappingName(MappingName, DisplayName);
			Widget->OnKeySelected.AddUObject(this, &ThisClass::OnKeySelected);
			Widget->OnIsSelectingKey.AddUObject(this, &ThisClass::OnIsSelectingKey);
			InputMappingWidgetMap.Add(Row.Key, Widget);
			if (DisplayCategory.EqualTo(FText::FromString("Combat")))
			{
				BSBox_Combat->AddChildToVerticalBox(Widget);
			}
			else if (DisplayCategory.EqualTo(FText::FromString("Movement")))
			{
				BSBox_Movement->AddChildToVerticalBox(Widget);
			}
			else if (DisplayCategory.EqualTo(FText::FromString("General")))
			{
				BSBox_General->AddChildToVerticalBox(Widget);
			}
		}

		if (!LegacyPlayerKeyArgs.IsEmpty())
		{
			const FMapPlayerKeyArgs* MatchingLegacy = LegacyPlayerKeyArgs.FindByPredicate(
				[&MappingName](const FMapPlayerKeyArgs& MapPlayerKeyArgs)
				{
					return MapPlayerKeyArgs.MappingName == MappingName;
				});

			for (const auto& KeyMapping : Row.Value.Mappings)
			{
				if (MatchingLegacy && MatchingLegacy->Slot == KeyMapping.GetSlot() && MatchingLegacy->NewKey !=
					KeyMapping.GetCurrentKey())
				{
					Widget->SetKeyForSlot(MatchingLegacy->Slot, MatchingLegacy->NewKey);
				}
				else
				{
					Widget->SetKeyForSlot(KeyMapping.GetSlot(), KeyMapping.GetCurrentKey());
				}
			}
			SavePlayerSettings(PlayerSettings_User);
		}
		else
		{
			for (const auto& KeyMapping : Row.Value.Mappings)
			{
				Widget->SetKeyForSlot(KeyMapping.GetSlot(), KeyMapping.GetCurrentKey());
			}
		}
	}

	UpdateBrushColors();
}

void USettingsMenuWidget_Input::OnKeySelected(const FName MappingName, const EPlayerMappableKeySlot& InSlot,
	const FInputChord SelectedKey)
{
	// UE_LOG(LogTemp, Display, TEXT("NewKeySelected for %s at Slot %s: %s"), *MappingName.ToString(),
	//	 *UEnum::GetDisplayValueAsText(InSlot).ToString(), *SelectedKey.GetInputText().ToString());

	if (SelectedKey.Key == FKey()) return;

	for (const TPair<FName, UInputMappingWidget*>& WidgetMapping : InputMappingWidgetMap)
	{
		FName CurrentMapping = WidgetMapping.Key;
		UInputMappingWidget* Widget = WidgetMapping.Value;

		// Iterate through matching Input Keys (2 max)
		for (EPlayerMappableKeySlot KeySlot : Widget->GetSlotsFromKey(SelectedKey.Key))
		{
			// Skip self
			if (CurrentMapping == MappingName && KeySlot == InSlot) continue;
			// Skip if Fire or KnifeAttack since these are bound to same by default
			if (MappingName == FName("Fire") && CurrentMapping == FName("KnifeAttack")) continue;
			if (MappingName == FName("KnifeAttack") && CurrentMapping == FName("Fire")) continue;

			// Unbind old key
			Widget->SetKeyForSlot(KeySlot, FKey());
		}
	}
}

void USettingsMenuWidget_Input::OnIsSelectingKey(UInputKeySelector* KeySelector)
{
	// Lame workaround to allow Mouse Wheel Up and Mouse Wheel Down for InputKeySelectors
	if (KeySelector == CurrentKeySelector)
	{
		bIsSelectingKey = false;
		CurrentKeySelector = nullptr;
		ScrollBox->SetConsumeMouseWheel(EConsumeMouseWheel::WhenScrollingPossible);
		return;
	}

	ScrollBox->SetConsumeMouseWheel(EConsumeMouseWheel::Never);
	bIsSelectingKey = true;
	CurrentKeySelector = KeySelector;
}

FReply USettingsMenuWidget_Input::NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// Lame workaround to allow Mouse Wheel Up and Mouse Wheel Down for InputKeySelectors
	if (bIsSelectingKey && CurrentKeySelector)
	{
		FInputChord Chord;
		if (InMouseEvent.GetWheelDelta() > 0)
		{
			Chord.Key = FKey(EKeys::MouseScrollUp);
		}
		else
		{
			Chord.Key = FKey(EKeys::MouseScrollDown);
		}
		CurrentKeySelector->SetSelectedKey(Chord);
		SetFocus();
		bIsSelectingKey = false;
		ScrollBox->SetConsumeMouseWheel(EConsumeMouseWheel::WhenScrollingPossible);
	}
	return Super::NativeOnMouseWheel(InGeometry, InMouseEvent);
}

void USettingsMenuWidget_Input::OnSliderTextBoxValueChanged(USliderTextBoxOptionWidget* Widget, const float Value)
{
	if (Widget == MenuOption_NewSensitivity)
	{
		MenuOption_NewSensitivityCsgo->SetValue(Value * Constants::CsgoMultiplier);
		Sensitivity = Value;
	}
	else if (Widget == MenuOption_NewSensitivityCsgo)
	{
		MenuOption_NewSensitivity->SetValue(Value / Constants::CsgoMultiplier);
		Sensitivity = Value / Constants::CsgoMultiplier;
	}
}

void USettingsMenuWidget_Input::OnButtonClicked_BSButton(const UBSButton* Button)
{
	switch (static_cast<ESettingButtonType>(Button->GetEnumValue()))
	{
	case ESettingButtonType::Save:
		OnButtonClicked_Save();
		break;
	case ESettingButtonType::Reset:
		OnButtonClicked_Reset();
		break;
	case ESettingButtonType::Revert:
		OnButtonClicked_Revert();
		break;
	default:
		break;
	}
}

void USettingsMenuWidget_Input::OnButtonClicked_Save()
{
	FPlayerSettings_User Settings_User = LoadPlayerSettings().User;
	Settings_User.Sensitivity = Sensitivity;
	MenuOption_CurrentSensitivity->EditableTextBox->SetText(FText::AsNumber(Sensitivity));

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
		GetOwningPlayer()->GetLocalPlayer());

	for (const TPair<FName, UInputMappingWidget*>& WidgetMapping : InputMappingWidgetMap)
	{
		FMapPlayerKeyArgs Args;
		FGameplayTagContainer Failure;

		Args.MappingName = WidgetMapping.Key;

		Args.NewKey = WidgetMapping.Value->GetKeyFromSlot(EPlayerMappableKeySlot::First).Key;;
		Args.Slot = EPlayerMappableKeySlot::First;
		Subsystem->GetUserSettings()->MapPlayerKey(Args, Failure);

		Args.NewKey = WidgetMapping.Value->GetKeyFromSlot(EPlayerMappableKeySlot::Second).Key;
		Args.Slot = EPlayerMappableKeySlot::Second;
		Subsystem->GetUserSettings()->MapPlayerKey(Args, Failure);
	}

	Subsystem->GetUserSettings()->SaveSettings();
	InitialPlayerMappingRows = Subsystem->GetUserSettings()->GetCurrentKeyProfile()->GetPlayerMappingRows();
	SavePlayerSettings(Settings_User);

	SavedTextWidget->SetSavedText(GetWidgetTextFromKey("SM_Saved_Input"));
	SavedTextWidget->PlayFadeInFadeOut();
}

void USettingsMenuWidget_Input::OnButtonClicked_Reset()
{
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
		GetOwningPlayer()->GetLocalPlayer());
	UEnhancedPlayerMappableKeyProfile* KeyProfile = Subsystem->GetUserSettings()->GetCurrentKeyProfile();
	KeyProfile->ResetToDefault();
	Subsystem->GetUserSettings()->SaveSettings();
	InitializeInputSettings();
}

void USettingsMenuWidget_Input::OnButtonClicked_Revert()
{
	InitializeInputSettings(InitialPlayerMappingRows);
}
