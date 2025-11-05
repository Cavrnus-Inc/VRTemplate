// Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include <Containers/Map.h>
#include "Types/CavrnusBinding.h"
#include "Types/CavrnusCallbackTypes.h"

namespace Cavrnus
{
	class SpacePermissionsModel
	{
	public:
		SpacePermissionsModel();
		virtual ~SpacePermissionsModel();

		void UpdatePolicyAllowed(FString policy, bool allowed);

		UCavrnusBinding* BindPolicyAllowed(FString policy, CavrnusPolicyUpdated callback);

	private:
		TMap<FString, bool> CurrPolicyAllowedValues;

		TMap<FString, TArray<CavrnusPolicyUpdated*>> PolicyBindings;
	};
} // namespace Cavrnus
