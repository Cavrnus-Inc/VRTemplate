// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "CavrnusEditorToolsManager.generated.h"

/**
 * 
 */
UCLASS()
class CAVRNUSCONNECTOREDITOR_API UCavrnusEditorToolsManager : public UObject
{
	GENERATED_BODY()
public:
	void Initialize();
	void Teardown();
};
