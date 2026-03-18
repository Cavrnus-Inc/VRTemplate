// Abstract/FileImporter/CavrnusLoaderRegistry_Abstract.h
#pragma once

#include "CoreMinimal.h"
#include "Abstract/FileImporter/CavrnusBaseLoader_Abstract.h"

class CAVRNUSCONNECTOR_API FCavrnusLoaderRegistry_Abstract
{
public:
    // Global accessor resolved entirely inside Connector
    static FCavrnusLoaderRegistry_Abstract& Get();

    // Application injects its implementation instance here
    static void Set(FCavrnusLoaderRegistry_Abstract* Impl);

    // Abstract contract
    virtual UCavrnusBaseLoader_Abstract* CreateMatchingLoader(const FString& Identifier, UObject* Outer) = 0;
    virtual void RegisterLoader(const FString& Identifier, TFunction<UCavrnusBaseLoader_Abstract* (UObject* Outer)> Factory) = 0;

protected:
    virtual ~FCavrnusLoaderRegistry_Abstract() = default;

private:
    static FCavrnusLoaderRegistry_Abstract* Singleton;
};
