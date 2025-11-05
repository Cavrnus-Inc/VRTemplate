// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Tools/CavrnizingHelpers/CavrnusComponentModifiers.h"
#include "UObject/Object.h"
#include "CavrnusComponentModifierViewModel.generated.h"

/**
 * Handles logic associated w/ attaching cavrnus components to current level SceneComponents
 */
UCLASS()
class CAVRNUSCONNECTOREDITOR_API UCavrnusComponentModifierViewModel : public UObject
{
	GENERATED_BODY()
public:
	TMulticastDelegate<void(const FActorStruct& NewSelectedLevelActor)> OnSelectedLevelActorChanged;
	
	void Setup();
	void UpdateCavrnusComponentOnActor(const FActorStruct& TargetActor, const FCavrnusComponentInfo& CavrnusComponent, const bool bTryAdd);
	
private:
	FActorStruct SelectedLevelActor = FActorStruct();

	void HandleObjectSelected(const TArray<UObject*>& NewSelection, bool bForceRefresh);
	void NotifySelectedLevelActorsChanged(const FActorStruct& Selected);

	bool IsValidSelection(UObject* Selected) const;
};
