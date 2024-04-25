// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "BSGameModeEnumTagMap.h"
#include "BSGameModeDataAsset.h"


UBSGameModeEnumTagMap::UBSGameModeEnumTagMap()
{
	EnumsTypes = TSet({
		StaticEnum<EBoundsScalingPolicy>(), StaticEnum<EMovingTargetDirectionMode>(),
		StaticEnum<EConsecutiveTargetScalePolicy>(), StaticEnum<ETargetDamageType>(),
		StaticEnum<ETargetActivationSelectionPolicy>(), StaticEnum<ERecentTargetMemoryPolicy>(),
		StaticEnum<ETargetDeactivationCondition>(), StaticEnum<ETargetDestructionCondition>(),
		StaticEnum<ETargetActivationResponse>(), StaticEnum<ETargetDeactivationResponse>(),
		StaticEnum<EDynamicBoundsScalingPolicy>(), StaticEnum<ETargetSpawningPolicy>(),
		StaticEnum<ETargetDistributionPolicy>(), StaticEnum<EReinforcementLearningMode>(),
		StaticEnum<EReinforcementLearningHyperParameterMode>(), StaticEnum<ETargetSpawnResponse>(),
		StaticEnum<ERuntimeTargetSpawningLocationSelectionMode>()
	});
	PopulateEnumTypes(EnumsTypes);
}
