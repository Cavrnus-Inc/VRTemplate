// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "Pawns/SpawnManagement/CavrnusLocalPawnSpawner.h"

#include "CavrnusConnectorModule.h"
#include "CavrnusFunctionLibrary.h"
#include "Core/Subsystems/CavrnusSubsystem.h"
#include "AssetManager/CavrnusDataAssetManager.h"
#include "AssetManager/DataAssets/CavrnusPawnSettingsDataAsset.h"
#include "Core/Contexts/CavrnusRuntimeContext.h"
#include "GameFramework/PlayerController.h"
#include "LivePropertyUpdates/CavrnusLiveStringPropertyUpdate.h"
#include "Pawns/CavrnusPawnFunctions.h"
#include "GameFramework/Pawn.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Engine/GameInstance.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
void UCavrnusLocalPawnSpawner::Initialize(const FCavrnusUser& LocalUser)
{
	Super::Initialize(LocalUser);

	if (!LocalUser.IsLocalUser)
	{
		UE_LOG(LogCavrnusConnector, Error, TEXT("Whoops! This user is not the local user!"))
		return;
	}
	
	if (const auto GI = GetWorld()->GetGameInstance())
	{
		GI->GetOnPawnControllerChanged().AddDynamic(this, &UCavrnusLocalPawnSpawner::LocalPawnControllerChanged);
		if (APlayerController* PlayerController = GI->GetFirstLocalPlayerController())
		{
			PlayerController->OnPossessedPawnChanged.AddDynamic(this, &UCavrnusLocalPawnSpawner::LocalPawnPossessedChanged);
		}

		SetupPawn();
	}

	TWeakObjectPtr<UCavrnusLocalPawnSpawner> WeakThis(this);
	UCavrnusFunctionLibrary::AwaitAnySpaceExited([WeakThis]
	{
		if (WeakThis.IsValid())
			WeakThis->CleanUpLocalUserOnExitSpace();
	});
}

void UCavrnusLocalPawnSpawner::SwitchPawnById(const FString& InPawnId)
{
	if (!IsValid(Updater))
	{
		Updater = UCavrnusFunctionLibrary::BeginTransientStringPropertyUpdate(
		User.SpaceConn,
		User.PropertiesContainerName,
		UCavrnusSubsystem::Get()->RuntimeContext->Get<UCavrnusDataAssetManager>()->GetAsset<UCavrnusPawnSettingsDataAsset>()->GetPropertyName(), InPawnId);
	} else
		Updater->UpdateWithNewData(InPawnId);
}

void UCavrnusLocalPawnSpawner::Teardown()
{
	Super::Teardown();
	CleanUpLocalUserOnExitSpace();
}

void UCavrnusLocalPawnSpawner::LocalPawnControllerChanged(APawn* InPawn, AController* InController)
{
	if (InController && InController->IsLocalPlayerController() && InPawn)
		SetupPawn();
}

void UCavrnusLocalPawnSpawner::LocalPawnPossessedChanged(APawn* OldPawn, APawn* NewPawn)
{
	if (IsValid(NewPawn))
		SetupPawn();
}

void UCavrnusLocalPawnSpawner::SetupPawn()
{
	APlayerController* Pc = GEngine->GetFirstLocalPlayerController(GEngine->GameViewport->GetWorld());
	if (Pc == nullptr)
	{
		UE_LOG(LogCavrnusConnector, Error, TEXT("APlayerController is null!"));
		return;
	}
	
	const APawn* Pawn = Pc->GetPawn();
	if (Pawn == nullptr)
	{
		UE_LOG(LogCavrnusConnector, Error, TEXT("APawn is null!"));
		return;
	}
	
	if (auto* Sc = UCavrnusPawnFunctions::CavrnusFindPawnComponent(Pawn))
	{
		UCavrnusFunctionLibrary::BeginTransientBoolPropertyUpdate(User.SpaceConn, User.PropertiesContainerName, "AvatarVis", true);
		Sc->NotifyAnyPawnReady(User);
		Sc->NotifyLocalPawnReady(User);
	}
	else
	{
		UE_LOG(LogCavrnusConnector, Warning, TEXT("[LocalPawnSpawner::SetupPawn] Pawn '%s' has no UCavrnusPawnComponent -- local sync will not work"), *Pawn->GetName());
	}
}

void UCavrnusLocalPawnSpawner::CleanUpLocalUserOnExitSpace()
{
	if (!GEngine || !GEngine->GameViewport)
		return;

	UWorld* World = GEngine->GameViewport->GetWorld();
	if (!World)
		return;

	APlayerController* PlayerController = World->GetFirstPlayerController();
	if (!PlayerController)
		return;

	if (APawn* Pawn = PlayerController->GetPawn())
	{
		// Reset the pawn component back to pre-space-join state
		if (auto* Sc = UCavrnusPawnFunctions::CavrnusFindPawnComponent(Pawn))
			Sc->ResetSpaceState();

		PlayerController->OnPossessedPawnChanged.RemoveDynamic(
			this, &UCavrnusLocalPawnSpawner::LocalPawnPossessedChanged);
	}

	if (UGameInstance* GI = World->GetGameInstance())
	{
		GI->GetOnPawnControllerChanged().RemoveDynamic(
			this, &UCavrnusLocalPawnSpawner::LocalPawnControllerChanged);
	}
}
