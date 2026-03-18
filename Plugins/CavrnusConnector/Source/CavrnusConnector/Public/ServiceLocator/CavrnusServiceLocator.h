// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/Disposable.h"
#include "Managers/CavrnusService.h"
#include "UObject/Object.h"
#include "CavrnusServiceLocator.generated.h"

UCLASS()
class CAVRNUSCONNECTOR_API UCavrnusServiceLocator : public UObject, public IDisposable
{
	GENERATED_BODY()
private:
	UPROPERTY()
	TMap<FName, UCavrnusService*> services;
	
public:
	virtual void Dispose() override
	{
		for (auto service : services)
		{
			if (IsValid(service.Value))
				service.Value->Dispose();
		}

		services.Empty();
	}
	
	template<typename T>
	void RegisterService(UWorld* World = nullptr)
	{
		if (Contains<T>())
			return;
		
		T* service = CreateService<T>(World);
		services.Add(GetKey<T>(), service);

		service->Initialize();
		
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
		if (UCavrnusService** Found = services.Find(GetKey<T>()))
			return Cast<T>(*Found);

		return nullptr;
	}

private:
	template<typename T>
	FName GetKey()
	{
		return FName(T::StaticClass()->GetName());
	}

	template<typename T>
	T* CreateService(UWorld* World)
	{
		if (!World)
			World = GetWorld();

		T* Service;
		
		if (World)
			Service = NewObject<T>(World);
		else
			Service = NewObject<T>(this);

		return Service;
	}
};
