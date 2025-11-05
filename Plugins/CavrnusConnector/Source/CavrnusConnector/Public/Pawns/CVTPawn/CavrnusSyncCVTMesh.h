// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "LivePropertyUpdates/CavrnusLiveVectorPropertyUpdate.h"
#include "Pawns/CavrnusPawnComponent.h"
#include "Pawns/CavrnusPawnSpeed.h"
#include "Types/CavrnusSpaceConnection.h"
#include "Pawns/PawnActorComponents/CavrnusPawnActorComponentBase.h"
#include "CavrnusSyncCVTMesh.generated.h"

class UCavrnusCVTWalkAnim;
class UWalk_AnimBP_C;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CAVRNUSCONNECTOR_API UCavrnusSyncCVTMesh : public UCavrnusPawnActorComponentBase
{
	GENERATED_BODY()
public:
	UCavrnusSyncCVTMesh();
	
protected:
	virtual void BeginPlay() override;

	virtual void HandleLocalTick_Implementation(float DeltaTime) override;
	virtual void HandleRemoteTick_Implementation(float DeltaTime) override;
	
	virtual void HandleAnyPawnSync_Implementation(
		const FCavrnusSpaceConnection& SpaceConnection,
		const FString& UserContainerName,
		const FCavrnusUser& CavrnusUser) override;
	
	virtual void HandleLocalSync_Implementation(
		const FCavrnusSpaceConnection& SpaceConnection,
		const FString& UserContainerName,
		const FCavrnusUser& CavrnusUser) override;

	virtual void HandleRemoteSync_Implementation(
		const FCavrnusSpaceConnection& SpaceConnection,
		const FString& UserContainerName,
		const FCavrnusUser& CavrnusUser) override;

private:
	UPROPERTY()
	TObjectPtr<UCavrnusCVTWalkAnim> WalkAnim = nullptr;

	UPROPERTY()
	TArray<UPrimitiveComponent*> MeshArray;

	UPROPERTY()
	TObjectPtr<UCavrnusPawnSpeed> PawnSpeed = nullptr;

	UPROPERTY()
	TObjectPtr<UCavrnusLiveVectorPropertyUpdate> HeadUpdater = nullptr;
};
