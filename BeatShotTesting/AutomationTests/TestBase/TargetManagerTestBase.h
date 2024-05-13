// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

namespace TargetManagerTestHelpers
{
	inline const TCHAR* DefaultGameModeDataAssetPath = TEXT(
		"/Game/Blueprints/GameModes/DA_DefaultGameModes.DA_DefaultGameModes");
	inline const TCHAR* EnumTagMapDataAssetPath = TEXT("/Game/Blueprints/DA_EnumTagMap.DA_EnumTagMap");
	inline const TCHAR* TargetManagerAssetPath = TEXT("/Game/Blueprints/Targets/BP_TargetManager.BP_TargetManager");
	inline const TCHAR* TargetAssetPath = TEXT("/Game/Blueprints/Targets/BP_Target.BP_Target");
}

class ATarget;
class ATargetManager;
class USpawnAreaManagerComponent;

/** Base class for unit tests dealing with the TargetManager. */
class FTargetManagerTestBase : public FAutomationTestBase
{
public:
	FTargetManagerTestBase(const FString& InName, const bool bInComplexTask) : FAutomationTestBase(InName,
		bInComplexTask)
	{
	}

	virtual ~FTargetManagerTestBase() override
	{
	}

	virtual uint32 GetTestFlags() const override
	{
		return EAutomationTestFlags::CommandletContext | EAutomationTestFlags::EditorContext |
			EAutomationTestFlags::HighPriorityAndAbove | EAutomationTestFlags::EngineFilter;
	}

	virtual bool IsStressTest() const { return false; }
	virtual uint32 GetRequiredDeviceNum() const override { return 1; }
	virtual FString GetBeautifiedTestName() const override { return "TargetManager"; }

	/** Initializes stuff. */
	virtual bool Init();
};
