// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "Modes/CavrnusModeBase.h"

#include "CavrnusConnectorModule.h"
#include "Core/Subsystems/CavrnusSubsystem.h"
#include "EnhancedInputSubsystems.h"
#include "AssetManager/CavrnusDataAssetManager.h"
#include "AssetManager/DataAssets/CavrnusInputActionsDataAsset.h"
#include "Core/Contexts/CavrnusRuntimeContext.h"
#include "Engine/LocalPlayer.h"
#include "Runtime/Launch/Resources/Version.h"
#include "GameFramework/PlayerController.h"

void UCavrnusModeBase::HandleLocalPlayerAdded(UWorld* World, ULocalPlayer* LocalPlayer, int32 Priority)
{
	PlayerController = World->GetFirstPlayerController();
	
	if (PlayerController.IsValid())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer))
		{
			InputSubsystem = Subsystem;

			UCavrnusSubsystem* SubsystemInstance = UCavrnusSubsystem::Get();
			if (!SubsystemInstance || !SubsystemInstance->IsRuntimeContextReady())
			{
				UE_LOG(LogCavrnusConnector, Warning, TEXT("UCavrnusModeBase::HandleLocalPlayerAdded - RuntimeContext not ready yet"));
				return;
			}

			UCavrnusDataAssetManager* DataAssetManager = SubsystemInstance->RuntimeContext->Get<UCavrnusDataAssetManager>();
			if (!DataAssetManager)
			{
				UE_LOG(LogCavrnusConnector, Warning, TEXT("UCavrnusModeBase::HandleLocalPlayerAdded - DataAssetManager not available"));
				return;
			}

			CavInputAsset = DataAssetManager->GetAsset<UCavrnusInputActionsDataAsset>();

			if (!CavInputAsset)
			{
				UE_LOG(LogCavrnusConnector, Warning, TEXT("UCavrnusModeBase::HandleLocalPlayerAdded No CavInputAsset found"));
				return;
			}

			const FString ContextName = GetInputMappingContextName();
			if (ContextName.IsEmpty())
			{
				// No input context requested — mode does not need input bindings
				return;
			}

			if (auto* Ctx = CavInputAsset->GetContext(ContextName))
			{
				InputMapCtx = Ctx;
				Subsystem->AddMappingContext(Ctx, Priority);

				if (auto* Ic = Cast<UEnhancedInputComponent>(PlayerController->InputComponent))
				{
					UE_LOG(LogCavrnusConnector, Verbose, TEXT("Binding input actions"));
					BindInputActions(Ic, CavInputAsset);
				}
				else
				{
					UE_LOG(LogCavrnusConnector, Error, TEXT("UCavrnusModeBase::HandleLocalPlayerAdded failed to bind input actions"));
				}
			}
			else
			{
				UE_LOG(LogCavrnusConnector, Error, TEXT("UCavrnusModeBase::HandleLocalPlayerAdded No Context found for '%s'"), *ContextName);
			}
		}
		else
		{
			UE_LOG(LogCavrnusConnector, Error, TEXT("UCavrnusModeBase::HandleLocalPlayerAdded No subsystem found"));
		}
	}
	else
	{
		UE_LOG(LogCavrnusConnector, Error,
			TEXT("UCavrnusModeBase::EnterMode - PlayerController is not valid after OnLocalPlayerAddedEvent"))
	}
}

void UCavrnusModeBase::EnterMode(UWorld* World, const int32 Priority)
{
	if (IsValid(World))
	{
		PlayerController = World->GetFirstPlayerController();
		if (PlayerController.IsValid())
		{
			HandleLocalPlayerAdded(World, PlayerController->GetLocalPlayer(), Priority);
		}
		else
		{
			UE_LOG(LogCavrnusConnector, Error, TEXT("CavrnusModeBase::EnterMode - PlayerController is not valid"));
		}
	}
	else
	{
		UE_LOG(LogCavrnusConnector, Error, TEXT("UCavrnusModeBase::EnterMode - World is not valid"));
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
