// Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include <Containers/Map.h>
#include "Types/CavrnusBinding.h"
#include "Types/CavrnusCallbackTypes.h"

#include "PropertyPostOptions.generated.h"

USTRUCT(BlueprintType)
struct CAVRNUSCONNECTOR_API FPropertyPostOptions
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|Properties")
	bool Smoothed = true;

	FPropertyPostOptions() = default;
};
