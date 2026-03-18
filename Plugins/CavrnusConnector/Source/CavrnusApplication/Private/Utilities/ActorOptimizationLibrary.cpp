// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "Utilities/ActorOptimizationLibrary.h"

#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"

#include "Logging/LogMacros.h"
#include "UObject/UObjectGlobals.h"
#include "Containers/Queue.h"
#include "Containers/Set.h"

DEFINE_LOG_CATEGORY_STATIC(LogActorOptimization, Warning, All);

void UActorOptimizationLibrary::GetAllActorsRecursive(AActor* RootActor, TArray<AActor*>& OutActors)
{
    if (!RootActor)
    {
        return;
    }

    // Use a set to avoid duplicates or circular references
    TSet<AActor*> Visited;
    TQueue<AActor*> ActorQueue;

    ActorQueue.Enqueue(RootActor);
    Visited.Add(RootActor);

    while (!ActorQueue.IsEmpty())
    {
        AActor* CurrentActor = nullptr;
        ActorQueue.Dequeue(CurrentActor);

        if (!CurrentActor)
        {
            continue;
        }

        OutActors.Add(CurrentActor);

        // Get all attached child actors
        TArray<AActor*> AttachedActors;
        CurrentActor->GetAttachedActors(AttachedActors);

        for (AActor* ChildActor : AttachedActors)
        {
            if (ChildActor && !Visited.Contains(ChildActor))
            {
                Visited.Add(ChildActor);
                ActorQueue.Enqueue(ChildActor);
            }
        }
    }
}

int32 UActorOptimizationLibrary::OptimizeActor(AActor* TargetActor, const FActorOptimizationOptions& Options)
{
    if (!TargetActor)
    {
        return 0;
    }

    int32 TotalProcessed = 0;

    if (Options.bForceLOD)
    {
        TotalProcessed += ForceLODOnStaticMeshes(TargetActor, Options.LODIndex);
    }

    if (Options.bForceMinimumLOD)
    {
        TotalProcessed += ForceMinimumLODOnStaticMeshes(TargetActor, Options.MinimumLODIndex);
    }

    if (Options.bDisableShadowCasting)
    {
        TotalProcessed += DisableShadowCasting(TargetActor);
    }

    if (Options.bDisableCollisionOnDecorative)
    {
        TotalProcessed += DisableCollisionOnDecorative(TargetActor);
    }

    if (Options.bOptimizeMaterials)
    {
        TotalProcessed += OptimizeMaterials(TargetActor);
    }

    return TotalProcessed;
}

int32 UActorOptimizationLibrary::ForceLODOnStaticMeshes(AActor* TargetActor, int LODIndex)
{
    if (!TargetActor)
    {
        return 0;
    }

    int32 Count = 0;

    TArray<AActor*> AllActors;
    GetAllActorsRecursive(TargetActor, AllActors);

    for (AActor* Actor : AllActors)
    {
        if (!IsValid(Actor))
        {
            continue;
        }

        TArray<UStaticMeshComponent*> MeshComponents;
        Actor->GetComponents<UStaticMeshComponent>(MeshComponents, false);

        for (UStaticMeshComponent* SMC : MeshComponents)
        {
            if (!IsValid(SMC))
            {
                continue;
            }

            // 0 means no forced LOD in UE; if intent is "always highest detail", use 1
            SMC->SetForcedLodModel(LODIndex);
            ++Count;
        }
    }

    return Count;
}

int32 UActorOptimizationLibrary::ForceMinimumLODOnStaticMeshes(AActor* TargetActor, int MinimumLODIndex)
{
    if (!TargetActor)
    {
        return 0;
    }

    int32 Count = 0;

    TArray<AActor*> AllActors;
    GetAllActorsRecursive(TargetActor, AllActors);

    for (AActor* Actor : AllActors)
    {
        if (!IsValid(Actor))
        {
            continue;
        }

        TArray<UStaticMeshComponent*> MeshComponents;
        Actor->GetComponents<UStaticMeshComponent>(MeshComponents, false);

        for (UStaticMeshComponent* SMC : MeshComponents)
        {
            if (!IsValid(SMC))
            {
                continue;
            }

            // Set minimum LOD on the component (runtime-safe, doesn't modify assets)
            // Note: SetMinLOD() is for skeletal meshes only. For UStaticMeshComponent, we use direct property assignment.
            SMC->MinLOD = MinimumLODIndex;
            ++Count;
        }
    }

    return Count;
}

int32 UActorOptimizationLibrary::DisableShadowCasting(AActor* TargetActor)
{
    if (!TargetActor)
    {
        return 0;
    }

    int32 Modified = 0;

    TArray<AActor*> AllActors;
    GetAllActorsRecursive(TargetActor, AllActors);

    for (AActor* Actor : AllActors)
    {
        if (!IsValid(Actor))
        {
            continue;
        }

        TArray<UStaticMeshComponent*> MeshComponents;
        Actor->GetComponents<UStaticMeshComponent>(MeshComponents, false);

        for (UStaticMeshComponent* SMC : MeshComponents)
        {
            if (!IsValid(SMC))
            {
                continue;
            }

            SMC->SetCastShadow(false);
            ++Modified;
        }
    }

    return Modified;
}

int32 UActorOptimizationLibrary::DisableCollisionOnDecorative(AActor* TargetActor)
{
    if (!TargetActor)
    {
        return 0;
    }

    int32 Modified = 0;

    TArray<AActor*> AllActors;
    GetAllActorsRecursive(TargetActor, AllActors);

    for (AActor* Actor : AllActors)
    {
        if (!IsValid(Actor))
        {
            continue;
        }

        TArray<UStaticMeshComponent*> MeshComponents;
        Actor->GetComponents<UStaticMeshComponent>(MeshComponents, false);

        for (UStaticMeshComponent* SMC : MeshComponents)
        {
            if (!IsValid(SMC))
            {
                continue;
            }

            const FBoxSphereBounds Bounds = SMC->Bounds;
            const FVector BoxSize = Bounds.BoxExtent * 2.0f;

            const bool bIsSmall = BoxSize.GetMax() < 100.0f;
            const bool bHasNoCollision = SMC->GetCollisionEnabled() == ECollisionEnabled::NoCollision;

            if (bIsSmall && !bHasNoCollision)
            {
                SMC->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                ++Modified;
            }
        }
    }

    return Modified;
}

int32 UActorOptimizationLibrary::OptimizeMaterials(AActor* TargetActor)
{
    if (!TargetActor)
    {
        return 0;
    }

    int32 Checked = 0;

    TArray<AActor*> AllActors;
    GetAllActorsRecursive(TargetActor, AllActors);

    for (AActor* Actor : AllActors)
    {
        if (!IsValid(Actor))
        {
            continue;
        }

        TArray<UStaticMeshComponent*> MeshComponents;
        Actor->GetComponents<UStaticMeshComponent>(MeshComponents, false);

        for (UStaticMeshComponent* SMC : MeshComponents)
        {
            if (!IsValid(SMC))
            {
                continue;
            }

            const int32 NumMaterials = SMC->GetNumMaterials();
            for (int32 i = 0; i < NumMaterials; ++i)
            {
                if (UMaterialInterface* MI = SMC->GetMaterial(i))
                {
                    ++Checked;
                    UE_LOG(LogActorOptimization, Verbose, TEXT("Checked material: %s on component: %s"),
                        *MI->GetName(), *SMC->GetName());
                }
            }
        }
    }

    return Checked;
}
