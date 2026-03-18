// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Disposable.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UDisposable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class CAVRNUSCONNECTOR_API IDisposable
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual void Dispose() = 0;
};
