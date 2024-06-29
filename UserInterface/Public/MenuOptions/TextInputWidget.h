// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MenuOptionWidget.h"
#include "TextInputWidget.generated.h"

UCLASS()
class USERINTERFACE_API UTextInputWidget : public UMenuOptionWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UEditableTextBox* EditableTextBox;

	/** The text alignment for the EditableTextBox. */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "TextInputWidget")
	TEnumAsByte<ETextJustify::Type> TextJustify_EditableTextBox = ETextJustify::Type::Left;

protected:
	virtual void SetStyling() override;
};
