// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "Pawns/PawnSyncComponents/CavrnusPawnSyncComponentBase.h"
#include "CavrnusFunctionLibrary.h"
#include "Pawns/CavrnusPawnComponent.h"

UCavrnusPawnSyncComponentBase::UCavrnusPawnSyncComponentBase()
{
}

void UCavrnusPawnSyncComponentBase::Teardown()
{
	if (BindingIds.IsEmpty())
		return;

	for (const auto Id : BindingIds)
		UCavrnusFunctionLibrary::UnbindWithId(Id);

	BindingIds.Empty();

	if (PawnSetupComp)
	{
		if (AnyHandle.IsValid())
			PawnSetupComp->OnAnyPawnReady.Remove(AnyHandle);

		if (LocalHandle.IsValid())
			PawnSetupComp->OnLocalPawnReady.Remove(LocalHandle);

		if (RemoteHandle.IsValid())
			PawnSetupComp->OnRemotePawnReady.Remove(RemoteHandle);

		AnyHandle.Reset();
		LocalHandle.Reset();
		RemoteHandle.Reset();
	}
}

void UCavrnusPawnSyncComponentBase::InitializePawnSetupComponent(UCavrnusPawnComponent* Psc)
{
	PawnSetupComp = Psc;
	
	if (PawnSetupComp)
	{
		// Ensure we also catch the already-ready case using deferred callback API
		FCavrnusPawnReady LocalDeferred;
		LocalDeferred.BindUFunction(this, FName("HandleLocalSync"));

		FCavrnusPawnReady AnyDeferred;
		AnyDeferred.BindUFunction(this, FName("HandleAnySync"));

		FCavrnusPawnReady RemoteDeferred;
		RemoteDeferred.BindUFunction(this, FName("HandleRemoteSync"));

		PawnSetupComp->AwaitCavrnusLocalPawnReady(LocalDeferred);
		PawnSetupComp->AwaitCavrnusAnyPawnReady(AnyDeferred);
		PawnSetupComp->AwaitCavrnusRemotePawnReady(RemoteDeferred);
	}
	else
		UE_LOG(LogTemp, Error, TEXT("The provided PawnSetupComponent is null!"));
}

void UCavrnusPawnSyncComponentBase::HandleAnySync(
	const FCavrnusSpaceConnection& SpaceConnection,
	const FString& UserContainerName,
	const FCavrnusUser& CavrnusUser)
{
	// Do some local AND/OR sync stuff!
}

void UCavrnusPawnSyncComponentBase::HandleLocalSync(
	const FCavrnusSpaceConnection& SpaceConnection,
	const FString& UserPropertyPath,
	const FCavrnusUser& CavrnusUser)
{
	// Do some local sync stuff!
}

void UCavrnusPawnSyncComponentBase::HandleRemoteSync(
	const FCavrnusSpaceConnection& SpaceConnection,
	const FString& UserPropertyPath,
	const FCavrnusUser& CavrnusUser)
{
	// Do some remote sync stuff!
}
