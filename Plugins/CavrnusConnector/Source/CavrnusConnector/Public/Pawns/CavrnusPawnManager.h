// Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include <Components/SceneComponent.h>

#include "Pawns/CavrnusPawnComponent.h"
#include "SpawnManagement/CavrnusLocalPawnSpawner.h"
#include "Managers/CavrnusService.h"
#include "CavrnusPawnManager.generated.h"

class UCavrnusRemotePawnSpawner;
class UCavrnusRemotePawnHandler;

USTRUCT()
struct FCavrnusSpawnedUserData
{
	GENERATED_BODY()
	
	UPROPERTY()
	AActor* OwningActor = nullptr;
	
	FString VisBindingId = FString();
	FCavrnusUser CavrnusUserData = FCavrnusUser();
};

UCLASS()
class CAVRNUSCONNECTOR_API UCavrnusPawnManager : public UCavrnusService
{
	GENERATED_BODY()
public:
    void RegisterUser(const FCavrnusUser& User);
    void UnregisterUser(const FCavrnusUser& User);
	
	void SwitchPawnRuntime(const FString& PawnId);
	virtual void Deinitialize() override;

    void Clear();
	void Teardown();
	
private:
	bool HasSetupLocalUser = false;
	
	FCavrnusUser LocalUser = FCavrnusUser();
	
	UPROPERTY()
	TMap<FString, UCavrnusPawnSpawner*> PawnSpawners;
};