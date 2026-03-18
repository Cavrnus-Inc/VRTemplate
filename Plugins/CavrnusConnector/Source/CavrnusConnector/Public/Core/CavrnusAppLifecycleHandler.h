// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/DisposableUObject.h"
#include "UObject/Object.h"
#include "Engine/World.h"
#include "CavrnusAppLifecycleHandler.generated.h"

UCLASS()
class CAVRNUSCONNECTOR_API UCavrnusAppLifecycleHandler : public UDisposableUObject
{
	GENERATED_BODY()
public:
	void AwaitAppStart(const TFunction<void(const bool IsEditor, UWorld* World)>& InStartCallback);
	void AwaitAppEnd(const TFunction<void(const bool IsEditor)>& InEndCallback);

	virtual void Dispose() override;

private:
	TFunction<void(const bool IsEditor, UWorld* World)> StartCallback;
	TFunction<void(const bool IsEditor)> EndCallback;

	FDelegateHandle WorldHandle = FDelegateHandle();

	void HandleWorldInit(UWorld* World, const UWorld::InitializationValues InitializationValues);
#if WITH_EDITOR
	void HandlePIEEnd(bool bIsSim);
#endif
};
