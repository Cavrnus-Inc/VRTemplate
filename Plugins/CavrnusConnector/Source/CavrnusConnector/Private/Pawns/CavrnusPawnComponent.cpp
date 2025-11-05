// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "Pawns/CavrnusPawnComponent.h"

#include "CavrnusFunctionLibrary.h"
#include "CavrnusSubsystem.h"
#include "Pawns/CavrnusPawnManager.h"

UCavrnusPawnComponent::UCavrnusPawnComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCavrnusPawnComponent::CavrnusSetRemotePawn(const FString& PawnType)
{
	UCavrnusFunctionLibrary::AwaitAnySpaceConnection([PawnType](const FCavrnusSpaceConnection&)
	{
		UCavrnusSubsystem::Get()->Services->Get<UCavrnusPawnManager>()->SwitchPawnRuntime(PawnType);
	});
}

void UCavrnusPawnComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if (!ActiveSyncComponents.IsEmpty())
	{
		for (UCavrnusPawnSyncComponentBase* Sc : ActiveSyncComponents)
			Sc->Teardown();
	}

	ActiveSyncComponents.Empty();
}