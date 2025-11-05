#include "PropertySync/CavrnusSyncPropertyNode.h"
#include "K2Node_CallFunction.h"
#include "K2Node_MakeStruct.h"
#include "KismetCompiler.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "PropertySyncers/CavrnusPropertySyncFunctionLibrary.h"
#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintNodeSpawner.h"
#include "Runtime/Launch/Resources/Version.h"

FText UCavrnusSyncPropertyNode::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
    return FText::FromString(TEXT("Cavrnus Sync Property"));
}

void UCavrnusSyncPropertyNode::AllocateDefaultPins()
{
    // Exec
    CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Exec, TEXT("Execute"))
        ->PinToolTip = TEXT("Executes the property sync operation.");

    // Owner (hidden)
    UEdGraphPin* OwnerPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Object, AActor::StaticClass(), TEXT("Owner"));
    OwnerPin->bHidden = true;
    OwnerPin->PinToolTip = TEXT("Actor that owns the property. Defaults to Self if not connected.");

    // SyncInfo
    UEdGraphPin* SyncInfoPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Struct, TEXT("SyncInfo"));
    SyncInfoPin->PinType.PinSubCategoryObject = FCavrnusSyncInfo::StaticStruct();
    SyncInfoPin->PinToolTip = TEXT("SyncInfo struct specifying container and property name. Use 'Make Cavrnus SyncInfo'.");

    // Property
    UEdGraphPin* PropertyPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Wildcard, TEXT("Property"));
    if (CachedPropertyPinType.PinCategory != NAME_None &&
        CachedPropertyPinType.PinCategory != UEdGraphSchema_K2::PC_Wildcard)
    {
        PropertyPin->PinType = CachedPropertyPinType;
    }

    // SyncOptions
    UEdGraphPin* SyncOptionsPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Wildcard, TEXT("SyncOptions"));
    if (CachedSyncOptionsPinType.PinCategory != NAME_None &&
        CachedSyncOptionsPinType.PinCategory != UEdGraphSchema_K2::PC_Wildcard)
    {
        SyncOptionsPin->PinType = CachedSyncOptionsPinType;
        if (!CachedSyncOptionsFriendlyName.IsEmpty())
        {
            SyncOptionsPin->PinFriendlyName = CachedSyncOptionsFriendlyName;
        }
    }

    // Then
    CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, TEXT("Then"))
        ->PinToolTip = TEXT("Executes after the property sync call.");

    // PropertySync (return)
    UEdGraphPin* SyncerPin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Object, UCavrnusPropertySyncer::StaticClass(), TEXT("PropertySync"));
    SyncerPin->PinToolTip = TEXT("Reference to the created PropertySync object.");
}

void UCavrnusSyncPropertyNode::ValidateNodeDuringCompilation(FCompilerResultsLog& MessageLog) const
{
    Super::ValidateNodeDuringCompilation(MessageLog);
}

void UCavrnusSyncPropertyNode::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
    Super::ExpandNode(CompilerContext, SourceGraph);

    // Source pins
    UEdGraphPin* ExecPin = FindPin(TEXT("Execute"));
    UEdGraphPin* OwnerPin = FindPin(TEXT("Owner"));
    UEdGraphPin* SyncInfoPin = FindPin(TEXT("SyncInfo"));
    UEdGraphPin* PropertyPin = FindPin(TEXT("Property"));
    UEdGraphPin* SyncOptionsPin = FindPin(TEXT("SyncOptions"));
    UEdGraphPin* ThenPin = FindPin(TEXT("Then"));
    UEdGraphPin* PropertySyncPin = FindPin(TEXT("PropertySync"));

    bool bHasError = false;

    // Required pins
    if (!PropertyPin || !PropertyPin->HasAnyConnections())
    {
        CompilerContext.MessageLog.Error(TEXT("Cavrnus Sync Property node @@ requires a property connection."), this);
        bHasError = true;
    }

    if (!SyncInfoPin || !SyncInfoPin->HasAnyConnections())
    {
        CompilerContext.MessageLog.Error(TEXT("Cavrnus Sync Property node @@ requires a SyncInfo struct."), this);
        bHasError = true;
    }

    // Optional pin
    /*
    if (!SyncOptionsPin || !SyncOptionsPin->HasAnyConnections())
    {
        CompilerContext.MessageLog.Note(TEXT("Cavrnus Sync Property node @@ has no SyncOptions. Defaults will be injected."), this);
    }
    */

    // Derive expected SyncOptions type from the Property pin's linked type
    UScriptStruct* ExpectedOptionsStruct = nullptr;
    if (PropertyPin && PropertyPin->LinkedTo.Num() > 0)
    {
        ExpectedOptionsStruct = GetOptionsStructForType(PropertyPin->LinkedTo[0]->PinType);
    }

    // Spawn intermediate function call node
    const FName FunctionName = GET_FUNCTION_NAME_CHECKED(UCavrnusPropertySyncFunctionLibrary, CavrnusInternalSyncProperty);
    UK2Node_CallFunction* CallNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
    CallNode->FunctionReference.SetExternalMember(FunctionName, UCavrnusPropertySyncFunctionLibrary::StaticClass());
    CallNode->AllocateDefaultPins();

    // If fatal errors, strip this node and bail
    if (bHasError)
    {
        BreakAllNodeLinks();
        return;
    }

    // Target pins
    UEdGraphPin* TargetExecPin = CallNode->GetExecPin();
    UEdGraphPin* TargetOwnerPin = CallNode->FindPin(TEXT("Owner"));
    UEdGraphPin* TargetSyncInfoPin = CallNode->FindPin(TEXT("SyncInfo"));
    UEdGraphPin* TargetPropertyPin = CallNode->FindPin(TEXT("Property"));
    UEdGraphPin* TargetSyncOptionsPin = CallNode->FindPin(TEXT("SyncOptions"));
    UEdGraphPin* TargetThenPin = CallNode->GetThenPin();
    UEdGraphPin* TargetReturnPin = CallNode->GetReturnValuePin();

    // Resolve wildcard types from connections
    auto ResolveFromLink = [&](UEdGraphPin* SourcePin, UEdGraphPin* DestPin)
        {
            if (SourcePin && DestPin && SourcePin->LinkedTo.Num() > 0)
            {
                const UEdGraphPin* Linked = SourcePin->LinkedTo[0];
                SourcePin->PinType = Linked->PinType;
                DestPin->PinType = SourcePin->PinType;
            }
        };

    // Safe move helper
    auto SafeMove = [&](UEdGraphPin* From, UEdGraphPin* To)
        {
            if (From && To)
            {
                CompilerContext.MovePinLinksToIntermediate(*From, *To);
            }
        };

    // Handle Property pin
    ResolveFromLink(PropertyPin, TargetPropertyPin);
    SafeMove(PropertyPin, TargetPropertyPin);

    // Handle SyncOptions pin
    auto HandleSyncOptions = [&](UEdGraphPin* SourcePin, UEdGraphPin* DestPin)
        {
            if (!SourcePin || !DestPin) return;

            if (SourcePin->LinkedTo.Num() > 0)
            {
                ResolveFromLink(SourcePin, DestPin);
                SafeMove(SourcePin, DestPin);
            }
            else if (ExpectedOptionsStruct)
            {
                // Force concrete type
                DestPin->PinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
                DestPin->PinType.PinSubCategoryObject = ExpectedOptionsStruct;

                // Inject default literal
                UK2Node_MakeStruct* MakeStructNode =
                    CompilerContext.SpawnIntermediateNode<UK2Node_MakeStruct>(this, SourceGraph);
                MakeStructNode->StructType = ExpectedOptionsStruct;
                MakeStructNode->AllocateDefaultPins();

                UEdGraphPin* MakeStructOut = nullptr;
                for (UEdGraphPin* Pin : MakeStructNode->Pins)
                {
                    if (Pin && Pin->Direction == EGPD_Output)
                    {
                        MakeStructOut = Pin;
                        break;
                    }
                }

                if (MakeStructOut)
                {
                    MakeStructOut->MakeLinkTo(DestPin);
                }
                else
                {
                    CompilerContext.MessageLog.Error(TEXT("Cavrnus Sync Property node @@ failed to inject default SyncOptions literal."), this);
                }
            }
        };
    HandleSyncOptions(SyncOptionsPin, TargetSyncOptionsPin);

    // Wire remaining pins
    SafeMove(ExecPin, TargetExecPin);
    SafeMove(OwnerPin, TargetOwnerPin);
    SafeMove(SyncInfoPin, TargetSyncInfoPin);
    SafeMove(ThenPin, TargetThenPin);
    SafeMove(PropertySyncPin, TargetReturnPin);

    // Always strip the editor node
    BreakAllNodeLinks();
}



void UCavrnusSyncPropertyNode::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
    UClass* ActionKey = GetClass();
    if (ActionRegistrar.IsOpenForRegistration(ActionKey))
    {
        UBlueprintNodeSpawner* NodeSpawner = UBlueprintNodeSpawner::Create(GetClass());
        ActionRegistrar.AddBlueprintAction(ActionKey, NodeSpawner);
    }
}

FLinearColor UCavrnusSyncPropertyNode::GetNodePinColor(UEdGraphPin* Pin) const
{
    if (Pin && Pin->PinName == TEXT("SyncOptions"))
    {
        if (UEdGraphPin* ValuePin = FindPin(TEXT("Property")))
        {
            const UEdGraphSchema_K2* Schema = GetDefault<UEdGraphSchema_K2>();
            return Schema->GetPinTypeColor(ValuePin->PinType);
        }
    }

    const UEdGraphSchema_K2* Schema = GetDefault<UEdGraphSchema_K2>();
    return Schema->GetPinTypeColor(Pin->PinType);
}

void UCavrnusSyncPropertyNode::NotifyPinConnectionListChanged(UEdGraphPin* ChangedPin)
{
    Super::NotifyPinConnectionListChanged(ChangedPin);

    if (!ChangedPin)
    {
        return;
    }

    // Handle Property pin changes
    if (ChangedPin->PinName == TEXT("Property"))
    {
        if (ChangedPin->LinkedTo.Num() > 0)
        {
            UEdGraphPin* LinkedPin = ChangedPin->LinkedTo[0];
            ChangedPin->PinType = LinkedPin->PinType;
            CachedPropertyPinType = ChangedPin->PinType; // persist

            if (UEdGraphPin* SyncOptionsPin = FindPin(TEXT("SyncOptions")))
            {
                if (UScriptStruct* OptionsStruct = GetOptionsStructForType(LinkedPin->PinType))
                {
                    SyncOptionsPin->PinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
                    SyncOptionsPin->PinType.PinSubCategoryObject = OptionsStruct;
                    CachedSyncOptionsPinType = SyncOptionsPin->PinType; // persist

                    FString TypeLabel = OptionsStruct->GetName().Replace(TEXT("FCavrnusSync"), TEXT(""));
                    SyncOptionsPin->PinFriendlyName = FText::FromString(TypeLabel);
                    SyncOptionsPin->PinToolTip = FString::Printf(TEXT("Options for type: %s"), *TypeLabel);
                    CachedSyncOptionsFriendlyName = SyncOptionsPin->PinFriendlyName; // persist
                }
            }
        }
        else
        {
            // Reset to wildcard if unlinked
            ChangedPin->PinType.PinCategory = UEdGraphSchema_K2::PC_Wildcard;
            ChangedPin->PinType.PinSubCategoryObject = nullptr;
            CachedPropertyPinType = ChangedPin->PinType;

            if (UEdGraphPin* SyncOptionsPin = FindPin(TEXT("SyncOptions")))
            {
                SyncOptionsPin->PinType.PinCategory = UEdGraphSchema_K2::PC_Wildcard;
                SyncOptionsPin->PinType.PinSubCategoryObject = nullptr;
                SyncOptionsPin->PinFriendlyName = FText::FromString(TEXT("SyncOptions"));
                SyncOptionsPin->PinToolTip = TEXT("Options for unresolved type");
                SyncOptionsPin->BreakAllPinLinks();

                CachedSyncOptionsPinType = SyncOptionsPin->PinType;
                CachedSyncOptionsFriendlyName = FText::GetEmpty(); // clear cached label
            }
        }
    }

    // Handle SyncOptions pin changes directly
    if (ChangedPin->PinName == TEXT("SyncOptions"))
    {
        CachedSyncOptionsPinType = ChangedPin->PinType;
        CachedSyncOptionsFriendlyName = ChangedPin->PinFriendlyName;
    }

    // Notify graph of structural change
    if (UEdGraph* Graph = GetGraph())
    {
        Graph->NotifyGraphChanged();
        if (UBlueprint* BP = FBlueprintEditorUtils::FindBlueprintForGraph(Graph))
        {
            FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(BP);
        }
    }
}



void UCavrnusSyncPropertyNode::PostReconstructNode()
{
    Super::PostReconstructNode();

    UEdGraphPin* PropertyPin = FindPin(TEXT("Property"));
    UEdGraphPin* SyncOptionsPin = FindPin(TEXT("SyncOptions"));

    if (PropertyPin)
    {
        if (CachedPropertyPinType.PinCategory != NAME_None &&
            CachedPropertyPinType.PinCategory != UEdGraphSchema_K2::PC_Wildcard)
        {
            PropertyPin->PinType = CachedPropertyPinType;
        }
        else if (PropertyPin->LinkedTo.Num() > 0)
        {
            PropertyPin->PinType = PropertyPin->LinkedTo[0]->PinType;
            CachedPropertyPinType = PropertyPin->PinType;
        }
    }

    if (SyncOptionsPin)
    {
        if (CachedSyncOptionsPinType.PinCategory != NAME_None &&
            CachedSyncOptionsPinType.PinCategory != UEdGraphSchema_K2::PC_Wildcard)
        {
            SyncOptionsPin->PinType = CachedSyncOptionsPinType;
            if (!CachedSyncOptionsFriendlyName.IsEmpty())
            {
                SyncOptionsPin->PinFriendlyName = CachedSyncOptionsFriendlyName;
            }
        }
        else if (PropertyPin)
        {
            if (UScriptStruct* Expected = GetOptionsStructForType(PropertyPin->PinType))
            {
                SyncOptionsPin->PinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
                SyncOptionsPin->PinType.PinSubCategoryObject = Expected;
                CachedSyncOptionsPinType = SyncOptionsPin->PinType;

                FString TypeLabel = Expected->GetName().Replace(TEXT("FCavrnusSync"), TEXT(""));
                SyncOptionsPin->PinFriendlyName = FText::FromString(TypeLabel);
                CachedSyncOptionsFriendlyName = SyncOptionsPin->PinFriendlyName;
            }
            else
            {
                // Explicitly reset cache if no valid struct found
                CachedSyncOptionsPinType.PinCategory = UEdGraphSchema_K2::PC_Wildcard;
                CachedSyncOptionsPinType.PinSubCategoryObject = nullptr;
                CachedSyncOptionsFriendlyName = FText::GetEmpty();
            }
        }
    }
}



UScriptStruct* UCavrnusSyncPropertyNode::GetOptionsStructForType(const FEdGraphPinType& PinType) const
{
    if (PinType.PinCategory == UEdGraphSchema_K2::PC_Real ||
        PinType.PinCategory == UEdGraphSchema_K2::PC_Int ||
        PinType.PinCategory == UEdGraphSchema_K2::PC_Boolean ||
        PinType.PinCategory == UEdGraphSchema_K2::PC_String)
    {
        return FCavrnusSyncOptions::StaticStruct();
    }
    if (PinType.PinCategory == UEdGraphSchema_K2::PC_Struct)
    {
        if (PinType.PinSubCategoryObject == TBaseStructure<FVector>::Get())
            return FCavrnusSyncVectorOptions::StaticStruct();
        if (PinType.PinSubCategoryObject == TBaseStructure<FLinearColor>::Get())
            return FCavrnusSyncColorOptions::StaticStruct();
        if (PinType.PinSubCategoryObject == TBaseStructure<FTransform>::Get())
            return FCavrnusSyncTransformOptions::StaticStruct();
        return FCavrnusSyncStructOptions::StaticStruct();
    }
    return nullptr;
}

void UCavrnusSyncPropertyNode::PostLoad()
{
    Super::PostLoad();

    if (UEdGraphPin* PropertyPin = FindPin(TEXT("Property")))
    {
        if (CachedPropertyPinType.PinCategory != NAME_None &&
            CachedPropertyPinType.PinCategory != UEdGraphSchema_K2::PC_Wildcard)
        {
            PropertyPin->PinType = CachedPropertyPinType;
        }
    }

    if (UEdGraphPin* SyncOptionsPin = FindPin(TEXT("SyncOptions")))
    {
        if (CachedSyncOptionsPinType.PinCategory != NAME_None &&
            CachedSyncOptionsPinType.PinCategory != UEdGraphSchema_K2::PC_Wildcard)
        {
            SyncOptionsPin->PinType = CachedSyncOptionsPinType;
            if (!CachedSyncOptionsFriendlyName.IsEmpty())
            {
                SyncOptionsPin->PinFriendlyName = CachedSyncOptionsFriendlyName;
            }
        }
    }
}
