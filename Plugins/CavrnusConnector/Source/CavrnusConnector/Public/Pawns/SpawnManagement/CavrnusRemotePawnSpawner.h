// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CavrnusPawnSpawner.h"
#include "AssetManager/DataAssets/CavrnusPawnSettingsDataAsset.h"
#include "Types/CavrnusUser.h"
#include "UObject/Object.h"
#include "CavrnusRemotePawnSpawner.generated.h"

struct FCavrnusUser;
/**
 * 
 */
UCLASS()
class CAVRNUSCONNECTOR_API UCavrnusRemotePawnSpawner : public UCavrnusPawnSpawner
{
	GENERATED_BODY()
public:
	virtual void Initialize(const FCavrnusUser& InUser) override;
	virtual void Teardown() override;
private:
	UPROPERTY()
	TObjectPtr<AActor> CurrentSpawnedPawnActor = nullptr;

	FString AvatarVisBinding = "";

	void TryDestroyingCurrentPawn();
	void HandlePawnSwitch(const FString& PawnTypeId);
	void SetupPawn(const FCavrnusPawnTypeData& TargetPawnData);
	AActor* Spawn(const TSubclassOf<AActor>& RemoteActor);
};
