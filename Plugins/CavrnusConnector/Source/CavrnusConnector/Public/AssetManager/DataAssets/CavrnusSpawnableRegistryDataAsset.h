#pragma once

#include "CoreMinimal.h"
#include "Core/Subsystems/CavrnusSubsystem.h"
#include "Engine/DataAsset.h"
#include "Misc/Optional.h"
#include "AssetRegistry/AssetData.h"
#include "AssetManager/CavrnusBaseDataAsset.h"
#include "Engine/StaticMesh.h"
#include "CavrnusSpawnableRegistryDataAsset.generated.h"

USTRUCT(BlueprintType)
struct FCavrnusSpawnableEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cavrnus|Spawnables")
    FName Key;

    // ActorClass: For Actor classes (Blueprint or native) used with CavrnusSpawnActorFromClass/ById.
    // These are classes that will be spawned as Actors in the world.
    // Must be AActor classes (matching Unreal Engine's SpawnActor behavior).
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cavrnus|Spawnables")
    TSoftClassPtr<AActor> ActorClass;

    // ObjectClass: For non-Actor UObject classes used with CavrnusConstructObjectFromClass.
    // These are classes that will be constructed as UObject instances (not Actors).
    // Must be UObject classes that are NOT AActor (matching Unreal Engine's ConstructObject behavior).
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cavrnus|Spawnables")
    TSoftClassPtr<UObject> ObjectClass;
};

UCLASS(BlueprintType)
class CAVRNUSCONNECTOR_API UCavrnusSpawnableRegistryDataAsset : public UCavrnusBaseDataAsset
{
    GENERATED_BODY()

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cavrnus|Spawnables")
    TArray<FCavrnusSpawnableEntry> Entries;

public:
    const TArray<FCavrnusSpawnableEntry>& GetEntries() const
    {
        return Entries;
    }

    void AddSpawnableActor(FName Identifier, TSoftClassPtr<AActor> InActorClass)
    {
        for (FCavrnusSpawnableEntry& Entry : Entries)
        {
            if (Entry.Key == Identifier)
            {
                Entry.ActorClass = InActorClass;
                MarkPackageDirty();
                return;
            }
        }

        FCavrnusSpawnableEntry NewEntry;
        NewEntry.Key = Identifier;
        NewEntry.ActorClass = InActorClass;
        Entries.Add(NewEntry);
        MarkPackageDirty();
    }

    void AddSpawnableObject(FName Identifier, TSoftClassPtr<UObject> InObjectClass)
    {
        for (FCavrnusSpawnableEntry& Entry : Entries)
        {
            if (Entry.Key == Identifier)
            {
                Entry.ObjectClass = InObjectClass;
                MarkPackageDirty();
                return;
            }
        }

        FCavrnusSpawnableEntry NewEntry;
        NewEntry.Key = Identifier;
        NewEntry.ObjectClass = InObjectClass;
        Entries.Add(NewEntry);
        MarkPackageDirty();
    }

    void RemoveSpawnable(FName Identifier)
    {
        int32 Removed = Entries.RemoveAll([&](const FCavrnusSpawnableEntry& Entry)
            {
                return Entry.Key == Identifier;
            });

        if (Removed > 0)
        {
            MarkPackageDirty();
        }
    }

    TOptional<TSoftClassPtr<AActor>> GetActorClassForKey(const FName& InKey) const
    {
        for (const FCavrnusSpawnableEntry& Entry : Entries)
        {
            if (Entry.Key == InKey && !Entry.ActorClass.IsNull())
            {
                return Entry.ActorClass;
            }
        }
        return TOptional<TSoftClassPtr<AActor>>();
    }

    TOptional<TSoftClassPtr<UObject>> GetObjectClassForKey(const FName& InKey) const
    {
        for (const FCavrnusSpawnableEntry& Entry : Entries)
        {
            if (Entry.Key == InKey && !Entry.ObjectClass.IsNull())
            {
                return Entry.ObjectClass;
            }
        }
        return TOptional<TSoftClassPtr<UObject>>();
    }

};
