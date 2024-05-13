// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MenuOptionWidget.h"
#include "CheckBoxWidget.generated.h"

UCLASS()
class USERINTERFACE_API UCheckBoxWidget : public UMenuOptionWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCheckBox* CheckBox;

protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnCheckStateChanged_CheckBox(const bool bChecked);
};
