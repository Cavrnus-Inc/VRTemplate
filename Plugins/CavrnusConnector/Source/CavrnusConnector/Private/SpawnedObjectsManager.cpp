// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "SpawnedObjectsManager.h"
#include "CavrnusConnectorModule.h"

#include "CavrnusFunctionLibrary.h"
#include "SpawnObjectHelpers.h"
#include "FlagComponents/CavrnusSpawnedObjectFlag.h"
#include "CavrnusPropertiesContainer.h"
#include "CoreMinimal.h"
#include <Kismet/GameplayStatics.h>

SpawnedObjectsManager* SpawnedObjectsManager::Instance = nullptr;

SpawnedObjectsManager* SpawnedObjectsManager::GetSpawnedObjectsManager()
{
	if (Instance == nullptr)
		Instance = new SpawnedObjectsManager();
	return Instance;
}

void SpawnedObjectsManager::Kill()
{
	if (Instance != nullptr)
	{
		delete Instance;
		Instance = nullptr;
	}
}

AActor* SpawnedObjectsManager::RegisterSpawnedObject(const FCavrnusSpawnedObject& SpawnedObject, const FString& Identifier, UWorld* world)
{
	if (!SpawnableIdentifiers.Contains(Identifier))
	{
		UE_LOG(LogCavrnusConnector, Error, TEXT("Could not find spawnable object with Unique ID %s in the Cavrnus Spatial Connector"), *Identifier);
		return nullptr;
	}

	auto actor = SpawnObjectHelpers::GetSpawnObjectHelpers()->SpawnObjectAndSetup(world, SpawnableIdentifiers[Identifier], SpawnedObject);

	spawnedActors.Add(FPropertiesContainer(SpawnedObject.PropertiesContainerName), actor);

	return actor;
}

void SpawnedObjectsManager::UnregisterSpawnedObject(const FCavrnusSpawnedObject& SpawnedObject, UWorld* World)
{
	if (!spawnedActors.Contains(FPropertiesContainer(SpawnedObject.PropertiesContainerName)))
	{
		UE_LOG(LogCavrnusConnector, Error, TEXT("Failed to destroy actor, could not find spawned object with Container Name %s"), *SpawnedObject.PropertiesContainerName);
		return;
	}

	spawnedActors[FPropertiesContainer(SpawnedObject.PropertiesContainerName)]->Destroy();
}

void SpawnedObjectsManager::Clear()
{
	for (auto a : spawnedActors)
	{
		if (a.Value.IsValid())
			a.Value->Destroy();
	}
	spawnedActors.Empty();
}

void SpawnedObjectsManager::RegisterSpawnableObjectType(const FString& Identifier, TSubclassOf<AActor> ClassType)
{
	if (SpawnableIdentifiers.Contains(Identifier))
	{
		SpawnableIdentifiers[Identifier] = ClassType;
	}
	else
	{
		SpawnableIdentifiers.Add(Identifier, ClassType);
	}
}

void SpawnedObjectsManager::UnregisterSpawnableObjectType(const FString& Identifier)
{
	if (SpawnableIdentifiers.Contains(Identifier))
	{
		SpawnableIdentifiers.Remove(Identifier);
	}
}
