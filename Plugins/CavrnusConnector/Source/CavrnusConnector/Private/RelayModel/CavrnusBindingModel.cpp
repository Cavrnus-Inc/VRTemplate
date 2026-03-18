// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "RelayModel/CavrnusBindingModel.h"
#include "CavrnusConnectorModule.h"

namespace Cavrnus
{
	CavrnusBindingModel* CavrnusBindingModel::Instance = nullptr;

	CavrnusBindingModel* CavrnusBindingModel::GetBindingModel()
	{
		if (Instance == nullptr)
			Instance = new Cavrnus::CavrnusBindingModel();
		return Instance;
	}

	FString CavrnusBindingModel::RegisterBinding(CavrnusUnbind bindingCallback)
	{
		auto guid = FGuid::NewGuid().ToString();

		CavrnusUnbind* callback = new CavrnusUnbind(bindingCallback);
		BindingCallbacks.Add(guid, callback);

		return guid;
	}

	void CavrnusBindingModel::UnbindBinding(FString binding)
	{
		if (BindingCallbacks.Contains(binding))
		{
			(*BindingCallbacks[binding])();

			delete BindingCallbacks[binding];
			BindingCallbacks.Remove(binding);
		}
		else
		{
			UE_LOG(LogCavrnusConnector, Warning,
				TEXT("[UnbindBinding] Binding ID '%s' not found. It may have already been cleaned up by a previous space session teardown. "
					 "Actors holding Cavrnus bindings should be destroyed and recreated on space exit."),
				*binding);
		}
	}

	void CavrnusBindingModel::RemoveBindingWithoutUnbind(const FString& bindingId)
	{
		if (BindingCallbacks.Contains(bindingId))
		{
			delete BindingCallbacks[bindingId];
			BindingCallbacks.Remove(bindingId);
		}
	}
}