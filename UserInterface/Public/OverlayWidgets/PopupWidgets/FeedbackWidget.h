﻿// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HttpRequestInterface.h"
#include "BSWidgetInterface.h"
#include "Blueprint/UserWidget.h"
#include "FeedbackWidget.generated.h"

class UMultiLineEditableTextBox;
class UTextBlock;
class UBSButton;
class UEditableTextBox;

/** Simple pop-up widget that sends feedback using an http request */
UCLASS()
class USERINTERFACE_API UFeedbackWidget : public UUserWidget, public IHttpRequestInterface
{
	GENERATED_BODY()

public:
	void ShowFeedbackWidget();

	/** Broadcast when the widget is hidden after animations have completed */
	FOnExitAnimationCompleted OnExitAnimationCompletedDelegate;

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* TextBlock_FeedbackResponseTitle;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* TextBlock_FeedbackResponseInfo;

	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	UEditableTextBox* Value_Title;
	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	UMultiLineEditableTextBox* Value_Content;

	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	UBSButton* Button_BugReport;
	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	UBSButton* Button_Feedback;

	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	UBSButton* Button_SubmitFeedback;
	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	UBSButton* Button_Back;
	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	UBSButton* Button_Okay;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* FadeOut;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* FadeOutResponse;

private:
	UFUNCTION()
	void OnButtonClicked_BSButton(const UBSButton* Button);
	UFUNCTION()
	void OnTextCommitted_Title(const FText& NewTitle, ETextCommit::Type CommitType);
	UFUNCTION()
	void OnTextCommitted_Content(const FText& NewContent, ETextCommit::Type CommitType);
	UFUNCTION()
	void PlayFadeIn() { PlayAnimationReverse(FadeOut); }

	UFUNCTION()
	void PlayFadeOut() { PlayAnimationForward(FadeOut); }

	UFUNCTION()
	void PlayFadeInResponse() { PlayAnimationReverse(FadeOutResponse); }

	UFUNCTION()
	void PlayFadeOutResponse() { PlayAnimationForward(FadeOutResponse); }

	/** Sets the response text based on http response */
	UFUNCTION()
	void OnPostFeedbackResponse(const bool bSuccess);

	/** Sets the widget's visibility to collapsed and unbinds FadeOutDelegate and FadeOutResponseDelegate */
	UFUNCTION()
	void SetCollapsedAndUnbindDelegates();

	/** Delegate passed to PostFeedback function that executes when an http response is received, bound to OnPostFeedbackResponse */
	FOnPostFeedbackResponse OnPostFeedbackResponseDelegate;

	/** Always bound to FadeOut animation and SetCollapsedAndUnbindDelegates */
	FWidgetAnimationDynamicEvent FadeOutDelegate;

	/** Always bound to FadeOutResponse animation and SetCollapsedAndUnbindDelegates */
	FWidgetAnimationDynamicEvent FadeOutResponseDelegate;

	FText TitlePrefix;
	FText BugReportTitle = FText::FromString("Bug Report: ");
	FText FeedbackTitle = FText::FromString("Feedback: ");
};
