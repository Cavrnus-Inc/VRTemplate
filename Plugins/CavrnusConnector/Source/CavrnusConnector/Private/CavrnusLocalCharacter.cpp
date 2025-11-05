// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "CavrnusLocalCharacter.h"
#include <GameFramework/InputSettings.h>

ACavrnusLocalCharacter::ACavrnusLocalCharacter()
{
	// Only want to add mappings if this is a real instance
	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		SetupInputs();
	}
}

void ACavrnusLocalCharacter::SetupInputs()
{
	UInputSettings* Settings = UInputSettings::GetInputSettings();
	if (Settings)
	{
		// Check to see if we've already added these
		TArray< FInputAxisKeyMapping> ForwardEntries;
		Settings->GetAxisMappingByName(FName("WalkForward"), ForwardEntries);
		if (ForwardEntries.IsEmpty())
		{
			/*
			Settings->AddAxisMapping(FInputAxisKeyMapping(FName("WalkForward"), EKeys::W, 1.0f));
			Settings->AddAxisMapping(FInputAxisKeyMapping(FName("WalkForward"), EKeys::S, -1.0f));
			Settings->AddAxisMapping(FInputAxisKeyMapping(FName("WalkForward"), EKeys::Up, 1.0f));
			Settings->AddAxisMapping(FInputAxisKeyMapping(FName("WalkForward"), EKeys::Down, -1.0f));
			Settings->AddAxisMapping(FInputAxisKeyMapping(FName("WalkForward"), EKeys::Gamepad_LeftY, 1.0f));

			Settings->AddAxisMapping(FInputAxisKeyMapping(FName("WalkRight"), EKeys::A, -1.0f));
			Settings->AddAxisMapping(FInputAxisKeyMapping(FName("WalkRight"), EKeys::D, 1.0f));
			Settings->AddAxisMapping(FInputAxisKeyMapping(FName("WalkRight"), EKeys::Right, 1.0f));
			Settings->AddAxisMapping(FInputAxisKeyMapping(FName("WalkRight"), EKeys::Left, -1.0f));
			Settings->AddAxisMapping(FInputAxisKeyMapping(FName("WalkRight"), EKeys::Gamepad_LeftX, 1.0f));

			Settings->AddAxisMapping(FInputAxisKeyMapping(FName("LookRight"), EKeys::MouseX, 1.0f));
			Settings->AddAxisMapping(FInputAxisKeyMapping(FName("LookRight"), EKeys::Gamepad_RightX, 1.0f));

			Settings->AddAxisMapping(FInputAxisKeyMapping(FName("LookUp"), EKeys::MouseY, 1.0f));
			Settings->AddAxisMapping(FInputAxisKeyMapping(FName("LookUp"), EKeys::Gamepad_RightY, 1.0f));
			
			Settings->AddActionMapping(FInputActionKeyMapping("RotateView", EKeys::RightMouseButton));
			*/
		}
	}
}
