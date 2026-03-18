// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "Pawns/PawnSyncComponents/CavrnusPawnSyncNameTag.h"
#include "CavrnusConnectorModule.h"
#include "CavrnusFunctionLibrary.h"
#include "Helpers/CavrnusUserHelpers.h"

void UCavrnusPawnSyncNameTag::Initialize(UCavrnusPawnComponent* PawnSetupComponent)
{
	InitializePawnSetupComponent(PawnSetupComponent);
}

void UCavrnusPawnSyncNameTag::HandleAnySync(
	const FCavrnusSpaceConnection& SpaceConnection,
	const FString& UserContainerName,
	const FCavrnusUser& CavrnusUser)
{
	Super::HandleAnySync(SpaceConnection, UserContainerName, CavrnusUser);

	OnIsLocalUser(CavrnusUser.IsLocalUser);

	BindingIds.Add(FCavrnusUserHelpers::BindUserName(CavrnusUser,
		[this, CavrnusUser](const FString& Name)
		{
			FString DisplayName = Name;
			if (DisplayName.IsEmpty())
				DisplayName = FCavrnusUserHelpers::GetUserEmail(CavrnusUser);
			if (CavrnusUser.IsLocalUser)
				DisplayName += TEXT(" (You)");

			OnNameUpdated(DisplayName);
		})->BindingId);
}
