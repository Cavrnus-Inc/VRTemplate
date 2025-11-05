// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "RelayModel/SpacePermissionsModel.h"
#include "RelayModel/CavrnusBindingModel.h"

namespace Cavrnus
{
	SpacePermissionsModel::SpacePermissionsModel()
	{
	}

	SpacePermissionsModel::~SpacePermissionsModel()
	{

	}

	void SpacePermissionsModel::UpdatePolicyAllowed(FString policy, bool allowed)
	{
		bool exists = CurrPolicyAllowedValues.Contains(policy);
		CurrPolicyAllowedValues.FindOrAdd(policy);

		bool changed = !CurrPolicyAllowedValues[policy] == allowed;
		CurrPolicyAllowedValues[policy] = allowed;

		if (PolicyBindings.Contains(policy) && (!exists || changed))
		{
			for (int i = 0; i < PolicyBindings[policy].Num(); i++)
				(*PolicyBindings[policy][i])(policy, allowed);
		}
	}

	UCavrnusBinding* SpacePermissionsModel::BindPolicyAllowed(FString policy, CavrnusPolicyUpdated callback)
	{
		if (CurrPolicyAllowedValues.Contains(policy))
			callback(policy, CurrPolicyAllowedValues[policy]);

		CavrnusPolicyUpdated* CallbackPtr = new CavrnusPolicyUpdated(callback);

		PolicyBindings.FindOrAdd(policy);
		PolicyBindings[policy].Add(CallbackPtr);

		auto bindingId = Cavrnus::CavrnusBindingModel::GetBindingModel()->RegisterBinding([this, policy, CallbackPtr]() {
			PolicyBindings[policy].Remove(CallbackPtr);
			if (PolicyBindings[policy].IsEmpty())
				PolicyBindings.Remove(policy);
			});


		UCavrnusBinding* binding;
		binding = NewObject<UCavrnusBinding>();
		binding->Setup(bindingId);

		return binding;
	}
} // namespace Cavrnus
