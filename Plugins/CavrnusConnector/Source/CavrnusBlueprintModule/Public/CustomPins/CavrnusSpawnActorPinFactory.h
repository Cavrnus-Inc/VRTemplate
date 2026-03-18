// Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"

#if WITH_EDITOR
#include "EdGraphUtilities.h"
#include "CustomPins/CavrnusSpawnActorClassPin.h"
#include "CustomPins/CavrnusSpawnActorIdPin.h"
#include "BlueprintNodes/K2Node_CavrnusSpawnActorFromClass.h"
#include "BlueprintNodes/K2Node_CavrnusConstructObjectFromClass.h"
#include "BlueprintNodes/K2Node_CavrnusSpawnActorById.h"

/**
 * @brief Node factory that replaces pins on Cavrnus-specific nodes only.
 * Only intercepts pins whose owning node is a Cavrnus K2Node — never
 * interferes with standard UE nodes like "Get Actor of Class".
 */
class FCavrnusSpawnActorPinFactory : public FGraphPanelPinFactory
{
public:
    virtual TSharedPtr<SGraphPin> CreatePin(UEdGraphPin* Pin) const override
    {
        if (!Pin)
        {
            return nullptr;
        }

        UEdGraphNode* Node = Pin->GetOwningNode();
        if (!Node)
        {
            return nullptr;
        }

        // ActorClass pin — only on Cavrnus spawn nodes
        if (Pin->PinName == TEXT("ActorClass") && Cast<UK2Node_CavrnusSpawnActorFromClass>(Node))
        {
            return SNew(SCavrnusSpawnActorClassPin, Pin);
        }

        // ObjectClass pin — only on Cavrnus construct nodes
        if (Pin->PinName == TEXT("ObjectClass") && Cast<UK2Node_CavrnusConstructObjectFromClass>(Node))
        {
            return SNew(SCavrnusSpawnActorClassPin, Pin);
        }

        // WellKnownObjectId pin — only on Cavrnus spawn-by-ID nodes
        if (Pin->PinName == TEXT("WellKnownObjectId") && Cast<UK2Node_CavrnusSpawnActorById>(Node))
        {
            return SNew(SCavrnusSpawnActorIdPin, Pin);
        }

        return nullptr;
    }
};
#endif
