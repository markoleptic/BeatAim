// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.

#pragma once

namespace BSCommon
{
	/** Returns true if any of the Args are equal to Actual.
	 *  @param Actual the actual value to test all args against
	 *  @param Rest the values to test against actual
	 *  @returns whether or not any of the Args are equal to Actual
	 */
	template <typename T, typename... Args>
	static bool Or(const T& Actual, const Args&... Rest);

	/** Returns true if all of the Args are equal to Actual.
	 *  @param Actual the actual value to test all args against
	 *  @param Rest the values to test against actual
	 *  @returns whether or not all of the Args are equal to Actual
	 */
	template <typename T, typename... Args>
	static bool And(const T& Actual, const Args&... Rest);
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
