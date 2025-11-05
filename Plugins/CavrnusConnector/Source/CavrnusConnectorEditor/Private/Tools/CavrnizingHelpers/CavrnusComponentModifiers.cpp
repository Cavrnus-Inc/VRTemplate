// // Copyright (c) 2025 Cavrnus. All rights reserved.

#if WITH_EDITOR
#include "Tools/CavrnizingHelpers/CavrnusComponentModifiers.h"
#include "EngineUtils.h"
#include "Editor.h"
#include "ScopedTransaction.h"
#include "SubobjectDataSubsystem.h"
#include "SubobjectData.h" // For FSubobjectData (UE 5.0)
#include "Runtime/Launch/Resources/Version.h" // ENGINE_MAJOR_VERSION / ENGINE_MINOR_VERSION
#include "SubobjectDataBlueprintFunctionLibrary.h" // for BP-style helpers in 5.0
#include "Templates/IsPointer.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/PointLight.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "FileHelpers.h"
#include "Selection.h"

inline USubobjectDataSubsystem* GetSubobjectDataSubsystemCompat()
{
	// In 5.0 it derives from UEngineSubsystem, so ask GEngine.
	return GEngine ? GEngine->GetEngineSubsystem<USubobjectDataSubsystem>() : nullptr;
}

// Define and optionally initialize the static array
TMap<FString, FString> CavrnusComponentModifiers::SupportedCavrnusComponentClassNames;

TArray<UClass*> CavrnusComponentModifiers::SupportedCavrnusComponentClasses;

void ToggleActorSelection(AActor* InActor = nullptr)
{
	if (GEditor)
	{
		USelection* SelectedActors = GEditor->GetSelectedActors();
		for (FSelectionIterator It(*SelectedActors); It; ++It)
		{
			if (AActor* Actor = Cast<AActor>(*It))
			{
				GEditor->SelectNone(false, true, false);
				GEditor->SelectActor(Actor, true, true, true);
			}
		}
	}
}

void CavrnusComponentModifiers::InitializeSupportedCavrnusComponentClasses()
{
	if (!SupportedCavrnusComponentClasses.IsEmpty())
	{
		return; // Already initialized
	}

	LoadBlueprintComponentClassesFromPath(TEXT("/CavrnusConnector/SamplePropertySynchronizers/"));

    // Map of class name strings to display names
	/*
    SupportedCavrnusComponentClassNames.Add(TEXT("AC_Cavrnus_SyncAvatarTransform_C"), TEXT("Sync Avatar Transform"));
    SupportedCavrnusComponentClassNames.Add(TEXT("AC_Cavrnus_SyncWalkAnimBP_HeadRotation_C"), TEXT("Sync Walk Head Rotation"));
    SupportedCavrnusComponentClassNames.Add(TEXT("AC_Cavrnus_SyncPawnSmoothedMovement_C"), TEXT("Sync Smoothed Movement"));
    SupportedCavrnusComponentClassNames.Add(TEXT("BP_Cavrnus_SyncPawnColor_C"), TEXT("Sync Pawn Color"));
    SupportedCavrnusComponentClassNames.Add(TEXT("AC_Cavrnus_SyncWalkAnimBP_Torso_C"), TEXT("Sync Walk Torso"));
    SupportedCavrnusComponentClassNames.Add(TEXT("AC_Cavrnus_BasePawnComponent_C"), TEXT("Base Pawn Component"));
	*/
	SupportedCavrnusComponentClassNames.Add(TEXT("AC_SyncRelativeTransform_C"), TEXT("Sync Relative Transform"));
	SupportedCavrnusComponentClassNames.Add(TEXT("AC_SyncWorldTransform_C"), TEXT("Sync World Transform"));
	SupportedCavrnusComponentClassNames.Add(TEXT("AC_SyncVisibility_C"), TEXT("Sync Visibility"));
//	SupportedCavrnusComponentClassNames.Add(TEXT("AC_SyncTextRender_C"), TEXT("Sync Text"));
//	SupportedCavrnusComponentClassNames.Add(TEXT("AC_SyncMeshColor_C"), TEXT("Sync Mesh Color"));

	/*
    SupportedCavrnusComponentClassNames.Add(TEXT("CavrnusValueSyncVector"), TEXT("Value Sync Vector"));
    SupportedCavrnusComponentClassNames.Add(TEXT("CavrnusValueSyncTransform"), TEXT("Value Sync Transform"));
    SupportedCavrnusComponentClassNames.Add(TEXT("CavrnusValueSyncString"), TEXT("Value Sync String"));
    SupportedCavrnusComponentClassNames.Add(TEXT("CavrnusValueSyncFloat"), TEXT("Value Sync Float"));
    SupportedCavrnusComponentClassNames.Add(TEXT("CavrnusValueSyncColor"), TEXT("Value Sync Color"));
    SupportedCavrnusComponentClassNames.Add(TEXT("CavrnusValueSyncBoolean"), TEXT("Value Sync Boolean"));
    SupportedCavrnusComponentClassNames.Add(TEXT("CavrnusUserAvatarFlag"), TEXT("User Avatar Flag"));
    SupportedCavrnusComponentClassNames.Add(TEXT("CavrnusSpawnedObjectFlag"), TEXT("Spawned Object Flag"));
    SupportedCavrnusComponentClassNames.Add(TEXT("CavrnusPropertiesContainer"), TEXT("Properties Container"));
    SupportedCavrnusComponentClassNames.Add(TEXT("CavrnusLocalUserFlag"), TEXT("Local User Flag"));
	*/

	TArray<UClass*> AllComponentClasses;
	for (TObjectIterator<UClass> It; It; ++It)
	{
		UClass* Class = *It;
		if (Class->IsChildOf(UActorComponent::StaticClass()) &&
			!Class->HasAnyClassFlags(CLASS_Abstract))
		{
			if (SupportedCavrnusComponentClassNames.Contains(Class->GetName()))
			{
				SupportedCavrnusComponentClasses.Add(Class);
			}
		}
	}

}

bool CavrnusComponentModifiers::HasComponent(AActor* Actor, UClass* ActorComponentClass, bool bIncludeFromChildActors)
{
	InitializeSupportedCavrnusComponentClasses();
	if (!Actor || !ActorComponentClass)
	{
		return false;
	}
	TArray<UActorComponent*> Components;
	Actor->GetComponents(Components, bIncludeFromChildActors);
	for (UActorComponent* Component : Components)
	{
		if (Component && Component->IsA(ActorComponentClass))
		{
			return true;
		}
	}
	return false;
}


bool CavrnusComponentModifiers::TryAddComponent(AActor* Actor, UClass* ActorComponentClass, UActorComponent*& OutCreated)
{
	OutCreated = nullptr;

	if (!Actor || !ActorComponentClass || !ActorComponentClass->IsChildOf(UActorComponent::StaticClass()))
	{
		return false;
	}

	// Optional: enforce supported set
	// if (!IsSupported(ActorComponentClass)) return false;

	if (Actor->HasAnyFlags(RF_ClassDefaultObject) || (GEditor && GEditor->PlayWorld))
	{
		return false;
	}

	USubobjectDataSubsystem* Subsys = GetSubobjectDataSubsystemCompat();
	if (!Subsys)
	{
		return false;
	}

	// Snapshot components to disambiguate the newly created one
	TSet<UActorComponent*> Before;
	{
		TInlineComponentArray<UActorComponent*> Existing(Actor);
		for (UActorComponent* C : Existing)
		{
			if (C) Before.Add(C);
		}
	}

	TArray<FSubobjectDataHandle> Handles;
	Subsys->K2_GatherSubobjectDataForInstance(Actor, Handles);
	if (Handles.Num() == 0)
	{
		return false;
	}

	const FSubobjectDataHandle ParentHandle = Handles[0];

	FScopedTransaction Tx(NSLOCTEXT("CavrnusConnectorEditor", "TryAddComponent", "Add Component To Actor"));
	Actor->Modify();

	FText FailReason;
	FAddNewSubobjectParams Params;
	Params.ParentHandle = ParentHandle;
	Params.NewClass = ActorComponentClass;
	Params.BlueprintContext = nullptr;          // instance path
	Params.bConformTransformToParent = true;
	Params.bSkipMarkBlueprintModified = true;

	const FSubobjectDataHandle NewHandle = Subsys->AddNewSubobject(Params, /*Out*/ FailReason);
	if (!NewHandle.IsValid())
	{
		Tx.Cancel();
		UE_LOG(LogTemp, Verbose, TEXT("Failed to add component: %s"), *FailReason.ToString());
		return false;
	}

	// Resolve created component by diff
	{
		TInlineComponentArray<UActorComponent*> After(Actor);
		for (UActorComponent* C : After)
		{
			if (C && !Before.Contains(C) && C->IsA(ActorComponentClass))
			{
				OutCreated = C;
				break;
			}
		}
	}

	// Optional: finalize scene component (only if needed)
	if (USceneComponent* SC = Cast<USceneComponent>(OutCreated))
	{
		if (!SC->IsRegistered())
		{
			// If you need a custom parent, prefer SetupAttachment(...) before registration.
			SC->RegisterComponent();
		}
		// Avoid overriding subsystem’s chosen parent/transform unless you must.
	}

	return OutCreated != nullptr;
}

UObject* Compat_GetObjectFromHandle(const FSubobjectDataHandle& Handle)
{
	if (!Handle.IsValid())
		return nullptr;

	if (const FSubobjectData* SubData = Handle.GetData())
	{
		if (const UObject* ConstObj = SubData->GetObject(false))
		{
			return const_cast<UObject*>(ConstObj);
		}
	}
	return nullptr;
}

bool CavrnusComponentModifiers::TryRemoveComponent(AActor* Actor, UClass* ActorComponentClass)
{
	if (!Actor || !ActorComponentClass)
		return false;

	if (USubobjectDataSubsystem* Subsys = USubobjectDataSubsystem::Get())
	{
		TArray<FSubobjectDataHandle> Handles;
		Subsys->GatherSubobjectData(Actor, Handles);

		for (const FSubobjectDataHandle& Handle : Handles)
		{
			if (UObject* Obj = Compat_GetObjectFromHandle(Handle))
			{
				if (UActorComponent* Comp = Cast<UActorComponent>(Obj))
				{
					if (Comp->IsA(ActorComponentClass))
					{
						Comp->Modify();
						Comp->DestroyComponent();
						
						if (Actor->HasAnyFlags(RF_Transactional))
                        {
                            Actor->Modify();
                        }
                        if (UBlueprint* Blueprint = Cast<UBlueprint>(Actor->GetClass()->ClassGeneratedBy))
                        {
                            FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
							FKismetEditorUtilities::CompileBlueprint(Blueprint);
							//FEditorFileUtils::SaveDirtyPackages(/*bSaveMapPackages=*/false, /*bSaveContentPackages=*/true, /*bPromptToSave=*/false, false, false, false);
							ToggleActorSelection();
                        }
						else
						{
							ToggleActorSelection(Actor);
						}
						return true;
					}
				}
			}
		}

	}
	return false;
}

FCavrnusComponentInfo CavrnusComponentModifiers::GetCavrnusComponentInfo(UClass* ActorComponentClass)
{
	InitializeSupportedCavrnusComponentClasses();
	FCavrnusComponentInfo OutStruct = FCavrnusComponentInfo();

	OutStruct.Class = ActorComponentClass;
	OutStruct.ClassName = ActorComponentClass->GetName();
	OutStruct.DisplayName = ActorComponentClass->GetDisplayNameText().ToString();
	FString* SanitizedName = SupportedCavrnusComponentClassNames.Find(OutStruct.ClassName);
	if (!SanitizedName)
	{
		// Fallback to class name if not found in the map
		SanitizedName = &OutStruct.ClassName;
	}
	OutStruct.SanitizedName = *SanitizedName;
	OutStruct.Description = ActorComponentClass->GetToolTipText().ToString();
	OutStruct.bIsUserAddable = true; // Placeholder, customize as needed
	return OutStruct;
}


void CavrnusComponentModifiers::GetMyCavrnusComponents(AActor* Actor, TArray<FCavrnusComponentInfo>& Components)
{
	InitializeSupportedCavrnusComponentClasses();
	Components.Empty();
	TArray<UActorComponent*> ThisActorsComponents;
	Actor->GetComponents(ThisActorsComponents, false);

	for (UActorComponent* Component : ThisActorsComponents)
	{
		if (!Component || !Component->GetClass())
		{
			continue;
		}
		UClass* ComponentClass = Component->GetClass();
		if (SupportedCavrnusComponentClasses.Contains(ComponentClass))
		{
			FCavrnusComponentInfo Info = GetCavrnusComponentInfo(ComponentClass);
			Components.Add(Info);
		}
	}
}

void CavrnusComponentModifiers::GetMyCavrnusComponents(UBlueprint* Blueprint, TArray<FCavrnusComponentInfo>& Components)
{
	InitializeSupportedCavrnusComponentClasses();

	if (!Blueprint || !Blueprint->SimpleConstructionScript)
	{
		Components.Empty();
		return;
	}

	AActor* CDO = Cast<AActor>(Blueprint->GeneratedClass->GetDefaultObject());
	TArray<UActorComponent*> ThisBlueprintComponents;
	CDO->GetComponents(ThisBlueprintComponents);

	for (UActorComponent* Component : ThisBlueprintComponents)
	{
		if (!Component || !Component->GetClass())
		{
			continue;
		}
		UClass* ComponentClass = Component->GetClass();
		if (SupportedCavrnusComponentClasses.Contains(ComponentClass))
		{
			FCavrnusComponentInfo Info = GetCavrnusComponentInfo(ComponentClass);
			Components.Add(Info);
		}
	}
}

void CavrnusComponentModifiers::GetAllCavrnusComponents(TArray<FCavrnusComponentInfo>& Components, UClass* FilterClass)
{
	InitializeSupportedCavrnusComponentClasses();
	Components.Empty();
	for (UClass* ComponentClass : SupportedCavrnusComponentClasses)
	{
		if (FilterClass && !ComponentClass->IsChildOf(FilterClass))
		{
			continue;
		}
		FCavrnusComponentInfo Info = GetCavrnusComponentInfo(ComponentClass);
		Components.Add(Info);
	}
}


void CavrnusComponentModifiers::GetWhitelistedCavrnusComponents(TArray<UClass*>& ComponentClasses)
{
	InitializeSupportedCavrnusComponentClasses();
	ComponentClasses = SupportedCavrnusComponentClasses;
}

void CavrnusComponentModifiers::GetLevelActorsStruct(TArray<FActorStruct>& LevelActors, UClass* ClassFilter)
{
	LevelActors.Empty();
	UWorld* World = nullptr;
	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		if (Context.WorldType == EWorldType::Editor)
		{
			World = Context.World();
			break;
		}
	}

	if (!World)
	{
		return;
	}

	for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
	{
		AActor* Actor = *ActorItr;
		if (!Actor) continue;
		if (ClassFilter && !Actor->IsA(ClassFilter)) continue;

		FActorStruct Entry;
		Entry.Actor = Actor;
		Entry.Name = Actor->GetActorLabel();
		GetMyCavrnusComponents(Actor, Entry.Components);

		LevelActors.Add(MoveTemp(Entry));
	}
}


void CavrnusComponentModifiers::GetLevelActors(TArray<AActor*>& LevelActors, UClass* ClassFilter)
{
	LevelActors.Empty();
	UWorld* World = nullptr;
	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		if (Context.WorldType == EWorldType::Editor)
		{
			World = Context.World();
			break;
		}
	}
	for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
	{
		AActor* Actor = *ActorItr;
		if (!Actor) continue;
		if (ClassFilter && !Actor->IsA(ClassFilter)) continue;
		LevelActors.Add(Actor);
	}
}

void CavrnusComponentModifiers::GetLevelActorsWithFilter(TArray<AActor*>& LevelActors, ESupportedActorTypesFilter FilterTypes)
{
	LevelActors.Empty();
	UWorld* World = nullptr;

	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		if (Context.WorldType == EWorldType::Editor)
		{
			World = Context.World();
			break;
		}
	}

	for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
	{
		AActor* Actor = *ActorItr;
		if (!Actor) continue;
		bool bMatchesFilter = false;
		switch (FilterTypes)
		{
		case ESupportedActorTypesFilter::Any:
			bMatchesFilter = true;
			break;
		case ESupportedActorTypesFilter::StaticMesh:
			bMatchesFilter = Actor->IsA<AStaticMeshActor>();
			break;
		case ESupportedActorTypesFilter::PointLight:
			bMatchesFilter = Actor->IsA<APointLight>();
			break;
		default:
			break;
		}
		if (bMatchesFilter)
		{
			LevelActors.Add(Actor);
		}
	}
}


bool CavrnusComponentModifiers::TryAddComponentToBlueprint(UBlueprint* Blueprint, UClass* ComponentClass, const FName& NewName, USCS_Node*& OutNode)
{
	OutNode = nullptr;

	if (!Blueprint || !ComponentClass || !ComponentClass->IsChildOf<UActorComponent>())
		return false;

	// Only Actor-based Blueprints have SCS for default components
	if (!Blueprint->SimpleConstructionScript)
		return false;

	// Check if it already exists
	for (USCS_Node* Node : Blueprint->SimpleConstructionScript->GetAllNodes())
	{
		if (Node && Node->ComponentClass == ComponentClass)
		{
			return false; // already there
		}
	}

	FScopedTransaction Transaction(NSLOCTEXT("CavrnusComponentModifiers", "TryAddComponentToBlueprint", "Add Component to Blueprint"));

	Blueprint->Modify();

	// Create a new SCS Node
	OutNode = Blueprint->SimpleConstructionScript->CreateNode(ComponentClass, NewName);
	Blueprint->SimpleConstructionScript->AddNode(OutNode);

	// Mark dirty so it can be saved & compiled
	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
	FKismetEditorUtilities::CompileBlueprint(Blueprint);
	ToggleActorSelection();

	return true;
}

bool CavrnusComponentModifiers::TryRemoveComponentFromBlueprint(UBlueprint* Blueprint, UClass* ComponentClass)
{
	if (!Blueprint || !ComponentClass || !ComponentClass->IsChildOf<UActorComponent>())
		return false;

	USimpleConstructionScript* SCS = Blueprint->SimpleConstructionScript;
	if (!SCS)
		return false;

	// Find the node(s) to remove
	TArray<USCS_Node*> NodesToRemove;
	for (USCS_Node* Node : SCS->GetAllNodes())
	{
		if (Node && Node->ComponentClass == ComponentClass)
		{
			NodesToRemove.Add(Node);
		}
	}

	if (NodesToRemove.Num() == 0)
		return false; // nothing to remove

	FScopedTransaction Transaction(NSLOCTEXT("CavrnusComponentModifiers", "TryRemoveComponentFromBlueprint", "Remove Component from Blueprint"));
	Blueprint->Modify();

	for (USCS_Node* Node : NodesToRemove)
	{
		SCS->RemoveNode(Node);
	}

	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
	FKismetEditorUtilities::CompileBlueprint(Blueprint);
	ToggleActorSelection();
	return true;
}


static void AddClassFilter(FARFilter& Filter, UClass* Class, bool bIncludeDerived)
{
#if (ENGINE_MAJOR_VERSION > 5) || (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1)
	Filter.ClassPaths.Add(Class->GetClassPathName());            // /Script/Module.Class
	Filter.bRecursiveClasses = bIncludeDerived;
#else
	Filter.ClassNames.Add(Class->GetFName());                    // Class short name
	Filter.bRecursiveClasses = bIncludeDerived;
#endif
}

static void ExcludeDerivedOf(FARFilter& Filter, UClass* Base)
{
#if (ENGINE_MAJOR_VERSION > 5) || (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1)
	Filter.RecursiveClassPathsExclusionSet.Add(Base->GetClassPathName());
#else
	Filter.RecursiveClassesExclusionSet.Add(Base->GetFName());
#endif
}


void CavrnusComponentModifiers::CompileAllDirtyBlueprints()
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

	FARFilter Filter;

	AddClassFilter(Filter, UBlueprint::StaticClass(), true); // Include all Blueprint classes
	Filter.bRecursiveClasses = true;

	TArray<FAssetData> BlueprintAssets;
	AssetRegistryModule.Get().GetAssets(Filter, BlueprintAssets);

	for (const FAssetData& Asset : BlueprintAssets)
	{
		if (UBlueprint* BP = Cast<UBlueprint>(Asset.GetAsset()))
		{
			if (BP->Status == EBlueprintStatus::BS_Dirty || BP->bIsRegeneratingOnLoad)
			{
				FKismetEditorUtilities::CompileBlueprint(BP, EBlueprintCompileOptions::None, nullptr);
			}
		}
	}
}

void CavrnusComponentModifiers::CavrnusComponentModifiers::SaveAll()
{
	FEditorFileUtils::SaveDirtyPackages(
		/*bPromptUserToSave=*/false,
		/*bSaveMapPackages=*/true,
		/*bSaveContentPackages=*/true);
}
#endif // WITH_EDITOR