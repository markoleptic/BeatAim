// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "Utilities/HyperlinkTextBlockDecorator.h"
#include "Slate/SBSRichTextHyperlink.h"


FHyperLinkTextBlockDecorator::FHyperLinkTextBlockDecorator(URichTextBlock* InOwner,
	UHyperlinkTextBlockDecorator* InDecorator) : FRichTextDecorator(InOwner), Decorator(InDecorator)
{
	OnHyperlinkClicked.BindLambda([this]()
	{
		OnBrowserLinkClicked();
	});
}

bool FHyperLinkTextBlockDecorator::Supports(const FTextRunParseResults& RunParseResult, const FString& Text) const
{
	if (RunParseResult.Name == TEXT("a") && RunParseResult.MetaData.Contains(TEXT("href")))
	{
		const FTextRange& IdRange = RunParseResult.MetaData[TEXT("href")];
		LinkText = Text.Mid(IdRange.BeginIndex, IdRange.EndIndex - IdRange.BeginIndex);
		return true;
	}
	return false;
}

void FHyperLinkTextBlockDecorator::OnBrowserLinkClicked() const
{
	if (!LinkText.IsEmpty())
	{
		FPlatformProcess::LaunchURL(*LinkText, nullptr, nullptr);
	}
}

TSharedPtr<SWidget> FHyperLinkTextBlockDecorator::CreateDecoratorWidget(const FTextRunInfo& RunInfo,
	const FTextBlockStyle& DefaultTextStyle) const
{
	TSharedPtr<FSlateHyperlinkRun::FWidgetViewModel> Model = MakeShareable(new FSlateHyperlinkRun::FWidgetViewModel);
	TSharedPtr<SBSRichTextHyperlink> Link = SNew(SBSRichTextHyperlink, Model.ToSharedRef())
		.Text(RunInfo.Content)
		.Style(&Decorator->Style)
		.OnNavigate(OnHyperlinkClicked);

	return Link;
}

UHyperlinkTextBlockDecorator::UHyperlinkTextBlockDecorator(const FObjectInitializer& ObjectInitializer): Super(
	ObjectInitializer)
{
}

TSharedPtr<ITextDecorator> UHyperlinkTextBlockDecorator::CreateDecorator(URichTextBlock* InOwner)
{
	return MakeShareable(new FHyperLinkTextBlockDecorator(InOwner, this));
}
