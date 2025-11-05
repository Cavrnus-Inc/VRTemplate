// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CavrnusPawnComponent.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CavrnusPawnFunctions.generated.h"

/**
 * 
 */
UCLASS()
class CAVRNUSCONNECTOR_API UCavrnusPawnFunctions : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category="Cavrnus|Pawn")
	static void CavrnusUpdateMeshColor(TArray<UPrimitiveComponent*> MeshArray, const FLinearColor& NewColor, const float Multiplier);

	UFUNCTION(BlueprintCallable, Category="Cavrnus|Pawn")
	static UCavrnusPawnComponent* CavrnusFindPawnComponent(const AActor* ActorOwner);
};