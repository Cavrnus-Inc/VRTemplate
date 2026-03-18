// Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "K2Node.h"
#include "Managers/SpawnedObjects/CavrnusSpawnPropertyHelpers.h"
#include "K2Node_CavrnusSpawnActorFromClass.generated.h"

/**
 * @brief Custom Blueprint node for CavrnusSpawnActorFromClass that dynamically exposes ExposeOnSpawn property pins.
 * Inherits from UK2Node (not UK2Node_CallFunction) to avoid validation errors for ExposeOnSpawn pins.
 */
UCLASS(meta = (DisplayName = "Cavrnus Spawn Actor From Class"))
class CAVRNUSBLUEPRINTMODULE_API UK2Node_CavrnusSpawnActorFromClass : public UK2Node
{
    GENERATED_BODY()

public:
    UK2Node_CavrnusSpawnActorFromClass(const FObjectInitializer& ObjectInitializer);

    // Override to add ExposeOnSpawn property pins
    virtual void AllocateDefaultPins() override;

    // Override to update ExposeOnSpawn pins when ActorClass changes
    virtual void NotifyPinConnectionListChanged(UEdGraphPin* Pin) override;

    // Override to detect when default values change (not just connections)
    virtual void PostReconstructNode() override;

    // Override to detect when pin default values change in the property editor
    virtual void PinDefaultValueChanged(UEdGraphPin* Pin) override;

    // Override to set ExposeOnSpawn property values on the actor after spawning
    virtual void ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;

    virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
    virtual FText GetMenuCategory() const override { return FText::FromString(TEXT("Cavrnus|Objects")); }
    virtual FText GetTooltipText() const override;

    virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
    /**
     * @brief Updates the ActorClass pin's filtering/metadata based on the current DataAsset.
     */
    void UpdateActorClassPinFiltering();

    /**
     * @brief Gets the valid actor classes from the DataAsset for the ActorClass pin.
     * This is called by the custom pin widget to get the filtered list.
     */
     TArray<UClass*> GetValidActorClasses() const;
private:
    /**
     * @brief Gets the actor class from the ActorClass pin.
     */
    UClass* GetActorClassFromPin() const;

    /**
     * @brief Creates pins for ExposeOnSpawn properties.
     */
    void CreateExposeOnSpawnPins();

    /**
     * @brief Removes all ExposeOnSpawn property pins.
     */
    void RemoveExposeOnSpawnPins();

    /**
     * @brief Gets the pin type for an ExposeOnSpawn property.
     */
    FEdGraphPinType GetPinTypeForProperty(FProperty* Prop) const;

    /**
     * @brief Gets all valid actor classes from the DataAsset for filtering.
     */
    TArray<UClass*> GetValidActorClassesFromDataAsset() const;



    /** Cached ExposeOnSpawn properties for the current actor class */
    UPROPERTY()
    TArray<FCavrnusExposeOnSpawnProperty> CachedExposeOnSpawnProperties;

    /** Cached actor class to detect changes */
    UPROPERTY()
    UClass* CachedActorClass;

    /** Cached valid classes from DataAsset for filtering */
    TArray<UClass*> CachedValidClasses;
};

