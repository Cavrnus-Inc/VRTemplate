// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"

class CAVRNUSCONNECTOR_API FCavrnusEnumHelpers
{
public:
	template <typename TEnum>
	static bool EnumToString(TEnum Value, FString& OutString)
	{
		static_assert(TIsEnum<TEnum>::Value, "EnumToString requires an enum type.");

		if (const UEnum* EnumPtr = StaticEnum<TEnum>())
		{
			OutString = EnumPtr->GetNameStringByValue(static_cast<int64>(Value));
			return true;
		}

		return false;
	}

	template <typename TEnum>
	static bool StringToEnum(const FString& InString, TEnum& OutEnum)
	{
		static_assert(TIsEnum<TEnum>::Value, "StringToEnum requires an enum type.");

		if (const UEnum* EnumPtr = StaticEnum<TEnum>())
		{
			int64 EnumValue = EnumPtr->GetValueByNameString(InString);

			if (EnumValue != INDEX_NONE)
			{
				OutEnum = static_cast<TEnum>(EnumValue);
				return true;
			}
		}

		return false;
	}
};
