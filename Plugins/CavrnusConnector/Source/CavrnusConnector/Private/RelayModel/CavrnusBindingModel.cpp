// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "RelayModel/CavrnusBindingModel.h"

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

			BindingCallbacks.Remove(binding);
		}
	}
}