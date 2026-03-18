// Copyright (c) 2025 Cavrnus. All rights reserved.
#include "Pawns/CavrnusPawnManager.h"
#include "CavrnusConnectorModule.h"
#include "CavrnusConnectorSettings.h"
#include "Engine/Engine.h"
#include "Pawns/SpawnManagement/CavrnusRemotePawnSpawner.h"
#include "Engine/GameViewportClient.h"
#include "GameFramework/Actor.h"

void UCavrnusPawnManager::RegisterUser(const FCavrnusUser& User)
{
	if (User.IsLocalUser && !HasSetupLocalUser)
	{
		if (UCavrnusConnectorSettings::Get()->bAutoSetupLocalPawn)
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
		if (UCavrnusConnectorSettings::Get()->bAutoSetupRemotePawns)
		{
			auto* RemoteSpawner = NewObject<UCavrnusRemotePawnSpawner>(GEngine->GameViewport->GetWorld());
			PawnSpawners.Add(User.PropertiesContainerName, RemoteSpawner);
			RemoteSpawner->Initialize(User);
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
	for (const auto& Spawner : PawnSpawners)
	{
		if (IsValid(Spawner.Value))
			Spawner.Value->Teardown();
	}
	PawnSpawners.Empty();
	HasSetupLocalUser = false;
}

void UCavrnusPawnManager::Dispose()
{
	Clear();
	HasSetupLocalUser = false;

}