// Copyright (c) 2025 Cavrnus. All rights reserved.

/**
 * @file ActorOptimizationEditorLibrary.h
 * @brief Editor-only utility for optimizing actors in the world.
 * Provides options for light removal, Nanite conversion, and collision asset optimizations.
 * This class can only be used in the editor and requires editor-only APIs.
 */

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "ActorOptimizationEditorLibrary.generated.h"

/**
 * @brief Mode for handling masked materials with Nanite.
 */
UENUM(BlueprintType)
enum class EMaskedMaterialNaniteMode : uint8
{
	/** Force Nanite for masked materials */
	ForceNanite,
	/** Use LODs for masked materials (disable/avoid Nanite for masked) */
	UseLODs
};

/**
 * @brief Configuration options for editor-only actor optimization.
 */
USTRUCT(BlueprintType)
struct FActorOptimizationEditorOptions
{
	GENERATED_BODY()

	/** Remove all light components from actor and subchildren */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Cavrnus | Optimization | Editor")
	bool bRemoveLights = false;

	/** Convert static meshes to use Nanite */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Cavrnus | Optimization | Editor")
	bool bEnableNanite = false;

	/** B: Use simple collision shapes where possible (modifies static mesh assets) */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Cavrnus | VR Optimizations | Editor", meta = (DisplayName = "B: Use Simple Collision"))
	bool bUseSimpleCollision = true;  // Default: true

	/** Mode for handling static meshes with masked materials */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Cavrnus | Optimization | Editor")
	EMaskedMaterialNaniteMode MaskedMaterialMode = EMaskedMaterialNaniteMode::UseLODs;
};

/**
 * @brief Editor-only utility library for optimizing actors in the world.
 * 
 * This class provides editor-only functions to optimize actors and their hierarchies:
 * - Remove all lights from actor hierarchy (requires editor-only APIs)
 * - Convert static meshes to use Nanite (requires asset modification)
 * - Optimize collision on static mesh assets (requires asset modification)
 * 
 * These functions modify assets and require editor-only subsystems and APIs.
 * For runtime-safe optimizations, use UActorOptimizationLibrary instead.
 */
UCLASS(Abstract)
class CAVRNUSCONNECTOREDITOR_API UActorOptimizationEditorLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * @brief Optimizes an actor based on the provided editor-only configuration options.
	 * 
	 * Recursively processes the actor and all its subchildren, applying editor-only optimizations
	 * that modify assets. Uses FScopedTransaction for undo/redo support.
	 * 
	 * @param TargetActor The root actor to optimize. If null, returns 0.
	 * @param Options Configuration options for which editor-only optimizations to apply.
	 * @return The number of components/assets that were processed.
	 */
	UFUNCTION(BlueprintCallable, Category = "Cavrnus | Actor Optimization | Editor",
		meta = (ToolTip = "Optimizes an actor with editor-only options for lights, Nanite, and collision assets. Requires editor-only APIs.",
			CallInEditor = "true"))
	static int32 OptimizeActorInEditor(AActor* TargetActor, const FActorOptimizationEditorOptions& Options);

private:
	/**
	 * @brief Removes all light components from the actor and its subchildren.
	 * 
	 * Uses editor-only USubobjectDataSubsystem to find and remove light components.
	 * 
	 * @param TargetActor The root actor to process.
	 * @return The number of light components removed.
	 */
	static int32 RemoveAllLightsFromActor(AActor* TargetActor);

	/**
	 * @brief Converts all static meshes in the actor hierarchy to use Nanite.
	 * 
	 * Modifies static mesh assets directly, requiring editor-only APIs.
	 * 
	 * @param TargetActor The root actor to process.
	 * @return The number of static meshes converted to Nanite.
	 */
	static int32 ConvertStaticMeshesToNanite(AActor* TargetActor);

	/**
	 * @brief VR Optimization B: Uses simple collision shapes where possible.
	 * 
	 * Modifies static mesh assets directly to optimize collision complexity.
	 * 
	 * @param TargetActor The root actor to process.
	 * @return The number of static meshes optimized.
	 */
	static int32 UseSimpleCollision(AActor* TargetActor);

	/**
	 * @brief Processes static meshes with masked materials based on the specified mode.
	 * 
	 * Only processes static meshes that use masked materials. Logs all processed actors.
	 * 
	 * @param TargetActor The root actor to process.
	 * @param Mode The mode for handling masked materials (ForceNanite or UseLODs).
	 * @return The number of static meshes processed.
	 */
	static int32 ProcessMaskedMaterials(AActor* TargetActor, EMaskedMaterialNaniteMode Mode);

	/**
	 * @brief Checks if a static mesh component uses masked materials.
	 * 
	 * @param SMC The static mesh component to check.
	 * @return True if any material slot uses masked blend mode, false otherwise.
	 */
	static bool HasMaskedMaterials(UStaticMeshComponent* SMC);
};

