// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"

class CAVRNUSCONNECTOR_API FCavrnusMathHelpers
{
public:
	static FVector4 RotatorToVector4(const FRotator& Rotator)
	{
		FVector4 Result = FVector4();
		Result.X = Rotator.Roll;
		Result.Y = Rotator.Pitch;
		Result.Z = Rotator.Yaw;

		return Result;
	}

	static int32 GetGCD(int32 A, int32 B)
	{
		while (B != 0)
		{
			const int32 Temp = B;
			B = A % B;
			A = Temp;
		}
		return A;
	}

	static bool AreTransformsApproximatelyEqual(const FTransform& A, const FTransform& B, float PosTol = 0.1f, float RotTolDeg = 0.5f, float ScaleTol = 0.01f)
	{
		return FVector::DistSquared(A.GetLocation(), B.GetLocation()) <= PosTol * PosTol
			&& FMath::Abs(A.GetRotation().AngularDistance(B.GetRotation())) <= FMath::DegreesToRadians(RotTolDeg)
			&& FVector::DistSquared(A.GetScale3D(), B.GetScale3D()) <= ScaleTol * ScaleTol;
	}
};
