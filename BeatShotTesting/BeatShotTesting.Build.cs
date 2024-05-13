// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class BeatShotTesting : ModuleRules
{
	public BeatShotTesting(ReadOnlyTargetRules Target) : base(Target)
	{
		bUseUnity = true;
		PublicDependencyModuleNames.AddRange(new[]
		{
			"Core", "CoreUObject", "Engine", "UnrealEd", "FunctionalTesting", "BeatShot", "BeatShotGlobal",
			"Json", "JsonUtilities"
		});
	}
}