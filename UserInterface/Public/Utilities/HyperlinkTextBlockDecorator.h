// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/RichTextBlockDecorator.h"
#include "HyperlinkTextBlockDecorator.generated.h"

class UHyperlinkTextBlockDecorator;

/** Underlying RichTextDecorator for UHyperlinkTextBlockDecorator. */
class FHyperLinkTextBlockDecorator : public FRichTextDecorator
{
public:
	FHyperLinkTextBlockDecorator(URichTextBlock* InOwner, UHyperlinkTextBlockDecorator* InDecorator);

	virtual bool Supports(const FTextRunParseResults& RunParseResult, const FString& Text) const override;

	virtual TSharedPtr<SWidget> CreateDecoratorWidget(const FTextRunInfo& RunInfo,
		const FTextBlockStyle& DefaultTextStyle) const override;

private:
	void OnBrowserLinkClicked() const;

	TDelegate<void()> OnHyperlinkClicked;
	UHyperlinkTextBlockDecorator* Decorator;
	mutable FString LinkText;
};

/** Opens RichText links in an external browser. */
UCLASS()
class USERINTERFACE_API UHyperlinkTextBlockDecorator : public URichTextBlockDecorator
{
	GENERATED_BODY()

public:
	UHyperlinkTextBlockDecorator(const FObjectInitializer& ObjectInitializer);

	virtual TSharedPtr<ITextDecorator> CreateDecorator(URichTextBlock* InOwner) override;

	UPROPERTY(EditAnywhere, Category = Appearance)
	FHyperlinkStyle Style;
};
