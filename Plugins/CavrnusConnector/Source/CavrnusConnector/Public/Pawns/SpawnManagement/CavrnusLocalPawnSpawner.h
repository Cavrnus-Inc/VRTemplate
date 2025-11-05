// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CavrnusPawnSpawner.h"
#include "LivePropertyUpdates/CavrnusLiveStringPropertyUpdate.h"
#include "Types/CavrnusUser.h"
#include "UObject/Object.h"
#include "CavrnusLocalPawnSpawner.generated.h"

class UCavrnusPawnComponent;
struct FCavrnusUser;
/**
 * 
 */
UCLASS()
class CAVRNUSCONNECTOR_API UCavrnusLocalPawnSpawner : public UCavrnusPawnSpawner
{
	GENERATED_BODY()
public:
	virtual void Initialize(const FCavrnusUser& LocalUse) override;
	void SwitchPawnById(const FString& InPawnId);
	virtual void Teardown() override;
	
private:
	UFUNCTION()
	void LocalPawnControllerChanged(APawn* InPawn, AController* InController);
	UFUNCTION()
	void LocalPawnPossessedChanged(APawn* OldPawn, APawn* NewPawn);
	
	void SetupPawn();
	void CleanUpLocalUserOnExitSpace();
	
	UPROPERTY()
	UCavrnusLiveStringPropertyUpdate* Updater;
};
