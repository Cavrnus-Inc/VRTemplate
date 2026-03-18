// Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CavrnusContextBase.h"
#include "Managers/CavrnusEditorAuthenticationManager.h"
#include "ServiceLocator/CavrnusServiceLocator.h"
#include "AssetManager/CavrnusDataAssetManager.h"
#include "UObject/Object.h"
#include "CavrnusEditorContext.generated.h"

UCLASS()
class CAVRNUSCONNECTOR_API UCavrnusEditorContext : public UCavrnusContextBase
{
	GENERATED_BODY()
public:
	virtual void Initialize(UWorld* World = nullptr)
	{
		Services = NewObject<UCavrnusServiceLocator>(this);
		Services->RegisterService<UCavrnusEditorAuthenticationManager>();
		Services->RegisterService<UCavrnusDataAssetManager>();
	}
};
