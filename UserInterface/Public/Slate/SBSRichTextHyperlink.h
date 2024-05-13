// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "Framework/Text/SlateHyperlinkRun.h"
#include "Widgets/Input/SButton.h"

enum class ETextFlowDirection : uint8;
enum class ETextShapingMethod : uint8;

/** Very similar to SRichTextHyperlink, but also changes the text color on hover. */
class SBSRichTextHyperlink : public SButton
{
public:
	SLATE_BEGIN_ARGS(SBSRichTextHyperlink)
			: _Text(), _Style(&FCoreStyle::Get().GetWidgetStyle<FHyperlinkStyle>("Hyperlink")), _TextStyle(nullptr),
			  _UnderlineStyle(nullptr), _Padding(), _OnNavigate(), _TextShapingMethod(), _TextFlowDirection(),
			  _HighlightColor(), _HighlightShape(), _HighlightText()
		{
		}

		SLATE_ATTRIBUTE(FText, Text)
		SLATE_STYLE_ARGUMENT(FHyperlinkStyle, Style)
		SLATE_STYLE_ARGUMENT(FTextBlockStyle, TextStyle)
		SLATE_STYLE_ARGUMENT(FButtonStyle, UnderlineStyle)
		SLATE_ATTRIBUTE(FMargin, Padding)
		SLATE_EVENT(FSimpleDelegate, OnNavigate)
		SLATE_ARGUMENT(TOptional<ETextShapingMethod>, TextShapingMethod)
		SLATE_ARGUMENT(TOptional<ETextFlowDirection>, TextFlowDirection)
		SLATE_ATTRIBUTE(FLinearColor, HighlightColor)
		SLATE_ATTRIBUTE(const FSlateBrush*, HighlightShape)
		SLATE_ATTRIBUTE(FText, HighlightText)
	SLATE_END_ARGS()

	/**
	 * Construct the hyperlink widgets from a declaration
	 * 
	 * @param InArgs    Widget declaration from which to construct the hyperlink.
	 * @param InViewModel View model
	 */
	void Construct(const FArguments& InArgs, const TSharedRef<FSlateHyperlinkRun::FWidgetViewModel>& InViewModel)
	{
		this->ViewModel = InViewModel;
		this->OnNavigate = InArgs._OnNavigate;
		check(InArgs._Style);
		this->ButtonStyle = InArgs._UnderlineStyle != nullptr ? InArgs._UnderlineStyle : &InArgs._Style->UnderlineStyle;
		const FTextBlockStyle* TextStyle = InArgs._TextStyle != nullptr ? InArgs._TextStyle : &InArgs._Style->TextStyle;
		const TAttribute<FMargin> Padding = InArgs._Padding.IsSet() ? InArgs._Padding : InArgs._Style->Padding;

		TextBlock = SNew(STextBlock)
			.TextStyle(TextStyle)
			.Text(InArgs._Text)
			.HighlightColor(InArgs._HighlightColor)
			.HighlightShape(InArgs._HighlightShape)
			.HighlightText(InArgs._HighlightText);

		SButton::Construct(
			SButton::FArguments().ContentPadding(Padding).ButtonStyle(this->ButtonStyle).
			                      OnClicked(this, &SBSRichTextHyperlink::Hyperlink_OnClicked).
			                      OnHovered(this, &SBSRichTextHyperlink::Hyperlink_OnHovered).OnUnhovered(this,
				                      &SBSRichTextHyperlink::Hyperlink_OnUnhovered).
			                      ForegroundColor(FSlateColor::UseForeground()).
			                      TextShapingMethod(InArgs._TextShapingMethod).TextFlowDirection(
				                      InArgs._TextFlowDirection)[TextBlock.ToSharedRef()]);
	}

	virtual FCursorReply OnCursorQuery(const FGeometry& MyGeometry, const FPointerEvent& CursorEvent) const override
	{
		return FCursorReply::Cursor(EMouseCursor::Hand);
	}

	virtual void OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		SButton::OnMouseEnter(MyGeometry, MouseEvent);
		ViewModel->SetIsHovered(IsHovered());
	}

	virtual void OnMouseLeave(const FPointerEvent& MouseEvent) override
	{
		SButton::OnMouseLeave(MouseEvent);
		ViewModel->SetIsHovered(IsHovered());
	}

	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		FReply Reply = SButton::OnMouseButtonDown(MyGeometry, MouseEvent);
		ViewModel->SetIsPressed(IsPressed());

		return Reply;
	}

	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		FReply Reply = SButton::OnMouseButtonUp(MyGeometry, MouseEvent);
		ViewModel->SetIsPressed(IsPressed());

		return Reply;
	}

protected:
	FReply Hyperlink_OnClicked()
	{
		OnNavigate.ExecuteIfBound();
		return FReply::Handled();
	}

	void Hyperlink_OnHovered()
	{
		if (ButtonStyle)
		{
			TextBlock->SetColorAndOpacity(ButtonStyle->Hovered.TintColor);
		}
	}

	void Hyperlink_OnUnhovered()
	{
		if (ButtonStyle)
		{
			TextBlock->SetColorAndOpacity(ButtonStyle->Normal.TintColor);
		}
	}

private:
	TSharedPtr<FSlateHyperlinkRun::FWidgetViewModel> ViewModel;
	FSimpleDelegate OnNavigate;
	TSharedPtr<STextBlock> TextBlock;
	const FButtonStyle* ButtonStyle = nullptr;
};
