// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TooltipData.generated.h"

class UButton;
class UTooltipIcon;

/** Contains data for the tooltip of a widget. */
UCLASS()
class UTooltipData : public UObject
{
	GENERATED_BODY()

public:
	UTooltipData();

	/** @return formatted tooltip text according to tooltip data. */
	FText GetTooltipText() const;

	/** @return true if text wrapping is allowed. */
	bool GetAllowTextWrap() const;

	/** @return weak pointer to the tooltip icon this data is for. */
	TWeakObjectPtr<UTooltipIcon> GetTooltipIcon() const;

	/** Sets the tooltip icon this data is for.
	 *  @param InTooltipIcon 
	 */
	void SetTooltipIcon(const TWeakObjectPtr<UTooltipIcon>& InTooltipIcon);

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
	 * @param Args arguments to pass to the formatted text slots.
	 */
	template <typename... ArgTypes>
	void SetFormattedTooltipText(ArgTypes... Args);

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

	/** Weak pointer to the tooltip icon this data is for. */
	UPROPERTY()
	TWeakObjectPtr<UTooltipIcon> TooltipIcon;

	/** The text to display on the tooltip widget when a user hovers over the tooltip icon. */
	FText TooltipText;

	/** Cached formatted text expression. */
	FTextFormat FormattedText;

	/** Whether text wrapping is allowed on the tooltip widget. */
	bool bAllowTextWrap;

public:
	FORCEINLINE bool operator==(const UTooltipData& Other) const
	{
		return Id == Other.Id;
	}

	friend FORCEINLINE uint32 GetTypeHash(const UTooltipData& Value)
	{
		return GetTypeHash(Value.Id);
	}
};
