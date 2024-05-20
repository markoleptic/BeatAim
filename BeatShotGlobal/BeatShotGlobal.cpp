// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#include "BeatShotGlobal.h"
#include "Modules/ModuleManager.h"

IMPLEMENT_MODULE(FDefaultModuleImpl, BeatShotGlobal);

void FBeatShotGlobal::StartupModule()
{
}

void FBeatShotGlobal::ShutdownModule()
{
	FDefaultGameModuleImpl::ShutdownModule();
}
