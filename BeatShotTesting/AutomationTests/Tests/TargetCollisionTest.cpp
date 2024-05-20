// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

// ReSharper disable CppNonExplicitConvertingConstructor
// ReSharper disable CppClassCanBeFinal
#include "CoreMinimal.h"
#include "../TestBase/TargetManagerTestWithWorld.h"
#include "SaveGames/SaveGamePlayerScore.h"
#include "SaveGames/SaveGamePlayerSettings.h"
#include "Target/Target.h"
#include "Target/TargetManager.h"

using namespace Constants;

class FTargetCollisionTest : public FTargetManagerTestWithWorld
{
public:
	FTargetCollisionTest(const FString& InName) : FTargetManagerTestWithWorld(InName, true)
	{
		static_assert(
			(EAutomationTestFlags::CommandletContext | EAutomationTestFlags::EditorContext |
				EAutomationTestFlags::HighPriorityAndAbove | EAutomationTestFlags::EngineFilter) &
			EAutomationTestFlags::ApplicationContextMask,
			"AutomationTest has no application flag.  It shouldn't run.  See AutomationTest.h.");
		static_assert(
			(((EAutomationTestFlags::CommandletContext | EAutomationTestFlags::EditorContext |
					EAutomationTestFlags::HighPriorityAndAbove | EAutomationTestFlags::EngineFilter) &
				EAutomationTestFlags::FilterMask) == EAutomationTestFlags::SmokeFilter) || (((
					EAutomationTestFlags::CommandletContext | EAutomationTestFlags::EditorContext |
					EAutomationTestFlags::HighPriorityAndAbove | EAutomationTestFlags::EngineFilter) &
				EAutomationTestFlags::FilterMask) == EAutomationTestFlags::EngineFilter) || (((
					EAutomationTestFlags::CommandletContext | EAutomationTestFlags::EditorContext |
					EAutomationTestFlags::HighPriorityAndAbove | EAutomationTestFlags::EngineFilter) &
				EAutomationTestFlags::FilterMask) == EAutomationTestFlags::ProductFilter) || (((
					EAutomationTestFlags::CommandletContext | EAutomationTestFlags::EditorContext |
					EAutomationTestFlags::HighPriorityAndAbove | EAutomationTestFlags::EngineFilter) &
				EAutomationTestFlags::FilterMask) == EAutomationTestFlags::PerfFilter) || (((
					EAutomationTestFlags::CommandletContext | EAutomationTestFlags::EditorContext |
					EAutomationTestFlags::HighPriorityAndAbove | EAutomationTestFlags::EngineFilter) &
				EAutomationTestFlags::FilterMask) == EAutomationTestFlags::StressFilter) || (((
					EAutomationTestFlags::CommandletContext | EAutomationTestFlags::EditorContext |
					EAutomationTestFlags::HighPriorityAndAbove | EAutomationTestFlags::EngineFilter) &
				EAutomationTestFlags::FilterMask) == EAutomationTestFlags::NegativeFilter),
			"All AutomationTests must have exactly 1 filter type specified.  See AutomationTest.h.");
	}

	virtual uint32 GetTestFlags() const override
	{
		return ((EAutomationTestFlags::CommandletContext | EAutomationTestFlags::EditorContext |
			EAutomationTestFlags::HighPriorityAndAbove | EAutomationTestFlags::EngineFilter) & ~(
			EAutomationTestFlags::SmokeFilter));
	}

	virtual bool IsStressTest() const override { return true; }
	virtual uint32 GetRequiredDeviceNum() const override { return 1; }
	virtual FString GetTestSourceFileName() const override { return "TargetCollisionTest.cpp"; }
	virtual int32 GetTestSourceFileLine() const override { return 7; }

protected:
	virtual void GetTests(TArray<FString>& OutBeautifiedNames, TArray<FString>& OutTestCommands) const override;
	virtual bool RunTest(const FString& Parameters) override;
	virtual FString GetBeautifiedTestName() const override { return "TargetManager.TargetCollision"; }
	void TestIntersection(const FSphere& SphereOne, const FSphere& SphereTwo);
	void ResetTestVariables();

	int32 TotalTargetsSpawned = 0;
	int32 TotalCollisions = 0;
	double MinDistance = 5000.f;
	const int32 NumIterations = 500;
	const FString CollisionTestDataAssetPath = "/Game/Blueprints/GameModes/DA_CollisionTest.DA_CollisionTest";
};

namespace
{
	FTargetCollisionTest FTargetCollisionTestAutomationTestInstance(L"FTargetCollisionTest");
}

void FTargetCollisionTest::GetTests(TArray<FString>& OutBeautifiedNames, TArray<FString>& OutTestCommands) const
{
	if (InitGameModeDataAsset(CollisionTestDataAssetPath))
	{
		for (const auto& Mode : GameModeDataAsset->GetGameModesMap())
		{
			OutBeautifiedNames.Add(Mode.Key.CustomGameModeName);
			OutTestCommands.Add(Mode.Key.CustomGameModeName);
			TestMap.Add(Mode.Key.CustomGameModeName, Mode.Value);
		}
	}
}

bool FTargetCollisionTest::RunTest(const FString& Parameters)
{
	if (!Init())
	{
		return false;
	}

	FBSConfig* FoundConfig = TestMap.Find(Parameters);
	if (!FoundConfig)
	{
		AddError(FString::Printf(TEXT("Failed to find Config for Parameters: %s"), *Parameters));
		return false;
	}

	BSConfig = MakeShared<FBSConfig>(*FoundConfig);
	TargetManager->Init(BSConfig, FCommonScoreInfo(), FPlayerSettings_Game());
	TargetManager->SetShouldSpawn(true);

	for (int Iter = 0; Iter < NumIterations; Iter++)
	{
		TargetManager->OnAudioAnalyzerBeat();
		TArray<FSphere> Spheres;
		for (auto [Guid, Target] : GetManagedTargets())
		{
			Spheres.Emplace(FSphere(Target->GetActorLocation(), Target->GetRadius()));
			Target->DamageSelf(true);
			TickWorld(UE_KINDA_SMALL_NUMBER);
		}
		for (int i = 0; i < Spheres.Num(); i++)
		{
			for (int j = i + 1; j < Spheres.Num(); j++)
			{
				TestIntersection(Spheres[i], Spheres[j]);
			}
		}
		TotalTargetsSpawned += Spheres.Num();
		TestTrue("All targets destroyed at iteration end", GetManagedTargets().IsEmpty());
	}

	AddInfo(FString::Printf(TEXT("Total targets spawned: %d"), TotalTargetsSpawned));
	AddInfo(FString::Printf(TEXT("Total collisions: %d"), TotalCollisions));
	AddInfo(FString::Printf(TEXT("Min Distance between two spheres: %.4lf"), MinDistance));
	AddInfo(FString::Printf(TEXT("Total time spent executing GetTargetSpawnParams: %.4lf"),
		TargetSpawnParamsExecutionTime));

	ResetTestVariables();
	CleanUpWorld();

	return true;
}

void FTargetCollisionTest::TestIntersection(const FSphere& SphereOne, const FSphere& SphereTwo)
{
	const double CenterDist = FVector::Dist(SphereOne.Center, SphereTwo.Center);
	const double RadiusSum = SphereOne.W + SphereTwo.W;
	const bool bIntersects = CenterDist <= RadiusSum;
	if (bIntersects)
	{
		TotalCollisions++;
	}

	const double Dist = CenterDist - RadiusSum;
	MinDistance = FMath::Min(MinDistance, Dist);

	const FString What = FString::Printf(
		TEXT("Sphere at (%.2f, %.2f) w/ R=%.2lf intersecting sphere at (%.2f, %.2f) w/ R=%.2lf by %.2lf"),
		SphereOne.Center.Y, SphereOne.Center.Z, SphereOne.W, SphereTwo.Center.Y, SphereTwo.Center.Z, SphereTwo.W, Dist);
	TestFalse(What, bIntersects);
}

void FTargetCollisionTest::ResetTestVariables()
{
	TargetManager->Clear();
	BSConfig.Reset();
	TotalTargetsSpawned = 0;
	TotalCollisions = 0;
	MinDistance = 5000.f;
}
