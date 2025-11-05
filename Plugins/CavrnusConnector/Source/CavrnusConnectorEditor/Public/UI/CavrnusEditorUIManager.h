// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Tabs/CavrnusEditorTabController.h"
#include "Toolbar/CavrnusToolbarEntriesController.h"
#include "UObject/Object.h"
#include "CavrnusEditorUIManager.generated.h"

UCLASS()
class CAVRNUSCONNECTOREDITOR_API UCavrnusEditorUIManager : public UObject
{
	GENERATED_BODY()
public:
	void Initialize();
	void Teardown();

	UCavrnusEditorTabController* GetTabController() const { return TabController;}
	UCavrnusToolbarEntriesController* Toolbar() const { return ToolbarController; };

private:
	FDelegateHandle MainFrameHandle = FDelegateHandle();

	UPROPERTY()
	TObjectPtr<UCavrnusEditorTabController> TabController = nullptr;
	
	UPROPERTY()
	TObjectPtr<UCavrnusToolbarEntriesController> ToolbarController = nullptr;
};
