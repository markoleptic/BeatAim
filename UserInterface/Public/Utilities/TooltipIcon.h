// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TooltipIcon.generated.h"

class UTooltipData;
class UImage;
class UButton;

/** The type of tooltip icon. */
UENUM()
enum class ETooltipIconType : uint8
{
	Default UMETA(DisplayName="Default"),
	Caution UMETA(DisplayName="Caution"),
	Warning UMETA(DisplayName="Warning"),
};

ENUM_RANGE_BY_FIRST_AND_LAST(ETooltipIconType, ETooltipIconType::Default, ETooltipIconType::Warning);

DECLARE_MULTICAST_DELEGATE_OneParam(FOnTooltipIconHovered, const TObjectPtr<UTooltipData>&);

/** A button and image representing an icon that executes a delegate when hovered over. Stores the data it passes to
 *  
 */
UCLASS()
class USERINTERFACE_API UTooltipIcon : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativePreConstruct() override;

	virtual void NativeConstruct() override;

	virtual void PostInitProperties() override;

	/** Broadcasts OnTooltipHovered delegate. */
	UFUNCTION()
	void HandleTooltipHovered();

public:
	UTooltipIcon(const FObjectInitializer& ObjectInitializer);

	/** Creator function. Binds its OnTooltipIconHovered delegate to the global static tooltip widget.
	 *  @param InOwningObject the object to parent the tooltip icon to.
	 *  @param Type the type of tooltip icon to create.
	 *  @return new tooltip icon instance.
	 */
	static UTooltipIcon* CreateTooltipIcon(UUserWidget* InOwningObject, ETooltipIconType Type);

	/**
	 * Modifies the appearance of the icon.
	 * @param Type the type of tooltip icon, which dictates its appearance.
	 */
	void SetTooltipIconType(ETooltipIconType Type);

	/**
	 * Sets the tooltip text in TooltipData that is broadcast when the tooltip icon is hovered over.
	 * @param InText text to store in tooltip data tooltip text field.
	 * @param bAllowTextWrap whether to allow text wrapping in the tooltip.
	 */
	void SetTooltipText(const FText& InText, const bool bAllowTextWrap = false);

	/** Called when Button is hovered over, provides the tooltip data that should be displayed. */
	FOnTooltipIconHovered OnTooltipIconHovered;

	/** @return shared pointer of tooltip data. */
	TObjectPtr<UTooltipData> GetTooltipData() const;

protected:
	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	UButton* Button;

	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	UImage* Image;

	/** Data that is broadcast to the tooltip widget. */
	TObjectPtr<UTooltipData> TooltipData;

	/** Maps each tooltip icon type to a brush. */
	UPROPERTY(EditDefaultsOnly, Category="TooltipIcon")
	TMap<ETooltipIconType, FSlateBrush> TooltipIconBrushMap;

	/** The type of tooltip icon. This determines the appearance of the tooltip icon. */
	UPROPERTY(EditAnywhere, Category = "TooltipIcon")
	ETooltipIconType TooltipIconType;
};
