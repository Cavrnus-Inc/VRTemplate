// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PawnSyncComponents/CavrnusPawnSyncColor.h"
#include "PawnSyncComponents/CavrnusPawnSyncTransform.h"
#include "PawnSyncComponents/CavrnusPawnSyncActorLocation.h"
#include "PawnSyncComponents/CavrnusPawnSyncNameTag.h"

#include "Types/CavrnusUser.h"
#include "Types/CavrnusSpaceConnection.h"
#include "CavrnusPawnComponent.generated.h"

class UCavrnusPawnManager;
class UCavrnusPawnSyncActorLocation;

DECLARE_DYNAMIC_DELEGATE_ThreeParams(
	FCavrnusPawnReady,
	const FCavrnusSpaceConnection&, SpaceConnection,
	const FString&, ContainerName,
	const FCavrnusUser&, CavrnusUser
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FOnCavrnusPawnReady,
	const FCavrnusSpaceConnection&, SpaceConnection,
	const FString&, ContainerName,
	const FCavrnusUser&, CavrnusUser
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCavrnusSpaceSessionEnded);

UCLASS(ClassGroup=(Cavrnus), meta=(BlueprintSpawnableComponent))
class CAVRNUSCONNECTOR_API UCavrnusPawnComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	UCavrnusPawnComponent();
	
	// ── Blueprint-assignable delegates (visible in Event Graph) ─────────────

	/** Fired when any pawn (local or remote) is ready. Survives space exit for rejoin. */
	UPROPERTY(BlueprintAssignable, Category = "Cavrnus|Pawn")
	FOnCavrnusPawnReady OnAnyPawnReady;

	/** Fired when the local pawn is ready. Survives space exit for rejoin. */
	UPROPERTY(BlueprintAssignable, Category = "Cavrnus|Pawn")
	FOnCavrnusPawnReady OnLocalPawnReady;

	/** Fired when a remote pawn is ready. Survives space exit for rejoin. */
	UPROPERTY(BlueprintAssignable, Category = "Cavrnus|Pawn")
	FOnCavrnusPawnReady OnRemotePawnReady;

	/** Fired when the space session ends. Use this to clean up custom state on your pawn.
	 *  All pawn-ready delegates remain registered and will re-fire on rejoin. */
	UPROPERTY(BlueprintAssignable, Category = "Cavrnus|Pawn")
	FOnCavrnusSpaceSessionEnded OnSpaceSessionEnded;

	// ── C++ native delegates (for internal AddLambda / AddUObject usage) ────

	TMulticastDelegate<void(const FCavrnusSpaceConnection& SpaceConn, const FString&, const FCavrnusUser&)> OnAnyPawnReadyNative;
	TMulticastDelegate<void(const FCavrnusSpaceConnection& SpaceConn, const FString&, const FCavrnusUser&)> OnLocalPawnReadyNative;
	TMulticastDelegate<void(const FCavrnusSpaceConnection& SpaceConn, const FString&, const FCavrnusUser&)> OnRemotePawnReadyNative;
	TMulticastDelegate<void()> OnSpaceSessionEndedNative;
	
	UFUNCTION(BlueprintCallable, Category="Cavrnus|Pawn",
		meta = (ToolTip = "Fires callback when any pawn (local or remote) is ready. If already ready, fires immediately. Cleared on space exit — bind OnAnyPawnReady if you need it to survive across sessions.", ShortToolTip = "Await any pawn ready"))
	void AwaitCavrnusAnyPawnReady(const FCavrnusPawnReady& OnPawnReady)
	{
		if (bAnyPawnReady)
			OnPawnReady.ExecuteIfBound(User.SpaceConn, User.PropertiesContainerName, User);
		else
			DeferredAnyCallbacks.Add(OnPawnReady);
	}

	UFUNCTION(BlueprintCallable, Category="Cavrnus|Pawn",
		meta = (ToolTip = "Fires callback when the local pawn is ready. If already ready, fires immediately. Cleared on space exit — bind OnLocalPawnReady if you need it to survive across sessions.", ShortToolTip = "Await local pawn ready"))
	void AwaitCavrnusLocalPawnReady(const FCavrnusPawnReady& OnPawnReady)
	{
		if (bLocalPawnReady)
			OnPawnReady.ExecuteIfBound(User.SpaceConn, User.PropertiesContainerName, User);
		else
			DeferredLocalCallbacks.Add(OnPawnReady);
	}

	UFUNCTION(BlueprintCallable, Category="Cavrnus|Pawn",
		meta = (ToolTip = "Fires callback when a remote pawn is ready. If already ready, fires immediately. Cleared on space exit — bind OnRemotePawnReady if you need it to survive across sessions.", ShortToolTip = "Await remote pawn ready"))
	void AwaitCavrnusRemotePawnReady(const FCavrnusPawnReady& OnPawnReady)
	{
		if (bRemotePawnReady)
			OnPawnReady.ExecuteIfBound(User.SpaceConn, User.PropertiesContainerName, User);
		else
			DeferredRemoteCallbacks.Add(OnPawnReady);
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Cavrnus|Pawn",
		meta = (ToolTip = "Returns true if the local pawn has been initialized by Cavrnus for this space session.", ShortToolTip = "Is local pawn ready"))
	bool IsLocalPawnReady() const { return bLocalPawnReady; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Cavrnus|Pawn",
		meta = (ToolTip = "Returns true if any pawn (local or remote) has been initialized by Cavrnus for this space session.", ShortToolTip = "Is any pawn ready"))
	bool IsAnyPawnReady() const { return bAnyPawnReady; }

	/** Tears down all sync components and resets ready flags back to pre-space-join state. */
	void ResetSpaceState();

	void NotifyAnyPawnReady(const FCavrnusUser& CavrnusUser)
	{
		bAnyPawnReady = true;
		User = CavrnusUser;

		if (OnAnyPawnReadyNative.IsBound())
			OnAnyPawnReadyNative.Broadcast(CavrnusUser.SpaceConn, CavrnusUser.PropertiesContainerName, CavrnusUser);
		if (OnAnyPawnReady.IsBound())
			OnAnyPawnReady.Broadcast(CavrnusUser.SpaceConn, CavrnusUser.PropertiesContainerName, CavrnusUser);

		for (FCavrnusPawnReady& Callback : DeferredAnyCallbacks)
			Callback.ExecuteIfBound(CavrnusUser.SpaceConn, CavrnusUser.PropertiesContainerName, CavrnusUser);
	}

	void NotifyLocalPawnReady(const FCavrnusUser& CavrnusUser)
	{
		bLocalPawnReady = true;
		User = CavrnusUser;

		if (OnLocalPawnReadyNative.IsBound())
			OnLocalPawnReadyNative.Broadcast(CavrnusUser.SpaceConn, CavrnusUser.PropertiesContainerName, CavrnusUser);
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

		if (OnRemotePawnReadyNative.IsBound())
			OnRemotePawnReadyNative.Broadcast(CavrnusUser.SpaceConn, CavrnusUser.PropertiesContainerName, CavrnusUser);
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

	UFUNCTION(BlueprintCallable, Category="Cavrnus|Pawn",
		meta = (ToolTip = "Syncs the actor transform for pawns. For local pawns: sends Actor Transform property using root component as proxy. For remote pawns: receives Transform property and moves via AI Controller and Navmesh. TransformPropertyName defaults to 'Location' but can be changed to any property name. The pawn must have a movement component and the level must have a NavMeshBoundsVolume (for remote). When AvatarVis becomes visible, remote pawns will teleport to the specified property. Note: Property type is Transform, not Vector.",
		ShortToolTip = "Syncs Actor Transform property - sends for local, receives and moves for remote"))
	UCavrnusPawnSyncActorLocation* CavrnusPawnSyncActorLocation(
		const FString& TransformPropertyName = "Location",
		float MaxTimeToLocation = 5.0f,
		float ReachThreshold = 100.0f,
		float MovementUpdateThreshold = 50.0f)
	{
		UCavrnusPawnSyncActorLocation* NewSync = NewObject<UCavrnusPawnSyncActorLocation>(this);
		NewSync->Initialize(this, TransformPropertyName, MaxTimeToLocation, ReachThreshold, MovementUpdateThreshold);
		
		ActiveSyncComponents.Add(NewSync);
		return NewSync;
	}

	UFUNCTION(BlueprintCallable, Category="Cavrnus|Pawn",
		meta = (ToolTip = "Creates a name tag sync component that binds to the user's display name. Override OnNameUpdated and OnIsLocalUser in a Blueprint subclass to drive your nametag widget.",
		ShortToolTip = "Syncs the user's display name for nametag UI"))
	UCavrnusPawnSyncNameTag* CavrnusPawnSyncNameTag()
	{
		UCavrnusPawnSyncNameTag* NewSync = NewObject<UCavrnusPawnSyncNameTag>(this);
		NewSync->Initialize(this);

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
