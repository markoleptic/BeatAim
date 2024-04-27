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
		StaticEnum<EDLSSEnabledMode>(), StaticEnum<UStreamlineReflexMode>(), StaticEnum<UDLSSMode>(),
		StaticEnum<UStreamlineDLSSGMode>(), StaticEnum<ENISEnabledMode>(), StaticEnum<UNISMode>()
	});

	PopulateEnumTypes(EnumTypes);
}
