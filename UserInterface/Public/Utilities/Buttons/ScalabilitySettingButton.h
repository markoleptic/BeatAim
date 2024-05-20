// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BSButton.h"
#include "Delegates/DelegateCombinations.h"
#include "Utilities/BSSettingTypes.h"
#include "ScalabilitySettingButton.generated.h"

/** Button representing a video setting designed to be stored in a linked list. Contains a pointer to the next one. */
UCLASS()
class USERINTERFACE_API UScalabilitySettingButton : public UBSButton
{
	GENERATED_BODY()

public:
	/** Create functions like this with additional parameters to store info about the button being pressed. Remember to
	 *  call SetHasSetDefaults in any child implementations. */
	void SetDefaults(const EVideoSettingType InVideoSettingType, const uint8 VideoSettingQuality,
		UBSButton* NextButton = nullptr);

	/** Returns the video setting type for this button widget. */
	EVideoSettingType GetVideoSettingType() const { return SettingType; }

	/** Returns the video setting quality for this button widget. */
	uint8 GetVideoSettingQuality() const { return Quality; }

	virtual UScalabilitySettingButton* GetNext() const override { return Cast<UScalabilitySettingButton>(Next); }

private:
	/** The video setting quality this button widget represents. */
	uint8 Quality;

	/** The video setting type this button widget represents. */
	EVideoSettingType SettingType;
};
