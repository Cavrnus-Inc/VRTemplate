// Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "CavrnusPropertiesContainer.generated.h"

/**
 * @brief UCavrnusPropertiesContainer is the component class used for storing properties within the Cavrnus system. This component is attached to actors and used to synchronize properties in the Cavrnized project with corresponding values in the Journal of a Cavrnus Space.
 */
UCLASS(BlueprintType, Blueprintable, HideCategories = (Transform, Physics, LOD, Collision, Rendering, ComponentTick), meta = (BlueprintSpawnableComponent))
class CAVRNUSCONNECTOR_API UCavrnusPropertiesContainer : public USceneComponent
{
	GENERATED_BODY()

public:
	UCavrnusPropertiesContainer();

	/**
	 * @brief Called when the component is initialized, allowing for custom initialization logic.
	 */
	virtual void InitializeComponent() override;

	/**
	 * @brief This property stores the name of the container, which can be used to identify it within the Cavrnus system.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cavrnus")
	FString ContainerName = "";
};
