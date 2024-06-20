// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "Overlays/GameModeSharingWidget.h"
#include "Components/MultiLineEditableTextBox.h"
#include "Utilities/BSWidgetInterface.h"
#include "Utilities/Buttons/BSButton.h"

FString UGameModeSharingWidget::GetImportString() const
{
	return MultilineTextBox->GetText().ToString();
}

void UGameModeSharingWidget::SetImportButton(TObjectPtr<UBSButton> InImportButton)
{
	ImportButton = InImportButton;
}

void UGameModeSharingWidget::NativeConstruct()
{
	Super::NativeConstruct();
	MultilineTextBox->SetHintText(IBSWidgetInterface::GetWidgetTextFromKey("GM_ExportCustomGameModeHint"));
	MultilineTextBox->OnTextChanged.AddDynamic(this, &ThisClass::OnTextChanged_MultilineTextBox);
}

void UGameModeSharingWidget::OnTextChanged_MultilineTextBox(const FText& NewText)
{
	if (ImportButton)
	{
		ImportButton->SetIsEnabled(!NewText.IsEmpty());
	}
}
