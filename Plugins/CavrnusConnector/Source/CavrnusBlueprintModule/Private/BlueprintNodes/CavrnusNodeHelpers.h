// Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "K2Node.h"
#include "EdGraphSchema_K2.h"
#include "Managers/SpawnedObjects/CavrnusSpawnPropertyHelpers.h"
#include "KismetCompiler.h"
#include "K2Node_CallFunction.h"
#include "K2Node_VariableGet.h"
#include "K2Node_MakeStruct.h"
#include "K2Node_MakeArray.h"
#include "CavrnusFunctionLibrary.h"
#include "CavrnusPropertiesContainer.h"

class UEdGraphPin;
class UEdGraph;
class FKismetCompilerContext;

/**
 * Static helper class for common K2Node operations shared across Cavrnus Blueprint nodes.
 */
class CAVRNUSBLUEPRINTMODULE_API FCavrnusNodeHelpers
{
public:
    /**
     * Converts an FProperty to an FEdGraphPinType for Blueprint pins.
     * Uses schema conversion when available, falls back to hardcoded types.
     */
    static FEdGraphPinType GetPinTypeForProperty(FProperty* Prop);

    /**
     * Removes all ExposeOnSpawn property pins from a node.
     * Checks both current class properties and cached properties to ensure all pins are removed.
     */
    static void RemoveExposeOnSpawnPins(
        UK2Node* Node,
        UClass* CurrentClass,
        const TArray<FCavrnusExposeOnSpawnProperty>& CachedProperties,
        TFunction<UClass*()> GetClassFunc
    );

    /**
     * Creates ExposeOnSpawn property pins on a node.
     * Preserves existing pins and their connections/values when possible.
     */
    static void CreateExposeOnSpawnPins(
        UK2Node* Node,
        UClass* CurrentClass,
        UClass*& CachedClass,
        TArray<FCavrnusExposeOnSpawnProperty>& CachedProperties,
        TFunction<UClass*()> GetClassFunc,
        TFunction<FEdGraphPinType(FProperty*)> GetPinTypeFunc,
        int32 InsertIndex
    );

    /**
     * Moves pins to a specific position in the node's Pins array.
     */
    static void MovePinsToPosition(
        UK2Node* Node,
        const TArray<UEdGraphPin*>& PinsToMove,
        int32 InsertIndex
    );

    /**
     * Expands ExposeOnSpawn properties during Blueprint compilation.
     * Creates intermediate nodes to set properties and post them to the journal.
     */
    static void ExpandExposeOnSpawnProperties(
        FKismetCompilerContext& CompilerContext,
        UEdGraph* SourceGraph,
        UK2Node* SourceNode,
        UEdGraphPin* ReturnPin,
        UEdGraphPin* SpaceConnectionPin,
        const TArray<FCavrnusExposeOnSpawnProperty>& ExposeProps
    );

    /**
     * Builds an array of MakeStruct nodes from ExposeOnSpawn properties.
     * Creates MakeStruct nodes for each property and optionally creates a MakeArray node to combine them.
     * 
     * @param CompilerContext The compiler context
     * @param SourceGraph The source graph
     * @param SourceNode The source node containing the ExposeOnSpawn pins
     * @param ExposeProps The ExposeOnSpawn properties to process
     * @param OutMakeStructNodes Output array of created MakeStruct nodes
     * @param OutMakeArrayNode Output pointer to the created MakeArray node (nullptr if no properties)
     * @return true if successful, false otherwise
     */
    static bool BuildExposeOnSpawnPropertyArray(
        FKismetCompilerContext& CompilerContext,
        UEdGraph* SourceGraph,
        UK2Node* SourceNode,
        const TArray<FCavrnusExposeOnSpawnProperty>& ExposeProps,
        TArray<UK2Node_MakeStruct*>& OutMakeStructNodes,
        UK2Node_MakeArray*& OutMakeArrayNode
    );

    /**
     * Wires a function call node with execution pins, parameter pins, and return value.
     * 
     * @param CompilerContext The compiler context
     * @param FunctionCallNode The function call node to wire
     * @param ParameterPinMappings Map of parameter names to source pins
     * @param ExecutePin Source execute pin
     * @param ThenPin Source then pin
     * @param ReturnPin Source return pin (optional)
     * @return true if successful, false otherwise
     */
    static bool WireFunctionCallNode(
        FKismetCompilerContext& CompilerContext,
        UK2Node_CallFunction* FunctionCallNode,
        const TMap<FString, UEdGraphPin*>& ParameterPinMappings,
        UEdGraphPin* ExecutePin,
        UEdGraphPin* ThenPin,
        UEdGraphPin* ReturnPin = nullptr
    );
};

