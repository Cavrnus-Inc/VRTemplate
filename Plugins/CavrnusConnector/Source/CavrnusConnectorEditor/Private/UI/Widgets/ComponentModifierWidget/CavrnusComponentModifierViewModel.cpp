// // Copyright (c) 2025 Cavrnus. All rights reserved.
#include "UI/Widgets/ComponentModifierWidget/CavrnusComponentModifierViewModel.h"

#include "Selection.h"
#include "Editor.h"
#include "CavrnusConnectorEditorModule.h"
#include "LevelEditor.h"
#include "Modules/ModuleManager.h"

void UCavrnusComponentModifierViewModel::Setup()
{
	if (FModuleManager::Get().IsModuleLoaded("LevelEditor"))
	{
		FLevelEditorModule& LevelEditor = FModuleManager::GetModuleChecked<FLevelEditorModule>("LevelEditor");
		LevelEditor.OnActorSelectionChanged().AddUObject(this, &UCavrnusComponentModifierViewModel::HandleObjectSelected);

	}
}

void UCavrnusComponentModifierViewModel::UpdateCavrnusComponentOnActor(const FActorStruct& TargetActor, const FCavrnusComponentInfo& CavrnusComponent, const bool bTryAdd)
{
	if (TargetActor.Actor != nullptr)
	{
		if (bTryAdd)
		{
			UActorComponent* Ac = nullptr;
			CavrnusComponentModifiers::TryAddComponent(TargetActor.Actor, CavrnusComponent.Class, Ac);
		}
		 else
		 {
			CavrnusComponentModifiers::TryRemoveComponent(TargetActor.Actor, CavrnusComponent.Class);
		 }
	} else
		UE_LOG(LogCavrnusConnectorEditor, Error, TEXT("[UCavrnusComponentModifierViewModel::HandleCavrnusComponentModified] TargetActor is null!!"))
}

bool UCavrnusComponentModifierViewModel::IsValidSelection(UObject* Selected) const
{
	if (!Selected)
		return false;
	if(!GEditor)
		return false;

	USelection* Selection = GEditor->GetSelectedActors();
	if (!Selection) 
		return false;

	if (Selection->Num() == 1)
	{
		if (AActor* SelectedActor = Cast<AActor>(Selection->GetSelectedObject(0)))
		{
			
			UE_LOG(LogTemp, Warning, TEXT("VALID"));
			return true;
		}
	}
	UE_LOG(LogTemp, Error, TEXT("INVALID"));
	return false;
}

void UCavrnusComponentModifierViewModel::HandleObjectSelected(const TArray<UObject*>& NewSelection, bool bForceRefresh)
{
	AActor* SelectedActor = nullptr;

	if (NewSelection.IsEmpty())
		return;
	
	if (NewSelection.Num() != 1)
	{
		NotifySelectedLevelActorsChanged(FActorStruct()); // Using empty
		return;
	}
	else
	{
		if (const UActorComponent* Component = Cast<UActorComponent>(NewSelection[0]))
			SelectedActor = Component->GetOwner();
		else
			SelectedActor = Cast<AActor>(NewSelection[0]);

		if (SelectedActor && IsValidSelection(NewSelection[0]))
		{
			FActorStruct ActorStruct;
			ActorStruct.Actor = SelectedActor;
			CavrnusComponentModifiers::GetAllCavrnusComponents(ActorStruct.Components);
			ActorStruct.Name = SelectedActor->GetName();

			SelectedLevelActor = ActorStruct;
			NotifySelectedLevelActorsChanged(ActorStruct);
		}
		else
		{
			NotifySelectedLevelActorsChanged(FActorStruct()); // Using empty
		}
	}
}

void UCavrnusComponentModifierViewModel::NotifySelectedLevelActorsChanged(const FActorStruct& Selected)
{
	if (OnSelectedLevelActorChanged.IsBound())
		OnSelectedLevelActorChanged.Broadcast(Selected);
}
