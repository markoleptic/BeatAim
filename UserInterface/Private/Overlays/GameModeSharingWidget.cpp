// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "Overlays/GameModeSharingWidget.h"
#include "Components/MultiLineEditableTextBox.h"
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
	MultilineTextBox->SetHintText(FText::FromString("Paste an exported game mode here."));
	MultilineTextBox->OnTextChanged.AddDynamic(this, &ThisClass::OnTextChanged_MultilineTextBox);
}

void UGameModeSharingWidget::OnTextChanged_MultilineTextBox(const FText& NewText)
{
	if (ImportButton)
	{
		ImportButton->SetIsEnabled(!NewText.IsEmpty());
	}
}
