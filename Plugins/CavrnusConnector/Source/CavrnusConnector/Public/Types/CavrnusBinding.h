// Copyright (c) 2025 Cavrnus. All rights reserved.

/**
 * @file CavrnusBinding.h
 * @brief This file defines the UCavrnusBinding class used for managing bindings in Cavrnus.
 *
 * The UCavrnusBinding class is utilized for setting up and managing bindings in the Cavrnus system. It includes methods to unbind these bindings as needed.
 * This class is particularly useful for handling dynamic bindings to events or properties in Cavrnus, allowing for clean and efficient resource management.
 */

#pragma once

#include "CoreMinimal.h"
#include "GenericPlatform/GenericPlatformMisc.h"

#include "CavrnusBinding.generated.h"		// Always last

namespace Cavrnus
{
	typedef TFunction<void()> CavrnusUnbind;
};

/**
 * @brief UCavrnusBinding class for managing bindings in Cavrnus.
 *
 * The UCavrnusBinding class provides functionality to set up and unbind bindings within the Cavrnus system. It ensures that resources tied to bindings are properly released when no longer needed.
 */
UCLASS(BlueprintType)
class CAVRNUSCONNECTOR_API UCavrnusBinding : public UObject
{
	GENERATED_BODY()

public:

	/**
	 * @brief Sets up a function to be called when the binding is unbound.
	 *
	 * @param unhook A function to be called upon unbinding.
	 */
	void Setup(FString binding);


	/**
	 * @brief Unbinds the binding, releasing any resources tied to it.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Cavrnus")
	void Unbind();

	FString BindingId;
};
