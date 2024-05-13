// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "Utilities/Buttons/MenuButton.h"

void UMenuButton::SetDefaults(UVerticalBox* BoxToShow, UMenuButton* NextButton)
{
	Box = BoxToShow;
	Next = NextButton;
	SetHasSetDefaults(true);
}
