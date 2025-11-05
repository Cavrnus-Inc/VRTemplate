#pragma once

#include "CoreMinimal.h"
#include "K2Node.h"
#include "PropertySyncers/CavrnusPropertySyncStructs.h"
#include "CavrnusSyncPropertyNode.generated.h"

UCLASS(meta = (DisplayName = "Cavrnus Sync Property"))
class CAVRNUSBLUEPRINTMODULE_API UCavrnusSyncPropertyNode : public UK2Node
{
    GENERATED_BODY()

private:
    // Functions whose signatures differ slightly in UE5.0 vs 5.1+
#if (ENGINE_MAJOR_VERSION == 5) && (ENGINE_MINOR_VERSION == 0)
    virtual void ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;
    virtual void NotifyPinConnectionListChanged(UEdGraphPin* Pin) override;
    virtual void PostReconstructNode() override;
    virtual void PostLoad() override;
    virtual void ValidateNodeDuringCompilation(FCompilerResultsLog& MessageLog) const;
#else
    virtual void ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;
    virtual void NotifyPinConnectionListChanged(UEdGraphPin* Pin) override;
    virtual void PostReconstructNode() override;
    virtual void PostLoad() override;
    virtual void ValidateNodeDuringCompilation(FCompilerResultsLog& MessageLog) const override;
#endif

    // In 5.0 and 5.1, GetNodePinColor is not virtual, so just declare a new function
    FLinearColor GetNodePinColor(UEdGraphPin* Pin) const;

    // These are consistent across 5.0-5.6, so always safe to mark override
    virtual void AllocateDefaultPins() override;
    virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
    virtual bool IsNodePure() const override { return false; }
    virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
    virtual FText GetMenuCategory() const override { return FText::FromString(TEXT("Cavrnus|Sync")); }

    // Helper
    UScriptStruct* GetOptionsStructForType(const FEdGraphPinType& PinType) const;

    // Persist resolved pin types across save/load
    UPROPERTY()
    FEdGraphPinType CachedPropertyPinType;

    UPROPERTY()
    FEdGraphPinType CachedSyncOptionsPinType;

    // Optional: track pin-friendly name chosen for options label
    UPROPERTY()
    FText CachedSyncOptionsFriendlyName;
};
