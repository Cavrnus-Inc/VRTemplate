// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CavrnusBaseUISystem.h"
#include "Core/DisposableUObject.h"
#include "Core/Settings/Setting.h"
#include "UI/CavrnusUI.h"
#include "UObject/Object.h"
#include "CavrnusUIArbiter.generated.h"

/**
 * This really just orchestrates the relationship between Screens and other UI systems
 * Screens take precedence over everything else and act as a stack for visibility.
 */
UENUM(BlueprintType)
enum class EUIVisibilityMode : uint8
{
	Normal,
	ScreenExclusive,
	NoUI
};

UCLASS()
class CAVRNUSCONNECTOR_API UCavrnusUIArbiter : public UDisposableUObject
{
	GENERATED_BODY()
public:
	TSharedPtr<TSetting<EUIVisibilityMode>> CurrentUIVisMode;
	
	void Initialize();
	void SetVisibilityMode(const EUIVisibilityMode NewMode);
	bool CanSpawnUIType(EUIType Type);
};
