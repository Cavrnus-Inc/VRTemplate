// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
#include "CavrnusBaseEditorUtilityWidget.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class CAVRNUSCONNECTOREDITOR_API UCavrnusBaseEditorUtilityWidget : public UEditorUtilityWidget
{
	GENERATED_BODY()
public:
	FName GetTabID() const { return TabID; }
	void SetTabID(const FName& Id) { TabID = Id; }
protected:
	FName TabID = FName();
};
