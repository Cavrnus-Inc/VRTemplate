// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "Managers/SpawnedObjects/CavrnusPendingSpawnObject.h"
#include "Managers/SpawnedObjects/SpawnedObjectsManager.h"
#include "Managers/SpawnedObjects/SpawnObjectHelpers.h"
#include "Managers/SpawnedObjects/CavrnusSpawnPropertyHelpers.h"
#include "CavrnusFunctionLibrary.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "CavrnusConnectorModule.h"
#include "UObject/UnrealType.h"
#include "Kismet/GameplayStatics.h"

UCavrnusPendingSpawnObject::UCavrnusPendingSpawnObject(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, ActorClass(nullptr)
	, ObjectClass(nullptr)
	, CollisionHandling(ESpawnActorCollisionHandlingMethod::AlwaysSpawn)
	, Manager(nullptr)
	, TransformBinding(nullptr)
	, ReceivedTransform(FTransform::Identity)
	, bTransformReceived(false)
	, bHasSpawned(false)
{
}

void UCavrnusPendingSpawnObject::SetupWithActorClass(
	const FCavrnusSpawnedObject& InSpawnedObject,
	const FString& InWellKnownObjectId,
	TSubclassOf<AActor> InActorClass,
	ESpawnActorCollisionHandlingMethod InCollisionHandling,
	USpawnedObjectsManager* InManager)
{
	SpawnedObject = InSpawnedObject;
	WellKnownObjectId = InWellKnownObjectId;
	ActorClass = InActorClass;
	CollisionHandling = InCollisionHandling;
	Manager = InManager;
	bHasSpawned = false;
}

void UCavrnusPendingSpawnObject::SetupWithObjectClass(
	const FCavrnusSpawnedObject& InSpawnedObject,
	const FString& InWellKnownObjectId,
	TSubclassOf<UObject> InObjectClass,
	USpawnedObjectsManager* InManager)
{
	SpawnedObject = InSpawnedObject;
	WellKnownObjectId = InWellKnownObjectId;
	ActorClass = nullptr;
	ObjectClass = InObjectClass;
	CollisionHandling = ESpawnActorCollisionHandlingMethod::AlwaysSpawn; // Not used for UObjects
	Manager = InManager;
	bHasSpawned = false;
}

void UCavrnusPendingSpawnObject::Initialize()
{
	if (!Manager || !IsValid(Manager))
	{
		UE_LOG(LogCavrnusConnector, Error, TEXT("UCavrnusPendingSpawnObject::Initialize: Manager is invalid"));
		return;
	}

	// Get ExposeOnSpawn properties from the actor class or object class
	UClass* TargetClass = ActorClass;
	if (!TargetClass && ObjectClass)
	{
		// For UObjects, use the ObjectClass
		TargetClass = ObjectClass;
	}

	if (TargetClass)
	{
		ExposeOnSpawnProperties = FCavrnusSpawnPropertyHelpers::GetExposeOnSpawnProperties(TargetClass);
	}

	// Bind to Transform property (only for Actors, not for UObjects)
	if (ActorClass)
	{
		UCavrnusFunctionLibrary::FTransformPropertyUpdated TransformDelegate;
		TransformDelegate.BindUFunction(this, FName("OnTransformReceived"));

		TransformBinding = UCavrnusFunctionLibrary::BindTransformPropertyValue(
			SpawnedObject.SpaceConnection,
			SpawnedObject.PropertiesContainerName,
			TEXT("Transform"),
			TransformDelegate
		);
	}
	else if (ObjectClass)
	{
		// For UObjects, we don't need Transform, so mark it as received immediately
		// and check if we can construct now
		bTransformReceived = true;
		if (AreAllPropertiesReceived())
		{
			ConstructObject();
		}
	}

	// Bind to each ExposeOnSpawn property
// Bind to each ExposeOnSpawn property
	for (const FCavrnusExposeOnSpawnProperty& ExposeProp : ExposeOnSpawnProperties)
	{
		if (!ExposeProp.Property)
		{
			continue;
		}

		FProperty* Prop = ExposeProp.Property;
		FString PropName = ExposeProp.PropertyName;

		// Determine property type and bind accordingly
		if (FBoolProperty* BoolProp = CastField<FBoolProperty>(Prop))
		{
			CavrnusBoolFunction BoolCallback = [this, PropName](bool Value, const FString&, const FString&)
				{
					Cavrnus::FPropertyValue CavrnusValue = Cavrnus::FPropertyValue::BoolPropValue(Value);
					OnExposeOnSpawnPropertyReceived(PropName, CavrnusValue);
				};

			UCavrnusBinding* Binding = UCavrnusFunctionLibrary::BindBooleanPropertyValue(
				SpawnedObject.SpaceConnection,
				FPropertiesContainer(SpawnedObject.PropertiesContainerName),
				PropName,
				BoolCallback
			);

			if (Binding)
			{
				ExposeOnSpawnBindings.Add(PropName, Binding);
			}
		}
		else if (FNumericProperty* NumericProp = CastField<FNumericProperty>(Prop))
		{
			CavrnusFloatFunction FloatCallback = [this, PropName](float Value, const FString&, const FString&)
				{
					Cavrnus::FPropertyValue CavrnusValue = Cavrnus::FPropertyValue::FloatPropValue(Value);
					OnExposeOnSpawnPropertyReceived(PropName, CavrnusValue);
				};

			UCavrnusBinding* Binding = UCavrnusFunctionLibrary::BindFloatPropertyValue(
				SpawnedObject.SpaceConnection,
				FPropertiesContainer(SpawnedObject.PropertiesContainerName),
				PropName,
				FloatCallback
			);

			if (Binding)
			{
				ExposeOnSpawnBindings.Add(PropName, Binding);
			}
		}
		else if (FStrProperty* StrProp = CastField<FStrProperty>(Prop))
		{
			CavrnusStringFunction StringCallback = [this, PropName](const FString& Value, const FString&, const FString&)
				{
					Cavrnus::FPropertyValue CavrnusValue = Cavrnus::FPropertyValue::StringPropValue(Value);
					OnExposeOnSpawnPropertyReceived(PropName, CavrnusValue);
				};

			UCavrnusBinding* Binding = UCavrnusFunctionLibrary::BindStringPropertyValue(
				SpawnedObject.SpaceConnection,
				FPropertiesContainer(SpawnedObject.PropertiesContainerName),
				PropName,
				StringCallback
			);

			if (Binding)
			{
				ExposeOnSpawnBindings.Add(PropName, Binding);
			}
		}
		else if (FTextProperty* TextProp = CastField<FTextProperty>(Prop))
		{
			// Text properties are sent/received as strings
			CavrnusStringFunction StringCallback = [this, PropName](const FString& Value, const FString&, const FString&)
				{
					Cavrnus::FPropertyValue CavrnusValue = Cavrnus::FPropertyValue::StringPropValue(Value);
					OnExposeOnSpawnPropertyReceived(PropName, CavrnusValue);
				};

			UCavrnusBinding* Binding = UCavrnusFunctionLibrary::BindStringPropertyValue(
				SpawnedObject.SpaceConnection,
				FPropertiesContainer(SpawnedObject.PropertiesContainerName),
				PropName,
				StringCallback
			);

			if (Binding)
			{
				ExposeOnSpawnBindings.Add(PropName, Binding);
			}
		}
		else if (FStructProperty* StructProp = CastField<FStructProperty>(Prop))
		{
			UScriptStruct* Struct = StructProp->Struct;
			if (Struct)
			{
				// Check for native struct types
				if (Struct == TBaseStructure<FVector>::Get() || Struct == TBaseStructure<FVector4>::Get())
				{
					CavrnusVectorFunction VectorCallback = [this, PropName](const FVector4& Value, const FString&, const FString&)
						{
							Cavrnus::FPropertyValue CavrnusValue = Cavrnus::FPropertyValue::VectorPropValue(Value);
							OnExposeOnSpawnPropertyReceived(PropName, CavrnusValue);
						};

					UCavrnusBinding* Binding = UCavrnusFunctionLibrary::BindVectorPropertyValue(
						SpawnedObject.SpaceConnection,
						FPropertiesContainer(SpawnedObject.PropertiesContainerName),
						PropName,
						VectorCallback
					);

					if (Binding)
					{
						ExposeOnSpawnBindings.Add(PropName, Binding);
					}
				}
				else if (Struct == TBaseStructure<FTransform>::Get())
				{
					CavrnusTransformFunction TransformCallback = [this, PropName](const FTransform& Value, const FString&, const FString&)
						{
							Cavrnus::FPropertyValue CavrnusValue = Cavrnus::FPropertyValue::TransformPropValue(Value);
							OnExposeOnSpawnPropertyReceived(PropName, CavrnusValue);
						};

					UCavrnusBinding* Binding = UCavrnusFunctionLibrary::BindTransformPropertyValue(
						SpawnedObject.SpaceConnection,
						FPropertiesContainer(SpawnedObject.PropertiesContainerName),
						PropName,
						TransformCallback
					);

					if (Binding)
					{
						ExposeOnSpawnBindings.Add(PropName, Binding);
					}
				}
				else if (Struct == TBaseStructure<FLinearColor>::Get() || Struct == TBaseStructure<FColor>::Get())
				{
					CavrnusColorFunction ColorCallback = [this, PropName](const FLinearColor& Value, const FString&, const FString&)
						{
							Cavrnus::FPropertyValue CavrnusValue = Cavrnus::FPropertyValue::ColorPropValue(Value);
							OnExposeOnSpawnPropertyReceived(PropName, CavrnusValue);
						};

					UCavrnusBinding* Binding = UCavrnusFunctionLibrary::BindColorPropertyValue(
						SpawnedObject.SpaceConnection,
						FPropertiesContainer(SpawnedObject.PropertiesContainerName),
						PropName,
						ColorCallback
					);

					if (Binding)
					{
						ExposeOnSpawnBindings.Add(PropName, Binding);
					}
				}
				else
				{
					// For arbitrary structs, bind as string property (they come as strings from journal)
					CavrnusStringFunction StringCallback = [this, PropName](const FString& Value, const FString&, const FString&)
						{
							Cavrnus::FPropertyValue CavrnusValue = Cavrnus::FPropertyValue::StringPropValue(Value);
							OnExposeOnSpawnPropertyReceived(PropName, CavrnusValue);
						};

					UCavrnusBinding* Binding = UCavrnusFunctionLibrary::BindStringPropertyValue(
						SpawnedObject.SpaceConnection,
						FPropertiesContainer(SpawnedObject.PropertiesContainerName),
						PropName,
						StringCallback
					);

					if (Binding)
					{
						ExposeOnSpawnBindings.Add(PropName, Binding);
					}
				}
			}
		}
	}
}

void UCavrnusPendingSpawnObject::OnTransformReceived(FTransform Transform, FString ContainerName, FString PropertyName)
{
	if (bHasSpawned)
	{
		return; // Already spawned, ignore
	}

	// Store the transform
	ReceivedTransform = Transform;
	bTransformReceived = true;

	// Check if we can spawn now (all ExposeOnSpawn properties must be received)
	if (AreAllPropertiesReceived())
	{
		SpawnActorWithTransform(ReceivedTransform);
	}
}

void UCavrnusPendingSpawnObject::OnExposeOnSpawnPropertyReceived(const FString& PropertyName, const Cavrnus::FPropertyValue& Value)
{
	if (bHasSpawned)
	{
		return; // Already spawned, ignore
	}

	// Store the received value
	ReceivedPropertyValues.Add(PropertyName, Value);

	// Check if we can spawn/construct now
	// For Actors: Transform must be received
	// For UObjects: No Transform needed
	if (ObjectClass)
	{
		// UObject - no Transform needed
		if (AreAllPropertiesReceived())
		{
			ConstructObject();
		}
	}
	else if (bTransformReceived && AreAllPropertiesReceived())
	{
		// Actor - Transform required
		SpawnActorWithTransform(ReceivedTransform);
	}
}

bool UCavrnusPendingSpawnObject::AreAllPropertiesReceived() const
{
	// Check if all ExposeOnSpawn properties have been received
	for (const FCavrnusExposeOnSpawnProperty& ExposeProp : ExposeOnSpawnProperties)
	{
		if (!ReceivedPropertyValues.Contains(ExposeProp.PropertyName))
		{
			return false;
		}
	}
	return true;
}

void UCavrnusPendingSpawnObject::SpawnActorWithTransform(const FTransform& Transform)
{
	if (bHasSpawned)
	{
		return;
	}

	if (!Manager || !IsValid(Manager))
	{
		UE_LOG(LogCavrnusConnector, Error, TEXT("UCavrnusPendingSpawnObject::SpawnActorWithTransform: Manager is invalid"));
		return;
	}

	// Need Helper?
	// Get world from GEngine GameViewport (same pattern as GetSafeWorld)
	UWorld* World = nullptr;
	if (GEngine && GEngine->GameViewport)
	{
		if (UGameViewportClient* ViewportClient = GEngine->GameViewport.Get())
		{
			World = ViewportClient->GetWorld();
		}
	}

	if (!World)
	{
		// Fallback: try to get from world contexts
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if (Context.WorldType == EWorldType::Game || Context.WorldType == EWorldType::PIE)
			{
				World = Context.World();
				break;
			}
		}
	}

	if (!World)
	{
		UE_LOG(LogCavrnusConnector, Error, TEXT("UCavrnusPendingSpawnObject::SpawnActorWithTransform: World is invalid"));
		return;
	}

	// Determine the actor class to spawn
	TSubclassOf<AActor> FinalActorClass = ActorClass;

	if (!FinalActorClass)
	{
		UE_LOG(LogCavrnusConnector, Error, TEXT("UCavrnusPendingSpawnObject::SpawnActorWithTransform: No valid ActorClass for WellKnownObjectId %s"), *WellKnownObjectId);
		return;
	}

	// Spawn the actor DEFERRED (don't finish spawning yet)
	// This allows us to set ExposeOnSpawn properties before BeginPlay() is called
	AActor* SpawnedActor = World->SpawnActorDeferred<AActor>(
		FinalActorClass, 
		Transform, 
		nullptr, 
		nullptr, 
		CollisionHandling
	);
	
	if (!SpawnedActor)
	{
		UE_LOG(LogCavrnusConnector, Error, TEXT("UCavrnusPendingSpawnObject::SpawnActorWithTransform: Failed to spawn actor for WellKnownObjectId %s"), *WellKnownObjectId);
		return;
	}

	// CRITICAL: Apply ExposeOnSpawn properties BEFORE finishing the spawn
	// This ensures they are set before BeginPlay() is called
	ApplyExposeOnSpawnProperties(SpawnedActor);

	// Set up Cavrnus components on the deferred actor
	SpawnObjectHelpers::GetSpawnObjectHelpers()->SetupCavrnusActorDeferred(SpawnedActor, SpawnedObject);

	// NOW finish spawning - this will trigger BeginPlay() with all ExposeOnSpawn properties already set
	UGameplayStatics::FinishSpawningActor(SpawnedActor, Transform);

	// Register with manager (this will also remove us from pending spawns)
	if (Manager)
	{
		Manager->RegisterSpawnedActor(SpawnedObject, SpawnedActor);
	}

	bHasSpawned = true;

	// Clean up bindings
	if (TransformBinding)
	{
		TransformBinding->Dispose();
		TransformBinding = nullptr;
	}

	for (auto& BindingPair : ExposeOnSpawnBindings)
	{
		if (BindingPair.Value)
		{
			BindingPair.Value->Dispose();
		}
	}
	ExposeOnSpawnBindings.Empty();

	// Mark for garbage collection (manager will clean up the reference)
	MarkAsGarbage();
}

void UCavrnusPendingSpawnObject::ConstructObject()
{
	if (bHasSpawned)
	{
		return;
	}

	if (!Manager || !IsValid(Manager))
	{
		UE_LOG(LogCavrnusConnector, Error, TEXT("UCavrnusPendingSpawnObject::ConstructObject: Manager is invalid"));
		return;
	}

	if (!ObjectClass)
	{
		UE_LOG(LogCavrnusConnector, Error, TEXT("UCavrnusPendingSpawnObject::ConstructObject: ObjectClass is null for WellKnownObjectId %s"), *WellKnownObjectId);
		return;
	}

	// Construct the object using NewObject (no World needed for UObjects)
	UObject* ConstructedObject = NewObject<UObject>(GetTransientPackage(), ObjectClass);
	
	if (!ConstructedObject)
	{
		UE_LOG(LogCavrnusConnector, Error, TEXT("UCavrnusPendingSpawnObject::ConstructObject: Failed to construct object for WellKnownObjectId %s"), *WellKnownObjectId);
		return;
	}

	// Apply ExposeOnSpawn property values to the object
	ApplyExposeOnSpawnProperties(ConstructedObject);

	// Register with manager (if tracking is needed)
	// Note: For now, we may not track UObjects the same way as Actors
	// This can be extended later if needed

	bHasSpawned = true;

	// Clean up bindings
	for (auto& BindingPair : ExposeOnSpawnBindings)
	{
		if (BindingPair.Value)
		{
			BindingPair.Value->Dispose();
		}
	}
	ExposeOnSpawnBindings.Empty();

	// Mark for garbage collection (manager will clean up the reference)
	MarkAsGarbage();
}

void UCavrnusPendingSpawnObject::ApplyExposeOnSpawnProperties(AActor* SpawnedActor)
{
	if (!SpawnedActor)
	{
		return;
	}

	// Apply each received ExposeOnSpawn property value to the actor
	for (const FCavrnusExposeOnSpawnProperty& ExposeProp : ExposeOnSpawnProperties)
	{
		if (!ExposeProp.Property)
		{
			continue;
		}

		const Cavrnus::FPropertyValue* ReceivedValue = ReceivedPropertyValues.Find(ExposeProp.PropertyName);
		if (!ReceivedValue)
		{
			continue; // Property not received yet (shouldn't happen if we check AreAllPropertiesReceived)
		}

		// Get the property value pointer from the actor
		void* ValuePtr = ExposeProp.Property->ContainerPtrToValuePtr<void>(SpawnedActor);
		if (!ValuePtr)
		{
			UE_LOG(LogCavrnusConnector, Warning,
				TEXT("[ApplyExposeOnSpawnProperties] Failed to get value pointer for property '%s' on actor '%s'"),
				*ExposeProp.PropertyName, *SpawnedActor->GetName());
			continue;
		}

		// Convert and apply the value
		FCavrnusSpawnPropertyHelpers::CavrnusValueToProperty(*ReceivedValue, ExposeProp.Property, ValuePtr);
	}
}

void UCavrnusPendingSpawnObject::ApplyExposeOnSpawnProperties(UObject* ConstructedObject)
{
	if (!ConstructedObject)
	{
		return;
	}

	// Apply each received ExposeOnSpawn property value to the object
	for (const FCavrnusExposeOnSpawnProperty& ExposeProp : ExposeOnSpawnProperties)
	{
		if (!ExposeProp.Property)
		{
			continue;
		}

		const Cavrnus::FPropertyValue* ReceivedValue = ReceivedPropertyValues.Find(ExposeProp.PropertyName);
		if (!ReceivedValue)
		{
			continue; // Property not received yet (shouldn't happen if we check AreAllPropertiesReceived)
		}

		// Get the property value pointer from the object
		void* ValuePtr = ExposeProp.Property->ContainerPtrToValuePtr<void>(ConstructedObject);
		if (!ValuePtr)
		{
			UE_LOG(LogCavrnusConnector, Warning,
				TEXT("[ApplyExposeOnSpawnProperties] Failed to get value pointer for property '%s' on object '%s'"),
				*ExposeProp.PropertyName, *ConstructedObject->GetName());
			continue;
		}

		// Convert and apply the value
		FCavrnusSpawnPropertyHelpers::CavrnusValueToProperty(*ReceivedValue, ExposeProp.Property, ValuePtr);
	}
}

