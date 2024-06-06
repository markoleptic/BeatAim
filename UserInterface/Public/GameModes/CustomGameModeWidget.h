// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BSGameModeConfig/BSGameModeValidator.h"
#include "Menus/GameModeMenuWidget.h"
#include "CustomGameModeWidget.generated.h"

enum class EGameModeCategory : uint8;
class UCustomGameModeCategoryWidget;
class UCustomGameModeStartWidget;

/** Contains data about the state of a CustomGameModesWidgetComponent. */
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

	/** Updates NumWarnings & NumCautions. Returns true if the values are different, otherwise false. */
	void Update(const uint32 InNumCautions, const uint32 InNumWarnings)
	{
		NumWarnings = InNumWarnings;
		NumCautions = InNumCautions;
	}

	/** Returns true if there are no warnings for this widget. Cautions are considered "valid". */
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
		return HashCombine(GetTypeHash(Value.NumWarnings), GetTypeHash(Value.NumCautions));
	}
};

/** Base class for the two types of CustomGameModesWidgets. */
UCLASS()
class USERINTERFACE_API UCustomGameModeWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Sets the value of BSConfig and GameModeDataAsset. Calls InitComponent on all widgets in ChildWidgets array. */
	virtual void Init(const TSharedPtr<FBSConfig>& InConfig);

	/** Calls UpdateOptionsFromConfig on all widgets in ChildWidgets array and calls
	 *  UpdateAllChildWidgetOptionsValid. */
	void UpdateOptionsFromConfig();

	/** Returns the NewCustomGameModeName from Widget_Start. */
	//FString GetNewCustomGameModeName() const;

	/** Sets the value of NewCustomGameModeName in Widget_Start. */
	//void SetNewCustomGameModeName(const FString& InCustomGameModeName) const;

	/** Returns the options for a start widget since they're not all shared with BSConfig pointer. */
	//FStartWidgetProperties GetStartWidgetProperties() const;

	/** Sets the options for a start widget since they're not all shared with BSConfig pointer. */
	//void SetStartWidgetProperties(const FStartWidgetProperties& InProperties);

	/** Returns whether all non Widget_Start child widget custom game mode options are valid. Iterates through
	 *  ChildWidgetValidityMap. */
	//bool GetAllNonStartChildWidgetOptionsValid() const;

	/** Clears all GameModeTemplate ComboBox options and repopulates. */
	//void RefreshGameModeTemplateComboBoxOptions(const TArray<FBSConfig>& CustomGameModes) const;

	TDelegate<void(const TSet<FPropertyHash>&)> OnPropertyChanged;

	UCustomGameModeStartWidget* GetStartWidget() const;

	TArray<TObjectPtr<UCustomGameModeCategoryWidget>> GetCustomGameModeCategoryWidgets() const;

protected:
	/** Adds child widgets to ChildWidgets array, binds to Widget_Start's RequestGameModeTemplateUpdate. */
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	void HandlePropertyChanged(const TSet<FPropertyHash>& Properties);

	/** Called any time Widget_Start broadcasts their RequestGameModeTemplateUpdate delegate. */
	//virtual void OnRequestGameModeTemplateUpdate(const FString& InGameMode, const EGameModeDifficulty& Difficulty);

	/** Called any time Widget_Start broadcasts their OnCustomGameModeNameChanged delegate. */
	//void OnStartWidget_CustomGameModeNameChanged();

	/** Bound to all child widget's RequestComponentUpdate delegates. */
	//void OnRequestComponentUpdate();

	/** Bound to all child widget's OnRequestGameModePreview delegates. */
	//void OnRequestGameModePreviewUpdate();

	/** Updates the value of bContainsGameModeBreakingOption and broadcasts OnGameModeBreakingChange only if it is
	 *  different from the current value. */
	//void UpdateContainsGameModeBreakingOption(const bool bGameModeBreakingOptionPresent);

	/** Calls UpdateAllOptionsValid for each child widget. */
	//virtual void UpdateAllChildWidgetOptionsValid();

	//UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	//TObjectPtr<UCustomGameModeStartWidget> Widget_Start;

	/** Pointer to Game Mode Config held in controlling GameModesWidget. */
	//TSharedPtr<FBSConfig> BSConfig;

	TMap<EGameModeCategory, TObjectPtr<UCustomGameModeCategoryWidget>> GameModeCategoryWidgetMap;

	/** Maps each child widget to struct representing if all its custom game mode options are valid. */
	//TMap<TObjectPtr<UCustomGameModeCategoryWidget>, FCustomGameModeCategoryInfo*> ChildWidgetValidityMap;

	bool bIsUpdatingFromComponentRequest = false;
	bool bContainsGameModeBreakingOption = false;
};
