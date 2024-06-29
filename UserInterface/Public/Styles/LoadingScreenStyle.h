// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BSConstants.h"
#include "Styling/SlateWidgetStyle.h"
#include "Styling/SlateWidgetStyleContainerBase.h"
#include "LoadingScreenStyle.generated.h"

/** Structure that defines the style of the Slate Loading Screen Widget. */
USTRUCT(BlueprintType)
struct USERINTERFACE_API FLoadingScreenStyle : public FSlateWidgetStyle
{
	GENERATED_USTRUCT_BODY()

	FLoadingScreenStyle();
	virtual ~FLoadingScreenStyle() override;

	// FSlateWidgetStyle
	virtual void GetResources(TArray<const FSlateBrush*>& OutBrushes) const override;
	static const FName TypeName;
	virtual const FName GetTypeName() const override { return TypeName; };
	static const FLoadingScreenStyle& GetDefault();

	/** Static texture of the Background. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Background")
	FSlateBrush BackgroundImage;

	/** Static texture of the logo. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Logo")
	FSlateBrush LogoImageTexture;

	/** Dynamic Material Instance of the logo. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Logo")
	FSlateBrush LogoImage;

	/** Font for left brand text. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brand")
	FSlateFontInfo BrandFontLeft;

	/** Font for right brand text. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brand")
	FSlateFontInfo BrandFontRight;

	/** Spacing between left and right brand text. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brand")
	FVector2D BrandTextSpacing = FVector2D(30, 0);

	/** Left brand text color. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brand")
	FLinearColor BrandTextLeftColor = FLinearColor::White;

	/** Right brand text color. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brand")
	FLinearColor BrandTextRightColor = Constants::BeatShotBlue;

	/** Left brand text. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brand")
	FText BrandTextLeft;

	/** Right brand text. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brand")
	FText BrandTextRight;

	/** Padding for the overlay slot containing both logo and brand text. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "General")
	FMargin MainOverlaySlotPadding = FMargin(0, 0, 0, 128);

	/** Padding for the horizontal box slot containing both logo and brand text. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "General")
	FMargin HorizontalBoxSlotPadding = FMargin(32, 0, 0, 16);

	/** How long the fade from black transition takes. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "General")
	float FadeFromBlackDuration = 0.25f;

	/** How long the fade to black transition takes. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "General")
	float FadeToBlackDuration = 0.5f;

	FLoadingScreenStyle& SetBackgroundImage(const FSlateBrush& InBackgroundImage)
	{
		BackgroundImage = InBackgroundImage;
		return *this;
	}

	FLoadingScreenStyle& SetLogoImageTexture(const FSlateBrush& InTexture)
	{
		LogoImageTexture = InTexture;
		return *this;
	}

	FLoadingScreenStyle& SetLogoImage(const FSlateBrush& InLogoImage)
	{
		LogoImage = InLogoImage;
		return *this;
	}

	FLoadingScreenStyle& SetBrandFontLeft(const FSlateFontInfo& InFont)
	{
		BrandFontLeft = InFont;
		return *this;
	}

	FLoadingScreenStyle& SetBrandFontRight(const FSlateFontInfo& InFont)
	{
		BrandFontRight = InFont;
		return *this;
	}

	FLoadingScreenStyle& SetBrandTextSpacing(const FVector2D& InSpacing)
	{
		BrandTextSpacing = InSpacing;
		return *this;
	}

	FLoadingScreenStyle& SetBrandTextLeftColor(const FLinearColor& InColor)
	{
		BrandTextLeftColor = InColor;
		return *this;
	}

	FLoadingScreenStyle& SetBrandTextRightColor(const FLinearColor& InColor)
	{
		BrandTextRightColor = InColor;
		return *this;
	}

	FLoadingScreenStyle& SetBrandTextLeft(const FText& InText)
	{
		BrandTextLeft = InText;
		return *this;
	}

	FLoadingScreenStyle& SetBrandTextRight(const FText& InText)
	{
		BrandTextRight = InText;
		return *this;
	}

	FLoadingScreenStyle& SetMainOverlaySlotPadding(const FMargin& InMargin)
	{
		MainOverlaySlotPadding = InMargin;
		return *this;
	}

	FLoadingScreenStyle& SetHorizontalBoxSlotPadding(const FMargin& InMargin)
	{
		HorizontalBoxSlotPadding = InMargin;
		return *this;
	}

	FLoadingScreenStyle& SetFadeFromBlackDuration(const float& InDuration)
	{
		FadeFromBlackDuration = InDuration;
		return *this;
	}

	FLoadingScreenStyle& SetFadeToBlackDuration(const float& InDuration)
	{
		FadeToBlackDuration = InDuration;
		return *this;
	}
};

/** Wrapper for the structure that defines the style of the Slate Loading Screen Widget. Allows creating a
 *  USlateWidgetStyleAsset blueprint.
 */
UCLASS(BlueprintType, hidecategories=Object, MinimalAPI)
class ULoadingScreenWidgetStyle : public USlateWidgetStyleContainerBase
{
	GENERATED_BODY()

public:
	/** The actual data describing the widget appearance. */
	UPROPERTY(Category = "Style", EditAnywhere, BlueprintReadWrite, meta=(ShowOnlyInnerProperties))
	FLoadingScreenStyle WidgetStyle;

	virtual const struct FSlateWidgetStyle* const GetStyle() const override
	{
		return static_cast<const struct FSlateWidgetStyle*>(&WidgetStyle);
	}
};
