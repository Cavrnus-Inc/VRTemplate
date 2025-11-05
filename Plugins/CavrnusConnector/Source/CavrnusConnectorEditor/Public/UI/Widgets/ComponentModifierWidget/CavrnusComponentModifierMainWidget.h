// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CavrnusComponentListItemWidget.h"
#include "CavrnusComponentModifierViewModel.h"
#include "UI/Components/Lists/CavrnusListContainerWidget.h"
#include "UI/Components/Lists/CavrnusUIListHandler.h"
#include "UI/Widgets/CavrnusBaseEditorUtilityWidget.h"
#include "CavrnusComponentModifierMainWidget.generated.h"

UCLASS(Abstract)
class CAVRNUSCONNECTOREDITOR_API UCavrnusComponentModifierMainWidget : public UCavrnusBaseEditorUtilityWidget
{
	GENERATED_BODY()
public:
	void Setup(UCavrnusComponentModifierViewModel* InVm);
	
protected:
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCavrnusUITextBlock> SelectedActorNameTextBlock = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UCavrnusListContainerWidget> CavrnusComponentListContainerWidget = nullptr;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Cavrnus|Editor")
	TSubclassOf<UCavrnusComponentListItemWidget> CavrnusComponentListItemBlueprint;

private:
	FDelegateHandle SelectionChangedHandle = FDelegateHandle();
	
	UPROPERTY()
	TWeakObjectPtr<UCavrnusComponentModifierViewModel> Vm;
	
	TUniquePtr<TCavrnusUIListHandler<FCavrnusComponentInfo>> ListHandler;

	void PopulateCavrnusComponentList(const FActorStruct& TargetLevelActor);
};
