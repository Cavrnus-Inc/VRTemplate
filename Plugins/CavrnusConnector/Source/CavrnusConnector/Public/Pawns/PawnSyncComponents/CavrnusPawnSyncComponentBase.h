// Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Types/CavrnusSpaceConnection.h"
#include "Types/CavrnusUser.h"
#include "CavrnusPawnSyncComponentBase.generated.h"

class UCavrnusPawnComponent;

UCLASS(Blueprintable, Abstract)
class CAVRNUSCONNECTOR_API UCavrnusPawnSyncComponentBase : public UObject
{
	GENERATED_BODY()
public:
	UCavrnusPawnSyncComponentBase();

	UFUNCTION(BlueprintCallable, Category = "Cavrnus|Pawn")
	virtual void Teardown();

protected:
	FDelegateHandle AnyHandle = FDelegateHandle();
	FDelegateHandle LocalHandle = FDelegateHandle();
	FDelegateHandle RemoteHandle = FDelegateHandle();
	
	TArray<FString> BindingIds;
	
	UPROPERTY()
	TObjectPtr<UCavrnusPawnComponent> PawnSetupComp = nullptr;

	void InitializePawnSetupComponent(UCavrnusPawnComponent* Psc);

	UFUNCTION()
	virtual void HandleAnySync(
		const FCavrnusSpaceConnection& SpaceConnection,
		const FString& UserContainerName,
		const FCavrnusUser& CavrnusUser);

	UFUNCTION()
	virtual void HandleLocalSync(
		const FCavrnusSpaceConnection& SpaceConnection,
		const FString& UserContainerName,
		const FCavrnusUser& CavrnusUser);

	UFUNCTION()
	virtual void HandleRemoteSync(
		const FCavrnusSpaceConnection& SpaceConnection,
		const FString& UserContainerName,
		const FCavrnusUser& CavrnusUser);
};
