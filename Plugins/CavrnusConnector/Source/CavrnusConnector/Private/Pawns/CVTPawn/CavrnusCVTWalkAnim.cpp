// // Copyright (c) 2025 Cavrnus. All rights reserved.


#include "Pawns/CVTPawn/CavrnusCVTWalkAnim.h"

void UCavrnusCVTWalkAnim::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	
	if (!Pawn)
	{
		if (APawn* OwnerPawn = TryGetPawnOwner())
		{
			Pawn = OwnerPawn;
		}
	}
}
