// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/Widgets/CavrnusMainEditorPanelWidget.h"
#include "UObject/Object.h"
#include "CavrnusEditorTabController.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnMainEditorWidgetSpawned, UCavrnusMainEditorPanelWidget*);

UCLASS()
class CAVRNUSCONNECTOREDITOR_API UCavrnusEditorTabController : public UObject
{
	GENERATED_BODY()
public:
	static FOnMainEditorWidgetSpawned OnMainEditorWidgetSpawned;
	
	void Initialize();
	void Teardown();
	
	static void ShowCavrnusTab();

private:
	static FName TabId;
	
	TSharedRef<SDockTab> HandleSpawnTab(const FSpawnTabArgs& TabSpawnArgs);
};
