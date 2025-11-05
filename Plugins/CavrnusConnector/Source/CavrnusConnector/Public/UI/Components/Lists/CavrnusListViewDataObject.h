// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "CavrnusListViewDataObject.generated.h"

UCLASS()
class CAVRNUSCONNECTOR_API UCavrnusListViewDataObject : public UObject
{
	GENERATED_BODY()
public:
	UPROPERTY()
	FString Id = "";
};
