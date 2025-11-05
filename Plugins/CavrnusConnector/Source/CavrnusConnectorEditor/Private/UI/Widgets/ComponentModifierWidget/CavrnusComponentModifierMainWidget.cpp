// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Widgets/ComponentModifierWidget/CavrnusComponentModifierMainWidget.h"
#include "CavrnusConnectorEditorModule.h"

void UCavrnusComponentModifierMainWidget::Setup(UCavrnusComponentModifierViewModel* InVm)
{
	Vm = InVm;
	
	if (SelectedActorNameTextBlock)
		SelectedActorNameTextBlock->SetText(FText());
	
	Vm->OnSelectedLevelActorChanged.Remove(SelectionChangedHandle);
	SelectionChangedHandle.Reset();

	if (SelectedActorNameTextBlock)
		SelectedActorNameTextBlock->SetText(FText::FromString("Select an actor in level"));
	
	SelectionChangedHandle = Vm->OnSelectedLevelActorChanged.AddWeakLambda(this, [this](const FActorStruct& ActorStruct)
	{
		if (SelectedActorNameTextBlock)
		{
			if (ActorStruct.Name.IsEmpty())
			{
				SelectedActorNameTextBlock->SetText(FText::FromString("Select an actor in level"));
			}
			else
			{
				SelectedActorNameTextBlock->SetText(FText::FromString(ActorStruct.Name));
			}
		}
		
		PopulateCavrnusComponentList(ActorStruct);
	});
}

void UCavrnusComponentModifierMainWidget::PopulateCavrnusComponentList(const FActorStruct& TargetLevelActor)
{
	if (ListHandler.IsValid())
		ListHandler->Teardown();

	ListHandler = nullptr;

	ListHandler = TCavrnusUIListHandler<FCavrnusComponentInfo>::Initialize(
		CavrnusComponentListContainerWidget,
		CavrnusComponentListItemBlueprint,
		[](const FCavrnusComponentInfo& Data)
		{
			return Data.ClassName;
		}, [](const FCavrnusComponentInfo& A, const FCavrnusComponentInfo& B)
		{
			return A.ClassName < B.ClassName;
		});
	
	ListHandler->OnWidgetCreated<UCavrnusComponentListItemWidget>([this, TargetLevelActor](UCavrnusComponentListItemWidget* Widget, const FCavrnusComponentInfo& Data)
	{
		Widget->Setup(TargetLevelActor, Data, [this, TargetLevelActor, Data](const bool bState)
		{
			Vm->UpdateCavrnusComponentOnActor(TargetLevelActor, Data, bState);
		});
	});

	ListHandler->OnItemSelected([](const FCavrnusComponentInfo& SelectedData)
	{
		UE_LOG(LogTemp, Error, TEXT("SelectedData: %s"), *SelectedData.ClassName);
		
	});
	
	ListHandler->AddItems(TargetLevelActor.Components);
}