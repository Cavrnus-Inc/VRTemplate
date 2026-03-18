#include "FileImporter/DatasmithRepairProcessor.h"
#include "FileImporter/DatasmithUtilities.h"
#include "FileImporter/DatasmithFileImporter.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "Core/Subsystems/CavrnusSubsystem.h"
#include "Core/Contexts/CavrnusRuntimeContext.h"
#include "Managers/SpawnedObjects/SpawnedObjectsManager.h"

FString UDatasmithRepairProcessor::ResolveMeshAssetPath(const UMeshComponent* Mesh)
{
    if (!Mesh) return FString();

    if (const UStaticMeshComponent* StaticMeshComp = Cast<UStaticMeshComponent>(Mesh))
    {
        return MeshUtils::GetStaticMeshPath(StaticMeshComp);
    }

    if (const USkeletalMeshComponent* SkeletalMeshComp = Cast<USkeletalMeshComponent>(Mesh))
    {
        return MeshUtils::GetSkeletalMeshPath(SkeletalMeshComp);
    }

    // Unsupported mesh type for this repair path
    return FString();
}

FString NormalizeMeshName(const UObject* MeshAsset)
{
    if (!MeshAsset) return FString();

    FString RawName = MeshAsset->GetName(); // e.g. "SM_545C0CCA4623746B79CF9499C19969BC_28"

    // Strip prefix and suffix
    FString Guid = RawName;
    Guid.RemoveFromStart(TEXT("SM_"));
    int32 UnderscoreIndex;
    if (Guid.FindLastChar(TEXT('_'), UnderscoreIndex))
    {
        Guid = Guid.Left(UnderscoreIndex);
    }
    return Guid;
}


void UDatasmithRepairProcessor::RepairActorMaterials(
    ADatasmithRuntimeActor* DatasmithActor,
    const TMap<FString, FDatasmithMaterialMetadata>& MaterialMap,
    const TMap<FString, FDatasmithMeshSlotMetadata>& MeshSlotMap,
    TFunction<void(const FCavrnusImportStatus&)> ReportProgress)
{
    if (!DatasmithActor) return;

    TArray<AActor*> ChildActors;
    UDatasmithFileImporter::GetAllRelevantActorsRecursive(DatasmithActor, ChildActors);

    for (AActor* Actor : ChildActors)
    {
        TArray<UMeshComponent*> Meshes;
        Actor->GetComponents<UMeshComponent>(Meshes);

        for (UMeshComponent* Mesh : Meshes)
        {
            // Resolve the asset path
            const FString MeshPath = ResolveMeshAssetPath(Mesh);
            if (MeshPath.IsEmpty())
            {
                continue;
            }

            // Normalize to the GUID‑like "name" used in <StaticMesh>
            // Example: /Game/Cars_Assets/AFEE4C2040D0DF1EC74F598DB165A374.udsmesh
            // → AFEE4C2040D0DF1EC74F598DB165A374
            FString MeshGuid;
            if (const UStaticMeshComponent* StaticMeshComp = Cast<UStaticMeshComponent>(Mesh))
            {
                MeshGuid = NormalizeMeshName(MeshUtils::ResolveMesh(StaticMeshComp));
            }
            else if (const USkeletalMeshComponent* SkeletalMeshComp = Cast<USkeletalMeshComponent>(Mesh))
            {
                MeshGuid = NormalizeMeshName(MeshUtils::ResolveMesh(SkeletalMeshComp));
            }

            const FDatasmithMeshSlotMetadata* MeshMeta = MeshSlotMap.Find(MeshGuid);
            if (!MeshMeta) continue;

            const int32 NumSlots = Mesh->GetNumMaterials();
            for (int32 SlotIndex = 0; SlotIndex < NumSlots; ++SlotIndex)
            {
                UMaterialInterface* Current = Mesh->GetMaterial(SlotIndex);
                const bool bNeedsRepair = (!Current) || Current->GetName().Contains(TEXT("WorldGrid"));
                if (!bNeedsRepair) continue;

                if (SlotIndex >= MeshMeta->MaterialPaths.Num()) continue;

                const FString& IntendedId = MeshMeta->MaterialPaths[SlotIndex];
                if (IntendedId.IsEmpty()) continue;

                // Look up material metadata by GUID/name
                const FDatasmithMaterialMetadata* Meta = MaterialMap.Find(IntendedId);
                if (!Meta)
                {
                    UE_LOG(LogTemp, Error, TEXT("[Repair] GUID %s not found in MaterialMap"), *IntendedId);
                    continue;
                }

                UMaterialInterface* Loaded = Cast<UMaterialInterface>(
                    StaticLoadObject(UMaterialInterface::StaticClass(), nullptr, *Meta->PathName)
                );

                if (Loaded)
                {
                    Mesh->SetMaterial(SlotIndex, Loaded);

                    FCavrnusImportStatus Status;
                    Status.Progress = 0.9f;
                    Status.StatusMessage = TEXT("Repaired material slot");
                    Status.SecondaryMessage = FString::Printf(
                        TEXT("%s slot %d -> %s"),
                        *MeshGuid, SlotIndex, *Loaded->GetName());
                    Status.bSuccess = true;
                    ReportProgress(Status);
                }
                else
                {
                    FCavrnusImportStatus Warn;
                    Warn.Progress = 0.9f;
                    Warn.StatusMessage = TEXT("Failed to load material");
                    Warn.SecondaryMessage = FString::Printf(
                        TEXT("%s slot %d intended %s"),
                        *MeshGuid, SlotIndex, *Meta->PathName);
                    Warn.bSuccess = false;
                    ReportProgress(Warn);
                }
            }
        }
    }
}

