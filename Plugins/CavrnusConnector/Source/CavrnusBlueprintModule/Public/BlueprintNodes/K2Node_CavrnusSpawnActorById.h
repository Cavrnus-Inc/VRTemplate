// Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "K2Node.h"
#include "Managers/SpawnedObjects/CavrnusSpawnPropertyHelpers.h"
#include "K2Node_CavrnusSpawnActorById.generated.h"

/**
 * @brief Custom Blueprint node for CavrnusSpawnActorById that dynamically exposes ExposeOnSpawn property pins.
 * Inherits from UK2Node (not UK2Node_CallFunction) to avoid validation errors for ExposeOnSpawn pins.
 */
UCLASS(meta = (DisplayName = "Cavrnus Spawn Actor By Id"))
class CAVRNUSBLUEPRINTMODULE_API UK2Node_CavrnusSpawnActorById : public UK2Node
{
    GENERATED_BODY()

public:
    UK2Node_CavrnusSpawnActorById(const FObjectInitializer& ObjectInitializer);

    // Override to add ExposeOnSpawn property pins
    virtual void AllocateDefaultPins() override;

    // Override to update ExposeOnSpawn pins when WellKnownObjectId changes
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
     * @brief Updates the WellKnownObjectId pin's filtering/metadata based on the current DataAsset.
     */
    void UpdateIdPinFiltering();

    /**
     * @brief Gets the valid IDs from the DataAsset for the WellKnownObjectId pin.
     * This is called by the custom pin widget to get the filtered list.
     */
    TArray<FName> GetValidIds() const;

private:
    /**
     * @brief Gets the actor class from the WellKnownObjectId pin by looking it up in the DataAsset.
     */
    UClass* GetActorClassFromId() const;

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
     * @brief Gets all valid IDs from the DataAsset for filtering.
     */
    TArray<FName> GetValidIdsFromDataAsset() const;

    /** Cached ExposeOnSpawn properties for the current actor class */
    UPROPERTY()
    TArray<FCavrnusExposeOnSpawnProperty> CachedExposeOnSpawnProperties;

    /** Cached actor class to detect changes */
    UPROPERTY()
    UClass* CachedActorClass;
};

