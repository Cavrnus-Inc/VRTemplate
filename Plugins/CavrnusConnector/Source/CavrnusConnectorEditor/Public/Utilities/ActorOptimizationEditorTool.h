// Copyright (c) 2025 Cavrnus, Inc. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"

#include "Utilities/ActorOptimizationLibrary.h"
#include "Utilities/ActorOptimizationEditorLibrary.h"

#include "ActorOptimizationEditorTool.generated.h"

/** Phases of the optimization pipeline (for progress + status) */
UENUM()
enum class EActorOptimizationPhase : uint8
{
    None,
    EditorRemoveLights,
    EditorNanite,
    EditorSimpleCollision,
    EditorMaskedMaterials,
    RuntimeOptimizations,
    Done
};

/** Per-phase "run" intent flags (split from parameter values) */
USTRUCT(BlueprintType)
struct FActorOptimizationRunFlags
{
    GENERATED_BODY()

    // Editor phases
    UPROPERTY(EditAnywhere, Category = "Cavrnus | Optimization | Editor")
    bool bRunRemoveLights = false;

    UPROPERTY(EditAnywhere, Category = "Cavrnus | Optimization | Editor")
    bool bRunNanite = false;

    UPROPERTY(EditAnywhere, Category = "Cavrnus | Optimization | Editor")
    bool bRunSimpleCollision = false;

    UPROPERTY(EditAnywhere, Category = "Cavrnus | Optimization | Editor")
    bool bRunMaskedMaterials = true;

    // Runtime phases
    UPROPERTY(EditAnywhere, Category = "Cavrnus | Optimization | Runtime")
    bool bRunForceLOD = false;

    UPROPERTY(EditAnywhere, Category = "Cavrnus | Optimization | Runtime")
    bool bRunMinimumLOD = false;

    UPROPERTY(EditAnywhere, Category = "Cavrnus | Optimization | Runtime")
    bool bRunDisableShadows = false;

    UPROPERTY(EditAnywhere, Category = "Cavrnus | Optimization | Runtime")
    bool bRunDisableDecorativeCollision = false;

    UPROPERTY(EditAnywhere, Category = "Cavrnus | Optimization | Runtime")
    bool bRunOptimizeMaterials = false;
};

/** Per-actor optimization record for this editor session */
USTRUCT()
struct FActorOptimizationRecord
{
    GENERATED_BODY()

    UPROPERTY()
    FActorOptimizationRunFlags RunFlags;

    UPROPERTY()
    FActorOptimizationEditorOptions EditorParams;

    UPROPERTY()
    FActorOptimizationOptions RuntimeParams;

    UPROPERTY()
    FDateTime Timestamp;
};

UCLASS(Transient)
class CAVRNUSCONNECTOREDITOR_API UActorOptimizationEditorTool : public UObject
{
    GENERATED_BODY()

public:
    /** Actor to optimize (auto-filled from editor selection in the panel) */
    UPROPERTY(EditAnywhere, Transient, Category = "Cavrnus | Actor Optimization")
    AActor* TargetActor = nullptr;

    /** Which optimizations should run this attempt */
    UPROPERTY(EditAnywhere, Category = "Cavrnus | Actor Optimization | Run Flags")
    FActorOptimizationRunFlags RunFlags;

    /** Editor-only parameter values (used only if the corresponding RunFlags are true) */
    UPROPERTY(EditAnywhere, Category = "Cavrnus | Actor Optimization | Editor Parameters")
    FActorOptimizationEditorOptions EditorOptions;

    /** Runtime parameter values (used only if the corresponding RunFlags are true) */
    UPROPERTY(EditAnywhere, Category = "Cavrnus | Actor Optimization | Runtime Parameters")
    FActorOptimizationOptions RuntimeOptions;

    /** Global safety: if false, no Nanite changes are applied at all */
    UPROPERTY(EditAnywhere, Category = "Cavrnus | Actor Optimization | Safety")
    bool bAllowNaniteChanges = true;

    /** Human-readable results summary */
    UPROPERTY(VisibleAnywhere, Transient, Category = "Cavrnus | Actor Optimization | Status")
    FString ResultsText;

    /** 0–1 overall progress */
    UPROPERTY(VisibleAnywhere, Transient, Category = "Cavrnus | Actor Optimization | Status")
    float Progress = 0.0f;

    /** Status text for current phase */
    UPROPERTY(VisibleAnywhere, Transient, Category = "Cavrnus | Actor Optimization | Status")
    FString ProgressStatusText;

    /** Rolling log lines (for UI display) */
    UPROPERTY(VisibleAnywhere, Transient, Category = "Cavrnus | Actor Optimization | Status")
    TArray<FString> LogLines;

    /** Contextual analysis summary for currently selected actor subtree */
    UPROPERTY(VisibleAnywhere, Transient, Category = "Cavrnus | Actor Optimization | Status")
    FString AnalysisSummary;

    /** "What will run" summary for current settings */
    UPROPERTY(VisibleAnywhere, Transient, Category = "Cavrnus | Actor Optimization | Status")
    FString WhatWillRunSummary;

public:
    /** Called by the UI button to begin async processing */
    void StartOptimization();

    /** Called from the Slate widget Tick to advance work in small chunks */
    void TickOptimization(int32 BatchSize);

    /** Whether an optimization run is currently active */
    bool IsRunning() const { return bIsRunning; }

    /** Append a line to the rolling log */
    void AddLogLine(const FString& Line);

    /** Called when selection changes in the editor */
    void OnSelectedActorChanged(AActor* NewActor);

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& Event) override;
#endif

private:
    /** Internal state */
    UPROPERTY(Transient)
    bool bIsRunning = false;

    UPROPERTY(Transient)
    EActorOptimizationPhase CurrentPhase = EActorOptimizationPhase::None;

    /** Phases actually enabled based on run flags (for normalized progress) */
    UPROPERTY(Transient)
    TArray<EActorOptimizationPhase> ActivePhases;

    UPROPERTY(Transient)
    int32 CurrentPhaseIndex = 0;

    /** Used for scaling per-phase progress into [0,1] */
    UPROPERTY(Transient)
    float PhaseStartProgress = 0.0f;

    UPROPERTY(Transient)
    float PhaseEndProgress = 1.0f;

    /** Total items processed across all phases */
    UPROPERTY(Transient)
    int32 TotalItemsProcessed = 0;

    /** Nanite-specific async state */
    UPROPERTY(Transient)
    TArray<UStaticMesh*> NaniteMeshesToProcess;

    UPROPERTY(Transient)
    int32 NaniteMeshIndex = 0;

    UPROPERTY(Transient)
    int32 NaniteConvertedCount = 0;

    UPROPERTY(Transient)
    int32 NaniteSkippedCount = 0;

    /** Per-session optimization history for root actors */
    UPROPERTY(Transient)
    TMap<TWeakObjectPtr<AActor>, FActorOptimizationRecord> OptimizationHistory;

    /** Analysis counts for current actor subtree */
    UPROPERTY(Transient)
    int32 NumLights = 0;

    UPROPERTY(Transient)
    int32 NumStaticMeshes = 0;

    UPROPERTY(Transient)
    int32 NumNaniteMeshes = 0;

    UPROPERTY(Transient)
    int32 NumMaskedMeshes = 0;

private:
    /** Build phase list based on run flags, and prepare any async queues */
    bool InitializePhases();

    /** Advance through phases and compute PhaseStart/End for normalized progress */
    void AdvanceToNextPhase();

    /** Per-phase workers (called from TickOptimization) */
    void TickPhase_RemoveLights();
    void TickPhase_Nanite(int32 BatchSize);
    void TickPhase_SimpleCollision();
    void TickPhase_MaskedMaterials();
    void TickPhase_RuntimeOptimizations();

    /** Nanite helpers */
    void BuildNaniteMeshList();
    void EnableNaniteOnSingleMesh(UStaticMesh* Mesh);

    /** Status helper */
    void SetStatus(const FString& InStatus, float InProgress);

    /** Per-session settings/history */
    FActorOptimizationRecord* FindRecordForActorOrParent(AActor* Actor);
    void StoreRecordForActor(AActor* Actor);

    /** Analysis of current actor subtree */
    void AnalyzeActorTree(AActor* Root);
    void BuildAnalysisSummary();

    /** Build "What will run" summary for current RunFlags + parameters + safety */
    void BuildWhatWillRunSummary();
};
