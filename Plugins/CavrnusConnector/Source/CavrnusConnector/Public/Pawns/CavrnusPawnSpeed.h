// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "CavrnusPawnSpeed.generated.h"

UCLASS()
class CAVRNUSCONNECTOR_API UCavrnusPawnSpeed : public UObject
{
	GENERATED_BODY()
public:
	float GetSpeed() const{ return CurrentSpeed; }
	
	void Initialize(APawn* InPawn);
	void UpdateTick(float DeltaSeconds);
	
private:
	UPROPERTY()
	TWeakObjectPtr<APawn> Pawn;

	float Acceleration = 2.0f;
	float Deceleration = 5.0f;
	float CurrentSpeed = 0.0f;
	float SmoothedSpeed = 0.0f;
	
	FVector CurrentPos = FVector();
	FVector PreviousPos = FVector();
	FVector MovementDirection = FVector();
};
