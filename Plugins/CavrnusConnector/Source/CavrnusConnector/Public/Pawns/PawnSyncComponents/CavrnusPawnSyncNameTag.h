// Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CavrnusPawnSyncComponentBase.h"
#include "CavrnusPawnSyncNameTag.generated.h"

class UCavrnusPawnComponent;

UCLASS(Blueprintable, ClassGroup=(Cavrnus), meta=(BlueprintSpawnableComponent))
class CAVRNUSCONNECTOR_API UCavrnusPawnSyncNameTag : public UCavrnusPawnSyncComponentBase
{
	GENERATED_BODY()
public:
	void Initialize(UCavrnusPawnComponent* PawnSetupComponent);

protected:
	virtual void HandleAnySync(const FCavrnusSpaceConnection& SpaceConnection, const FString& UserContainerName, const FCavrnusUser& CavrnusUser) override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Cavrnus|Pawn")
	void OnNameUpdated(const FString& DisplayName);

	UFUNCTION(BlueprintImplementableEvent, Category = "Cavrnus|Pawn")
	void OnIsLocalUser(bool bIsLocal);
};
