// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "CavrnusTickableObject.generated.h"

/**
 * Reusable tickable UObject
 */
UCLASS()
class CAVRNUSCONNECTOR_API UCavrnusTickableObject : public UObject, public FTickableGameObject
{
	GENERATED_BODY()
	
public:
	static UCavrnusTickableObject* Create(
		UObject* Outer,
		const TFunction<void(const float DeltaSeconds)>& TickFunc,
		const bool bEnabledByDefault = true)
	{
		auto* Obj = NewObject<UCavrnusTickableObject>(Outer);
		Obj->Initialize(TickFunc)->SetTickEnabled(bEnabledByDefault);

		return Obj;
	}
	
	UCavrnusTickableObject* Initialize(const TFunction<void(const float DeltaSeconds)>& TickFunc);
	UCavrnusTickableObject* SetTickEnabled(const bool bEnabled);
	
	// FTickableGameObject overrides
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickable() const override;
	virtual bool IsTickableInEditor() const override { return false; }
	virtual bool IsTickableWhenPaused() const override { return false; }
	
private:
	bool bIsEnabled;
	TFunction<void(const float DeltaSeconds)> Func;
};
