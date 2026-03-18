// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Disposable.h"
#include "UObject/Object.h"
#include "DisposableUObject.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class CAVRNUSCONNECTOR_API UDisposableUObject : public UObject, public IDisposable
{
	GENERATED_BODY()
public:
	virtual void Dispose() override;

	template<typename T>
	T* AlsoDispose(T* Obj)
	{
		if (Obj && Obj->GetClass()->ImplementsInterface(UDisposable::StaticClass()))
			UObjectDisposables.Add(TScriptInterface<IDisposable>(Obj));
		
		return Obj;
	}

	template<typename T>
	TSharedPtr<T> AlsoDispose(TSharedPtr<T> Obj)
	{
		if (Obj.IsValid())
			NativeDisposables.Add(Obj);
		
		return Obj;
	}
	
private:
	UPROPERTY()
	TArray<TScriptInterface<IDisposable>> UObjectDisposables;
	
	TArray<TSharedPtr<IDisposable>> NativeDisposables;
};
