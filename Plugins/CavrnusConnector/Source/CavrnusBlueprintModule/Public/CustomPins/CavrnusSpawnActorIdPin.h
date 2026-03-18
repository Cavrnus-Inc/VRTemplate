// Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "SGraphPin.h"

class SCavrnusSpawnActorIdPin : public SGraphPin
{
public:
    SLATE_BEGIN_ARGS(SCavrnusSpawnActorIdPin) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs, UEdGraphPin* InPin);
    
protected:
    virtual TSharedRef<SWidget> GetDefaultValueWidget() override;

private:
    TArray<FName> CachedValidIds;

    FName GetSelectedId() const;

    void OnIdPicked(FName NewId);

    TArray<FName> GetValidIdsFromNode() const;
};

