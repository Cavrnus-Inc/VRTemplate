#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "CavrnusBaseDataAsset.generated.h"


UCLASS(BlueprintType)
class CAVRNUSCONNECTOR_API UCavrnusBaseDataAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    virtual void Initialize() {};
    virtual void Deinitialize() {};

};


