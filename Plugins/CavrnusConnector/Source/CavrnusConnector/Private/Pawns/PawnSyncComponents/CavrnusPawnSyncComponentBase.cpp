// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "Pawns/PawnSyncComponents/CavrnusPawnSyncComponentBase.h"
#include "CavrnusConnectorModule.h"
#include "CavrnusFunctionLibrary.h"
#include "Pawns/CavrnusPawnComponent.h"

UCavrnusPawnSyncComponentBase::UCavrnusPawnSyncComponentBase()
{
}

void UCavrnusPawnSyncComponentBase::Teardown()
{
	if (bTornDown)
		return;
	bTornDown = true;

	for (const auto Id : BindingIds)
		UCavrnusFunctionLibrary::UnbindWithId(Id);
	BindingIds.Empty();

	if (PawnSetupComp)
	{
		if (AnyHandle.IsValid())
			PawnSetupComp->OnAnyPawnReadyNative.Remove(AnyHandle);

		if (LocalHandle.IsValid())
			PawnSetupComp->OnLocalPawnReadyNative.Remove(LocalHandle);

		if (RemoteHandle.IsValid())
			PawnSetupComp->OnRemotePawnReadyNative.Remove(RemoteHandle);

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
		UE_LOG(LogCavrnusConnector, Error, TEXT("[InitializePawnSetupComponent] The provided PawnSetupComponent is null!"));
}

void UCavrnusPawnSyncComponentBase::HandleAnySync(
	const FCavrnusSpaceConnection& SpaceConnection,
	const FString& UserContainerName,
	const FCavrnusUser& CavrnusUser)
{
}

void UCavrnusPawnSyncComponentBase::HandleLocalSync(
	const FCavrnusSpaceConnection& SpaceConnection,
	const FString& UserPropertyPath,
	const FCavrnusUser& CavrnusUser)
{
}

void UCavrnusPawnSyncComponentBase::HandleRemoteSync(
	const FCavrnusSpaceConnection& SpaceConnection,
	const FString& UserPropertyPath,
	const FCavrnusUser& CavrnusUser)
{
}
