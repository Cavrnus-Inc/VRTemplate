// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CavrnusModeBase.h"
#include "Core/Settings/Setting.h"
#include "Managers/CavrnusService.h"
#include "CavrnusModeManager.generated.h"

class UCavrnusModeBase;
/**
 * Facade for mode management
 */
UCLASS()
class CAVRNUSCONNECTOR_API UCavrnusModeManager : public UCavrnusService
{
	GENERATED_BODY()
private:
	UPROPERTY()
	TArray<UCavrnusModeBase*> ModeStack;

public:
	TSetting<UClass*> CurrentActiveMode;
	TSetting<UClass*> CurrentExplicitMode;
	
	template <typename T>
	bool IsCurrentModeOfType()
	{
		if (ModeStack.IsEmpty())
			return false;

		if (Cast<T>(ModeStack.Top()))
			return true;

		return false;
	}

	template <typename T>
	void SetExplicitMode(UWorld* World)
	{
		while (ModeStack.Num() > 0)
			ModeStack.Pop()->ExitMode();

		auto NewMode = NewObject<T>(this);
		ModeStack.Push(NewMode);
		NewMode->IsExplicitMode = true;
		NewMode->EnterMode(World, ModeStack.Num());

		CurrentActiveMode.Set(T::StaticClass());
		CurrentExplicitMode.Set(T::StaticClass());
		
		UE_LOG(LogTemp, Display, TEXT("Pushing new explicit mode! %s"), *NewMode->GetName());
	}

	template <typename T>
	void PushTransientMode(UWorld* World)
	{
		if (IsCurrentModeOfType<T>())
			return;

		auto NewMode = NewObject<T>(this);
		ModeStack.Push(NewMode);
		NewMode->EnterMode(World, ModeStack.Num());

		CurrentActiveMode.Set(T::StaticClass());
		
		UE_LOG(LogTemp, Display, TEXT("Pushing new transient mode! %s"), *NewMode->GetName());
	}

	void PopTransientMode()
	{
		if (ModeStack.IsEmpty())
			return;

		// Only pop transients!
		if (ModeStack.Top()->IsExplicitMode)
			return;
		
		if (auto* Mode = ModeStack.Pop())
		{
			Mode->ExitMode();
			UE_LOG(LogTemp, Display, TEXT("Popping mode! - %s"), *Mode->GetName());
		}

		 CurrentActiveMode.Set(ModeStack.Top()->GetClass());
	}

	template <typename T>
	T* GetCurrentMode()
	{
		if (ModeStack.IsEmpty())
			return nullptr;
		
		return Cast<T>(ModeStack.Top());
	}
	
	virtual void Teardown() override
	{
		ModeStack.Empty();
	}
};
