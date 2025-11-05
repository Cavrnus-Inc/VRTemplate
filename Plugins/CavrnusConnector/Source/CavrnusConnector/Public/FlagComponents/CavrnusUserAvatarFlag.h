// Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Types/CavrnusSpawnedObject.h"
#include "Types/CavrnusUser.h"
#include "CavrnusUserAvatarFlag.generated.h"

// Class definition
UCLASS(BlueprintType, Blueprintable)
class CAVRNUSCONNECTOR_API UCavrnusUserAvatarFlag : public USceneComponent
{
	GENERATED_BODY()

public:
	// Constructor
	UCavrnusUserAvatarFlag(){}

	// Destructor
	~UCavrnusUserAvatarFlag(){}

	/** The Cavrnus user associated with the avatar. */
	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus")
	FCavrnusUser CavrnusUser = FCavrnusUser();
};
