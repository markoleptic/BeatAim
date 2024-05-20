// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "FloatingTextActor.h"
#include "BSConstants.h"
#include "Components/WidgetComponent.h"
#include "Overlays/CombatTextWidget.h"

AFloatingTextActor::AFloatingTextActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CombatTextWidgetComponent = CreateDefaultSubobject<UWidgetComponent>("Combat Text Widget Component");
	CombatTextWidgetComponent->SetDrawAtDesiredSize(true);
	CombatTextWidgetComponent->SetWidgetClass(CombatTextWidgetClass);
	CombatTextWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	RootComponent = CombatTextWidgetComponent;
}

void AFloatingTextActor::BeginPlay()
{
	Super::BeginPlay();

	if (UCombatTextWidget* CombatTextWidget = GetCombatTextWidget())
	{
		if (bDisplayBelow)
		{
			CombatTextWidget->ShowCombatTextBottom();
		}
		else
		{
			CombatTextWidget->ShowCombatText();
		}
	}
}

void AFloatingTextActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AFloatingTextActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (CombatTextWidgetComponent)
	{
		CombatTextWidgetComponent->InitWidget();
	}
	if (UCombatTextWidget* CombatTextWidget = GetCombatTextWidget())
	{
		CombatTextWidget->GetFadeOutDelegate().BindDynamic(this, &ThisClass::OnCombatTextFadeOutCompleted);
		CombatTextWidget->SetText(CombatText);
	}
}

void AFloatingTextActor::SetText(const FText& InText)
{
	CombatText = InText;
	if (const UCombatTextWidget* CombatTextWidget = GetCombatTextWidget())
	{
		CombatTextWidget->SetText(InText);
	}
}

FTransform AFloatingTextActor::GetTextTransform(const FTransform& InTargetTransform, const bool bDisplayAbove)
{
	bDisplayBelow = !bDisplayAbove;
	if (bDisplayAbove)
	{
		return FTransform(FVector(InTargetTransform.GetLocation().X, InTargetTransform.GetLocation().Y,
			InTargetTransform.GetLocation().Z + Constants::SphereTargetRadius * InTargetTransform.GetScale3D().Z));
	}
	return FTransform(FVector(InTargetTransform.GetLocation().X, InTargetTransform.GetLocation().Y,
		InTargetTransform.GetLocation().Z - Constants::SphereTargetRadius * InTargetTransform.GetScale3D().Z));
}

void AFloatingTextActor::OnCombatTextFadeOutCompleted()
{
	Destroy();
}

UCombatTextWidget* AFloatingTextActor::GetCombatTextWidget() const
{
	if (CombatTextWidgetComponent && CombatTextWidgetComponent->GetWidget())
	{
		return Cast<UCombatTextWidget>(CombatTextWidgetComponent->GetWidget());
	}
	return nullptr;
}
