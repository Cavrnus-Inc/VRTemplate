// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "CavrnusPropertiesContainer.h"
#include <Engine/World.h>
#include <GameFramework/Actor.h>
#include "SpawnObjectHelpers.h"

UCavrnusPropertiesContainer::UCavrnusPropertiesContainer()
{
	bWantsInitializeComponent = true;
}

void UCavrnusPropertiesContainer::InitializeComponent()
{
	Super::InitializeComponent();

	if (GetOwner() && (GetOwner()->GetWorld()->WorldType == EWorldType::Game || GetOwner()->GetWorld()->WorldType == EWorldType::PIE))
	{
		SpawnObjectHelpers::GetSpawnObjectHelpers()->InitializeCavrnusActor(this->GetOwner());
	}
}
