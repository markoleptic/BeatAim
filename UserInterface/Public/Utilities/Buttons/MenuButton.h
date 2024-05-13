// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BSButton.h"
#include "MenuButton.generated.h"

class UVerticalBox;

/** Button used for most major menu buttons in the user interface. */
UCLASS()
class USERINTERFACE_API UMenuButton : public UBSButton
{
	GENERATED_BODY()

public:
	virtual UMenuButton* GetNext() const override { return Cast<UMenuButton>(Next); }

	/** Sets the vertical box to associate with this button. */
	void SetDefaults(UVerticalBox* BoxToShow, UMenuButton* NextButton);

	/** Returns the vertical box to associate with this button. */
	UVerticalBox* GetBox() const
	{
		if (Box && Box.Get())
		{
			return Box.Get();
		}
		return nullptr;
	}

protected:
	/** The vertical box to associate with this button. */
	UPROPERTY()
	TObjectPtr<UVerticalBox> Box;
};
