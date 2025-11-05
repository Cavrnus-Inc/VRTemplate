// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "Pawns/CavrnusPawnSpeed.h"
#include "GameFramework/Pawn.h"
#include "Engine/GameInstance.h"

void UCavrnusPawnSpeed::Initialize(APawn* InPawn)
{
	Pawn = InPawn;
}

void UCavrnusPawnSpeed::UpdateTick(float DeltaSeconds)
{
	if (!Pawn.IsValid()) return;

	CurrentPos = Pawn->GetActorLocation();
	MovementDirection = CurrentPos - PreviousPos;

	const float RawSpeed = MovementDirection.Size() / DeltaSeconds;

	// Use acceleration if speeding up, deceleration if slowing down
	const float InterpSpeed = (RawSpeed > SmoothedSpeed) ? Acceleration : Deceleration;
	SmoothedSpeed = FMath::FInterpTo(SmoothedSpeed, RawSpeed, DeltaSeconds, InterpSpeed);

	CurrentSpeed = SmoothedSpeed;
	PreviousPos = CurrentPos;
}
