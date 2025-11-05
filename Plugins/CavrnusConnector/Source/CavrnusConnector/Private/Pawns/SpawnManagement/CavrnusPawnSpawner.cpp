// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "Pawns/SpawnManagement/CavrnusPawnSpawner.h"
#include "CavrnusFunctionLibrary.h"

void UCavrnusPawnSpawner::Initialize(const FCavrnusUser& InUser)
{
	User = InUser;
}

void UCavrnusPawnSpawner::Teardown()
{
	for (const FString& Bid : BindingIds)
		UCavrnusFunctionLibrary::UnbindWithId(Bid);

	BindingIds.Empty();
}
