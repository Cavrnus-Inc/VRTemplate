// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/CavrnusUI.h"
#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"

UCavrnusUISubsystem* UCavrnusUI::Get(const UObject* WorldContextObject)
{
    if (!WorldContextObject)
    {
        if (GEngine && GEngine->GameViewport)
        {
            if (UGameViewportClient* ViewportClient = GEngine->GameViewport.Get())
                WorldContextObject = ViewportClient->GetWorld();
        }
        
        if (!WorldContextObject)
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to get WorldContextObject from GameViewport! Returning nullptr."));
            return nullptr;
        }
    }

    UWorld* World = WorldContextObject->GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("No valid UWorld found!"));
        return nullptr;
    }

    if (UCavrnusUISubsystem* Subsystem = World->GetSubsystem<UCavrnusUISubsystem>())
    {
        return Subsystem;
    }

    UE_LOG(LogTemp, Error, TEXT("CavrnusUISubsystem cannot be found!"));
    return nullptr;
}