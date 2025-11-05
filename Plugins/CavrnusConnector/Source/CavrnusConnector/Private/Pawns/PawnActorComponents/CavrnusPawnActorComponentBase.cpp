// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "Pawns/PawnActorComponents/CavrnusPawnActorComponentBase.h"

#include "CavrnusFunctionLibrary.h"
#include "Pawns/CavrnusPawnComponent.h"
#include "GameFramework/Pawn.h"
UCavrnusPawnActorComponentBase::UCavrnusPawnActorComponentBase()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCavrnusPawnActorComponentBase::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bDoLocalTick)
		HandleLocalTick(DeltaTime);

	if (bDoRemoteTick)
		HandleRemoteTick(DeltaTime);
}

void UCavrnusPawnActorComponentBase::Teardown()
{
	for (const auto Id : BindingIds)
		UCavrnusFunctionLibrary::UnbindWithId(Id);

	BindingIds.Empty();
	
	for (auto* Updater : Updaters)
	{
		if (IsValid(Updater))
		{
			Updater->Cancel();
			Updater = nullptr;
		}
	}
	Updaters.Empty();
	
	if (PawnSetupComp)
	{
		PawnSetupComp->OnAnyPawnReady.Remove(AnyHandle);
		PawnSetupComp->OnLocalPawnReady.Remove(LocalHandle);
		PawnSetupComp->OnRemotePawnReady.Remove(RemoteHandle);

		AnyHandle.Reset();
		LocalHandle.Reset();
		RemoteHandle.Reset();
	}
}

void UCavrnusPawnActorComponentBase::BeginPlay()
{
	Super::BeginPlay();

	Pawn = Cast<APawn>(GetOwner());
	if (!Pawn)
	{
		UE_LOG(LogTemp, Error, TEXT("CavrnusPawnActorComponentBase: Owner is not a Pawn!"));
		return;
	}

	// Try to find the PawnSetupComponent directly on the pawn
	if (UCavrnusPawnComponent* SetupComp = Pawn->FindComponentByClass<UCavrnusPawnComponent>())
		InitializePawnSetupComponent(SetupComp);
	else
		UE_LOG(LogTemp, Error, TEXT("CavrnusPawnActorComponentBase: No UCavrnusPawnComponent found on %s"), *Pawn->GetName());
}

void UCavrnusPawnActorComponentBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	Teardown();
}

void UCavrnusPawnActorComponentBase::InitializePawnSetupComponent(UCavrnusPawnComponent* Psc)
{
	PawnSetupComp = Psc;
	
	if (PawnSetupComp)
	{
		AnyHandle = PawnSetupComp->OnAnyPawnReady.AddUObject(this, &UCavrnusPawnActorComponentBase::HandleAnyPawnSync);
		LocalHandle = PawnSetupComp->OnLocalPawnReady.AddUObject(this, &UCavrnusPawnActorComponentBase::HandleLocalSync);
		RemoteHandle = PawnSetupComp->OnRemotePawnReady.AddUObject(this, &UCavrnusPawnActorComponentBase::HandleRemoteSync);
	}
	else
		UE_LOG(LogTemp, Error, TEXT("The provided PawnSetupComponent is null!"));
}

void UCavrnusPawnActorComponentBase::HandleRemoteTick_Implementation(float DeltaTime)
{
}

void UCavrnusPawnActorComponentBase::HandleLocalTick_Implementation(float DeltaTime)
{
}

void UCavrnusPawnActorComponentBase::HandleAnyPawnSync_Implementation(
	const FCavrnusSpaceConnection& SpaceConnection,
	const FString& UserContainerName,
	const FCavrnusUser& CavrnusUser)
{
	User = CavrnusUser;
}

void UCavrnusPawnActorComponentBase::HandleRemoteSync_Implementation(
	const FCavrnusSpaceConnection& SpaceConnection,
	const FString& UserContainerName,
	const FCavrnusUser& CavrnusUser)
{
	User = CavrnusUser;
}

void UCavrnusPawnActorComponentBase::HandleLocalSync_Implementation(
	const FCavrnusSpaceConnection& SpaceConnection,
	const FString& UserContainerName,
	const FCavrnusUser& CavrnusUser)
{
	User = CavrnusUser;
}