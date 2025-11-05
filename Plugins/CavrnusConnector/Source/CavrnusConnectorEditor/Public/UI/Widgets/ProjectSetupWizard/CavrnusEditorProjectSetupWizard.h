// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/Menus/SetupWizard/CavrnusSetupWizardWidgetBase.h"
#include "CavrnusEditorProjectSetupWizard.generated.h"

UCLASS(Abstract)
class CAVRNUSCONNECTOREDITOR_API UCavrnusEditorProjectSetupWizard : public UCavrnusSetupWizardWidgetBase
{
	GENERATED_BODY()

private:
	static FString GameModePath;
	
	UPROPERTY()
	TObjectPtr<UCavrnusSetupWizardButton> SetupPawnButton = nullptr;
};
