// Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "SGraphPin.h"
#include "ClassViewerFilter.h"
#include "ClassViewerModule.h"
#include "Kismet2/BlueprintEditorUtils.h"

class SCavrnusSpawnActorClassPin : public SGraphPin
{
public:
    SLATE_BEGIN_ARGS(SCavrnusSpawnActorClassPin) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs, UEdGraphPin* InPin);
    
protected:
    virtual TSharedRef<SWidget> GetDefaultValueWidget() override;

private:
    TArray<UClass*> CachedValidClasses;

    UClass* GetSelectedClass() const;

    void OnClassPicked(UClass* NewClass);

    TArray<UClass*> GetValidClassesFromPin(UEdGraphPin* Pin) const;
};
