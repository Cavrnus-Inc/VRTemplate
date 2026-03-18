// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CavrnusPawnSyncComponentBase.h"
#include "Components/ActorComponent.h"
#include "CavrnusPawnSyncActorLocation.generated.h"

class UCavrnusPawnComponent;
class AAIController;
class USceneComponent;
class UCavrnusLiveTransformPropertyUpdate;

UCLASS(ClassGroup=(Cavrnus), meta=(BlueprintSpawnableComponent))
class CAVRNUSCONNECTOR_API UCavrnusPawnSyncActorLocation : public UCavrnusPawnSyncComponentBase
{
	GENERATED_BODY()
public:
	UCavrnusPawnSyncActorLocation();

	void Initialize(
		UCavrnusPawnComponent* PawnSetupComponent, 
		const FString& TransformPropertyName = "Location",
		float InMaxTimeToLocationSeconds = 5.0f,
		float InReachThresholdDistance = 100.0f,
		float InMovementUpdateThreshold = 50.0f);
	
	virtual void Teardown() override;
	
protected:
	virtual void HandleLocalSync(const FCavrnusSpaceConnection& SpaceConnection, const FString& ContainerName, const FCavrnusUser& CavrnusUser) override;
	virtual void HandleRemoteSync(const FCavrnusSpaceConnection& SpaceConnection, const FString& ContainerName, const FCavrnusUser& CavrnusUser) override;

private:
	FString TransformPropertyName;
	
	float MaxTimeToLocationSeconds;
	float ReachThresholdDistance;
	float MovementUpdateThreshold;
	
	FTimerHandle TimeoutTimerHandle;
	FVector CurrentTargetLocation;
	bool bIsMovingToLocation;
	
	UPROPERTY()
	TObjectPtr<AAIController> AIController = nullptr;
	
	UPROPERTY()
	TObjectPtr<APawn> TargetPawn = nullptr;
	
	FCavrnusSpaceConnection CachedSpaceConnection;
	FString CachedContainerName;
	
	// Local sync members
	UPROPERTY()
	TObjectPtr<UCavrnusLiveTransformPropertyUpdate> TransformUpdater = nullptr;
	FTransform LastSentTransform = FTransform();
	UPROPERTY()
	TObjectPtr<USceneComponent> RootComponentProxy = nullptr;
	FDelegateHandle RootComponentTransformDelegateHandle;
	
	void EnsureAIController();
	void MoveToLocation(const FVector& TargetLocation);
	void OnTimeoutExpired();
	bool ValidateRequirements();
	void HandleAvatarVisChanged(bool bIsVisible);
	void TeleportToTransformLocation();
	void OnRootComponentTransformUpdated(USceneComponent* SceneComponent, EUpdateTransformFlags UpdateFlags, ETeleportType TeleportType);
};

