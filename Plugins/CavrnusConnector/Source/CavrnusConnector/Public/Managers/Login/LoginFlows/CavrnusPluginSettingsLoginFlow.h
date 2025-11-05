// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CavrnusLoginBaseFlow.h"
#include "CavrnusPluginSettingsLoginFlow.generated.h"

UCLASS()
class CAVRNUSCONNECTOR_API UCavrnusPluginSettingsLoginFlow : public UCavrnusLoginBaseFlow
{
	GENERATED_BODY()
public:
	virtual void DoLogin(const FCavrnusLoginConfig& InLoginConfig) override;

private:
	void HandleMemberFlow();
	void HandleGuestFlow();
};
