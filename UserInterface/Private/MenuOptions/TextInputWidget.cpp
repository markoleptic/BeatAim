// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "MenuOptions/TextInputWidget.h"
#include "Components/EditableTextBox.h"
#include "Styles/MenuOptionStyle.h"

void UTextInputWidget::SetStyling()
{
	Super::SetStyling();
	if (MenuOptionStyle)
	{
		if (EditableTextBox)
		{
			EditableTextBox->WidgetStyle.SetFont(MenuOptionStyle->Font_EditableText);
			EditableTextBox->WidgetStyle.SetPadding(MenuOptionStyle->Padding_EditableText);
			EditableTextBox->SetJustification(TextJustify_EditableTextBox);
		}
	}
}
