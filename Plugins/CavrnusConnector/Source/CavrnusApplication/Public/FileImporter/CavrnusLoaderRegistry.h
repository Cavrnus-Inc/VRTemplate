// FileImporter/CavrnusLoaderRegistry.h (Application)
#pragma once

#include "CoreMinimal.h"
#include "Abstract/FileImporter/CavrnusLoaderRegistry_Abstract.h"
#include "FileImporter/CavrnusBaseLoader.h"

class CAVRNUSAPPLICATION_API FCavrnusLoaderRegistry final : public FCavrnusLoaderRegistry_Abstract
{
public:
    static FCavrnusLoaderRegistry& Instance()
    {
        static FCavrnusLoaderRegistry S;
        return S;
    }

    // Factory-based API per abstract contract
    virtual void RegisterLoader(const FString& Identifier,
        TFunction<UCavrnusBaseLoader_Abstract* (UObject* Outer)> Factory) override
    {
        Factories.Add(Identifier, Factory);
    }

    virtual UCavrnusBaseLoader_Abstract* CreateMatchingLoader(const FString& Identifier, UObject* Outer) override
    {
        if (TFunction<UCavrnusBaseLoader_Abstract* (UObject* Outer)>* Factory = Factories.Find(Identifier))
        {
            return (*Factory)(Outer);
        }
        return nullptr;
    }

private:
    TMap<FString, TFunction<UCavrnusBaseLoader_Abstract* (UObject* Outer)>> Factories;
};