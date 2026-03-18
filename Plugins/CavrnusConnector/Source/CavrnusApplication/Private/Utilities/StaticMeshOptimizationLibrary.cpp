// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "Utilities/StaticMeshOptimizationLibrary.h"
#include "Utilities/ActorOptimizationLibrary.h"
#include "Engine/StaticMesh.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "GameFramework/Actor.h"
#include "UObject/UObjectGlobals.h"
#include "Containers/Queue.h"
#include "Containers/Set.h"
#include "Containers/Map.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFilemanager.h"
#include "XmlParser.h"
#include "Logging/LogMacros.h"
#include "DatasmithRuntime.h"
#include "TimerManager.h"
#include "Engine/World.h"

DEFINE_LOG_CATEGORY_STATIC(LogStaticMeshOptimization, Warning, All);

/**
 * @brief Helper function to validate if a static mesh component is ready for processing
 */
static bool IsComponentReady(UStaticMeshComponent* MeshComp, FString& OutWarning)
{
	if (!IsValid(MeshComp))
	{
		OutWarning = TEXT("Component is not valid");
		return false;
	}

	// Check if component is registered
	if (!MeshComp->IsRegistered())
	{
		OutWarning = FString::Printf(TEXT("Component '%s' is not registered"), *MeshComp->GetName());
		return false;
	}

	// Check if static mesh exists
	UStaticMesh* StaticMesh = MeshComp->GetStaticMesh();
	if (!IsValid(StaticMesh))
	{
		OutWarning = FString::Printf(TEXT("Component '%s' has no static mesh assigned"), *MeshComp->GetName());
		return false;
	}

	// Check if material slot names are available
	TArray<FName> SlotNames = MeshComp->GetMaterialSlotNames();
	if (SlotNames.Num() == 0)
	{
		OutWarning = FString::Printf(TEXT("Component '%s' has no material slots"), *MeshComp->GetName());
		return false;
	}

	// Check if static mesh render data is ready (alternative to IsReadyForStreaming)
	// GetRenderData() returns nullptr if the mesh is not ready for rendering
	if (!StaticMesh->GetRenderData())
	{
		OutWarning = FString::Printf(TEXT("Component '%s' static mesh '%s' render data is not ready"), 
			*MeshComp->GetName(), *StaticMesh->GetName());
		return false;
	}

	return true;
}

/**
 * @brief Helper function to process a single static mesh component
 */
static void ProcessStaticMeshComponent(AActor* Actor, UStaticMeshComponent* MeshComp, bool bErrorCheck)
{
	if (!IsValid(MeshComp))
	{
		return;
	}

	// Set MinLOD to 1 on the static mesh asset
	UStaticMesh* StaticMesh = MeshComp->GetStaticMesh();
	if (IsValid(StaticMesh))
	{
		// Set MinLOD on the asset using SetMinLOD method
		StaticMesh->SetMinLOD(1);
		
		// Mark the asset as modified if we're in the editor
		#if WITH_EDITOR
		StaticMesh->Modify();
		#endif
	}

	// Set forced LOD model to 1 on the component
	MeshComp->SetForcedLodModel(1);

	// Get material slot names for remapping
	TArray<FName> SlotNames = MeshComp->GetMaterialSlotNames();
	int32 NumMaterials = SlotNames.Num();
	int a = 0;
	bool bModified = false;
	if (Actor->GetName() == "43C981D74CD6A8263266C999C2317D26")
	{
		a = 1;
		UE_LOG(LogTemp, Error, TEXT("Something bad is happening"));
		int count = 0;
		for (int32 n = 0; n < NumMaterials; ++n)
		{
			UMaterialInterface* Material = MeshComp->GetMaterial(n);
			if (!Material)
			{
				UE_LOG(LogTemp, Error, TEXT("Slot %d (%s) is empty"), n, *SlotNames[n].ToString());
			}
			else
			{
				count++;
				UE_LOG(LogTemp, Error, TEXT("Slot %d (%s) has material %s"), n, *SlotNames[n].ToString(), *Material->GetName());
			}
		}
		UE_LOG(LogTemp, Warning, TEXT("Component %s has %d/%d materials assigned"), *MeshComp->GetName(), count, NumMaterials);
	}

	// Remap empty material slots
	for (int32 i = 0; i < NumMaterials; ++i)
	{
		UMaterialInterface* Material = MeshComp->GetMaterial(i);

		// If this slot is empty, try to find another slot with the same name that has a valid material
		if (!Material)
		{
			if (bErrorCheck)
			{
				UE_LOG(LogTemp, Warning, TEXT("StaticMeshOptimization: Remapping empty material slot '%s' on component '%s'"),
					*SlotNames[i].ToString(), *MeshComp->GetName());
			}
			bool bSlotModified = false;
			// Search for another slot with the same slot name (starting from current index + 1)
			for (int32 j = i + 1; j < NumMaterials; ++j)
			{
				if (SlotNames[i] == SlotNames[j])
				{
					UMaterialInterface* SourceMaterial = MeshComp->GetMaterial(j);
					if (SourceMaterial)
					{
						// Copy the material from the matching slot
						MeshComp->SetMaterial(i, SourceMaterial);
						bModified = true;
						bSlotModified = true;
						if (bErrorCheck)
						{
							UE_LOG(LogTemp, Warning, TEXT("StaticMeshOptimization: Mapped material from slot '%s' (index %d) to empty slot '%s' (index %d) on component '%s'"),
								*SlotNames[j].ToString(), j, *SlotNames[i].ToString(), i, *MeshComp->GetName());
						}
						break;
					}
				}
			}
			if (!bSlotModified && bErrorCheck)
			{
				UE_LOG(LogTemp, Warning, TEXT("StaticMeshOptimization: No matching material found for slot '%s' on component '%s' for actor '%s'"),
					*SlotNames[i].ToString(), *MeshComp->GetName(), *Actor->GetName());
			}
		}
	}

	// Mark render state dirty if materials were modified
	if (bModified)
	{
		MeshComp->MarkRenderStateDirty();
	}
}

int32 UStaticMeshOptimizationLibrary::OptimizeStaticMeshesForActor(AActor* TargetActor)
{
	// Use the tracking version with an empty set for backward compatibility
	TSet<AActor*> ProcessedActors;
	return OptimizeStaticMeshesForActorWithTracking(TargetActor, ProcessedActors);
}

int32 UStaticMeshOptimizationLibrary::OptimizeStaticMeshesForActorWithTracking(
	AActor* TargetActor,
	TSet<AActor*>& InOutProcessedActors)
{
	if (!TargetActor)
	{
		return 0;
	}

	// First, collect all actors in the hierarchy (including the root and all child actors)
	TArray<AActor*> AllActors;
	UActorOptimizationLibrary::GetAllActorsRecursive(TargetActor, AllActors);

	int32 ProcessedCount = 0;

	// Process static mesh components from each actor
	for (AActor* Actor : AllActors)
	{
		if (!IsValid(Actor))
		{
			continue;
		}

		// Check if this actor has already been processed
		if (InOutProcessedActors.Contains(Actor))
		{
			// Skip this actor - already processed
			continue;
		}

		// Get all static mesh components from this actor (non-recursive since we're already traversing actors)
		TArray<UStaticMeshComponent*> MeshComponents;
		Actor->GetComponents<UStaticMeshComponent>(MeshComponents, false);

		// Process each static mesh component
		for (UStaticMeshComponent* MeshComp : MeshComponents)
		{
			ProcessStaticMeshComponent(Actor, MeshComp, false);
			ProcessedCount++;
		}

		// Mark this actor as processed
		InOutProcessedActors.Add(Actor);
	}

	return ProcessedCount;
}

void UStaticMeshOptimizationLibrary::ClearProcessedActors(TSet<AActor*>& ProcessedActors)
{
	ProcessedActors.Empty();
}
void UStaticMeshOptimizationLibrary::ClearProcessedActorsArray(TArray<AActor*>& ProcessedActors)
{
	ProcessedActors.Empty();
}

int32 UStaticMeshOptimizationLibrary::OptimizeStaticMeshesForActorWithValidation(
	AActor* TargetActor,
	TArray<AActor*>& InOutProcessedActors,
	TArray<AActor*>& OutUnreadyActors,
	TArray<FString>& OutWarnings,
	bool bErrorCheck)
{
	if (!TargetActor)
	{
		return 0;
	}

	// Convert arrays to sets for efficient lookups (remove duplicates)
	TSet<AActor*> ProcessedActorsSet(InOutProcessedActors);
	TSet<AActor*> PreviouslyUnreadyActors(OutUnreadyActors);
	
	// Clear the output unready actors array - we'll rebuild it with current unready actors
	OutUnreadyActors.Empty();
	OutWarnings.Empty();

	// First, collect all actors in the hierarchy (including the root and all child actors)
	TArray<AActor*> AllActors;
	UActorOptimizationLibrary::GetAllActorsRecursive(TargetActor, AllActors);

	int32 ProcessedCount = 0;
	TSet<AActor*> NewUnreadyActors;
	TSet<AActor*> NewProcessedActors;

	// Process static mesh components from each actor
	for (AActor* Actor : AllActors)
	{
		if (!IsValid(Actor))
		{
			continue;
		}

		// Check if this actor has already been processed
		if (ProcessedActorsSet.Contains(Actor))
		{
			// Skip this actor - already processed
			continue;
		}

		// Track if this actor was previously unready
		bool bWasPreviouslyUnready = PreviouslyUnreadyActors.Contains(Actor);

		// Get all static mesh components from this actor (non-recursive since we're already traversing actors)
		TArray<UStaticMeshComponent*> MeshComponents;
		Actor->GetComponents<UStaticMeshComponent>(MeshComponents, false);

		bool bActorHasUnreadyComponents = false;
		int32 ActorProcessedCount = 0;

		// Process each static mesh component
		for (UStaticMeshComponent* MeshComp : MeshComponents)
		{
			// Validate component readiness
			FString Warning;
			if (!IsComponentReady(MeshComp, Warning))
			{
				// Component is not ready
				bActorHasUnreadyComponents = true;
				FString FullWarning = FString::Printf(TEXT("Actor '%s': %s"), *Actor->GetName(), *Warning);
				OutWarnings.Add(FullWarning);
				UE_LOG(LogStaticMeshOptimization, Warning, TEXT("[StaticMeshOptimization] %s"), *FullWarning);
				continue;
			}

			// Component is ready, process it
			ProcessStaticMeshComponent(Actor, MeshComp, bErrorCheck);
			ActorProcessedCount++;
			ProcessedCount++;
		}

		// If actor had any unready components, add it to unready set
		if (bActorHasUnreadyComponents)
		{
			NewUnreadyActors.Add(Actor);
			// Don't mark as processed - we'll retry later
		}
		else
		{
			// All components were ready and processed
			// If this actor was previously unready, log success
			if (bWasPreviouslyUnready)
			{
				UE_LOG(LogStaticMeshOptimization, Warning, 
					TEXT("[StaticMeshOptimization] Successfully processed actor '%s' that was previously unready"), 
					*Actor->GetName());
			}

			// Mark this actor as processed
			NewProcessedActors.Add(Actor);
			ProcessedActorsSet.Add(Actor);
		}
	}

	// Convert sets back to arrays for output
	OutUnreadyActors = NewUnreadyActors.Array();
	InOutProcessedActors = ProcessedActorsSet.Array();

	return ProcessedCount;
}

/**
 * @brief Helper function to recursively count Actor and ActorMesh nodes in XML
 */
static int32 CountActorNodesRecursive(const FXmlNode* Node)
{
	if (!Node)
	{
		return 0;
	}

	int32 Count = 0;
	FString Tag = Node->GetTag();

	// Count Actor and ActorMesh nodes
	if (Tag.Equals(TEXT("Actor"), ESearchCase::IgnoreCase) || 
		Tag.Equals(TEXT("ActorMesh"), ESearchCase::IgnoreCase))
	{
		Count = 1;
	}

	// Recurse into children
	for (const FXmlNode* Child : Node->GetChildrenNodes())
	{
		Count += CountActorNodesRecursive(Child);
	}

	return Count;
}

int32 UStaticMeshOptimizationLibrary::FindDatasmithFiles(TArray<FString>& OutFilePaths, const FString& SearchDirectory)
{
	OutFilePaths.Empty();

	FString SearchPath;
	if (SearchDirectory.IsEmpty())
	{
		// Default to Content directory
		SearchPath = FPaths::ProjectContentDir();
	}
	else
	{
		SearchPath = SearchDirectory;
	}

	// Ensure the path exists
	if (!FPaths::DirectoryExists(SearchPath))
	{
		return 0;
	}

	// Use file manager to find files recursively
	IFileManager::Get().FindFilesRecursive(OutFilePaths, *SearchPath, TEXT("*.udatasmith"), true, false);

	return OutFilePaths.Num();
}

int32 UStaticMeshOptimizationLibrary::CountActorsInDatasmithFile(const FString& FilePath)
{
	if (!FPaths::FileExists(FilePath))
	{
		return -1;
	}

	// Parse the XML file
	FXmlFile XmlFile(FilePath, EConstructMethod::ConstructFromFile);
	if (!XmlFile.IsValid())
	{
		return -1;
	}

	const FXmlNode* RootNode = XmlFile.GetRootNode();
	if (!RootNode)
	{
		return -1;
	}

	// Count Actor and ActorMesh nodes recursively
	return CountActorNodesRecursive(RootNode);
}

/**
 * @brief Internal struct to track active Datasmith load monitors
 */
struct FDatasmithMonitorState
{
	FTimerHandle TimerHandle;
	bool bWaitingForLoadStart;
	FOnDatasmithLoadComplete OnComplete;
	float TimeoutSeconds;
	double StartTime;
	UWorld* World;

	FDatasmithMonitorState()
		: bWaitingForLoadStart(true)
		, TimeoutSeconds(0.0f)
		, StartTime(0.0)
		, World(nullptr)
	{
	}
};

// Static map to track active monitors (keyed by weak actor pointer to handle destruction)
static TMap<TWeakObjectPtr<ADatasmithRuntimeActor>, FDatasmithMonitorState> ActiveMonitors;

/**
 * @brief Polling function that checks Datasmith actor loading state
 */
static void PollDatasmithActorStatus(ADatasmithRuntimeActor* DatasmithActor)
{
	TWeakObjectPtr<ADatasmithRuntimeActor> WeakActor = DatasmithActor;
	
	if (!WeakActor.IsValid())
	{
		// Actor was destroyed, find and clean up any monitor for it
		for (auto It = ActiveMonitors.CreateIterator(); It; ++It)
		{
			if (!It.Key().IsValid())
			{
				// Found destroyed actor, fire delegate and remove
				It.Value().OnComplete.ExecuteIfBound(false, nullptr);
				if (It.Value().World)
				{
					It.Value().World->GetTimerManager().ClearTimer(It.Value().TimerHandle);
				}
				It.RemoveCurrent();
				break;
			}
		}
		return;
	}

	FDatasmithMonitorState* MonitorState = ActiveMonitors.Find(WeakActor);
	if (!MonitorState)
	{
		// Monitor was removed, nothing to do
		return;
	}

	// Check timeout
	if (MonitorState->TimeoutSeconds > 0.0f)
	{
		double CurrentTime = FPlatformTime::Seconds();
		if (CurrentTime - MonitorState->StartTime >= MonitorState->TimeoutSeconds)
		{
			// Timeout reached
			MonitorState->OnComplete.ExecuteIfBound(false, DatasmithActor);
			if (MonitorState->World)
			{
				MonitorState->World->GetTimerManager().ClearTimer(MonitorState->TimerHandle);
			}
			ActiveMonitors.Remove(DatasmithActor);
			return;
		}
	}

	// Get the two booleans (following DatasmithFileImporter pattern)
	const bool bIsBuilding = DatasmithActor->bBuilding;
	const bool bIsReceiving = DatasmithActor->IsReceiving();

	if (MonitorState->bWaitingForLoadStart)
	{
		// Wait for one boolean to become true (loading started)
		if (bIsBuilding || bIsReceiving)
		{
			MonitorState->bWaitingForLoadStart = false;
			// Loading has started, continue monitoring
		}
		return;
	}

	// Active monitoring phase - check if still loading
	if (bIsBuilding || bIsReceiving)
	{
		// Still loading, continue polling
		return;
	}

	// Both are false - loading is complete!
	MonitorState->OnComplete.ExecuteIfBound(true, DatasmithActor);
	
	// Clean up
	if (MonitorState->World)
	{
		MonitorState->World->GetTimerManager().ClearTimer(MonitorState->TimerHandle);
	}
	ActiveMonitors.Remove(DatasmithActor);
}

void UStaticMeshOptimizationLibrary::MonitorDatasmithLoadCompletion(
	ADatasmithRuntimeActor* DatasmithActor,
	float PollInterval,
	float TimeoutSeconds,
	const FOnDatasmithLoadComplete& OnComplete)
{
	if (!DatasmithActor)
	{
		// Actor is null, fire delegate immediately with failure
		OnComplete.ExecuteIfBound(false, nullptr);
		return;
	}

	// Check if already monitoring this actor
	TWeakObjectPtr<ADatasmithRuntimeActor> WeakActor = DatasmithActor;
	if (ActiveMonitors.Contains(WeakActor))
	{
		UE_LOG(LogStaticMeshOptimization, Warning, 
			TEXT("[StaticMeshOptimization] Already monitoring Datasmith actor %s"), 
			*DatasmithActor->GetName());
		return;
	}

	// Get world from actor
	UWorld* World = DatasmithActor->GetWorld();
	if (!World)
	{
		UE_LOG(LogStaticMeshOptimization, Warning, 
			TEXT("[StaticMeshOptimization] Datasmith actor %s has no valid world"), 
			*DatasmithActor->GetName());
		OnComplete.ExecuteIfBound(false, DatasmithActor);
		return;
	}

	// Create monitor state
	FDatasmithMonitorState MonitorState;
	MonitorState.bWaitingForLoadStart = true;
	MonitorState.OnComplete = OnComplete;
	MonitorState.TimeoutSeconds = TimeoutSeconds;
	MonitorState.StartTime = FPlatformTime::Seconds();
	MonitorState.World = World;

	// Set up polling timer (capture weak pointer to handle actor destruction)
	TWeakObjectPtr<ADatasmithRuntimeActor> WeakActorForTimer = DatasmithActor;
	World->GetTimerManager().SetTimer(
		MonitorState.TimerHandle,
		[WeakActorForTimer]()
		{
			PollDatasmithActorStatus(WeakActorForTimer.IsValid() ? WeakActorForTimer.Get() : nullptr);
		},
		PollInterval,
		true // Loop
	);

	// Store in active monitors map (using weak pointer as key)
	ActiveMonitors.Add(WeakActor, MonitorState);

	UE_LOG(LogStaticMeshOptimization, Log, 
		TEXT("[StaticMeshOptimization] Started monitoring Datasmith actor %s (PollInterval: %.2f, Timeout: %.2f)"), 
		*DatasmithActor->GetName(), PollInterval, TimeoutSeconds);
}

