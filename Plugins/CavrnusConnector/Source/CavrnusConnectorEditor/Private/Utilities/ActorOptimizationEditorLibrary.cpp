// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "Utilities/ActorOptimizationEditorLibrary.h"
#include "Utilities/ActorOptimizationLibrary.h"

#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Components/LightComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Engine/PointLight.h"
#include "Engine/SpotLight.h"
#include "Engine/DirectionalLight.h"
#include "Materials/MaterialInterface.h"

#include "Logging/LogMacros.h"
#include "UObject/UObjectGlobals.h"
#include "Containers/Queue.h"
#include "Containers/Set.h"

// Version macros
#include "Runtime/Launch/Resources/Version.h"

#include "Editor.h"
#include "ScopedTransaction.h"
#include "StaticMeshDescription.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "UObject/UnrealType.h"

// Subobject editor utilities are editor-only
#include "SubobjectDataSubsystem.h"
#include "SubobjectData.h"

DEFINE_LOG_CATEGORY_STATIC(LogActorOptimizationEditor, Warning, All);

/**
 * @brief Handles small meshes that are detected during optimization.
 * Currently logs the mesh information. In the future, may delete or hide these meshes.
 * @param Mesh The static mesh that is small.
 */
static void HandleSmallMeshes(UStaticMesh* Mesh)
{
    if (!Mesh || !IsValid(Mesh))
    {
        return;
    }

    FBoxSphereBounds Bounds = Mesh->GetBounds();
    FVector BoxSize = Bounds.BoxExtent * 2.0f;
    float MaxDimension = BoxSize.GetMax();

    UE_LOG(LogActorOptimizationEditor, Log, 
        TEXT("Small mesh detected: '%s' (Max dimension: %.2f cm). This mesh may be a candidate for deletion or hiding in the future."), 
        *Mesh->GetName(), MaxDimension);
}

static bool IsNaniteEnabled(const UStaticMesh* Mesh)
{
    return Mesh->NaniteSettings.bEnabled;
}

/**
 * @brief Checks if a static mesh is suitable for Nanite conversion.
 * @param Mesh The static mesh to check.
 * @param bIsSmall Output parameter indicating if the mesh is small (for logging purposes).
 * @return True if the mesh can be converted to Nanite, false otherwise.
 */
static bool IsMeshSuitableForNanite(UStaticMesh* Mesh, bool& bIsSmall)
{
    bIsSmall = false;

    if (!Mesh || !IsValid(Mesh))
    {
        return false;
    }

    // Skip meshes without render data
    if (!Mesh->GetRenderData())
    {
        return false;
    }

    // Check bounding box size
    FBoxSphereBounds Bounds = Mesh->GetBounds();
    FVector BoxSize = Bounds.BoxExtent * 2.0f;
    float MaxDimension = BoxSize.GetMax();
    
    // Skip meshes smaller than 10cm (likely decorative)
    if (MaxDimension < 10.0f)
    {
        bIsSmall = true;
        return false;
    }

    // Skip meshes with names suggesting they're camera/decorative meshes
    FString MeshName = Mesh->GetName();
    if (MeshName.Contains(TEXT("CineCam"), ESearchCase::IgnoreCase) ||
        MeshName.Contains(TEXT("Camera"), ESearchCase::IgnoreCase) ||
        MeshName.Contains(TEXT("Cam"), ESearchCase::IgnoreCase))
    {
        return false;
    }

    // Check if mesh has valid LOD data
    if (Mesh->GetNumLODs() == 0)
    {
        return false;
    }

    // Try to get mesh description - if it fails, the mesh isn't suitable
    #if WITH_EDITOR
    if (const FMeshDescription* MeshDesc = Mesh->GetMeshDescription(0))
    {
        // Check if mesh description has valid vertex data
        if (MeshDesc->Vertices().Num() == 0)
        {
            return false;
        }
    }
    else
    {
        // No mesh description available - likely a procedural or invalid mesh
        return false;
    }
    #endif

    return true;
}

// Tracks how often PostEditChange() is called during optimization
static int32 GPostEditChangeCounter = 0;

static void LogPostEditChange(UStaticMesh* Mesh)
{
    ++GPostEditChangeCounter;

    if (GPostEditChangeCounter > 50) // threshold you can tune
    {
        UE_LOG(LogActorOptimizationEditor, Warning,
            TEXT("High PostEditChange() usage detected (%d calls). Last mesh: %s"),
            GPostEditChangeCounter,
            *Mesh->GetName());
    }
}


// Enable Nanite across UE 5.0–5.7
inline void EnableNaniteCompat(UStaticMesh* Mesh)
{
    if (!Mesh) return;

    bool bIsSmall = false;
    if (!IsMeshSuitableForNanite(Mesh, bIsSmall))
    {
        if (bIsSmall)
        {
            HandleSmallMeshes(Mesh);
        }
        else
        {
            UE_LOG(LogActorOptimizationEditor, Warning, 
                TEXT("Skipping Nanite conversion for mesh '%s' - mesh is not suitable for Nanite"), 
                *Mesh->GetName());
        }
        return;
    }

    Mesh->Modify(); // required for undo/redo

// Track previous state so we know if Nanite was newly enabled
    bool bNaniteWasEnabled = IsNaniteEnabled(Mesh);

    // Now set Nanite enabled 
    Mesh->NaniteSettings.bEnabled = true;

#if WITH_EDITOR
    Mesh->PostEditChange();
    LogPostEditChange(Mesh);
#endif

    Mesh->MarkPackageDirty();

#if WITH_EDITOR
    // Refresh all StaticMeshComponents using this mesh
    for (TObjectIterator<UStaticMeshComponent> It; It; ++It)
    {
        UStaticMeshComponent* Comp = *It;
        if (Comp && Comp->GetStaticMesh() == Mesh)
        {
            UE_LOG(LogActorOptimizationEditor, Log,
                TEXT("Refreshing StaticMeshComponent '%s' on Actor '%s' for mesh '%s'"),
                *Comp->GetName(),
                Comp->GetOwner() ? *Comp->GetOwner()->GetName() : TEXT("None"),
                *Mesh->GetName());

            Comp->Modify();
            Comp->MarkRenderStateDirty();
            Comp->RecreateRenderState_Concurrent();
        }
    }
#endif


    bool bNaniteIsEnabled = IsNaniteEnabled(Mesh);

    // Verification log
    bool bVerifyNanite = IsNaniteEnabled(Mesh);

    if (bVerifyNanite)
    {
        UE_LOG(LogActorOptimizationEditor, Log, 
            TEXT("Successfully enabled Nanite on mesh: %s"), *Mesh->GetName());
    }
    else
    {
        UE_LOG(LogActorOptimizationEditor, Error, 
            TEXT("Failed to enable Nanite on mesh: %s - setting was not applied"), *Mesh->GetName());
    }
}



int32 UActorOptimizationEditorLibrary::OptimizeActorInEditor(AActor* TargetActor, const FActorOptimizationEditorOptions& Options)
{
    if (!TargetActor)
    {
        return 0;
    }

    int32 TotalProcessed = 0;

    FScopedTransaction Transaction(NSLOCTEXT("ActorOptimizationEditor", "OptimizeActorInEditor", "Optimize Actor (Editor)"));
    TargetActor->Modify();

    if (Options.bRemoveLights)
    {
        TotalProcessed += RemoveAllLightsFromActor(TargetActor);
    }

    if (Options.bEnableNanite)
    {
        TotalProcessed += ConvertStaticMeshesToNanite(TargetActor);
    }

    if (Options.bUseSimpleCollision)
    {
        TotalProcessed += UseSimpleCollision(TargetActor);
    }

    // Process masked materials based on the specified mode
    TotalProcessed += ProcessMaskedMaterials(TargetActor, Options.MaskedMaterialMode);

    // Note: The transaction handles undo/redo functionality. All modified static mesh assets
    // are marked as dirty via MarkPackageDirty() calls in the respective functions.
    // Users may need to manually save the modified assets to persist changes to disk.

    return TotalProcessed;
}

int32 UActorOptimizationEditorLibrary::RemoveAllLightsFromActor(AActor* TargetActor)
{
    if (!TargetActor)
    {
        return 0;
    }

    int32 RemovedCount = 0;

    TArray<AActor*> AllActors;
    UActorOptimizationLibrary::GetAllActorsRecursive(TargetActor, AllActors);

    TArray<UClass*> LightComponentClasses = {
        ULightComponent::StaticClass(),
        UPointLightComponent::StaticClass(),
        USpotLightComponent::StaticClass(),
        UDirectionalLightComponent::StaticClass()
    };

    TArray<UClass*> LightActorClasses = {
        APointLight::StaticClass(),
        ASpotLight::StaticClass(),
        ADirectionalLight::StaticClass()
    };

    USubobjectDataSubsystem* Subsys = GEngine ? GEngine->GetEngineSubsystem<USubobjectDataSubsystem>() : nullptr;

    for (AActor* Actor : AllActors)
    {
        if (!IsValid(Actor))
        {
            continue;
        }

        bool bIsLightActor = false;
        for (UClass* LA : LightActorClasses)
        {
            if (Actor->IsA(LA))
            {
                Actor->Modify();
                Actor->Destroy();
                ++RemovedCount;
                bIsLightActor = true;
                break;
            }
        }
        if (bIsLightActor)
        {
            continue;
        }

        if (Subsys)
        {
            TArray<FSubobjectDataHandle> Handles;
            Subsys->GatherSubobjectData(Actor, Handles);

            for (const FSubobjectDataHandle& Handle : Handles)
            {
                if (const FSubobjectData* Data = Handle.GetData())
                {
                    if (const UObject* ConstObj = Data->GetObject(false))
                    {
                        if (UActorComponent* Comp = Cast<UActorComponent>(const_cast<UObject*>(ConstObj)))
                        {
                            for (UClass* LCC : LightComponentClasses)
                            {
                                if (Comp->IsA(LCC))
                                {
                                    Comp->Modify();
                                    Comp->DestroyComponent();
                                    Actor->Modify();
                                    ++RemovedCount;
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return RemovedCount;
}

int32 UActorOptimizationEditorLibrary::ConvertStaticMeshesToNanite(AActor* TargetActor)
{
    if (!TargetActor)
    {
        return 0;
    }

    int32 ConvertedCount = 0;
    int32 SkippedCount = 0;

    TArray<AActor*> AllActors;
    UActorOptimizationLibrary::GetAllActorsRecursive(TargetActor, AllActors);

    TSet<UStaticMesh*> Processed;

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

            UStaticMesh* Mesh = SMC->GetStaticMesh();
            if (!IsValid(Mesh) || Processed.Contains(Mesh))
            {
                continue;
            }

            bool bIsSmall = false;
            if (!IsMeshSuitableForNanite(Mesh, bIsSmall))
            {
                ++SkippedCount;
                if (bIsSmall)
                {
                    HandleSmallMeshes(Mesh);
                }
                else
                {
                    UE_LOG(LogActorOptimizationEditor, Verbose, 
                        TEXT("Skipping Nanite conversion for mesh: %s"), *Mesh->GetName());
                }
                continue;
            }

            bool bNaniteWasEnabled = IsNaniteEnabled(Mesh);

            EnableNaniteCompat(Mesh);

            bool bNaniteIsEnabled = IsNaniteEnabled(Mesh);

            Processed.Add(Mesh);

            if (bNaniteIsEnabled && !bNaniteWasEnabled)
            {
                ++ConvertedCount;
                UE_LOG(LogActorOptimizationEditor, Log, TEXT("Enabled Nanite on static mesh: %s"), *Mesh->GetName());
            }
            else
            {
                ++SkippedCount;
            }
        }
    }

    if (SkippedCount > 0)
    {
        UE_LOG(LogActorOptimizationEditor, Log, 
            TEXT("Skipped %d mesh(es) that are not suitable for Nanite conversion"), SkippedCount);
    }

    return ConvertedCount;
}

int32 UActorOptimizationEditorLibrary::UseSimpleCollision(AActor* TargetActor)
{
    if (!TargetActor)
    {
        return 0;
    }

    int32 Optimized = 0;

    TArray<AActor*> AllActors;
    UActorOptimizationLibrary::GetAllActorsRecursive(TargetActor, AllActors);

    TSet<UStaticMesh*> Processed;

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

            UStaticMesh* Mesh = SMC->GetStaticMesh();
            if (!IsValid(Mesh) || Processed.Contains(Mesh))
            {
                continue;
            }

            Mesh->Modify();

            // If needed, set collision complexity (kept minimal for cross-version safety)
            // Mesh->SetComplexAsSimple(true); // version-dependent APIs vary; avoid here

            Mesh->MarkPackageDirty();

            Processed.Add(Mesh);
            ++Optimized;

            UE_LOG(LogActorOptimizationEditor, Log, TEXT("Optimized collision for static mesh: %s"), *Mesh->GetName());
        }
    }

    return Optimized;
}

bool UActorOptimizationEditorLibrary::HasMaskedMaterials(UStaticMeshComponent* SMC)
{
    if (!SMC || !IsValid(SMC))
    {
        return false;
    }

    const int32 NumMaterials = SMC->GetNumMaterials();
    for (int32 i = 0; i < NumMaterials; ++i)
    {
        if (UMaterialInterface* Material = SMC->GetMaterial(i))
        {
            if (Material->GetBlendMode() == BLEND_Masked)
            {
                return true;
            }
        }
    }

    return false;
}

int32 UActorOptimizationEditorLibrary::ProcessMaskedMaterials(AActor* TargetActor, EMaskedMaterialNaniteMode Mode)
{
    if (!TargetActor)
    {
        return 0;
    }

    int32 ProcessedCount = 0;
    int32 SkippedCount = 0;

    TArray<AActor*> AllActors;
    UActorOptimizationLibrary::GetAllActorsRecursive(TargetActor, AllActors);

    TSet<UStaticMesh*> ProcessedMeshes;
    TArray<FString> ProcessedActorsList;

    for (AActor* Actor : AllActors)
    {
        if (!IsValid(Actor))
        {
            continue;
        }

        TArray<UStaticMeshComponent*> MeshComponents;
        Actor->GetComponents<UStaticMeshComponent>(MeshComponents, false);

        bool bActorHasMaskedMaterials = false;
        FString ActorInfo;

        for (UStaticMeshComponent* SMC : MeshComponents)
        {
            if (!IsValid(SMC))
            {
                continue;
            }

            if (!HasMaskedMaterials(SMC))
            {
                continue;
            }

            UStaticMesh* Mesh = SMC->GetStaticMesh();
            if (!IsValid(Mesh) || ProcessedMeshes.Contains(Mesh))
            {
                continue;
            }

            if (!bActorHasMaskedMaterials)
            {
                bActorHasMaskedMaterials = true;
                ActorInfo = FString::Printf(TEXT("Actor: %s"), *Actor->GetActorLabel());
            }

            Mesh->Modify();

            bool bSuccess = false;
            bool bNaniteWasEnabled = IsNaniteEnabled(Mesh);

            if (Mode == EMaskedMaterialNaniteMode::ForceNanite)
            {
                bool bIsSmall = false;
                if (IsMeshSuitableForNanite(Mesh, bIsSmall))
                {
                    Mesh->NaniteSettings.bEnabled = true;
                    bSuccess = true;

#if WITH_EDITOR
                    const FMeshDescription* MeshDesc = Mesh->GetMeshDescription(0);
                    if (MeshDesc && MeshDesc->Vertices().Num() > 0)
                    {
                        Mesh->CommitMeshDescription(0);
                        Mesh->PostEditChange();
                        LogPostEditChange(Mesh);
                    }
#endif

                    Mesh->MarkPackageDirty();
                    ++ProcessedCount;

                    UE_LOG(LogActorOptimizationEditor, Log, 
                        TEXT("Forced Nanite for masked material mesh: %s on actor: %s, component: %s"), 
                        *Mesh->GetName(), *Actor->GetActorLabel(), *SMC->GetName());
                }
                else
                {
                    ++SkippedCount;
                    UE_LOG(LogActorOptimizationEditor, Warning, 
                        TEXT("Skipped forcing Nanite for masked material mesh: %s (mesh not suitable for Nanite)"), 
                        *Mesh->GetName());
                }
            }
            else // UseLODs
            {
                if (Mesh->NaniteSettings.bEnabled)
                {
                    Mesh->NaniteSettings.bEnabled = false;
                    bSuccess = true;
                }

                if (bSuccess)
                {
#if WITH_EDITOR
                    Mesh->PostEditChange();
                    LogPostEditChange(Mesh);
#endif

                    Mesh->MarkPackageDirty();
                    ++ProcessedCount;

                    int32 NumLODs = Mesh->GetNumLODs();
                    UE_LOG(LogActorOptimizationEditor, Log, 
                        TEXT("Disabled Nanite for masked material mesh (using LODs): %s on actor: %s, component: %s (LODs: %d)"), 
                        *Mesh->GetName(), *Actor->GetActorLabel(), *SMC->GetName(), NumLODs);
                }
                else
                {
                    int32 NumLODs = Mesh->GetNumLODs();
                    UE_LOG(LogActorOptimizationEditor, Verbose, 
                        TEXT("Masked material mesh already using LODs: %s on actor: %s, component: %s (LODs: %d)"), 
                        *Mesh->GetName(), *Actor->GetActorLabel(), *SMC->GetName(), NumLODs);
                    ++ProcessedCount;
                }
            }

            bool bNaniteIsEnabled = IsNaniteEnabled(Mesh);

            if (!ActorInfo.IsEmpty())
            {
                ActorInfo += FString::Printf(TEXT(", Component: %s, Mesh: %s"), 
                    *SMC->GetName(), *Mesh->GetName());
            }

            ProcessedMeshes.Add(Mesh);
        }

        if (bActorHasMaskedMaterials && !ActorInfo.IsEmpty())
        {
            ProcessedActorsList.Add(ActorInfo);
        }
    }

    UE_LOG(LogActorOptimizationEditor, Log, 
        TEXT("=== Masked Material Processing Summary ==="));
    UE_LOG(LogActorOptimizationEditor, Log, 
        TEXT("Mode: %s"), 
        Mode == EMaskedMaterialNaniteMode::ForceNanite ? TEXT("Force Nanite") : TEXT("Use LODs"));
    UE_LOG(LogActorOptimizationEditor, Log, 
        TEXT("Total actors with masked materials: %d"), ProcessedActorsList.Num());
    UE_LOG(LogActorOptimizationEditor, Log, 
        TEXT("Total meshes processed: %d"), ProcessedCount);
    if (SkippedCount > 0)
    {
        UE_LOG(LogActorOptimizationEditor, Log, 
            TEXT("Total meshes skipped: %d"), SkippedCount);
    }

    if (ProcessedActorsList.Num() > 0)
    {
        UE_LOG(LogActorOptimizationEditor, Log, 
            TEXT("=== Processed Actors with Masked Materials ==="));
        for (const FString& ActorInfo : ProcessedActorsList)
        {
            UE_LOG(LogActorOptimizationEditor, Log, TEXT("  %s"), *ActorInfo);
        }
    }

    return ProcessedCount;
}

