// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "MenuOptions/CheckBoxWidget.h"
#include "Components/CheckBox.h"

void UCheckBoxWidget::NativeConstruct()
{
	Super::NativeConstruct();
	CheckBox->OnCheckStateChanged.AddUniqueDynamic(this, &ThisClass::OnCheckStateChanged_CheckBox);
}

void UCheckBoxWidget::OnCheckStateChanged_CheckBox(const bool bChecked)
{
}
