// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CavrnusPawnSyncComponentBase.h"
#include "Components/ActorComponent.h"
#include "LivePropertyUpdates/CavrnusLiveTransformPropertyUpdate.h"
#include "CavrnusPawnSyncTransform.generated.h"

// Forward declaration to break circular dependency
class UCavrnusPawnComponent;

USTRUCT(BlueprintType)
struct FTransformSyncTarget
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cavrnus")
	FString PropertyName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cavrnus")
	USceneComponent* TargetComponent;

	FTransform LastWorldTransform = FTransform();
	FTransform LastRelativeTransform = FTransform();
};

UCLASS(ClassGroup=(Cavrnus), meta=(BlueprintSpawnableComponent))
class CAVRNUSCONNECTOR_API UCavrnusPawnSyncTransform : public UCavrnusPawnSyncComponentBase
{
	GENERATED_BODY()
public:
	UCavrnusPawnSyncTransform();

	void Initialize(UCavrnusPawnComponent* PawnSetupComponent, bool bRelativeTransform, const FString& PropertyName, USceneComponent* Target);
	void InitializeMany(UCavrnusPawnComponent* PawnSetupComponent, const TArray<FTransformSyncTarget>& Targets);
	
	virtual void Teardown() override;
	
protected:
	virtual void HandleLocalSync(const FCavrnusSpaceConnection& SpaceConnection, const FString& ContainerName, const FCavrnusUser& CavrnusUser) override;
	virtual void HandleRemoteSync(const FCavrnusSpaceConnection& SpaceConnection, const FString& ContainerName, const FCavrnusUser& CavrnusUser) override;

private:
	bool bIsRelativeTransform = false;
	
	TArray<FDelegateHandle> DelegateHandles;
	
	UPROPERTY()
	TArray<FTransformSyncTarget> SyncTargets;

	UPROPERTY()
	TObjectPtr<UCavrnusLiveTransformPropertyUpdate> TransformUpdater = nullptr;
};
