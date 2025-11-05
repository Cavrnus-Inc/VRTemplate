// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "Modes/CavrnusSceneCaptureMode.h"

#include "CavrnusSubsystem.h"
#include "Modes/CavrnusModeManager.h"
#include "UI/CavrnusUI.h"
#include "UI/Systems/Screens/Types/ScreenCapture/CavrnusScreenCaptureModeWidget.h"

void UCavrnusSceneCaptureMode::BindInputActions(
	UEnhancedInputComponent* InputComponent,
	UCavrnusInputActionsDataAsset* Data)
{
	Super::BindInputActions(InputComponent, Data);
	

	if (const UInputAction* Action = GetInputAction("primaryClick"))
		InputBindings.Add(&InputComponent->BindAction(Action, ETriggerEvent::Triggered, this, &UCavrnusSceneCaptureMode::PrimaryClick));
	
	if (const UInputAction* Action = GetInputAction("back"))
		InputBindings.Add(&InputComponent->BindAction(Action, ETriggerEvent::Triggered, this, &UCavrnusSceneCaptureMode::Back));
}

void UCavrnusSceneCaptureMode::EnterMode(UWorld* World, const int32 Priority)
{
	Super::EnterMode(World, Priority);

	// Show ScreenCapture panel
	// this will hide all other popups and UI
	
	Screen = UCavrnusUI::Get()->Screens()->ShowScreen<UCavrnusScreenCaptureModeWidget>();
}

void UCavrnusSceneCaptureMode::ExitMode()
{
	Super::ExitMode();

	if (Screen)
		UCavrnusUI::Get()->Screens()->Close(Screen);
}

void UCavrnusSceneCaptureMode::PrimaryClick(const FInputActionValue& Value)
{
	if (const bool Input = Value.Get<bool>())
	{
		// Take Picture!
		UCavrnusSubsystem::Get()->Services->Get<UCavrnusModeManager>()->PopTransientMode();
	}
}

void UCavrnusSceneCaptureMode::Back(const FInputActionValue& Value)
{
	if (const bool Input = Value.Get<bool>())
	{
		UCavrnusSubsystem::Get()->Services->Get<UCavrnusModeManager>()->PopTransientMode();
	}
}
