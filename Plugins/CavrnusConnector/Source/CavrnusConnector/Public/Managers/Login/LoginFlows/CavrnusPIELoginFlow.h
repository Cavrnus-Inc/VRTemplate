// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CavrnusLoginBaseFlow.h"
#include "CavrnusPIELoginFlow.generated.h"

UCLASS()
class CAVRNUSCONNECTOR_API UCavrnusPIELoginFlow : public UCavrnusLoginBaseFlow
{
	GENERATED_BODY()
public:
	virtual void DoLogin(const FCavrnusLoginConfig& InInitialLoginConfig) override;

private:
	void HandlePieLogin();
};
