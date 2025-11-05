// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "TimerManager.h"
#include "Templates/Function.h"

/**
 * 
 */
class CAVRNUSCONNECTOR_API FCavrnusUIScheduler
{
public:
	static void WaitOneFrame(const UWorld* InWorld, TFunction<void()> InCallback);
	
	static FTimerHandle SetTimer(const UWorld* InWorld, const float InDurationSeconds, TFunction<void()> InCallback);
	static void ClearTimer(const UWorld* InWorld, FTimerHandle& InTimerHandle);
};