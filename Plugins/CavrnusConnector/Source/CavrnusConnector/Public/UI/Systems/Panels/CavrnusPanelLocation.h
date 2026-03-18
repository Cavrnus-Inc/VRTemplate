// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CavrnusPanelLocation.generated.h"

UENUM(BlueprintType)
enum class EPanelLocation : uint8
{
	LeftMiddle     UMETA(DisplayName = "Left Middle"),
	RightMiddle    UMETA(DisplayName = "Right Middle"),
	BottomMiddle   UMETA(DisplayName = "Bottom Middle"),
	TopMiddle      UMETA(DisplayName = "Top Middle"),
};

