// Copyright (c) 2025 Cavrnus, Inc. All rights reserved.

#include "Utilities/ActorOptimizationEditorTool.h"

#include "Utilities/ActorOptimizationEditorLibrary.h"
#include "Utilities/ActorOptimizationLibrary.h"

#include "Logging/LogMacros.h"
#include "UObject/UObjectGlobals.h"
#include "Containers/Queue.h"
#include "Editor.h"
#include "Engine/Selection.h"
#include "Components/StaticMeshComponent.h"
#include "Components/LightComponent.h"
#include "Engine/StaticMesh.h"
#include "StaticMeshDescription.h"
#include "Runtime/Launch/Resources/Version.h"

DEFINE_LOG_CATEGORY_STATIC(LogActorOptimizationTool, Warning, All);

// ---------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------
static bool IsNaniteEnabled_Local(const UStaticMesh* Mesh)
{
    return Mesh->NaniteSettings.bEnabled;
}

static bool IsMeshSuitableForNanite_Local(UStaticMesh* Mesh, bool& bIsSmall)
{
    bIsSmall = false;

    if (!Mesh || !IsValid(Mesh))
    {
        return false;
    }

    if (!Mesh->GetRenderData())
    {
        return false;
    }

    FBoxSphereBounds Bounds = Mesh->GetBounds();
    FVector BoxSize = Bounds.BoxExtent * 2.0f;
    float MaxDimension = BoxSize.GetMax();

    if (MaxDimension < 10.0f)
    {
        bIsSmall = true;
        return false;
    }

    FString MeshName = Mesh->GetName();
    if (MeshName.Contains(TEXT("CineCam"), ESearchCase::IgnoreCase) ||
        MeshName.Contains(TEXT("Camera"), ESearchCase::IgnoreCase) ||
        MeshName.Contains(TEXT("Cam"), ESearchCase::IgnoreCase))
    {
        return false;
    }

    if (Mesh->GetNumLODs() == 0)
    {
        return false;
    }

#if WITH_EDITOR
    if (const FMeshDescription* MeshDesc = Mesh->GetMeshDescription(0))
    {
        if (MeshDesc->Vertices().Num() == 0)
        {
            return false;
        }
    }
    else
    {
        return false;
    }
#endif

    return true;
}

#if WITH_EDITOR
static int32 GPostEditChangeCounter_Local = 0;
static void LogPostEditChange_Local(UStaticMesh* Mesh)
{
    ++GPostEditChangeCounter_Local;
    if (GPostEditChangeCounter_Local > 50)
    {
        UE_LOG(LogActorOptimizationTool, Warning,
            TEXT("High PostEditChange() usage detected (%d calls). Last mesh: %s"),
            GPostEditChangeCounter_Local,
            *Mesh->GetName());
    }
}
#endif

static bool HasMaskedMaterials_Local(UStaticMeshComponent* SMC)
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

// ---------------------------------------------------------------------
// Logging / status
// ---------------------------------------------------------------------
void UActorOptimizationEditorTool::AddLogLine(const FString& Line)
{
    LogLines.Add(Line);

    const int32 MaxLines = 2000;
    if (LogLines.Num() > MaxLines)
    {
        LogLines.RemoveAt(0, LogLines.Num() - MaxLines);
    }

    UE_LOG(LogActorOptimizationTool, Log, TEXT("%s"), *Line);
}

void UActorOptimizationEditorTool::EnableNaniteOnSingleMesh(UStaticMesh* Mesh)
{
    if (!Mesh) return;

    bool bIsSmall = false;
    if (!IsMeshSuitableForNanite_Local(Mesh, bIsSmall))
    {
        if (bIsSmall)
        {
            FBoxSphereBounds Bounds = Mesh->GetBounds();
            FVector BoxSize = Bounds.BoxExtent * 2.0f;
            float MaxDimension = BoxSize.GetMax();

            FString Msg = FString::Printf(
                TEXT("Small mesh detected (Nanite skipped): '%s' (Max dimension: %.2f cm)"),
                *Mesh->GetName(), MaxDimension);
            AddLogLine(Msg);
            ++NaniteSkippedCount;
        }
        else
        {
            AddLogLine(FString::Printf(
                TEXT("Skipping Nanite conversion for mesh '%s' - not suitable"),
                *Mesh->GetName()));
        }
        return;
    }

    Mesh->Modify();

    bool bWasNanite = IsNaniteEnabled_Local(Mesh);

    Mesh->NaniteSettings.bEnabled = true;

#if WITH_EDITOR
    Mesh->PostEditChange();
    LogPostEditChange_Local(Mesh);
#endif

    Mesh->MarkPackageDirty();

#if WITH_EDITOR
    for (TObjectIterator<UStaticMeshComponent> It; It; ++It)
    {
        UStaticMeshComponent* Comp = *It;
        if (Comp && Comp->GetStaticMesh() == Mesh)
        {
            Comp->Modify();
            Comp->MarkRenderStateDirty();
            Comp->RecreateRenderState_Concurrent();
        }
    }
#endif

    bool bIsNanite = IsNaniteEnabled_Local(Mesh);
    if (bIsNanite && !bWasNanite)
    {
        ++NaniteConvertedCount;
        AddLogLine(FString::Printf(TEXT("Enabled Nanite on static mesh: %s"), *Mesh->GetName()));
    }
    else
    {
        ++NaniteSkippedCount;
    }
}

void UActorOptimizationEditorTool::SetStatus(const FString& InStatus, float InProgress)
{
    ProgressStatusText = InStatus;
    Progress = FMath::Clamp(InProgress, 0.0f, 1.0f);
    UE_LOG(LogActorOptimizationTool, Verbose,
        TEXT("%s (Progress: %.2f)"), *InStatus, Progress);
}

// ---------------------------------------------------------------------
// Per-session history helpers
// ---------------------------------------------------------------------
FActorOptimizationRecord* UActorOptimizationEditorTool::FindRecordForActorOrParent(AActor* Actor)
{
    AActor* Current = Actor;

    while (Current)
    {
        if (FActorOptimizationRecord* Rec = OptimizationHistory.Find(Current))
        {
            return Rec;
        }

        Current = Current->GetAttachParentActor();
    }

    return nullptr;
}

void UActorOptimizationEditorTool::StoreRecordForActor(AActor* Actor)
{
    if (!Actor)
        return;

    FActorOptimizationRecord& Rec = OptimizationHistory.FindOrAdd(Actor);
    Rec.RunFlags = RunFlags;
    Rec.EditorParams = EditorOptions;
    Rec.RuntimeParams = RuntimeOptions;
    Rec.Timestamp = FDateTime::Now();
}

// ---------------------------------------------------------------------
// Analysis helpers
// ---------------------------------------------------------------------
void UActorOptimizationEditorTool::AnalyzeActorTree(AActor* Root)
{
    NumLights = 0;
    NumStaticMeshes = 0;
    NumNaniteMeshes = 0;
    NumMaskedMeshes = 0;

    if (!Root)
        return;

    TArray<AActor*> AllActors;
    UActorOptimizationLibrary::GetAllActorsRecursive(Root, AllActors);

    for (AActor* Actor : AllActors)
    {
        if (!IsValid(Actor))
            continue;

        TArray<UActorComponent*> Components;
        Actor->GetComponents(Components);

        for (UActorComponent* Comp : Components)
        {
            if (!IsValid(Comp))
                continue;

            if (Comp->IsA<ULightComponent>())
            {
                ++NumLights;
            }

            if (UStaticMeshComponent* SMC = Cast<UStaticMeshComponent>(Comp))
            {
                ++NumStaticMeshes;

                if (UStaticMesh* Mesh = SMC->GetStaticMesh())
                {
                    if (IsNaniteEnabled_Local(Mesh))
                    {
                        ++NumNaniteMeshes;
                    }
                }

                if (HasMaskedMaterials_Local(SMC))
                {
                    ++NumMaskedMeshes;
                }
            }
        }
    }

    BuildAnalysisSummary();
}

void UActorOptimizationEditorTool::BuildAnalysisSummary()
{
    FString Summary;
    Summary += TEXT("Actor Tree Analysis (Selected subtree):\n");
    Summary += FString::Printf(TEXT("- Lights: %d\n"), NumLights);
    Summary += FString::Printf(TEXT("- Static Meshes: %d\n"), NumStaticMeshes);
    Summary += FString::Printf(TEXT("- Nanite Meshes: %d\n"), NumNaniteMeshes);
    Summary += FString::Printf(TEXT("- Masked-material Meshes: %d\n"), NumMaskedMeshes);

    AnalysisSummary = Summary;
}

// ---------------------------------------------------------------------
// "What will run" summary builder
// ---------------------------------------------------------------------
void UActorOptimizationEditorTool::BuildWhatWillRunSummary()
{
    FString Summary;
    Summary += TEXT("What Will Run:\n");

    // Editor
    Summary += FString::Printf(TEXT("- Remove Lights: %s\n"),
        RunFlags.bRunRemoveLights ? TEXT("Yes") : TEXT("No"));

    if (RunFlags.bRunNanite)
    {
        if (bAllowNaniteChanges)
        {
            Summary += TEXT("- Nanite Optimization: Yes\n");
        }
        else
        {
            Summary += TEXT("- Nanite Optimization: Yes (blocked by safety setting)\n");
        }
    }
    else
    {
        Summary += TEXT("- Nanite Optimization: No\n");
    }

    Summary += FString::Printf(TEXT("- Simple Collision Optimization: %s\n"),
        RunFlags.bRunSimpleCollision ? TEXT("Yes") : TEXT("No"));

    Summary += FString::Printf(TEXT("- Masked Material Handling: %s (Mode: %s)\n"),
        RunFlags.bRunMaskedMaterials ? TEXT("Yes") : TEXT("No"),
        *UEnum::GetValueAsString(EditorOptions.MaskedMaterialMode));

    // Runtime
    Summary += FString::Printf(TEXT("- Force LOD: %s (LOD Index: %d)\n"),
        RunFlags.bRunForceLOD ? TEXT("Yes") : TEXT("No"),
        RuntimeOptions.LODIndex);

    Summary += FString::Printf(TEXT("- Minimum LOD: %s (Min LOD Index: %d)\n"),
        RunFlags.bRunMinimumLOD ? TEXT("Yes") : TEXT("No"),
        RuntimeOptions.MinimumLODIndex);

    Summary += FString::Printf(TEXT("- Disable Shadows: %s\n"),
        RunFlags.bRunDisableShadows ? TEXT("Yes") : TEXT("No"));

    Summary += FString::Printf(TEXT("- Disable Decorative Collision: %s\n"),
        RunFlags.bRunDisableDecorativeCollision ? TEXT("Yes") : TEXT("No"));

    Summary += FString::Printf(TEXT("- Optimize Materials: %s\n"),
        RunFlags.bRunOptimizeMaterials ? TEXT("Yes") : TEXT("No"));

    // Safety
    Summary += FString::Printf(TEXT("- Allow Nanite Changes: %s\n"),
        bAllowNaniteChanges ? TEXT("Yes") : TEXT("No"));

    WhatWillRunSummary = Summary;
}

// ---------------------------------------------------------------------
// Selection hook
// ---------------------------------------------------------------------
void UActorOptimizationEditorTool::OnSelectedActorChanged(AActor* NewActor)
{
    TargetActor = NewActor;

    // Analyze the selected actor's subtree, not a fixed root
    AnalyzeActorTree(TargetActor);

    // Restore previous run flags + params if this actor (or parent) was optimized this session
    if (FActorOptimizationRecord* Rec = FindRecordForActorOrParent(TargetActor))
    {
        RunFlags = Rec->RunFlags;
        EditorOptions = Rec->EditorParams;
        RuntimeOptions = Rec->RuntimeParams;

        AddLogLine(FString::Printf(
            TEXT("Restored optimization settings from previous run on '%s' (or parent)."),
            *TargetActor->GetName()));
    }
    else
    {
        AddLogLine(TEXT("No previous optimization settings found for this actor tree. Using current options."));
    }

    BuildWhatWillRunSummary();
}

#if WITH_EDITOR
void UActorOptimizationEditorTool::PostEditChangeProperty(FPropertyChangedEvent& Event)
{
    Super::PostEditChangeProperty(Event);

    const FName PropertyName = Event.Property ? Event.Property->GetFName() : NAME_None;

    if (PropertyName == GET_MEMBER_NAME_CHECKED(UActorOptimizationEditorTool, TargetActor))
    {
        // User changed TargetActor via the DetailsView arrow
        OnSelectedActorChanged(TargetActor);
    }

    BuildWhatWillRunSummary();
}
#endif


// ---------------------------------------------------------------------
// StartOptimization: entry point from UI
// ---------------------------------------------------------------------
void UActorOptimizationEditorTool::StartOptimization()
{
    // Clear previous run UI state immediately
    ResultsText = TEXT("");
    Progress = 0.0f;
    ProgressStatusText = TEXT("Preparing optimization...");
    LogLines.Reset();
    AddLogLine(TEXT("Beginning optimization..."));

    if (!TargetActor)
    {
        ResultsText = TEXT("No target actor selected.");
        SetStatus(TEXT("Idle"), 0.0f);
        bIsRunning = false;
        CurrentPhase = EActorOptimizationPhase::None;
        ActivePhases.Reset();
        return;
    }

    // Rebuild "what will run" summary and log it
    BuildWhatWillRunSummary();
    AddLogLine(WhatWillRunSummary);

    AddLogLine(FString::Printf(TEXT("Starting optimization for actor: %s"), *TargetActor->GetName()));
    TotalItemsProcessed = 0;
    NaniteConvertedCount = 0;
    NaniteSkippedCount = 0;

    if (!InitializePhases())
    {
        ResultsText = TEXT("No optimization phases enabled.");
        SetStatus(TEXT("Idle"), 0.0f);
        bIsRunning = false;
        CurrentPhase = EActorOptimizationPhase::None;
        return;
    }

    bIsRunning = true;
    CurrentPhaseIndex = 0;

    const float PhaseCount = (float)ActivePhases.Num();
    PhaseStartProgress = 0.0f;
    PhaseEndProgress = (PhaseCount > 0.0f) ? (1.0f / PhaseCount) : 1.0f;

    CurrentPhase = ActivePhases[CurrentPhaseIndex];

    if (ActivePhases.Contains(EActorOptimizationPhase::EditorNanite))
    {
        BuildNaniteMeshList();
    }

    // Store run flags + params into per-session history now
    StoreRecordForActor(TargetActor);

    AddLogLine(FString::Printf(
        TEXT("RunFlags at start: Nanite=%s, OptimizeMaterials=%s"),
        RunFlags.bRunNanite ? TEXT("true") : TEXT("false"),
        RunFlags.bRunOptimizeMaterials ? TEXT("true") : TEXT("false")));
}

// ---------------------------------------------------------------------
// InitializePhases: determine which phases run based on run flags
// ---------------------------------------------------------------------
bool UActorOptimizationEditorTool::InitializePhases()
{
    ActivePhases.Reset();

    // Editor
    if (RunFlags.bRunRemoveLights)
    {
        ActivePhases.Add(EActorOptimizationPhase::EditorRemoveLights);
    }

    if (RunFlags.bRunNanite && bAllowNaniteChanges)
    {
        ActivePhases.Add(EActorOptimizationPhase::EditorNanite);
    }

    if (RunFlags.bRunSimpleCollision)
    {
        ActivePhases.Add(EActorOptimizationPhase::EditorSimpleCollision);
    }

    if (RunFlags.bRunMaskedMaterials)
    {
        ActivePhases.Add(EActorOptimizationPhase::EditorMaskedMaterials);
    }

    // Runtime: group into a single phase, behavior depends on run flags
    const bool bAnyRuntime =
        RunFlags.bRunForceLOD ||
        RunFlags.bRunMinimumLOD ||
        RunFlags.bRunDisableShadows ||
        RunFlags.bRunDisableDecorativeCollision ||
        RunFlags.bRunOptimizeMaterials;

    if (bAnyRuntime)
    {
        ActivePhases.Add(EActorOptimizationPhase::RuntimeOptimizations);
    }

    return ActivePhases.Num() > 0;
}

// ---------------------------------------------------------------------
// AdvanceToNextPhase
// ---------------------------------------------------------------------
void UActorOptimizationEditorTool::AdvanceToNextPhase()
{
    ++CurrentPhaseIndex;

    if (!ActivePhases.IsValidIndex(CurrentPhaseIndex))
    {
        CurrentPhase = EActorOptimizationPhase::Done;
        bIsRunning = false;

        FString Summary = FString::Printf(
            TEXT("Optimization complete. Total items processed: %d. Nanite: %d converted, %d skipped."),
            TotalItemsProcessed, NaniteConvertedCount, NaniteSkippedCount);

        ResultsText = Summary;
        SetStatus(TEXT("Done"), 1.0f);
        AddLogLine(Summary);
        return;
    }

    CurrentPhase = ActivePhases[CurrentPhaseIndex];

    const float PhaseCount = (float)ActivePhases.Num();
    PhaseStartProgress = (float)CurrentPhaseIndex / PhaseCount;
    PhaseEndProgress = (float)(CurrentPhaseIndex + 1) / PhaseCount;

    FString PhaseName;
    switch (CurrentPhase)
    {
    case EActorOptimizationPhase::EditorRemoveLights:
        PhaseName = TEXT("Removing lights...");
        break;
    case EActorOptimizationPhase::EditorNanite:
        PhaseName = TEXT("Converting static meshes to Nanite...");
        break;
    case EActorOptimizationPhase::EditorSimpleCollision:
        PhaseName = TEXT("Optimizing collision on static meshes...");
        break;
    case EActorOptimizationPhase::EditorMaskedMaterials:
        PhaseName = TEXT("Processing masked materials...");
        break;
    case EActorOptimizationPhase::RuntimeOptimizations:
        PhaseName = TEXT("Applying runtime optimizations...");
        break;
    default:
        PhaseName = TEXT("Unknown phase...");
        break;
    }

    SetStatus(PhaseName, PhaseStartProgress);
    AddLogLine(PhaseName);
}

// ---------------------------------------------------------------------
// BuildNaniteMeshList: precompute static meshes to process
// ---------------------------------------------------------------------
void UActorOptimizationEditorTool::BuildNaniteMeshList()
{
    NaniteMeshesToProcess.Reset();
    NaniteMeshIndex = 0;

    if (!TargetActor)
        return;

    TArray<AActor*> AllActors;
    UActorOptimizationLibrary::GetAllActorsRecursive(TargetActor, AllActors);

    TSet<UStaticMesh*> UniqueMeshes;

    for (AActor* Actor : AllActors)
    {
        if (!IsValid(Actor))
            continue;

        TArray<UStaticMeshComponent*> MeshComponents;
        Actor->GetComponents<UStaticMeshComponent>(MeshComponents, false);

        for (UStaticMeshComponent* SMC : MeshComponents)
        {
            if (!IsValid(SMC))
                continue;

            UStaticMesh* Mesh = SMC->GetStaticMesh();
            if (!IsValid(Mesh))
                continue;

            if (!UniqueMeshes.Contains(Mesh))
            {
                UniqueMeshes.Add(Mesh);
                NaniteMeshesToProcess.Add(Mesh);
            }
        }
    }

    AddLogLine(FString::Printf(
        TEXT("Prepared %d unique static meshes for Nanite conversion."),
        NaniteMeshesToProcess.Num()));
}

// ---------------------------------------------------------------------
// TickOptimization: called from Slate Tick with a BatchSize
// ---------------------------------------------------------------------
void UActorOptimizationEditorTool::TickOptimization(int32 BatchSize)
{
    if (!bIsRunning || CurrentPhase == EActorOptimizationPhase::None || CurrentPhase == EActorOptimizationPhase::Done)
    {
        return;
    }

    switch (CurrentPhase)
    {
    case EActorOptimizationPhase::EditorRemoveLights:
        TickPhase_RemoveLights();
        break;

    case EActorOptimizationPhase::EditorNanite:
        TickPhase_Nanite(BatchSize);
        break;

    case EActorOptimizationPhase::EditorSimpleCollision:
        TickPhase_SimpleCollision();
        break;

    case EActorOptimizationPhase::EditorMaskedMaterials:
        TickPhase_MaskedMaterials();
        break;

    case EActorOptimizationPhase::RuntimeOptimizations:
        TickPhase_RuntimeOptimizations();
        break;

    default:
        break;
    }
}

// ---------------------------------------------------------------------
// Phase implementations
// ---------------------------------------------------------------------
void UActorOptimizationEditorTool::TickPhase_RemoveLights()
{
    if (!TargetActor)
    {
        AdvanceToNextPhase();
        return;
    }

    SetStatus(TEXT("Removing lights..."), PhaseStartProgress);
    AddLogLine(TEXT("Removing lights from actor hierarchy..."));

    FActorOptimizationEditorOptions LocalOptions = EditorOptions;
    LocalOptions.bRemoveLights = true;
    LocalOptions.bEnableNanite = false;
    LocalOptions.bUseSimpleCollision = false;

    int32 Removed = UActorOptimizationEditorLibrary::OptimizeActorInEditor(TargetActor, LocalOptions);
    TotalItemsProcessed += Removed;

    AddLogLine(FString::Printf(TEXT("Removed %d light components/actors."), Removed));

    SetStatus(TEXT("Lights removed."), PhaseEndProgress);
    AdvanceToNextPhase();
}

void UActorOptimizationEditorTool::TickPhase_Nanite(int32 BatchSize)
{
    if (!bAllowNaniteChanges)
    {
        AddLogLine(TEXT("Nanite changes are disabled by user preference. Skipping Nanite phase."));
        SetStatus(TEXT("Nanite changes disabled; skipping."), PhaseEndProgress);
        AdvanceToNextPhase();
        return;
    }

    if (!TargetActor || NaniteMeshesToProcess.Num() == 0)
    {
        SetStatus(TEXT("No static meshes found for Nanite."), PhaseEndProgress);
        AddLogLine(TEXT("No static meshes found for Nanite conversion."));
        AdvanceToNextPhase();
        return;
    }

    const int32 Total = NaniteMeshesToProcess.Num();
    int32 ProcessedThisTick = 0;

    while (NaniteMeshIndex < Total && ProcessedThisTick < BatchSize)
    {
        UStaticMesh* Mesh = NaniteMeshesToProcess[NaniteMeshIndex];
        EnableNaniteOnSingleMesh(Mesh);

        ++NaniteMeshIndex;
        ++ProcessedThisTick;
        ++TotalItemsProcessed;
    }

    const float LocalPhaseAlpha = (float)NaniteMeshIndex / (float)Total;
    const float OverallProgress = FMath::Lerp(PhaseStartProgress, PhaseEndProgress, LocalPhaseAlpha);

    FString Status = FString::Printf(
        TEXT("Converting meshes to Nanite... (%d / %d)"),
        NaniteMeshIndex, Total);

    SetStatus(Status, OverallProgress);

    if (NaniteMeshIndex >= Total)
    {
        FString DoneStatus = FString::Printf(
            TEXT("Nanite conversion complete. %d converted, %d skipped."),
            NaniteConvertedCount, NaniteSkippedCount);

        SetStatus(DoneStatus, PhaseEndProgress);
        AddLogLine(DoneStatus);
        AdvanceToNextPhase();
    }
}

void UActorOptimizationEditorTool::TickPhase_SimpleCollision()
{
    if (!TargetActor)
    {
        AdvanceToNextPhase();
        return;
    }

    SetStatus(TEXT("Optimizing simple collision..."), PhaseStartProgress);
    AddLogLine(TEXT("Optimizing simple collision on static meshes..."));

    FActorOptimizationEditorOptions LocalOptions = EditorOptions;
    LocalOptions.bRemoveLights = false;
    LocalOptions.bEnableNanite = false;
    LocalOptions.bUseSimpleCollision = true;

    int32 Optimized = UActorOptimizationEditorLibrary::OptimizeActorInEditor(TargetActor, LocalOptions);
    TotalItemsProcessed += Optimized;

    AddLogLine(FString::Printf(TEXT("Optimized collision on %d static meshes."), Optimized));

    SetStatus(TEXT("Simple collision optimization complete."), PhaseEndProgress);
    AdvanceToNextPhase();
}

void UActorOptimizationEditorTool::TickPhase_MaskedMaterials()
{
    if (!TargetActor)
    {
        AdvanceToNextPhase();
        return;
    }

    SetStatus(TEXT("Processing masked materials..."), PhaseStartProgress);
    AddLogLine(TEXT("Processing masked materials according to selected mode..."));

    FActorOptimizationEditorOptions LocalOptions = EditorOptions;
    LocalOptions.bRemoveLights = false;
    LocalOptions.bEnableNanite = false;
    LocalOptions.bUseSimpleCollision = false;

    int32 Processed = UActorOptimizationEditorLibrary::OptimizeActorInEditor(TargetActor, LocalOptions);
    TotalItemsProcessed += Processed;

    AddLogLine(FString::Printf(TEXT("Processed %d meshes for masked materials."), Processed));

    SetStatus(TEXT("Masked materials processed."), PhaseEndProgress);
    AdvanceToNextPhase();
}

void UActorOptimizationEditorTool::TickPhase_RuntimeOptimizations()
{
    if (!TargetActor)
    {
        AdvanceToNextPhase();
        return;
    }

    SetStatus(TEXT("Applying runtime optimizations..."), PhaseStartProgress);
    AddLogLine(TEXT("Applying runtime-safe optimizations to components..."));

    FActorOptimizationOptions LocalRuntimeOptions = RuntimeOptions;
    LocalRuntimeOptions.bForceLOD = RunFlags.bRunForceLOD;
    LocalRuntimeOptions.bForceMinimumLOD = RunFlags.bRunMinimumLOD;
    LocalRuntimeOptions.bDisableShadowCasting = RunFlags.bRunDisableShadows;
    LocalRuntimeOptions.bDisableCollisionOnDecorative = RunFlags.bRunDisableDecorativeCollision;
    LocalRuntimeOptions.bOptimizeMaterials = RunFlags.bRunOptimizeMaterials;

    int32 RuntimeProcessed = UActorOptimizationLibrary::OptimizeActor(TargetActor, LocalRuntimeOptions);
    TotalItemsProcessed += RuntimeProcessed;

    AddLogLine(FString::Printf(TEXT("Runtime optimizations affected %d components."), RuntimeProcessed));

    SetStatus(TEXT("Runtime optimizations complete."), PhaseEndProgress);
    AdvanceToNextPhase();
}
