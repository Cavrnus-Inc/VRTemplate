// UK2Node_ResolvePropertyMetadata.h
#pragma once

#include "K2Node.h"
#include "K2Node_ResolvePropertyMetadata.generated.h"

UCLASS()
class CAVRNUSBLUEPRINTMODULE_API UK2Node_ResolvePropertyMetadata : public UK2Node
{
    GENERATED_BODY()

public:
    virtual void AllocateDefaultPins() override;
    virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
    virtual void ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;
    virtual bool IsNodePure() const override { return true; }

    void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const;

};
