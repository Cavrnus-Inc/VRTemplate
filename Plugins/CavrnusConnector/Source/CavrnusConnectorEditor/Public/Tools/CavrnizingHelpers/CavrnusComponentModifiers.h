// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/SCS_Node.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "CavrnusComponentModifiers.generated.h"
enum class ESupportedActorTypesFilter : uint8
{
	Any = 0x0,
	StaticMesh = 0x1,
	PointLight = 0x2
};

USTRUCT()
struct FCavrnusComponentInfo
{
	GENERATED_BODY()

	UClass* Class;
	FString ClassName;
	FString DisplayName;
	FString SanitizedName;
	FString Description;
	bool bIsUserAddable;
	FCavrnusComponentInfo()
		: Class(nullptr), ClassName(TEXT(""))
		  , DisplayName(TEXT(""))
		  , SanitizedName(TEXT(""))
		  , Description(TEXT(""))
		  , bIsUserAddable(false)
	{
	}

	bool operator==(const FCavrnusComponentInfo& Other) const
	{
		return ClassName == Other.ClassName;
	}
};

USTRUCT()
struct FActorStruct
{
	GENERATED_BODY()

	AActor* Actor;
	FString Name;
	TArray<FCavrnusComponentInfo> Components;

	bool operator==(const FActorStruct& Other) const
	{
		return Name == Other.Name;
	}
};


/**
 * 
 */
class CavrnusComponentModifiers
{
public:
	static bool HasComponent(AActor* Actor, UClass* ActorComponentClass, bool bIncludeFromChildActors = false);
	static bool TryAddComponent(AActor* Actor, UClass* ActorComponentClass, UActorComponent*& OutCreated);
	static bool TryRemoveComponent(AActor* Actor, UClass* ActorComponentClass);

	static void GetAllCavrnusComponents(TArray<FCavrnusComponentInfo>& Components, UClass* FilterClass=nullptr);
	static void GetWhitelistedCavrnusComponents(TArray<UClass*>& ComponentClasses);

	static void GetMyCavrnusComponents(AActor* Actor, TArray<FCavrnusComponentInfo>& Components);
	static void GetMyCavrnusComponents(UBlueprint* Blueprint, TArray<FCavrnusComponentInfo>& Components);


	static void GetLevelActorsStruct(TArray<FActorStruct>& LevelActors, UClass* ClassFilter = nullptr);
	static void GetLevelActors(TArray<AActor*>& LevelActors, UClass* ClassFilter = nullptr);
	static void GetLevelActorsWithFilter(TArray<AActor*>& LevelActors, ESupportedActorTypesFilter FilterTypes = ESupportedActorTypesFilter::Any);


	static bool TryAddComponentToBlueprint(UBlueprint* Blueprint, UClass* ComponentClass, const FName& NewName, USCS_Node*& OutNode);
	static bool TryRemoveComponentFromBlueprint(UBlueprint* Blueprint, UClass* ComponentClass);

	static void CompileAllDirtyBlueprints();
	static void SaveAll();

	static FCavrnusComponentInfo GetCavrnusComponentInfo(UClass* ActorComponentClass);

private:
	static TMap<FString, FString> SupportedCavrnusComponentClassNames;
	static TArray<UClass*> SupportedCavrnusComponentClasses;
	static void InitializeSupportedCavrnusComponentClasses();

	static TMap<UClass*, FCavrnusComponentInfo> WhitelistedCavrnusComponents;

	// Do not love this, but we don't seem to find all of the blueprints in the plugin
	static void LoadBlueprintComponentClassesFromPath(const FString& AssetPath)
	{
		// Convert to long package path
		FString PackagePath = TEXT("/CavrnusConnector/");

		// Use Asset Registry to find assets
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
		IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

		// Ensure registry is up to date
		TArray<FString> PathsToScan = { PackagePath };
		AssetRegistry.ScanPathsSynchronous(PathsToScan);

		// Query for Blueprint assets
		FARFilter Filter;
		Filter.PackagePaths.Add(*PackagePath);
		Filter.bRecursivePaths = true;

		TArray<FAssetData> AssetDataList;
		AssetRegistry.GetAssets(Filter, AssetDataList);

		for (const FAssetData& Asset : AssetDataList)
		{
			UBlueprint* Blueprint = Cast<UBlueprint>(Asset.GetAsset());
			if (Blueprint && Blueprint->SimpleConstructionScript)
				TArray<USCS_Node*> SCSNodes = Blueprint->SimpleConstructionScript->GetAllNodes();
		}
	}
};

