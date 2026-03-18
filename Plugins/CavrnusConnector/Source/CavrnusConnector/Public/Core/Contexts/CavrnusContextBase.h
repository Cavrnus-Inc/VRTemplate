// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/DisposableUObject.h"
#include "ServiceLocator/CavrnusServiceLocator.h"
#include "UObject/Object.h"
#include "CavrnusContextBase.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class CAVRNUSCONNECTOR_API UCavrnusContextBase : public UDisposableUObject
{
	GENERATED_BODY()
protected:
	UPROPERTY()
	UCavrnusServiceLocator* Services = nullptr;

public:
	virtual void Dispose() override
	{
		if (IsValid(Services))
		{
			Services->Dispose();
			Services = nullptr;
		}
	}
	
	template<typename TService>
	TService* Get() const
	{
		// Check both that Services pointer is non-null AND that the object is valid
		// This prevents crashes when Services is a dangling pointer or has been disposed
		return (Services && IsValid(Services)) ? Services->Get<TService>() : nullptr;
	}
};
