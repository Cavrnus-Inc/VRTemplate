// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "Pawns/PawnSyncComponents/CavrnusPawnSyncColor.h"
#include "CavrnusFunctionLibrary.h"
#include "Pawns/CavrnusPawnFunctions.h"

void UCavrnusPawnSyncColor::Initialize(
	UCavrnusPawnComponent* PawnSetupComponent,
	const FString& PropertyName,
	const TArray<UPrimitiveComponent*>& InMeshArray)
{
	MeshArray = InMeshArray;
	ColorPropName = PropertyName.IsEmpty() ? "primaryColor" : PropertyName;
	
	InitializePawnSetupComponent(PawnSetupComponent);
}

void UCavrnusPawnSyncColor::HandleAnySync(
	const FCavrnusSpaceConnection& SpaceConnection,
	const FString& UserContainerName,
	const FCavrnusUser& CavrnusUser)
{
	Super::HandleAnySync(SpaceConnection, UserContainerName, CavrnusUser);
	
	BindingIds.Add(UCavrnusFunctionLibrary::BindColorPropertyValue(
		SpaceConnection,
		UserContainerName,
		TEXT("primaryColor"),
		[this](const FLinearColor& Val, const FString&, const FString&)
	{
	})->BindingId);
}
