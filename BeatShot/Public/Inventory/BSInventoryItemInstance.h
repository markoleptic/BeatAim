// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "System/GameplayTagStack.h"
#include "UObject/Object.h"
#include "BSInventoryItemInstance.generated.h"

class UBSInventoryItemDefinition;
class UBSInventoryItemFragment;
struct FFrame;
struct FGameplayTag;

/** Represents a unique instanced inventory item. Basically just contains the item definition and the stack count. */
UCLASS(BlueprintType)
class BEATSHOT_API UBSInventoryItemInstance : public UObject
{
	GENERATED_BODY()

public:
	UBSInventoryItemInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~UObject interface
	virtual bool IsSupportedForNetworking() const override { return true; }
	//~End of UObject interface

	/** Add a unique tag to the tag container. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Inventory)
	void AddIdentifierTags(const FGameplayTagContainer& Tags);

	/** Remove a tag from the tag container. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Inventory)
	void RemoveIdentifierTag(const FGameplayTag& Tag);

	/** Returns true if the tag container contains the tag. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Inventory)
	bool HasIdentifierTag(const FGameplayTag& Tag);

	/** Adds a specified number of stacks to the tag (does nothing if StackCount is below 1). */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Inventory)
	void AddStatTagStack(FGameplayTag Tag, int32 StackCount);

	/** Removes a specified number of stacks from the tag (does nothing if StackCount is below 1). */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category= Inventory)
	void RemoveStatTagStack(FGameplayTag Tag, int32 StackCount);

	/** Returns the stack count of the specified tag (or 0 if the tag is not present). */
	UFUNCTION(BlueprintCallable, Category=Inventory)
	int32 GetStatTagStackCount(FGameplayTag Tag) const;

	/** Returns true if there is at least one stack of the specified tag. */
	UFUNCTION(BlueprintCallable, Category=Inventory)
	bool HasStatTag(FGameplayTag Tag) const;

	/** Returns the item definition for this instance. */
	TSubclassOf<UBSInventoryItemDefinition> GetItemDef() const { return ItemDef; }

	/** Returns the specified fragment of this instance if it exists. */
	UFUNCTION(BlueprintCallable, BlueprintPure=false, meta=(DeterminesOutputType=FragmentClass))
	const UBSInventoryItemFragment* FindFragmentByClass(TSubclassOf<UBSInventoryItemFragment> FragmentClass) const;

	/** Returns the specified fragment of this instance if it exists. */
	template <typename ResultClass>
	const ResultClass* FindFragmentByClass() const
	{
		if (const UBSInventoryItemFragment* Fragment = FindFragmentByClass(ResultClass::StaticClass()))
		{
			if (const ResultClass* Result = Cast<ResultClass>(Fragment))
			{
				return Result;
			}
		}
		return nullptr;
	}

private:
	/** Sets the item definition for this instance. */
	void SetItemDef(TSubclassOf<UBSInventoryItemDefinition> InDef);

	friend struct FBSInventoryList;

	/** Basic gameplay tag container used only for identifying types of equipment. */
	UPROPERTY(Replicated)
	FGameplayTagContainer IdentifierTags;

	/** Gameplay tag container for items with more extensive amount of stats, to be used with StatFragments. */
	UPROPERTY(Replicated)
	FGameplayTagStackContainer StatTags;

	/** The item definition. */
	UPROPERTY(Replicated)
	TSubclassOf<UBSInventoryItemDefinition> ItemDef;
};
