// Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "K2Node.h"
#include "Managers/SpawnedObjects/CavrnusSpawnPropertyHelpers.h"
#include "K2Node_CavrnusConstructObjectFromClass.generated.h"

/**
 * @brief Custom Blueprint node for CavrnusConstructObjectFromClass that dynamically exposes ExposeOnSpawn property pins.
 * Inherits from UK2Node (not UK2Node_CallFunction) to avoid validation errors for ExposeOnSpawn pins.
 */
UCLASS(meta = (DisplayName = "Cavrnus Construct Object From Class"))
class CAVRNUSBLUEPRINTMODULE_API UK2Node_CavrnusConstructObjectFromClass : public UK2Node
{
    GENERATED_BODY()

public:
    UK2Node_CavrnusConstructObjectFromClass(const FObjectInitializer& ObjectInitializer);

    // Override to add ExposeOnSpawn property pins
    virtual void AllocateDefaultPins() override;

    // Override to update ExposeOnSpawn pins when ObjectClass changes
    virtual void NotifyPinConnectionListChanged(UEdGraphPin* Pin) override;

    // Override to detect when default values change (not just connections)
    virtual void PostReconstructNode() override;

    // Override to detect when pin default values change in the property editor
    virtual void PinDefaultValueChanged(UEdGraphPin* Pin) override;

    // Override to set ExposeOnSpawn property values on the object after construction
    virtual void ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;

    virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
    virtual FText GetMenuCategory() const override { return FText::FromString(TEXT("Cavrnus|Objects")); }
    virtual FText GetTooltipText() const override;

    virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;

    /**
     * @brief Updates the ObjectClass pin's filtering/metadata based on the current DataAsset.
     */
    void UpdateObjectClassPinFiltering();

    /**
     * @brief Gets the valid object classes from the DataAsset for the ObjectClass pin.
     * This is called by the custom pin widget to get the filtered list.
     */
    TArray<UClass*> GetValidObjectClasses() const;

private:
    /**
     * @brief Gets the object class from the ObjectClass pin.
     */
    UClass* GetObjectClassFromPin() const;

    /**
     * @brief Creates pins for ExposeOnSpawn properties.
     */
    void CreateExposeOnSpawnPins();

    /**
     * @brief Removes all ExposeOnSpawn property pins.
     */
    void RemoveExposeOnSpawnPins();

    /**
     * @brief Checks if ExposeOnSpawn pins exist and match the current class's properties.
     * @return true if all required pins exist, false otherwise.
     */
    bool DoExposeOnSpawnPinsExist() const;

    /**
     * @brief Gets the pin type for an ExposeOnSpawn property.
     */
    FEdGraphPinType GetPinTypeForProperty(FProperty* Prop) const;

    /**
     * @brief Gets all valid object classes from the DataAsset for filtering.
     */
    TArray<UClass*> GetValidObjectClassesFromDataAsset() const;

    /** Cached ExposeOnSpawn properties for the current object class */
    UPROPERTY()
    TArray<FCavrnusExposeOnSpawnProperty> CachedExposeOnSpawnProperties;

    /** Cached object class to detect changes */
    UPROPERTY()
    UClass* CachedObjectClass;

    /** Cached valid classes from DataAsset for filtering */
    TArray<UClass*> CachedValidClasses;
};

