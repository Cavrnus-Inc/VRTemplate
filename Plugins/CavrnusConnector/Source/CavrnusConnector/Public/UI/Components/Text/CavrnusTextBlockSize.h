// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CavrnusTextBlockSize.generated.h"

UENUM(BlueprintType)
enum class ECavrnusTextBlockSize : uint8
{
	H1,
	H2,
	Button,
	Body,
	Small,
	Custom,
};