// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "CoreMinimal.h"

UENUM(BlueprintType, Category="Cavrnus|UI|Messages")
enum class ECavrnusInfoToastMessageEnum : uint8
{
	Info     UMETA(DisplayName="Info"),
	Success  UMETA(DisplayName="Success"),
	Warning  UMETA(DisplayName="Warning"),
	Error    UMETA(DisplayName="Error")
};