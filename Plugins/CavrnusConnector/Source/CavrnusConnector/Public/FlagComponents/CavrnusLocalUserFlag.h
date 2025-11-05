// Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CavrnusFunctionLibrary.h"
#include <Components/SceneComponent.h>
#include <Materials/MaterialInstanceDynamic.h>
#include "CavrnusLocalUserFlag.generated.h"

// Class definition
UCLASS(BlueprintType, Blueprintable)
class CAVRNUSCONNECTOR_API UCavrnusLocalUserFlag : public USceneComponent
{
	GENERATED_BODY()

public:
	// Constructor
	UCavrnusLocalUserFlag(){}

	// Destructor
	~UCavrnusLocalUserFlag(){}

	/** Reference to the local user. */
	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus")
	FCavrnusUser LocalUser = FCavrnusUser();
};
