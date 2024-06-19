// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"

namespace BSCommon
{
	/** Returns true if any of the Args are equal to Actual.
	 *  @param Actual the actual value to test all args against
	 *  @param Rest the values to test against actual
	 *  @returns whether any of the Args are equal to Actual
	 */
	template <typename T, typename... Args>
	static bool Or(const T& Actual, const Args&... Rest);

	/** Returns true if all the Args are equal to Actual.
	 *  @param Actual the actual value to test all args against
	 *  @param Rest the values to test against actual
	 *  @returns whether all the Args are equal to Actual
	 */
	template <typename T, typename... Args>
	static bool And(const T& Actual, const Args&... Rest);

	template <class T>
	[[nodiscard]] static constexpr FORCEINLINE T GridSnapFromZero(T Location, int32 Grid)
	{
		return Grid == T{}
			? Location
			: Location > T{}
			? ceil(Location / Grid) * Grid
			: FMath::Floor(Location / Grid) * Grid;
	}

	template <class T>
	[[nodiscard]] static constexpr FORCEINLINE T GridSnapToZero(T Location, int32 Grid)
	{
		return Grid == T{}
			? Location
			: Location > T{}
			? FMath::Floor(Location / Grid) * Grid
			: ceil(Location / Grid) * Grid;
	}
}


template <typename T, typename... Args>
bool BSCommon::Or(const T& Actual, const Args&... Rest)
{
	return ((Actual == Rest) || ...);
}

template <typename T, typename... Args>
bool BSCommon::And(const T& Actual, const Args&... Rest)
{
	return ((Actual == Rest) && ...);
}
