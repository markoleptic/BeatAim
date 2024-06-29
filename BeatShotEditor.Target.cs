// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

using UnrealBuildTool;

public class BeatShotEditorTarget : TargetRules
{
	public BeatShotEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.AddRange(new[] { "BeatShot", "UserInterface", "BeatShotGlobal", "BeatShotTesting" });
	}
}