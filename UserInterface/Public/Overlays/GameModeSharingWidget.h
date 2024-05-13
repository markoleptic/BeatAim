// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PopupMessageWidget.h"
#include "GameModeSharingWidget.generated.h"

class UMultiLineEditableTextBox;

UCLASS()
class USERINTERFACE_API UGameModeSharingWidget : public UPopupMessageWidget
{
	GENERATED_BODY()

public:
	FString GetImportString() const;

	void SetImportButton(TObjectPtr<UBSButton> InImportButton);

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UMultiLineEditableTextBox* MultilineTextBox;

	UFUNCTION()
	void OnTextChanged_MultilineTextBox(const FText& NewText);

	UPROPERTY()
	TObjectPtr<UBSButton> ImportButton;
};
