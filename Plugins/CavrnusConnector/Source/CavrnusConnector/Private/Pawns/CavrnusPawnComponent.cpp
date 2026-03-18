// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "Pawns/CavrnusPawnComponent.h"
#include "Core/Contexts/CavrnusRuntimeContext.h"
#include "Core/Subsystems/CavrnusSubsystem.h"
#include "Pawns/CavrnusPawnManager.h"

UCavrnusPawnComponent::UCavrnusPawnComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCavrnusPawnComponent::CavrnusSetRemotePawn(const FString& PawnType)
{
	if (bLocalPawnReady)
		UCavrnusSubsystem::Get()->RuntimeContext->Get<UCavrnusPawnManager>()->SwitchPawnRuntime(PawnType);
	else
	{
		TFunction<void()> cb = [PawnType]
		{
			UCavrnusSubsystem::Get()->RuntimeContext->Get<UCavrnusPawnManager>()->SwitchPawnRuntime(PawnType);
		};
		DeferredLocalUserCallbacks.Add(cb);
	}
}

void UCavrnusPawnComponent::ResetSpaceState()
{
	// Guard: multiple callers (AwaitAnySpaceExited, Teardown, etc.) may trigger this.
	// Only broadcast and tear down once per session.
	const bool bWasActive = bAnyPawnReady || bLocalPawnReady || bRemotePawnReady;

	// Tear down all sync components (bindings, live updaters, etc.)
	for (UCavrnusPawnSyncComponentBase* Sc : ActiveSyncComponents)
	{
		if (Sc)
			Sc->Teardown();
	}
	ActiveSyncComponents.Empty();

	// Reset ready flags so Notify*PawnReady can fire again on rejoin
	bAnyPawnReady = false;
	bLocalPawnReady = false;
	bRemotePawnReady = false;

	// Clear one-shot deferred callbacks (stale; hold old SpaceConn)
	DeferredAnyCallbacks.Empty();
	DeferredLocalCallbacks.Empty();
	DeferredRemoteCallbacks.Empty();
	DeferredLocalUserCallbacks.Empty();

	// DO NOT clear multicast delegates — they are durable registrations from
	// developer code (e.g., OnLocalPawnReady in BeginPlay).
	// They will re-fire on the next NotifyLocalPawnReady() call after rejoin.

	// Only broadcast session-ended if we were actually in an active session.
	// Prevents duplicate broadcasts from multiple callers.
	if (bWasActive)
	{
		if (OnSpaceSessionEndedNative.IsBound())
			OnSpaceSessionEndedNative.Broadcast();
		if (OnSpaceSessionEnded.IsBound())
			OnSpaceSessionEnded.Broadcast();
	}

	User = FCavrnusUser();
}

void UCavrnusPawnComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	ResetSpaceState();

	// On destruction, also clear all delegates to prevent dangling references
	OnAnyPawnReadyNative.Clear();
	OnLocalPawnReadyNative.Clear();
	OnRemotePawnReadyNative.Clear();
	OnSpaceSessionEndedNative.Clear();
	OnAnyPawnReady.Clear();
	OnLocalPawnReady.Clear();
	OnRemotePawnReady.Clear();
	OnSpaceSessionEnded.Clear();
}
