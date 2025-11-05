// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AssetManager/CavrnusBaseDataAsset.h"
#include "CavrnusPawnSettingsDataAsset.generated.h"

USTRUCT(BlueprintType)
struct FCavrnusPawnTypeData
{
	GENERATED_BODY()

	FCavrnusPawnTypeData() : RemoteActor(nullptr) {}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cavrnus|Pawn")
	TSubclassOf<AActor> RemoteActor;
};

UCLASS()
class CAVRNUSCONNECTOR_API UCavrnusPawnSettingsDataAsset : public UCavrnusBaseDataAsset
{
	GENERATED_BODY()
public:
	UCavrnusPawnSettingsDataAsset(): OtherRemotePawns() {}

	FString GetDefaultPawnTypeId() { return DefaultPropertyValue; }
	FString GetPropertyName() { return PropertyName; }
	
	FCavrnusPawnTypeData* GetDefaultPawnData()
	{
		return &DefaultRemotePawn;
	}
	
	FCavrnusPawnTypeData* FindPawnById(const FString& PawnId);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cavrnus|Pawn Configuration")
	FCavrnusPawnTypeData DefaultRemotePawn;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cavrnus|Pawn Configuration")
	TMap<FString, FCavrnusPawnTypeData> OtherRemotePawns;

private:
	FString DefaultPropertyValue = TEXT("default");
	FString PropertyName = TEXT("PawnType");
};
