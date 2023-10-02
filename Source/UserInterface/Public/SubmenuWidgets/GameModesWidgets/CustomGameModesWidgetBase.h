﻿// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BSGameModeDataAsset.h"
#include "SaveLoadInterface.h"
#include "Blueprint/UserWidget.h"
#include "SubMenuWidgets/GameModesWidgets/GameModesWidget.h"
#include "CustomGameModesWidgetBase.generated.h"

class UCustomGameModesWidgetComponent;
class UCustomGameModesWidget_Start;

DECLARE_MULTICAST_DELEGATE(FRequestButtonStateUpdate);
DECLARE_MULTICAST_DELEGATE_TwoParams(FRequestGameModeTemplateUpdate, const FString& GameMode, const EGameModeDifficulty& Difficulty);
DECLARE_MULTICAST_DELEGATE(FRequestGameModePreviewUpdate);

/** Contains data about the state of a CustomGameModesWidgetComponent */
USTRUCT(BlueprintType)
struct FCustomGameModeCategoryInfo
{
	GENERATED_BODY()

	uint32 NumCautions;
	uint32 NumWarnings;
	
	FCustomGameModeCategoryInfo()
	{
		NumCautions = 0;
		NumWarnings = 0;
	}

	/** Updates NumWarnings & NumCautions. Returns true if the values are different, otherwise false */
	void Update(const uint32 InNumCautions, const uint32 InNumWarnings)
	{
		NumWarnings = InNumWarnings;
		NumCautions = InNumCautions;
	}

	/** Returns true if there are no warnings for this widget. Cautions are considered "valid" */
	bool GetAllOptionsValid() const
	{
		return NumWarnings == 0;
	}

	FORCEINLINE bool operator==(const FCustomGameModeCategoryInfo& Other) const
	{
		if (NumWarnings != Other.NumWarnings)
		{
			return false;
		}
		if (NumCautions != Other.NumCautions)
		{
			return false;
		}
		return true;
	}
	
	friend FORCEINLINE uint32 GetTypeHash(const FCustomGameModeCategoryInfo& Value)
	{
		return HashCombine(GetTypeHash(Value.NumWarnings), GetTypeHash(Value. NumCautions));
	}
};

/** Base class for the two types of CustomGameModesWidgets */
UCLASS()
class USERINTERFACE_API UCustomGameModesWidgetBase : public UUserWidget, public ISaveLoadInterface
{
	GENERATED_BODY()

public:
	/** Sets the value of BSConfig and GameModeDataAsset. Calls InitComponent on all widgets in ChildWidgets array */
	virtual void Init(FBSConfig* InConfig, const TObjectPtr<UBSGameModeDataAsset> InGameModeDataAsset);

	/** Calls UpdateOptionsFromConfig on all widgets in ChildWidgets array and calls UpdateAllChildWidgetOptionsValid */
	UFUNCTION()
	void UpdateOptionsFromConfig();
	
	/** Returns the NewCustomGameModeName from Widget_Start */
	FString GetNewCustomGameModeName() const;
	
	/** Sets the value of NewCustomGameModeName in Widget_Start */
	void SetNewCustomGameModeName(const FString& InCustomGameModeName) const;

	/** Returns the options for a start widget since they're not all shared with BSConfig pointer */
	FStartWidgetProperties GetStartWidgetProperties() const;

	/** Sets the options for a start widget since they're not all shared with BSConfig pointer */
	void SetStartWidgetProperties(const FStartWidgetProperties& InProperties);

	/** Returns whether or not all non Widget_Start child widget custom game mode options are valid. Iterates through ChildWidgetValidityMap */
	bool GetAllNonStartChildWidgetOptionsValid() const;

	/** Clears all GameModeTemplate ComboBox options and repopulates */
	void RefreshGameModeTemplateComboBoxOptions() const;
	
	/** Broadcast any time Widget_Start broadcasts their RequestGameModeTemplateUpdate delegate */
	FRequestGameModeTemplateUpdate RequestGameModeTemplateUpdate;

	/** Broadcast after calling UpdateAllOptionsValid on each child widget, or when the Widget_Start CustomGameModeName changes */
	FRequestButtonStateUpdate RequestButtonStateUpdate;

	/** Broadcast true if there is a game mode breaking option */
	FOnGameModeBreakingChange OnGameModeBreakingChange;

	/** Broadcast when a widget wants to refresh the preview after a change to the config */
	FRequestGameModePreviewUpdate RequestGameModePreviewUpdate;

	FBSConfig* GetBSConfig() const { return BSConfig; }

protected:
	/** Adds child widgets to ChildWidgets array, binds to Widget_Start's RequestGameModeTemplateUpdate */
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/** Override to provide widget with access to FindPresetGameMode functions from ISaveLoadInterface */
	virtual UBSGameModeDataAsset* GetGameModeDataAsset() const override;

	/** Called any time Widget_Start broadcasts their RequestGameModeTemplateUpdate delegate */
	virtual void OnRequestGameModeTemplateUpdate(const FString& InGameMode, const EGameModeDifficulty& Difficulty);
	
	/** Called any time Widget_Start broadcasts their OnCustomGameModeNameChanged delegate */
	void OnStartWidget_CustomGameModeNameChanged();
	
	/** Bound to all child widget's RequestComponentUpdate delegates */
	void OnRequestComponentUpdate();

	/** Bound to all child widget's OnRequestGameModePreview delegates */
	void OnRequestGameModePreviewUpdate();
	
	/** Adds a widget to ChildWidgets array. Binds to its RequestComponentUpdate delegate */
	void AddChildWidget(const TObjectPtr<UCustomGameModesWidgetComponent> Component);

	/** Updates the value of bContainsGameModeBreakingOption and broadcasts OnGameModeBreakingChange only if its different than the current value */
	void UpdateContainsGameModeBreakingOption(const bool bGameModeBreakingOptionPresent);

	/** Calls UpdateAllOptionsValid for each child widget. */
	virtual void UpdateAllChildWidgetOptionsValid();
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCustomGameModesWidget_Start> Widget_Start;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCustomGameModesWidgetComponent> Widget_SpawnArea;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCustomGameModesWidgetComponent> Widget_Activation;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCustomGameModesWidgetComponent> Widget_Deactivation;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCustomGameModesWidgetComponent> Widget_General;
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCustomGameModesWidgetComponent> Widget_Spawning;

	UPROPERTY()
	TObjectPtr<UBSGameModeDataAsset> GameModeDataAsset;

	/** Pointer to Game Mode Config held in controlling GameModesWidget*/
	FBSConfig* BSConfig;

	/** Maps each child widget to struct representing if all its custom game mode options are valid */
	TMap<TObjectPtr<UCustomGameModesWidgetComponent>, FCustomGameModeCategoryInfo*> ChildWidgetValidityMap;

	bool bIsUpdatingFromComponentRequest = false;
	bool bContainsGameModeBreakingOption = false;
};