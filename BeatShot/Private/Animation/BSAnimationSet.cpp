// Copyright 2022-2023 Markoleptic Games, SP. All Rights Reserved.


#include "Animation/BSAnimationSet.h"

UAnimSequence* UBSAnimationSet::FindAnimSequence(const EBSAnimMovementMode MovementMode,
	const EBSAnimStateType StateType, const EBSAnimCardinal AnimCardinal) const
{
	if (const auto AnimSet = AnimMap.Find(MovementMode))
	{
		if (const auto CardinalSet = AnimSet->Map.Find(StateType))
		{
			if (const auto Cardinal = CardinalSet->Map.Find(AnimCardinal))
			{
				return (*Cardinal);
			}
		}
	}
	return nullptr;
}
