// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Types/CavrnusUser.h"
#include "UObject/Object.h"
#include "CavrnusPawnSpawner.generated.h"

struct FCavrnusUser;
/**
 * 
 */
UCLASS(Abstract)
class CAVRNUSCONNECTOR_API UCavrnusPawnSpawner : public UObject
{
	GENERATED_BODY()
public:
	virtual void Initialize(const FCavrnusUser& InUser);
	virtual void Teardown();

protected:
	TArray<FString> BindingIds;
	
	UPROPERTY()
	FCavrnusUser User = FCavrnusUser();
};
