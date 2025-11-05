// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "SpawnObjectHelpers.h"
#include "CavrnusConnectorModule.h"
#include "CavrnusFunctionLibrary.h"
#include "FlagComponents\CavrnusUserAvatarFlag.h"
#include "FlagComponents\CavrnusSpawnedObjectFlag.h"
#include <Engine/World.h>
#include <Kismet/GameplayStatics.h>

SpawnObjectHelpers* SpawnObjectHelpers::Instance = nullptr;

SpawnObjectHelpers* SpawnObjectHelpers::GetSpawnObjectHelpers()
{
	if (Instance == nullptr)
		Instance = new SpawnObjectHelpers();
	return Instance;
}

void SpawnObjectHelpers::Kill()
{
	if (Instance != nullptr)
	{
		delete Instance;
		Instance = nullptr;
	}
}

AActor* SpawnObjectHelpers::SpawnObjectAndSetup(UWorld* World, TSubclassOf<AActor> ActorClass, const FCavrnusSpawnedObject& SpawnedObject)
{
	FTransform SpawnTransform = UCavrnusFunctionLibrary::GetTransformPropertyValue(SpawnedObject.SpaceConnection, SpawnedObject.PropertiesContainerName, "Transform");

	AActor* SpawnedActor = World->SpawnActorDeferred<AActor>(ActorClass, SpawnTransform, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

	objectSpawns.Add(SpawnedActor, SpawnedObject);

	UGameplayStatics::FinishSpawningActor(SpawnedActor, SpawnTransform);

	// Complete setup for Actors that did not set up via an UCavrnusPropertiesContainer
	if (!SpawnedActor->GetComponentByClass(UCavrnusSpawnedObjectFlag::StaticClass()))
	{
		InitializeCavrnusActor(SpawnedActor);
	}

	return SpawnedActor;
}

void SpawnObjectHelpers::InitializeCavrnusActor(AActor* InActor)
{
	if (objectSpawns.Contains(InActor))
	{
		const FCavrnusSpawnedObject CavrnusSpawnedObject = objectSpawns[InActor];
		objectSpawns.Remove(InActor);
		HandleSpawnedObject(InActor, CavrnusSpawnedObject);
	}
}

void SpawnObjectHelpers::HandleSpawnedObject(AActor* actor, const FCavrnusSpawnedObject& object)
{
	UCavrnusSpawnedObjectFlag* ActorComponent = Cast<UCavrnusSpawnedObjectFlag>(actor->AddComponentByClass(
		UCavrnusSpawnedObjectFlag::StaticClass(),
		true,
		FTransform::Identity,
		false));

	ActorComponent->SpawnedObject = object;
	ActorComponent->AttachToComponent(actor->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);

	SetupObjectPropsContainers(actor, object.PropertiesContainerName);
}

AActor* SpawnObjectHelpers::SetupObjectPropsContainers(AActor* spawnedActor, const FString& propsContainerName)
{
	if (spawnedActor == nullptr)
	{
		UE_LOG(LogCavrnusConnector, Error, TEXT("Failed to successfully spawn actor"));
		return nullptr;
	}

	TArray<USceneComponent*> PropertiesContainers;
	USceneComponent* ActorRoot = spawnedActor->GetRootComponent();

	if (ActorRoot == nullptr)
	{
		UE_LOG(LogCavrnusConnector, Error, TEXT("Failed to successfully detect spawned actor root"));
		return nullptr;
	}

	TArray<USceneComponent*> RootChildren;
	ActorRoot->GetChildrenComponents(false, RootChildren);
	PropertiesContainers = RootChildren.FilterByPredicate([](USceneComponent* SceneComponent) {
		return SceneComponent->IsA<UCavrnusPropertiesContainer>();
	});

	if (PropertiesContainers.IsEmpty())
	{
		// Add a properties container to the root if none found
		UCavrnusPropertiesContainer* RootContainer = Cast<UCavrnusPropertiesContainer>(spawnedActor->AddComponentByClass(
			UCavrnusPropertiesContainer::StaticClass(),
			true,
			FTransform::Identity,
			false));

		if (RootContainer)
		{
			RootContainer->AttachToComponent(ActorRoot, FAttachmentTransformRules::KeepRelativeTransform);
		}

		ActorRoot->GetChildrenComponents(false, RootChildren);
		PropertiesContainers = RootChildren.FilterByPredicate([](USceneComponent* SceneComponent) {
			return SceneComponent->IsA<UCavrnusPropertiesContainer>();
		});
	}

	checkf(PropertiesContainers.Num() == 1, TEXT("Expected a single properties container component on the root of the actor"));
	for (USceneComponent* Component : PropertiesContainers)
	{
		if (UCavrnusPropertiesContainer* PropertiesContainer = Cast<UCavrnusPropertiesContainer>(Component))
		{
			PropertiesContainer->ContainerName = propsContainerName;
		}
	}

	SpawnObjectHelpers::ResetLiveHierarchyRootName(spawnedActor, propsContainerName);

	return spawnedActor;
}

void SpawnObjectHelpers::ResetLiveHierarchyRootName(AActor* Actor, const FString& NewRootName)
{
	//What is the ContainerName of the top-level props container? That is what we'll be replacing.
	TArray<UCavrnusPropertiesContainer*> RootComponent;
	Actor->GetComponents(RootComponent);
	//If nothing on the root, do nothing
	if (RootComponent.Num() == 0)
		return;
	FString rootString = RootComponent[0]->ContainerName;

	TArray<UCavrnusPropertiesContainer*> PropertiesContainers;
	Actor->GetComponents(PropertiesContainers, true);
	for (UCavrnusPropertiesContainer* PropertiesContainer : PropertiesContainers)
	{
		ReplacePlaceholderInPropertiesContainer(PropertiesContainer, rootString, NewRootName);
	}
}

void SpawnObjectHelpers::ReplacePlaceholderInPropertiesContainer(UCavrnusPropertiesContainer* PropertiesContainer, const FString& Placeholder, const FString& ReplacementString)
{
	FString ContainerName = PropertiesContainer->ContainerName;
	if (ContainerName.IsEmpty())
	{
		PropertiesContainer->ContainerName = ReplacementString;
	}
	else if (ContainerName.StartsWith(Placeholder))
	{
		ContainerName.RemoveFromStart(Placeholder);
		ContainerName.InsertAt(0, ReplacementString);
		PropertiesContainer->ContainerName = ContainerName;
	}
}
