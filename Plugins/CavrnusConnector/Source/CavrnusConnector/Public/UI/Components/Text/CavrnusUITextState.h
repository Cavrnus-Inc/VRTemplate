#pragma once

#include "CoreMinimal.h"
#include "CavrnusUITextState.generated.h"

UENUM(BlueprintType)
enum class ECavrnusUITextState : uint8
{
	Normal,
	Hovered,
	Disabled,
	Toggled
};