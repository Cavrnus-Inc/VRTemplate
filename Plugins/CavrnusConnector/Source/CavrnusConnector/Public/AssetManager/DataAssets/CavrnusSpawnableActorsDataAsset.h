#pragma once

#include "CoreMinimal.h"
#include "CavrnusSubsystem.h"
#include "Engine/DataAsset.h"
#include "Misc/Optional.h"
#include "AssetRegistry/AssetData.h"
#include "AssetManager/CavrnusBaseDataAsset.h"
#include "Engine/StaticMesh.h"
#include "CavrnusSpawnableActorsDataAsset.generated.h"

USTRUCT(BlueprintType)
struct FCavrnusSpawnableEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cavrnus|Spawnables")
    FName Key;

    // For Blueprint actor classes (or native actor classes)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cavrnus|Spawnables")
    TSoftClassPtr<AActor> ActorClass;

    // For raw meshes you�ll wrap in a StaticMeshActor
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cavrnus|Spawnables")
    TSoftObjectPtr<UStaticMesh> StaticMesh;
};

UCLASS(BlueprintType)
class CAVRNUSCONNECTOR_API UCavrnusSpawnableActorsDataAsset : public UCavrnusBaseDataAsset
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

    void AddSpawnableMesh(FName Identifier, TSoftObjectPtr<UStaticMesh> InMesh)
    {
        for (FCavrnusSpawnableEntry& Entry : Entries)
        {
            if (Entry.Key == Identifier)
            {
                Entry.StaticMesh = InMesh;
                MarkPackageDirty();
                return;
            }
        }

        FCavrnusSpawnableEntry NewEntry;
        NewEntry.Key = Identifier;
        NewEntry.StaticMesh = InMesh;
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
            if (Entry.Key == InKey && Entry.ActorClass.IsValid())
            {
                return Entry.ActorClass;
            }
        }
        return TOptional<TSoftClassPtr<AActor>>();
    }

    TOptional<TSoftObjectPtr<UStaticMesh>> GetMeshForKey(const FName& InKey) const
    {
        for (const FCavrnusSpawnableEntry& Entry : Entries)
        {
            if (Entry.Key == InKey && Entry.StaticMesh.IsValid())
            {
                return Entry.StaticMesh;
            }
        }
        return TOptional<TSoftObjectPtr<UStaticMesh>>();
    }

};