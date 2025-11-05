// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Managers/CavrnusService.h"
#include "UObject/Object.h"
#include "CavrnusServiceLocator.generated.h"

UCLASS()
class CAVRNUSCONNECTOR_API UCavrnusServiceLocator : public UObject
{
	GENERATED_BODY()
private:
	UPROPERTY()
	TMap<FName,UObject*> services;
	
public:
	template<typename T>
	void RegisterService()
	{
		if (Contains<T>())
			return;
		
		auto* service = CreateService<T>();
		services.Add(GetKey<T>(),service);
		
		UE_LOG(LogTemp, Verbose, TEXT("Service registered: %s"), *GetKey<T>().ToString());
	}

	template<typename T>
	bool Contains()
	{
		if (services.Contains(GetKey<T>()))
		{
			UE_LOG(LogTemp, Verbose, TEXT("Service already contains service!: %s"), *GetKey<T>().ToString());
			
			return true;
		}

		return false;
	}
	
	template<typename T>
	T* Get()
	{
		if (UObject** Found = services.Find(GetKey<T>()))
			return Cast<T>(*Found);

		return nullptr;
	}

	void Teardown()
	{
		for (TPair<FName, TWeakObjectPtr<UObject>> Service : services)
		{
			if (auto* Found = Cast<UCavrnusService>(Service.Value))
				Found->Teardown();
		}

		services.Empty();
	}

private:
	template<typename T>
	FName GetKey()
	{
		return FName(T::StaticClass()->GetName());
	}

	template<typename T>
	T* CreateService()
	{
		auto* service = NewObject<T>(this);
		if constexpr (TIsDerivedFrom<T, UCavrnusService>::Value)
		{
			UE_LOG(LogTemp, Log, TEXT("T is derived from UCavrnusService"));
			service->Initialize();
		}
		
		return service;
	}
};
