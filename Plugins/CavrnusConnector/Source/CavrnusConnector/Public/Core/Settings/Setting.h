// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * Templated reactive setting
 */
template <typename T>
class CAVRNUSCONNECTOR_API TSetting
{
public:
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnChanged, const T&);

private:
	T CurrVal;
	FOnChanged OnChanged;

public:
	TSetting() = default;
	explicit TSetting(const T& InValue) : CurrVal(InValue) {}
	
	const T& Get() { return CurrVal; }

	void Set(const T& NewValue)
	{
		if (NewValue == CurrVal)
			return;

		CurrVal = NewValue;

		if (OnChanged.IsBound())
			OnChanged.Broadcast(NewValue);
	}

	FDelegateHandle Bind(UObject* Owner, const TFunction<void(const T&)>& Callback) 
	{
		TWeakObjectPtr WeakOwner = Owner;
		const FDelegateHandle Handle = OnChanged.AddLambda([WeakOwner, Callback](const T& NewValue)
		{
			if (WeakOwner.IsValid())
				Callback(NewValue);
		});

		if (WeakOwner.IsValid())
			Callback(CurrVal);

		return Handle;
	}

	void Unbind(FDelegateHandle Handle)
	{
		OnChanged.Remove(Handle);
	}
	
	TSharedPtr<TSetting<bool>> Translating(TFunction<bool(const T&)> Transform);
};

template <typename T>
TSharedPtr<TSetting<bool>> TSetting<T>::Translating(TFunction<bool(const T&)> Transform)
{
	// Initialize derived with current translated value
	TSharedPtr<TSetting<bool>> Derived = MakeShared<TSetting<bool>>(Transform(CurrVal));
	
	// Keep derived in sync with source
	OnChanged.AddLambda([WeakDerived = TWeakPtr<TSetting<bool>>(Derived), Transform](const T& NewValue)
	{
		if (const auto Shared = WeakDerived.Pin())
			Shared->Set(Transform(NewValue));
	});

	return Derived;
}