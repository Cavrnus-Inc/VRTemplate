#pragma once

#include "CoreMinimal.h"
#include "AssetTypeActions_Base.h"

class FCavrnusAssetActionsRegistry
{
public:
    static void RegisterOverrides();
    static void UnregisterOverrides();

private:
    static void TryRegister(UClass* ClassType);
    static TArray<TSharedPtr<IAssetTypeActions>> Registered;
};
