// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CavrnusButtonState.generated.h"

UENUM(BlueprintType)
enum class ECavrnusButtonState : uint8
{
	Normal,
	Hovered,
	Pressed,
	Disabled,
	Focused,
	Toggled
};