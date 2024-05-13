// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Styles/LoadingScreenStyle.h"
#include "Widgets/SCompoundWidget.h"


UENUM(BlueprintType)
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
class USERINTERFACE_API SLoadingScreenWidget : public SCompoundWidget
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
	void Construct(const FArguments& InArgs);

	/** Paints the widget on tick. */
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
		FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle,
		bool bParentEnabled) const override;

	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	/** Begins fading the MainOverlay, revealing the black background. */
	void SetLoadingScreenState(const ELoadingScreenState& InLoadingScreenState) const;

	/** Sets the opacity of the MainOverlay depending on LoadingScreenState. Callback for FadeTimerHandle. */
	void SetMainOverlayRenderOpacity(const double CurrentTime, const float DeltaTime) const;

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
