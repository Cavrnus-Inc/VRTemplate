// Abstract/FileImporter/CavrnusLoaderRegistry_Abstract.cpp
#include "Abstract/FileImporter/CavrnusLoaderRegistry_Abstract.h"

FCavrnusLoaderRegistry_Abstract* FCavrnusLoaderRegistry_Abstract::Singleton = nullptr;

FCavrnusLoaderRegistry_Abstract& FCavrnusLoaderRegistry_Abstract::Get()
{
    checkf(Singleton != nullptr, TEXT("FCavrnusLoaderRegistry_Abstract::Set must be called by CavrnusApplication before use"));
    return *Singleton;
}

void FCavrnusLoaderRegistry_Abstract::Set(FCavrnusLoaderRegistry_Abstract* Impl)
{
    Singleton = Impl;
}
