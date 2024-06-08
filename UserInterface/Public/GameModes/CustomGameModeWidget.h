// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Menus/GameModeMenuWidget.h"
#include "CustomGameModeWidget.generated.h"

enum class EGameModeCategory : uint8;
class UCustomGameModeCategoryWidget;
class UCustomGameModeStartWidget;

/** Base class for the two types of CustomGameModesWidgets. */
UCLASS()
class USERINTERFACE_API UCustomGameModeWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Sets the value of BSConfig in all custom game mode category child widgets. */
	virtual void Init(const TSharedPtr<FBSConfig>& InConfig);

	/** Calls UpdateOptionsFromConfig on all custom game mode category child widgets. */
	void UpdateOptionsFromConfig();

	/** Executed when a all custom game mode category child widget changes a property. */
	TDelegate<void(const TSet<uint32>&)> OnPropertyChanged;

	/** @return the start widget for this custom game mode widget. */
	UCustomGameModeStartWidget* GetStartWidget() const;

	/** @return all custom game mode category child widgets. */
	TArray<TObjectPtr<UCustomGameModeCategoryWidget>> GetCustomGameModeCategoryWidgets() const;

protected:
	/** Adds child widgets to GameModeCategoryWidgetMap. */
	virtual void NativeConstruct() override;

	virtual void NativeDestruct() override;

	/** Executes OnPropertyChanged when a child widget changes a property.
	 *  @param Properties properties that were modified by a custom game mode category widget.
	 */
	void HandlePropertyChanged(const TSet<uint32>& Properties);

	/** Holds custom game mode category child widgets. */
	TMap<EGameModeCategory, TObjectPtr<UCustomGameModeCategoryWidget>> GameModeCategoryWidgetMap;
};
