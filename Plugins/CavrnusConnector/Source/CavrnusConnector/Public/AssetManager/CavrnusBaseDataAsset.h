#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "CavrnusBaseDataAsset.generated.h"


UCLASS(BlueprintType)
class CAVRNUSCONNECTOR_API UCavrnusBaseDataAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cavrnus|Internal")
    int32 AssetVersion = 0;

    virtual void Initialize() {};
    virtual void Deinitialize() {};

};


