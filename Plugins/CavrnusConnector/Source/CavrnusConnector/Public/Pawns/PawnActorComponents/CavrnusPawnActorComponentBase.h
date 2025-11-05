// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LivePropertyUpdates/CavrnusLivePropertyUpdate.h"
#include "Types/CavrnusSpaceConnection.h"
#include "Types/CavrnusUser.h"
#include "CavrnusPawnActorComponentBase.generated.h"

class UCavrnusPawnComponent;

UCLASS(Abstract, Blueprintable, ClassGroup=(Cavrnus), meta=(BlueprintSpawnableComponent))
class CAVRNUSCONNECTOR_API UCavrnusPawnActorComponentBase : public UActorComponent
{
	GENERATED_BODY()
public:
	UCavrnusPawnActorComponentBase();

	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|Pawn")
	TObjectPtr<APawn> Pawn = nullptr;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category = "Cavrnus|Pawn")
	virtual void Teardown();

public:
	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

protected:
	FDelegateHandle AnyHandle = FDelegateHandle();
	FDelegateHandle LocalHandle = FDelegateHandle();
	FDelegateHandle RemoteHandle = FDelegateHandle();
	
	bool bDoLocalTick = false;
	bool bDoRemoteTick = false;
	
	FCavrnusUser User = FCavrnusUser();
	
	TArray<FString> BindingIds;

	UPROPERTY()
	TArray<UCavrnusLivePropertyUpdate*> Updaters;
	
	UPROPERTY()
	TObjectPtr<UCavrnusPawnComponent> PawnSetupComp = nullptr;

	void InitializePawnSetupComponent(UCavrnusPawnComponent* Psc);

	UFUNCTION(BlueprintNativeEvent, Category="Cavrnus|Pawn")
	void HandleLocalTick(float DeltaTime);

	UFUNCTION(BlueprintNativeEvent, Category="Cavrnus|Pawn")
	void HandleRemoteTick(float DeltaTime);

	UFUNCTION(BlueprintNativeEvent, Category="Cavrnus|Pawn")
	void HandleAnyPawnSync(
		const FCavrnusSpaceConnection& SpaceConnection,
		const FString& UserContainerName,
		const FCavrnusUser& CavrnusUser);

	UFUNCTION(BlueprintNativeEvent, Category="Cavrnus|Pawn")
	void HandleLocalSync(
		const FCavrnusSpaceConnection& SpaceConnection,
		const FString& UserContainerName,
		const FCavrnusUser& CavrnusUser);

	UFUNCTION(BlueprintNativeEvent)
	void HandleRemoteSync(
		const FCavrnusSpaceConnection& SpaceConnection,
		const FString& UserContainerName,
		const FCavrnusUser& CavrnusUser);
};
