// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TooltipData.generated.h"

struct FValidationCheckData;
class UButton;
class UTooltipIcon;

/** Contains data for the tooltip of a widget, such as the text. */
USTRUCT()
struct FTooltipData
{
	GENERATED_BODY()

public:
	FTooltipData();

	/** @return formatted tooltip text according to tooltip data. */
	FText GetTooltipText() const;

	/** @return true if text wrapping is allowed. */
	bool GetAllowTextWrap() const;

	/** @return weak pointer to the tooltip icon this data is for. */
	TObjectPtr<UTooltipIcon> GetTooltipIcon() const;

	/** Sets the tooltip icon this data is for.
	 *  @param InTooltipIcon the tooltip icon.
	 */
	void SetTooltipIcon(const TObjectPtr<UTooltipIcon>& InTooltipIcon);

	/** Sets the tooltip text variable directly, bypassing formatting.
	 *  @param InTooltipText text to move and set.
	 */
	void SetTooltipText(const FText& InTooltipText);

	/** Sets whether text wrapping is allowed on the tooltip widget. 
	 *  @param InbAllowTextWrap whether to allow.
	 */
	void SetAllowTextWrap(bool InbAllowTextWrap);

	/**
	 *  Compiles and initializes an FTextFormat for a formatted text.
	 *  @param InText the text to create the formatted text with.
	 */
	void CreateFormattedText(const FText& InText);

	/**
	 *  Sets the tooltip text using arguments.
	 *  @param Data data about the validation check
	 */
	void SetFormattedTooltipText(const FValidationCheckData& Data);

	/** @return an array of argument names for formatted text. */
	TArray<FString> GetFormattedTextArgs() const;

	/** @return number of arguments for formatted text. */
	int32 GetNumberOfFormattedTextArgs() const;

	/** @return true if FormattedText has been compiled. */
	bool HasFormattedText() const;

private:
	/** Unique ID for this tooltip */
	int32 Id;

	/** Static next ID for this class. */
	static int32 GId;

	/** Tooltip icon this data is for. */
	UPROPERTY()
	TObjectPtr<UTooltipIcon> TooltipIcon;

	/** The text to display on the tooltip widget when a user hovers over the tooltip icon. */
	UPROPERTY()
	FText TooltipText;

	/** Cached formatted text expression. */
	FTextFormat FormattedText;

	/** Whether text wrapping is allowed on the tooltip widget. */
	bool bAllowTextWrap;

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
