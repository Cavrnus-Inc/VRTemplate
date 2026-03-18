#pragma once

#include "CoreMinimal.h"
#include "DatasmithRuntime.h"
#include "FileImporter/CavrnusFileImporter.h"
#include "Managers/SpawnedObjects/CavrnusImportDelegates.h"
#include "FileImporter/DatasmithUtilities.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInterface.h"   // ✅ ensures UMaterialInterface is fully defined
#include "DatasmithRepairProcessor.generated.h"

namespace MeshUtils
{
    inline USkeletalMesh* ResolveMesh(const USkeletalMeshComponent* Comp)
    {
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION == 0
        // UE 5.0 still exposed SkeletalMesh directly
        return Comp->SkeletalMesh;
/*
#elif ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION < 1
        return Comp->GetSkeletalMesh();
*/
#elif ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1
        // UE 5.5+ unified under SkinnedAsset — cast back to USkeletalMesh
        return Cast<USkeletalMesh>(Comp->GetSkinnedAsset());
#else
        // Fallback for future versions
        return nullptr;
#endif
    }

    inline UStaticMesh* ResolveMesh(const UStaticMeshComponent* Comp)
    {
        return Comp->GetStaticMesh();
    }

    static FString GetStaticMeshPath(const UStaticMeshComponent* StaticMeshComp)
    {
        if (!StaticMeshComp) return FString();
        if (UStaticMesh* StaticMesh = StaticMeshComp->GetStaticMesh())
        {
            return StaticMesh->GetPathName();
        }
        return FString();
    }

    // UE5.0–5.7 skeletal accessor shim
    static FString GetSkeletalMeshPath(const USkeletalMeshComponent* SkeletalMeshComp)
    {
        if (!SkeletalMeshComp) return FString();

        // Use version-appropriate accessor to avoid deprecation warnings and future breakage.
#if (ENGINE_MAJOR_VERSION > 5) || (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1)
        // UE5.1+ preferred API
        if (USkeletalMesh* SkeletalMesh = SkeletalMeshComp->GetSkeletalMeshAsset())
        {
            return SkeletalMesh->GetPathName();
        }
#else
        // UE5.0 legacy member
        if (USkeletalMesh* SkeletalMesh = SkeletalMeshComp->SkeletalMesh)
        {
            return SkeletalMesh->GetPathName();
        }
#endif
        return FString();
    }
}

UCLASS()
class CAVRNUSAPPLICATION_API UDatasmithRepairProcessor : public UObject
{
    GENERATED_BODY()

public:
    // Repairs materials using MaterialMap (key: material PathName) and MeshSlotMap (key: mesh asset PathName).
    static void RepairActorMaterials(
        ADatasmithRuntimeActor* DatasmithActor,
        const TMap<FString, FDatasmithMaterialMetadata>& MaterialMap,
        const TMap<FString, FDatasmithMeshSlotMetadata>& MeshSlotMap,
        TFunction<void(const FCavrnusImportStatus&)> ReportProgress);

private:
    // Returns the asset path for a mesh component's underlying mesh (static or skeletal).
    // Empty string if unknown or not supported.
    static FString ResolveMeshAssetPath(const UMeshComponent* Mesh);
};
