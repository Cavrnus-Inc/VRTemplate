// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "Modes/CavrnusModeBase.h"

#include "CavrnusSubsystem.h"
#include "EnhancedInputSubsystems.h"
#include "AssetManager/CavrnusDataAssetManager.h"
#include "AssetManager/DataAssets/CavrnusInputActionsDataAsset.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"

void UCavrnusModeBase::EnterMode(UWorld* World, const int32 Priority)
{
	if (IsValid(World))
	{
		PlayerController = World->GetFirstPlayerController();
		if (const ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer))
			{
				InputSubsystem = Subsystem;
				
				CavInputAsset = UCavrnusSubsystem::Get()->Services->Get<UCavrnusDataAssetManager>()->GetAsset<UCavrnusInputActionsDataAsset>();
				if (CavInputAsset == nullptr)
					return;
				
				if (auto* Ctx = CavInputAsset->GetContext(GetInputMappingContextName()))
				{
					InputMapCtx = Ctx;
					Subsystem->AddMappingContext(Ctx, Priority);
					
					if (auto* Ic = Cast<UEnhancedInputComponent>(PlayerController->InputComponent))
					{
						BindInputActions(Ic, CavInputAsset);
					}
				}
			}
		}
	}
}

void UCavrnusModeBase::ExitMode()
{
	if (IsValid(InputSubsystem) && IsValid(InputMapCtx))
		InputSubsystem->RemoveMappingContext(InputMapCtx);

	if (!InputBindings.IsEmpty())
	{
		if (PlayerController != nullptr)
		{
			if (auto* Ic = Cast<UEnhancedInputComponent>(PlayerController->InputComponent))
			{
				for (FEnhancedInputActionEventBinding* InputBinding : InputBindings)
				{
					if (InputBinding)
						Ic->RemoveActionEventBinding(InputBinding->GetHandle());
				}
			}
		}
	}

	InputBindings.Empty();
	InputMapCtx = nullptr;
	InputSubsystem = nullptr;
	PlayerController = nullptr;
}

FString UCavrnusModeBase::GetInputMappingContextName()
{
	return FString(); // Just provide the name of the ctx in the input data asset!
}

void UCavrnusModeBase::BindInputActions(UEnhancedInputComponent* InputComponent, UCavrnusInputActionsDataAsset* Data)
{
	// Ready to bind here!
}

UInputAction* UCavrnusModeBase::GetInputAction(const FString& InputActionName) const
{
	if (CavInputAsset)
		return CavInputAsset->GetAction(InputActionName);

	return nullptr;
}
