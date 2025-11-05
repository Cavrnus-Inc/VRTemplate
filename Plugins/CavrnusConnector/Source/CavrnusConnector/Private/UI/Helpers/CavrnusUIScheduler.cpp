// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Helpers/CavrnusUIScheduler.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "CoreMinimal.h"

void FCavrnusUIScheduler::WaitOneFrame(const UWorld* InWorld, TFunction<void()> InCallback)
{
	if (InWorld == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot wait frame, InWorld is null!"));
		return;
	}

	InWorld->GetTimerManager().SetTimerForNextTick([InCallback]()
	{
		if (InCallback)
			InCallback();
	});
}

FTimerHandle FCavrnusUIScheduler::SetTimer(const UWorld* InWorld, const float InDurationSeconds, TFunction<void()> InCallback)
{
	FTimerHandle Timer;

	if (InWorld)
	{
		const FTimerDelegate TimerDelegate = FTimerDelegate::CreateLambda([InCallback]
		{
			if (InCallback)
				InCallback();
		});
		
		InWorld->GetTimerManager().SetTimer(Timer, TimerDelegate, InDurationSeconds, false);
	}
	
	return Timer;
}

void FCavrnusUIScheduler::ClearTimer(const UWorld* InWorld, FTimerHandle& InTimerHandle)
{
	if (InWorld)
		InWorld->GetTimerManager().ClearTimer(InTimerHandle);
}
