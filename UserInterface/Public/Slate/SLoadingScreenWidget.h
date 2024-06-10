// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Styles/LoadingScreenStyle.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Images/SSpinningImage.h"


UENUM()
enum class ELoadingScreenState : uint8
{
	None UMETA(DisplayName="None"),
	FadingIn UMETA(DisplayName="FadingIn"),
	FadingOut UMETA(DisplayName="FadingOut"),
	FullOpacity UMETA(DisplayName="FullOpacity")};

ENUM_RANGE_BY_FIRST_AND_LAST(ELoadingScreenState, ELoadingScreenState::None, ELoadingScreenState::FullOpacity);

DECLARE_DELEGATE(FOnFadeOutComplete);

/**
 * Slate loading screen widget. Used instead of UUserWidget since UUserWidgets do not like to tick from any thread
 * but the game thread, and the ASyncLoadingThread ticks widgets supplied to the MoviePlayer.
 */
class SLoadingScreenWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLoadingScreenWidget) : _LoadingScreenStyle(
		                                         &FAppStyle::Get().GetWidgetStyle<
			                                         FLoadingScreenStyle>("LoadingScreen")), _OnFadeOutComplete(),
	                                         _bIsInitialLoadingScreen(false)
		{
		}

		SLATE_STYLE_ARGUMENT(FLoadingScreenStyle, LoadingScreenStyle)
		SLATE_EVENT(FOnFadeOutComplete, OnFadeOutComplete)
		SLATE_ARGUMENT(bool, bIsInitialLoadingScreen)
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs. */
	void Construct(const FArguments& InArgs)
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
			SAssignNew(LogoImage, SSpinningImage)
			.Image(&LoadingScreenStyle->LogoImageTexture)
			.Period(4.0f);
		}
		else
		{
			SAssignNew(LogoImage, SImage)
			.Image(&LoadingScreenStyle->LogoImage);
		}

		ChildSlot
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SAssignNew(MainOverlay, SOverlay)
				.RenderOpacity(0.f)
				+ SOverlay::Slot()
				.ZOrder(10000)
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					SNew(SImage)
					.Image(&LoadingScreenStyle->BackgroundImage)
				]
				+ SOverlay::Slot()
				.ZOrder(10001)
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Bottom)
				.Padding(LoadingScreenStyle->MainOverlaySlotPadding)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Bottom)
					.Padding(LoadingScreenStyle->HorizontalBoxSlotPadding)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						[
							LogoImage
							.ToSharedRef()
						]
						+ SVerticalBox::Slot()
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Bottom)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.HAlign(HAlign_Fill)
							.VAlign(VAlign_Fill)
							[
								SNew(STextBlock)
								.Font(LoadingScreenStyle->BrandFontLeft)
								.Text(LoadingScreenStyle->BrandTextLeft)
								.ColorAndOpacity(LoadingScreenStyle->BrandTextLeftColor)
							]
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.HAlign(HAlign_Fill)
							.VAlign(VAlign_Fill)
							[
								SNew(SSpacer)
								.Size(LoadingScreenStyle->BrandTextSpacing)
							]
							+ SHorizontalBox::Slot()
							.AutoWidth()
							.HAlign(HAlign_Fill)
							.VAlign(VAlign_Fill)
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

	/** Paints the widget on tick. */
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
		FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle,
		bool bParentEnabled) const override
	{
		const int32 MaxLayerId = SCompoundWidget::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements,
			LayerId, InWidgetStyle, bParentEnabled);
		SetMainOverlayRenderOpacity(Args.GetCurrentTime(), Args.GetDeltaTime());
		return MaxLayerId;
	}

	/** Begins fading the MainOverlay, revealing the black background. */
	void SetLoadingScreenState(const ELoadingScreenState& InLoadingScreenState) const
	{
		if (LoadingScreenState == ELoadingScreenState::FadingIn && InLoadingScreenState ==
			ELoadingScreenState::FadingOut && !bStartFadeOutOnFadeInEnd)
		{
			bStartFadeOutOnFadeInEnd = true;
		}
		else
		{
			LoadingScreenState = InLoadingScreenState;
		}
	}

	/** Sets the opacity of the MainOverlay depending on LoadingScreenState. Callback for FadeTimerHandle. */
	void SetMainOverlayRenderOpacity(const double CurrentTime, const float DeltaTime) const
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

	/** Executed when the MainOverlay has completed fading to black. */
	FOnFadeOutComplete OnFadeOutComplete;

	/** The style of the loading screen. */
	const FLoadingScreenStyle* LoadingScreenStyle;

	/** The overlay containing the background and logo textures. */
	TSharedPtr<SOverlay> MainOverlay;

	/** The overlay containing the background and logo textures. */
	TSharedPtr<SImage> LogoImage;

	/** The opacity state of the MainOverlay. */
	mutable ELoadingScreenState LoadingScreenState = ELoadingScreenState::None;

	/** The time at which the widget began fading to black. */
	mutable double FadeStartTime = 0.0f;

	/** The time at which the widget will complete fading to black. */
	mutable double FadeEndTime = 0.0f;

	/** The current time or progress of fading to black. */
	mutable double FadeCurrentTime = 0.0f;

	/** Whether to immediately begin fading out once fading in is completed. */
	mutable bool bStartFadeOutOnFadeInEnd = false;

	/** Whether this is an initial loading screen. */
	bool bIsInitialLoadingScreen = false;
};
