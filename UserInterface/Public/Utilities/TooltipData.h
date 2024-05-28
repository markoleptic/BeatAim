// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TooltipData.generated.h"

class UButton;
class UTooltipIcon;

/** The type of tooltip icon. */
UENUM()
enum class ETooltipIconType : uint8
{
	Default UMETA(DisplayName="Default"),
	Caution UMETA(DisplayName="Caution"),
	Warning UMETA(DisplayName="Warning"),
};

ENUM_RANGE_BY_FIRST_AND_LAST(ETooltipIconType, ETooltipIconType::Default, ETooltipIconType::Warning);

/** Contains data for the tooltip of a widget. */
USTRUCT(BlueprintType)
struct FTooltipData
{
	GENERATED_BODY()

	FTooltipData();

	explicit FTooltipData(UTooltipIcon* InTooltipIcon);

	FText GetTooltipText() const;

	bool GetAllowTextWrap() const;

	ETooltipIconType TooltipIconType;

	/** Weak pointer to the tooltip this data is for. */
	UPROPERTY()
	TWeakObjectPtr<UTooltipIcon> TooltipIcon;

	/** The text to display on the tooltip widget when a user hovers over the tooltip icon. */
	FText TooltipText;

	/** Whether to allow Auto Wrap text. */
	bool bAllowTextWrap;

private:
	int32 Id;

	static int32 GId;

public:
	FORCEINLINE bool operator==(const FTooltipData& Other) const
	{
		return Id == Other.Id;
	}

	friend FORCEINLINE uint32 GetTypeHash(const FTooltipData& Value)
	{
		return GetTypeHash(Value.Id);
	}
};
