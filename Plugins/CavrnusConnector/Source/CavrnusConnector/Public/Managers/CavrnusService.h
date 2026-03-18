// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/DisposableUObject.h"
#include "UObject/Object.h"
#include "CavrnusService.generated.h"

/**
 * 
 */
UCLASS()
class CAVRNUSCONNECTOR_API UCavrnusService : public UDisposableUObject
{
	GENERATED_BODY()
public:
	virtual void Initialize();
	virtual void Dispose() override;

protected:
	virtual void OnAppInit() {}
	virtual void OnAppShutdown() {}
	virtual void OnBeginPIE(bool bIsSimulating) {}
	virtual void OnEndPIE(bool bIsSimulating) {}

private:
	FDelegateHandle EngineInitHandle;
	FDelegateHandle EnginePreExitHandle;

#if WITH_EDITOR
	FDelegateHandle BeginPIEHandle;
	FDelegateHandle EndPIEHandle;
#endif
};
