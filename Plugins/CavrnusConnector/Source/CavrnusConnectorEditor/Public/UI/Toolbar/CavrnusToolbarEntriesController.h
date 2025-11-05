// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "CavrnusToolbarEntriesController.generated.h"

class UCavrnusEditorUIManager;

UCLASS()
class CAVRNUSCONNECTOREDITOR_API UCavrnusToolbarEntriesController : public UObject
{
	GENERATED_BODY()
public:
	void Initialize();
	void Teardown();
	
	void AddParentToolbarEntry(FMenuBarBuilder& MenuBuilder);
	void AddCavrnusSubEntries(FMenuBuilder& MenuBuilder);
};

