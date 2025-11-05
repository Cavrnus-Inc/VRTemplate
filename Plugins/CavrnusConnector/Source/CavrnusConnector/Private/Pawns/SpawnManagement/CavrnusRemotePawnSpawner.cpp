// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "Pawns/SpawnManagement/CavrnusRemotePawnSpawner.h"

#include "CavrnusFunctionLibrary.h"
#include "CavrnusSubsystem.h"
#include "AssetManager/CavrnusDataAssetManager.h"
#include "Kismet/GameplayStatics.h"
#include "Pawns/CavrnusPawnFunctions.h"

void UCavrnusRemotePawnSpawner::Initialize(const FCavrnusUser& InUser)
{
	Super::Initialize(InUser);

	UCavrnusFunctionLibrary::DefineStringPropertyDefaultValue(
		InUser.SpaceConn,
		InUser.PropertiesContainerName,
		UCavrnusSubsystem::Get()->Services->Get<UCavrnusDataAssetManager>()->GetAsset<UCavrnusPawnSettingsDataAsset>()->GetPropertyName(),
		UCavrnusSubsystem::Get()->Services->Get<UCavrnusDataAssetManager>()->GetAsset<UCavrnusPawnSettingsDataAsset>()->GetDefaultPawnTypeId());
	
	BindingIds.Add(UCavrnusFunctionLibrary::BindStringPropertyValue(
		InUser.SpaceConn,
		InUser.PropertiesContainerName,
		UCavrnusSubsystem::Get()->Services->Get<UCavrnusDataAssetManager>()->GetAsset<UCavrnusPawnSettingsDataAsset>()->GetPropertyName(),
		[this](const FString& PawnTypeId, const FString&, const FString&)
		{
			if (PawnTypeId.IsEmpty())
				return;
			
			HandlePawnSwitch(PawnTypeId);
		})->BindingId);
}

void UCavrnusRemotePawnSpawner::Teardown()
{
	Super::Teardown();

	TryDestroyingCurrentPawn();
	UCavrnusFunctionLibrary::UnbindWithId(AvatarVisBinding);
}

void UCavrnusRemotePawnSpawner::HandlePawnSwitch(const FString& PawnTypeId)
{
	if (PawnTypeId == UCavrnusSubsystem::Get()->Services->Get<UCavrnusDataAssetManager>()->GetAsset<UCavrnusPawnSettingsDataAsset>()->GetDefaultPawnTypeId())
		SetupPawn(*UCavrnusSubsystem::Get()->Services->Get<UCavrnusDataAssetManager>()->GetAsset<UCavrnusPawnSettingsDataAsset>()->GetDefaultPawnData());
	else
	{
		if (const FCavrnusPawnTypeData* PawnDataPtr = UCavrnusSubsystem::Get()->Services->Get<UCavrnusDataAssetManager>()->GetAsset<UCavrnusPawnSettingsDataAsset>()->FindPawnById(PawnTypeId))
			SetupPawn(*PawnDataPtr);
	}
}

void UCavrnusRemotePawnSpawner::TryDestroyingCurrentPawn()
{
	if (CurrentSpawnedPawnActor)
		CurrentSpawnedPawnActor->Destroy();

	CurrentSpawnedPawnActor = nullptr;
}

void UCavrnusRemotePawnSpawner::SetupPawn(const FCavrnusPawnTypeData& TargetPawnData)
{
	TryDestroyingCurrentPawn();

	UCavrnusFunctionLibrary::UnbindWithId(AvatarVisBinding);
	
	if (AActor* SpawnedPawn = Spawn(TargetPawnData.RemoteActor))
	{
		CurrentSpawnedPawnActor = SpawnedPawn;
		
		if (auto* Sc = UCavrnusPawnFunctions::CavrnusFindPawnComponent(SpawnedPawn))
		{
			UCavrnusFunctionLibrary::DefineBoolPropertyDefaultValue(User.SpaceConn, User.PropertiesContainerName, "AvatarVis", false);
			AvatarVisBinding = UCavrnusFunctionLibrary::BindBooleanPropertyValue(User.SpaceConn, User.PropertiesContainerName, "AvatarVis",
				[SpawnedPawn](const bool bVis, const FString&, const FString&)
				{
					SpawnedPawn->GetRootComponent()->SetVisibility(bVis, true);
				})->BindingId;
		
			Sc->NotifyRemotePawnReady(User);
			Sc->NotifyAnyPawnReady(User);
		}
	}
}

AActor* UCavrnusRemotePawnSpawner::Spawn(const TSubclassOf<AActor>& RemoteActor)
{
	const FTransform SpawnTransform = UCavrnusFunctionLibrary::GetTransformPropertyValue(User.SpaceConn, User.PropertiesContainerName, "Transform");
	
	AActor* SpawnedActor = GetWorld()->SpawnActorDeferred<AActor>(RemoteActor, SpawnTransform, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	UGameplayStatics::FinishSpawningActor(SpawnedActor, SpawnTransform);
	
	return SpawnedActor;
}
