// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "CavrnusCVTWalkAnim.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class CAVRNUSCONNECTOR_API UCavrnusCVTWalkAnim : public UAnimInstance
{
	GENERATED_BODY()
public:
	virtual void NativeInitializeAnimation() override;
	
	void SetPawnOwner(APawn* InPawn)
	{
		Pawn = InPawn;
	}
	
	void SetSpeed(const float InSpeed)
	{
		Speed = InSpeed;
	}

	void SetHeadLookAt(const FRotator& InRotation)
	{
		HeadLookAt = InRotation;
	}

	void SetHeadLookAt(const FVector4& InRotation)
	{
		FRotator Rotator;
		Rotator.Roll = InRotation.X;
		Rotator.Pitch = InRotation.Y;
		Rotator.Yaw = InRotation.Z;
		HeadLookAt = Rotator;
	}

protected:
	UPROPERTY(BlueprintReadWrite, Category="Cavrnus")
	TObjectPtr<APawn> Pawn = nullptr;
	
	UPROPERTY(BlueprintReadWrite, Category="Cavrnus")
	float Speed = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category="Cavrnus")
	FRotator HeadLookAt = FRotator();
};
