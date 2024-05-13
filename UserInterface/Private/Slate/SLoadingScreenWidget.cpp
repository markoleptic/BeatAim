// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "Slate/SLoadingScreenWidget.h"
#include "Widgets/Images/SSpinningImage.h"

int32 SLoadingScreenWidget::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId,
	const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	SetMainOverlayRenderOpacity(Args.GetCurrentTime(), Args.GetDeltaTime());
	return SCompoundWidget::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle,
		bParentEnabled);
}

void SLoadingScreenWidget::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
}

void SLoadingScreenWidget::SetLoadingScreenState(const ELoadingScreenState& InLoadingScreenState) const
{
	if (LoadingScreenState == ELoadingScreenState::FadingIn && InLoadingScreenState == ELoadingScreenState::FadingOut &&
		!bStartFadeOutOnFadeInEnd)
	{
		bStartFadeOutOnFadeInEnd = true;
	}
	else
	{
		LoadingScreenState = InLoadingScreenState;
	}
}

void SLoadingScreenWidget::SetMainOverlayRenderOpacity(const double CurrentTime, const float DeltaTime) const
{
	if (!MainOverlay.IsValid())
	{
		return;
	}

	switch (LoadingScreenState)
	{
	case ELoadingScreenState::None:
		MainOverlay->SetRenderOpacity(0.f);
		return;
	case ELoadingScreenState::FadingIn:
		{
			// Tick before fading in begins
			if (FadeStartTime == 0)
			{
				FadeStartTime = CurrentTime;
				FadeCurrentTime = CurrentTime;
				FadeEndTime = FadeStartTime + LoadingScreenStyle->FadeFromBlackDuration;
				return;
			}
			// Fading from black
			if (FadeStartTime > 0)
			{
				FadeCurrentTime += DeltaTime;
				const float Opacity = (FadeCurrentTime - FadeStartTime) / LoadingScreenStyle->FadeFromBlackDuration;
				if (Opacity >= 0.f && Opacity < 1.f)
				{
					MainOverlay->SetRenderOpacity(Opacity);
					return;
				}

				// Fade from black completed, check if should fade in immediately
				if (bStartFadeOutOnFadeInEnd)
				{
					SetLoadingScreenState(ELoadingScreenState::FadingOut);
					FadeStartTime = 0.f;
					return;
				}

				// If not fade in immediately, set state to FullOpacity and reset FadeStartTime
				SetLoadingScreenState(ELoadingScreenState::FullOpacity);
				FadeStartTime = 0.f;
			}
		}
		break;
	case ELoadingScreenState::FadingOut:
		{
			// Tick before fading out begins
			if (FadeStartTime == 0)
			{
				if (LoadingScreenStyle->FadeToBlackDuration == 0.f)
				{
					SetLoadingScreenState(ELoadingScreenState::None);
					FadeStartTime = 0.f;
					if (OnFadeOutComplete.IsBound())
					{
						OnFadeOutComplete.Execute();
					}
				}
				FadeStartTime = CurrentTime;
				FadeCurrentTime = CurrentTime;
				FadeEndTime = FadeStartTime + LoadingScreenStyle->FadeToBlackDuration;
				return;
			}
			// Fading to black
			if (FadeStartTime > 0)
			{
				FadeCurrentTime += DeltaTime;
				const float Opacity = (FadeCurrentTime - FadeStartTime) / LoadingScreenStyle->FadeToBlackDuration;
				if (Opacity > 0.f && Opacity <= 1.f)
				{
					MainOverlay->SetRenderOpacity(1 - Opacity);
					return;
				}

				// Fade to black completed, broadcast delegate so loading screen can be removed
				SetLoadingScreenState(ELoadingScreenState::None);
				FadeStartTime = 0.f;
				if (OnFadeOutComplete.IsBound())
				{
					OnFadeOutComplete.Execute();
				}
			}
		}
		break;
	case ELoadingScreenState::FullOpacity:
		MainOverlay->SetRenderOpacity(1);
	}
}

void SLoadingScreenWidget::Construct(const FArguments& InArgs)
{
	OnFadeOutComplete = InArgs._OnFadeOutComplete;
	LoadingScreenStyle = InArgs._LoadingScreenStyle;
	bIsInitialLoadingScreen = InArgs._bIsInitialLoadingScreen;

	if (LoadingScreenStyle->FadeFromBlackDuration == 0.f)
	{
		SetLoadingScreenState(ELoadingScreenState::FullOpacity);
	}
	else
	{
		SetLoadingScreenState(ELoadingScreenState::FadingIn);
	}

	// Use fallback since material instance won't load on initial
	if (bIsInitialLoadingScreen)
	{
		SAssignNew(LogoImage, SSpinningImage).Image(&LoadingScreenStyle->LogoImageTexture).Period(4.0f);
	}
	else
	{
		SAssignNew(LogoImage, SImage).Image(&LoadingScreenStyle->LogoImage);
	}

	ChildSlot
	[
		SNew(SOverlay)
		+ SOverlay::Slot()
		[
			SAssignNew(MainOverlay, SOverlay).RenderOpacity(0.f)
			+ SOverlay::Slot().ZOrder(10000).HAlign(HAlign_Fill).VAlign(VAlign_Fill)
			[
				SNew(SImage).Image(&LoadingScreenStyle->BackgroundImage)
			]
			+ SOverlay::Slot().ZOrder(10001).HAlign(HAlign_Fill).VAlign(VAlign_Bottom).Padding(
				LoadingScreenStyle->MainOverlaySlotPadding)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().HAlign(HAlign_Fill).VAlign(VAlign_Bottom).Padding(
					LoadingScreenStyle->HorizontalBoxSlotPadding)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).VAlign(VAlign_Center)
					[
						LogoImage.ToSharedRef()
					]
					+ SVerticalBox::Slot().HAlign(HAlign_Center).VAlign(VAlign_Bottom)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().HAlign(HAlign_Fill).VAlign(VAlign_Fill)
						[
							SNew(STextBlock)
							.Font(LoadingScreenStyle->BrandFontLeft)
							.Text(LoadingScreenStyle->BrandTextLeft)
							.ColorAndOpacity(LoadingScreenStyle->BrandTextLeftColor)
						]
						+ SHorizontalBox::Slot().AutoWidth().HAlign(HAlign_Fill).VAlign(VAlign_Fill)
						[
							SNew(SSpacer).Size(LoadingScreenStyle->BrandTextSpacing)
						]
						+ SHorizontalBox::Slot().AutoWidth().HAlign(HAlign_Fill).VAlign(VAlign_Fill)
						[
							SNew(STextBlock)
							.Font(LoadingScreenStyle->BrandFontRight)
							.Text(LoadingScreenStyle->BrandTextRight)
							.ColorAndOpacity(LoadingScreenStyle->BrandTextRightColor)
						]
					]
				]
			]
		]
	];
}
