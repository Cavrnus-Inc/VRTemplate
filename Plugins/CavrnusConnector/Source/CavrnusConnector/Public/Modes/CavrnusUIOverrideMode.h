// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CavrnusModeBase.h"
#include "CavrnusUIOverrideMode.generated.h"

/**
 * 
 */
UCLASS()
class CAVRNUSCONNECTOR_API UCavrnusUIOverrideMode : public UCavrnusModeBase
{
	GENERATED_BODY()
public:
	virtual FString GetInputMappingContextName() override { return "blank"; }
};
