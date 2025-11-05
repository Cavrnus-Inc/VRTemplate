// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CavrnusPawnSyncComponentBase.h"
#include "UObject/Object.h"
#include "CavrnusPawnSyncColor.generated.h"

/**
 * 
 */
UCLASS()
class CAVRNUSCONNECTOR_API UCavrnusPawnSyncColor : public UCavrnusPawnSyncComponentBase
{
	GENERATED_BODY()
public:
	void Initialize(UCavrnusPawnComponent* PawnSetupComponent, const FString& PropertyName, const TArray<UPrimitiveComponent*>& InMeshArray);

protected:
	virtual void HandleAnySync(const FCavrnusSpaceConnection& SpaceConnection, const FString& UserContainerName, const FCavrnusUser& CavrnusUser) override;

private:
	FString ColorPropName = "primaryColor";
	
	UPROPERTY()
	TArray<UPrimitiveComponent*> MeshArray;
};
