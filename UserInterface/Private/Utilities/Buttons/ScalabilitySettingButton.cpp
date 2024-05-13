// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "Utilities/Buttons/ScalabilitySettingButton.h"

void UScalabilitySettingButton::SetDefaults(const EVideoSettingType InVideoSettingType, const uint8 VideoSettingQuality,
	UBSButton* NextButton)
{
	SettingType = InVideoSettingType;
	Quality = VideoSettingQuality;
	Next = NextButton;
	SetHasSetDefaults(true);
}
