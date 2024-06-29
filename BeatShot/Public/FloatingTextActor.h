// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FloatingTextActor.generated.h"

class UCombatTextWidget;
class UWidgetComponent;

/** Provides a way to display floating text on player's screen showing their current target streak. */
UCLASS(Abstract)
class BEATSHOT_API AFloatingTextActor : public AActor
{
	GENERATED_BODY()

	AFloatingTextActor();

	/* Called when the game starts or when spawned. */
	virtual void BeginPlay() override;

	/* Called every frame. */
	virtual void Tick(float DeltaTime) override;

	virtual void PostInitializeComponents() override;

public:
	/** Sets the CombatTextWidgetComponent's widget's text. */
	void SetText(const FText& InText);

	FTransform GetTextTransform(const FTransform& InTargetTransform, const bool bDisplayAbove);

	UFUNCTION()
	void OnCombatTextFadeOutCompleted();

protected:
	UCombatTextWidget* GetCombatTextWidget() const;

	UPROPERTY(EditDefaultsOnly, Category = "BeatShot|Components")
	UWidgetComponent* CombatTextWidgetComponent;

	UPROPERTY(EditDefaultsOnly, Category = "BeatShot|Classes")
	TSubclassOf<UUserWidget> CombatTextWidgetClass;

	FText CombatText;

	bool bDisplayBelow;
};
