#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/DataAsset.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetManager/CavrnusBaseDataAsset.h"
#include "CavrnusDataAssetRegistry.generated.h"

UCLASS(BlueprintType)
class CAVRNUSCONNECTOR_API UCavrnusDataAssetRegistry : public UCavrnusBaseDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cavrnus | Assets")
    TArray<TSoftObjectPtr<UCavrnusBaseDataAsset>> ManagedAssets;

    template<typename T>
    T* GetAsset() const
    {
        static_assert(TIsDerivedFrom<T, UCavrnusBaseDataAsset>::IsDerived, "T must derive from UCavrnusBaseDataAsset");

        for (const TSoftObjectPtr<UCavrnusBaseDataAsset>& AssetRef : ManagedAssets)
        {
            if (UCavrnusBaseDataAsset* BaseAsset = AssetRef.Get())
            {
                if (T* Typed = Cast<T>(BaseAsset))
                {
                    return Typed;
                }
            }
        }
        return nullptr;
    }
};