// Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Types/CavrnusSpawnedObject.h"
#include "CavrnusSpawnedObjectFlag.generated.h"

// Class definition
UCLASS(BlueprintType, Blueprintable)
class CAVRNUSCONNECTOR_API UCavrnusSpawnedObjectFlag : public USceneComponent
{
	GENERATED_BODY()

public:
	// Constructor
	UCavrnusSpawnedObjectFlag(){}

	// Destructor
	~UCavrnusSpawnedObjectFlag(){}

	/** The spawned object referenced by the component. */
	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus")
	FCavrnusSpawnedObject SpawnedObject;
};
