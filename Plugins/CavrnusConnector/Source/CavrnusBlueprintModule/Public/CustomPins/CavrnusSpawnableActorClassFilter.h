// Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "ClassViewerFilter.h"
#include "CavrnusBlueprintModule.h" 
/**
 * @brief ClassViewer filter that restricts the class picker to a whitelist of allowed classes.
 */
class FCavrnusSpawnableActorClassFilter : public IClassViewerFilter
{
public:
    /** Whitelist of allowed classes */
    TArray<UClass*> AllowedClasses;

    /** Only allow classes explicitly in AllowedClasses */
    virtual bool IsClassAllowed(const FClassViewerInitializationOptions& InInitOptions,
        const UClass* InClass,
        TSharedRef<FClassViewerFilterFuncs> InFilterFuncs) override
    {
        if (!InClass)
        {
            return false;
        }

        // If no allowed classes specified, allow all (shouldn't happen, but be safe)
        if (AllowedClasses.Num() == 0)
        {
            UE_LOG(LogCavrnusBlueprintModule, Warning, TEXT("[Filter] No allowed classes - allowing all"));
            return true;
        }

        // Check if the class is in the allowed list
        bool bIsAllowed = AllowedClasses.FindByPredicate([InClass](const UClass* AllowedClass)
        {
            return AllowedClass == InClass;
        }) != nullptr;

        UE_LOG(LogCavrnusBlueprintModule, Warning, TEXT("[Filter] Class %s: %s"), 
                *InClass->GetPathName(), 
                bIsAllowed ? TEXT("ALLOWED") : TEXT("DENIED"));

        return bIsAllowed;
    }

    virtual bool IsUnloadedClassAllowed(const FClassViewerInitializationOptions& InInitOptions,
        const TSharedRef<const IUnloadedBlueprintData> InUnloadedClassData,
        TSharedRef<FClassViewerFilterFuncs> InFilterFuncs) override
    {
        return false;
    }
};
