// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PawnSyncComponents/CavrnusPawnSyncColor.h"
#include "PawnSyncComponents/CavrnusPawnSyncTransform.h"
#include "Types/CavrnusUser.h"
#include "Types/CavrnusSpaceConnection.h"
#include "CavrnusPawnComponent.generated.h"

class UCavrnusPawnManager;
DECLARE_DYNAMIC_DELEGATE_ThreeParams(
	FCavrnusPawnReady,
	const FCavrnusSpaceConnection&, SpaceConnection,
	const FString&, ContainerName,
	const FCavrnusUser&, CavrnusUser
);

UCLASS(ClassGroup=(Cavrnus), meta=(BlueprintSpawnableComponent))
class CAVRNUSCONNECTOR_API UCavrnusPawnComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	UCavrnusPawnComponent();
	
	TMulticastDelegate<void(const FCavrnusSpaceConnection& SpaceConn,const FString&,const FCavrnusUser&)> OnAnyPawnReady;
	TMulticastDelegate<void(const FCavrnusSpaceConnection& SpaceConn,const FString&,const FCavrnusUser&)> OnLocalPawnReady;
	TMulticastDelegate<void(const FCavrnusSpaceConnection& SpaceConn,const FString&,const FCavrnusUser&)> OnRemotePawnReady;
	
	UFUNCTION(BlueprintCallable, Category="Cavrnus|Pawn")
	void AwaitCavrnusAnyPawnReady(const FCavrnusPawnReady& OnPawnReady)
	{
		if (bAnyPawnReady)
			OnPawnReady.ExecuteIfBound(User.SpaceConn, User.PropertiesContainerName, User);
		else
			DeferredAnyCallbacks.Add(OnPawnReady);
	}
	
	UFUNCTION(BlueprintCallable, Category="Cavrnus|Pawn")
	void AwaitCavrnusLocalPawnReady(const FCavrnusPawnReady& OnPawnReady)
	{
		if (bLocalPawnReady)
			OnPawnReady.ExecuteIfBound(User.SpaceConn, User.PropertiesContainerName, User);
		else
			DeferredLocalCallbacks.Add(OnPawnReady);
	}

	UFUNCTION(BlueprintCallable, Category="Cavrnus|Pawn")
	void AwaitCavrnusRemotePawnReady(const FCavrnusPawnReady& OnPawnReady)
	{
		if (bRemotePawnReady)
			OnPawnReady.ExecuteIfBound(User.SpaceConn, User.PropertiesContainerName, User);
		else
			DeferredRemoteCallbacks.Add(OnPawnReady);
	}
	
	void NotifyAnyPawnReady(const FCavrnusUser& CavrnusUser)
	{
		bAnyPawnReady = true;
		User = CavrnusUser;
		if (OnAnyPawnReady.IsBound())
			OnAnyPawnReady.Broadcast(CavrnusUser.SpaceConn, CavrnusUser.PropertiesContainerName, CavrnusUser);

		for (FCavrnusPawnReady& Callback : DeferredAnyCallbacks)
			Callback.ExecuteIfBound(CavrnusUser.SpaceConn, CavrnusUser.PropertiesContainerName, CavrnusUser);
	}

	void NotifyLocalPawnReady(const FCavrnusUser& CavrnusUser)
	{
		bLocalPawnReady = true;
		User = CavrnusUser;
		if (OnLocalPawnReady.IsBound())
			OnLocalPawnReady.Broadcast(CavrnusUser.SpaceConn, CavrnusUser.PropertiesContainerName, CavrnusUser);

		for (FCavrnusPawnReady& Callback : DeferredLocalCallbacks)
			Callback.ExecuteIfBound(CavrnusUser.SpaceConn, CavrnusUser.PropertiesContainerName, CavrnusUser);

		for (auto& CallBack : DeferredLocalUserCallbacks)
			CallBack();
	}

	void NotifyRemotePawnReady(const FCavrnusUser& CavrnusUser)
	{
		bRemotePawnReady = true;
		User = CavrnusUser;
		if (OnRemotePawnReady.IsBound())
			OnRemotePawnReady.Broadcast(CavrnusUser.SpaceConn, CavrnusUser.PropertiesContainerName, CavrnusUser);

		for (FCavrnusPawnReady& Callback : DeferredRemoteCallbacks)
			Callback.ExecuteIfBound(CavrnusUser.SpaceConn, CavrnusUser.PropertiesContainerName, CavrnusUser);
	}

	UFUNCTION(BlueprintCallable, Category="Cavrnus|Pawn",
		meta = (ToolTip ="Updates the visual pawn type that other clients see when viewing your character. This changes only your remote representation, not your local pawn. PawnType must match the key name defined in the PawnSettingsDataAsset map.",
		ShortToolTip = "Changes what others see as your pawn"))
	void CavrnusSetRemotePawn(const FString& PawnType);

	UFUNCTION(BlueprintCallable, Category="Cavrnus|Pawn")
	UCavrnusPawnSyncTransform* CavrnusPawnSyncTransform(const FString& PropertyName = "Transform", const bool Relative = false, USceneComponent* SceneComponent = nullptr)
	{
		UCavrnusPawnSyncTransform* NewSync = NewObject<UCavrnusPawnSyncTransform>(this);
		NewSync->Initialize(this, Relative, PropertyName, SceneComponent);
		
		ActiveSyncComponents.Add(NewSync);
		return NewSync;
	}

	UFUNCTION(BlueprintCallable, Category="Cavrnus|Pawn")
	UCavrnusPawnSyncColor* CavrnusPawnSyncColor(const TArray<UPrimitiveComponent*>& MeshArray, const FString& PropertyName = TEXT("primaryColor"))
	{
		UCavrnusPawnSyncColor* NewSync = NewObject<UCavrnusPawnSyncColor>(this);
		NewSync->Initialize(this, PropertyName, MeshArray);

		ActiveSyncComponents.Add(NewSync);
		return NewSync;
	}

protected:
	UPROPERTY()
	TArray<UCavrnusPawnSyncComponentBase*> ActiveSyncComponents;
	
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	FCavrnusUser User = FCavrnusUser();
	
	bool bAnyPawnReady = false;
	bool bLocalPawnReady = false;
	bool bRemotePawnReady = false;
	TArray<FCavrnusPawnReady> DeferredAnyCallbacks;
	TArray<FCavrnusPawnReady> DeferredLocalCallbacks;
	TArray<FCavrnusPawnReady> DeferredRemoteCallbacks;
	TArray<TFunction<void()>> DeferredLocalUserCallbacks;
};
