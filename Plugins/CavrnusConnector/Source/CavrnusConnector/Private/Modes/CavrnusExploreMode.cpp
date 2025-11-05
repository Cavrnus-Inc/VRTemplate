// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "Modes/CavrnusExploreMode.h"
#include "AssetManager/DataAssets/CavrnusInputActionsDataAsset.h"
#include "Core/Tick/CavrnusTickableObject.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"

void UCavrnusExploreMode::BindInputActions(UEnhancedInputComponent* InputComponent, UCavrnusInputActionsDataAsset* Data)
{
	Super::BindInputActions(InputComponent, Data);

	const TFunction<void(const float DeltaTick)> LookFunc = [this](const float DeltaTick)
	{
		// Gradually shrink every frame...
		TargetLookDelta *= FMath::Clamp(1.f - DeltaTick * 10.f, 0.f, 1.f);

		const auto SmoothDelta = FMath::Vector2DInterpTo(
			CurrentLookDelta,
			TargetLookDelta,
			DeltaTick,
			40.0f);
		
		if (PlayerController.IsValid())
		{
			PlayerController->AddYawInput(SmoothDelta.X);
			PlayerController->AddPitchInput(SmoothDelta.Y);
		}
	};

	TickObj = UCavrnusTickableObject::Create(this, LookFunc);
	
	if (const UInputAction* Action = GetInputAction("move"))
		InputBindings.Add(&InputComponent->BindAction(Action, ETriggerEvent::Triggered, this, &UCavrnusExploreMode::Move));
	if (const UInputAction* Action = GetInputAction("look"))
		InputBindings.Add(&InputComponent->BindAction(Action, ETriggerEvent::Triggered, this, &UCavrnusExploreMode::Look));
	if (const UInputAction* Action = GetInputAction("fly"))
		InputBindings.Add(&InputComponent->BindAction(Action, ETriggerEvent::Triggered, this, &UCavrnusExploreMode::Fly));
}

void UCavrnusExploreMode::Move(const FInputActionValue& Value)
{
	const FVector2D Input = Value.Get<FVector2D>();
	if (GetPawn())
	{
		GetPawn()->AddMovementInput(GetPawn()->GetActorRightVector(), Input.X);
		GetPawn()->AddMovementInput(GetPawn()->GetActorForwardVector(), Input.Y);
	}
}

void UCavrnusExploreMode::Look(const FInputActionValue& Value)
{
	FVector2D Input;
	Input.X =  Value.Get<FVector2D>().X;
	Input.Y =  -Value.Get<FVector2D>().Y;

	Input *= 2.0f;
	
	TargetLookDelta = Input;
}

void UCavrnusExploreMode::Fly(const FInputActionValue& Value)
{
	const float Input = Value.Get<float>();
	if (GetPawn())
		GetPawn()->AddMovementInput(GetPawn()->GetActorUpVector(), Input);
}
