// Copyright (c) 2025 Cavrnus. All rights reserved.
#include "Pawns/CavrnusPawnManager.h"
#include "CavrnusConnectorModule.h"
#include "CavrnusSubsystem.h"
#include "Engine/Engine.h"
#include "Pawns/SpawnManagement/CavrnusRemotePawnSpawner.h"
#include "Engine/GameViewportClient.h"
#include "GameFramework/Actor.h"

void UCavrnusPawnManager::RegisterUser(const FCavrnusUser& User)
{
	if (User.IsLocalUser && !HasSetupLocalUser)
	{
		if (UCavrnusSubsystem::Get()->Services->Get<UCavrnusConnectorSettings>()->bAutoSetupLocalPawn)
		{
			LocalUser = User;
		
			auto* LocalSpawner = NewObject<UCavrnusLocalPawnSpawner>(GEngine->GameViewport->GetWorld());
			PawnSpawners.Add(LocalUser.PropertiesContainerName, LocalSpawner);

			LocalSpawner->Initialize(User);
			
			HasSetupLocalUser = true;
		}
		else
			UE_LOG(LogCavrnusConnector, Warning, TEXT("bAutoSetupLocalPawn is false! Enable bAutoSetupLocalPawn in Cavrnus Plugin Settings if you need auto local pawn support!"));
	
	} else
	{
		if (UCavrnusSubsystem::Get()->Services->Get<UCavrnusConnectorSettings>()->bAutoSetupRemotePawns)
		{
			auto* RemoteSpawner = NewObject<UCavrnusRemotePawnSpawner>(GEngine->GameViewport->GetWorld());
			RemoteSpawner->Initialize(User);
			PawnSpawners.Add(User.PropertiesContainerName, RemoteSpawner);
		}
		else
			UE_LOG(LogCavrnusConnector, Warning, TEXT("bAutoSetupRemotePawns is false! Enable bAutoSetupRemotePawns in Cavrnus Plugin Settings if you need auto remote pawn support!"));
	}
}

void UCavrnusPawnManager::UnregisterUser(const FCavrnusUser& User)
{
	if (const auto* FoundSpawnerPtr = PawnSpawners.Find(User.PropertiesContainerName))
	{
		if (auto* FoundSpawner = *FoundSpawnerPtr)
			FoundSpawner->Teardown();
	}
	else
		UE_LOG(LogCavrnusConnector, Warning, TEXT("Failed to cleanup PawnSpawner, could not find spawner with ConnectionId %s"), *User.PropertiesContainerName);
}

void UCavrnusPawnManager::SwitchPawnRuntime(const FString& PawnId)
{
	if (auto* SpawnerPtr = PawnSpawners.Find(LocalUser.PropertiesContainerName))
	{
		if (auto* Spawner = *SpawnerPtr)
		{
			if (UCavrnusLocalPawnSpawner* CastedLocalSpawner = Cast<UCavrnusLocalPawnSpawner>(Spawner))
				CastedLocalSpawner->SwitchPawnById(PawnId);
		}
	}
}

void UCavrnusPawnManager::Clear()
{
	UE_LOG(LogTemp, Warning, TEXT("UCavrnusPawnManager::Clear"));

	for (const auto Spawner : PawnSpawners)
	{
		UE_LOG(LogTemp, Warning, TEXT("Tearing Down Spawner"));
		Spawner.Value->Teardown();
		UE_LOG(LogTemp, Warning, TEXT("TORN DOWN"));
	}
	PawnSpawners.Empty();
}

void UCavrnusPawnManager::Teardown()
{
	UE_LOG(LogTemp, Warning, TEXT("UCavrnusPawnManager::Teardown"));
	Clear();
	HasSetupLocalUser = false;

}

void UCavrnusPawnManager::Deinitialize()
{
	UE_LOG(LogTemp, Warning, TEXT("UCavrnusPawnManager::DeinitializeManager"));
	Teardown();
	UE_LOG(LogTemp, Warning, TEXT("UCavrnusPawnManager::DeinitializeManager - Starting Super::DeinitializeManager"));
	Super::Deinitialize();
	UE_LOG(LogTemp, Warning, TEXT("UCavrnusPawnManager::DeinitializeManager - Finished Super::DeinitializeManager"));

}