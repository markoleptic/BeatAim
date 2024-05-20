// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TooltipImage.generated.h"

class UButton;
class UTooltipImage;

/** The type of tooltip image. */
UENUM()
enum class ETooltipImageType : uint8
{
	Default UMETA(DisplayName="Default"),
	Caution UMETA(DisplayName="Caution"),
	Warning UMETA(DisplayName="Warning"),
};

ENUM_RANGE_BY_FIRST_AND_LAST(ETooltipImageType, ETooltipImageType::Default, ETooltipImageType::Warning);

/** Data structure to pass the actual value and the max allowed value. */
USTRUCT()
struct FDynamicTooltipState
{
	GENERATED_BODY()

	float Actual;
	float MaxAllowed;
	bool bOverride;

	FDynamicTooltipState()
	{
		Actual = -1.f;
		MaxAllowed = -1.f;
		bOverride = false;
	}

	FDynamicTooltipState(const float InActual, const float InMax, const bool bInOverride = false)
	{
		Actual = InActual;
		MaxAllowed = InMax;
		bOverride = bInOverride;
	}
};

DECLARE_DELEGATE_RetVal(bool, FUpdateTooltipState);
DECLARE_DELEGATE_RetVal(FDynamicTooltipState, FUpdateDynamicTooltipState);

/** Data structure for dynamic tooltip text. */
USTRUCT()
struct FDynamicTooltipData
{
	GENERATED_BODY()

	float MinAllowed;
	int32 Precision = 0;
	FText FallbackText;
	FString TryChangeString;

	FDynamicTooltipData();
	FDynamicTooltipData(const float InMin, const FString& InFallbackStringTableKey);
};

/** Contains data for the tooltip of a widget. */
USTRUCT(BlueprintType)
struct FTooltipData
{
	GENERATED_BODY()

	/** Weak pointer to the tooltip this data is for. */
	UPROPERTY()
	TWeakObjectPtr<UTooltipImage> TooltipImage;

	/** The text to display on the TooltipImage. */
	FText TooltipText;

	/** whether to allow Auto Wrap text. */
	bool bAllowTextWrap;

	/** The type of tooltip. */
	ETooltipImageType TooltipType;

	/** The String Table Key to fetch the TooltipText from. */
	FString TooltipStringTableKey;

	/** Additional text to append to TooltipText. */
	FText AdditionalTooltipText;

	/** Optional delegate that can be used to call SetShouldShowTooltipImage by executing it. */
	FUpdateTooltipState UpdateTooltipState;

	/** Optional delegate that can be used to call UpdateDynamicTooltipText by executing it. */
	FUpdateDynamicTooltipState UpdateDynamicTooltipState;

	FTooltipData();
	FTooltipData(const FText& InTooltipText, const bool InbAllowTextWrap);
	FTooltipData(const FString& InStringTableKey, const ETooltipImageType& InType,
		const FText& InAdditionalTooltipText = FText());

	/** Returns true if SetDynamicData was called. */
	bool IsDynamic() const { return bIsDynamic; }

	/** Returns true if the TooltipImage should be shown. */
	bool ShouldShowTooltipImage() const { return bShouldShowTooltipImage; }

	/** Returns true if TooltipStringTableKey is not empty. */
	bool HasStringTableKey() const { return !TooltipStringTableKey.IsEmpty(); }

	/** Returns true if the third construct was used. */
	bool HasBeenInitialized() const { return bHasBeenInitialized; }

	/** Returns true if the TooltipText == FallbackText. */
	bool IsUsingFallbackText() const { return TooltipText.EqualTo(DynamicTooltipData.FallbackText); }

	/** Sets up the DynamicTooltipData struct. Used for tooltips that should display a single float value. */
	void SetDynamicData(const float InMin, const FString& InFallbackStringTableKey, const int32 InPrecision = 0);

	/** Updates TooltipText by getting the TooltipText from a string table using TooltipStringTableKey and appending
	 *  any AdditionalTooltipText. */
	void UpdateTooltipData(const FTooltipData& InUpdateData);

	/** Sets up the value of bShouldShowTooltipImage. Sets bIsDirty to true if changed. */
	void SetShouldShowTooltipImage(const bool bShow);

	/** Updates TooltipText if Actual > MaxAllowed. Sets bIsDirty to true if changed. */
	void UpdateDynamicTooltipText(const FDynamicTooltipState& InUpdateData);

	/** Returns true if SetShouldShowTooltipImage changed the value of bShouldShowTooltipImage, or if
	 *  UpdateDynamicTooltipText changed the TooltipText. */
	bool IsDirty() const;

	/** Sets up the value of bIsDirty with the opposite. */
	void SetIsClean(const bool bClean) { bIsDirty = !bClean; }

	/** Removes the TooltipImage from parent and clears the TooltipImage pointer. */
	void RemoveTooltipImage();

private:
	/** Internal initialize. */
	void InitTooltipText();

	/** Optional Data structure for dynamic tooltip text. */
	FDynamicTooltipData DynamicTooltipData;

	/** whether the tooltip has been initialized. */
	bool bHasBeenInitialized;

	/** whether SetDynamicData was called. */
	bool bIsDynamic;

	/** whether the TooltipImage should be shown. */
	bool bShouldShowTooltipImage;

	/** Last value of InActual for a dynamic tooltip. */
	float LastActual;

	/** Last value of InMaxAllowed for a dynamic tooltip. */
	float LastMaxAllowed;

	/** whether the bShouldShowTooltipImage or TooltipText were updated but the tooltip still requires action. */
	bool bIsDirty;

public:
	FORCEINLINE bool operator==(const FTooltipData& Other) const
	{
		if (!TooltipStringTableKey.Equals(Other.TooltipStringTableKey))
		{
			return false;
		}
		if (TooltipType != Other.TooltipType)
		{
			return false;
		}
		return true;
	}

	friend FORCEINLINE uint32 GetTypeHash(const FTooltipData& Value)
	{
		return HashCombine(GetTypeHash(Value.TooltipStringTableKey), GetTypeHash(Value.TooltipType));
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTooltipHovered, UTooltipImage*, TooltipImage, const FTooltipData&,
	TooltipData);

/** The image to draw on a TooltipWidget. */
UCLASS()
class USERINTERFACE_API UTooltipImage : public UUserWidget
{
	GENERATED_BODY()

	virtual void NativeConstruct() override;

public:
	UFUNCTION()
	void OnTooltipImageHoveredCallback();

	/** Constructs the FTooltipData and sets the TooltipText that is accessed when the tooltip is hovered over. */
	void SetupTooltipImage(const FText& InText, const bool bAllowTextWrap = false);

	/** Sets the TooltipData that is accessed when the tooltip is hovered over. */
	void SetTooltipData(const FTooltipData& InTooltipData);

	/** Returns a pointer to the TooltipData. */
	FTooltipData* GetTooltipData() { return &TooltipData; }

	/** Returns the OnTooltipHovered delegate. */
	FOnTooltipHovered& GetTooltipHoveredDelegate() { return OnTooltipHovered; }

protected:
	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	UButton* Button;

	/** Info about what this Tooltip image should display. */
	UPROPERTY()
	FTooltipData TooltipData;

	/** Called when Button is hovered over, provides the tooltip data that should be displayed. */
	FOnTooltipHovered OnTooltipHovered;
};
