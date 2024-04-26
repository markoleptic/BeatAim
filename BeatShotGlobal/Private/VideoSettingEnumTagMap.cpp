// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "VideoSettingEnumTagMap.h"
#include "BSSettingTypes.h"
#include "DLSSLibrary.h"
#include "NISLibrary.h"
#include "StreamlineLibraryDLSSG.h"
#include "StreamlineLibraryReflex.h"

UVideoSettingEnumTagMap::UVideoSettingEnumTagMap()
{
	EnumTypes = TSet({
		StaticEnum<EVideoSettingType>(), StaticEnum<ENvidiaSettingType>(), StaticEnum<EDLSSEnabledMode>(),
		StaticEnum<UStreamlineReflexMode>(), StaticEnum<UDLSSMode>(), StaticEnum<UStreamlineDLSSGMode>(),
		StaticEnum<ENISEnabledMode>(), StaticEnum<UNISMode>()
	});
	PopulateEnumTypes(EnumTypes);

	for (auto& [Key, Value] : EnumTagMap)
	{
		for (auto& [Key2, Value2] : Value.NewEnumTagPairs) UE_LOG(LogTemp, Display, TEXT("%d %s"), Value2.Index,
			*Value2.DisplayName)		;
	}
}
