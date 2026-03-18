// // Copyright (c) 2025 Cavrnus. All rights reserved.


#include "Modes/CavrnusModeManager.h"
#include "Core/Contexts/CavrnusRuntimeContext.h"
#include "AssetManager/CavrnusDataAssetManager.h"
#include "Modes/CavrnusExploreMode.h"
#include "CavrnusFunctionLibrary.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Core/Subsystems/CavrnusSubsystem.h"

void UCavrnusModeManager::Initialize()
{
	Super::Initialize();

	UCavrnusFunctionLibrary::AwaitAnySpaceConnection([this](FCavrnusSpaceConnection SpaceConnection)
	{
		UWorld* World = nullptr;
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if (Context.WorldType == EWorldType::Game || Context.WorldType == EWorldType::PIE)
			{
				World = Context.World();
				break;
			}
		}
		if (!World)
		{
			UE_LOG(LogTemp, Error, TEXT("UCavrnusModeManager::Initialize() Unable to Get World"));
		}
		else
		{
			UCavrnusSubsystem::Get()->RuntimeContext->Get<UCavrnusModeManager>()->SetExplicitMode<UCavrnusExploreMode>(World);
		}
	});
}