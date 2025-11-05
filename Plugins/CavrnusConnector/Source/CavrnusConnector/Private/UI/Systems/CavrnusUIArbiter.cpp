// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Systems/CavrnusUIArbiter.h"
#include "UI/Systems/Messages/ToastMessages/CavrnusToastMessageUISystem.h"

void UCavrnusUIArbiter::Initialize()
{
	CurrentUIVisMode.Set(EUIVisibilityMode::Normal);
}

void UCavrnusUIArbiter::SetVisibilityMode(const EUIVisibilityMode NewMode)
{
	if (CurrentUIVisMode.Get() == NewMode)
		return;
	
	CurrentUIVisMode.Set(NewMode);
}

bool UCavrnusUIArbiter::CanSpawnUIType(const EUIType Type)
{
	switch (CurrentUIVisMode.Get())
	{
	case EUIVisibilityMode::Normal:
		return true;
	case EUIVisibilityMode::ScreenExclusive:
		return Type == EUIType::Screen;
	case EUIVisibilityMode::NoUI:
	default:
		return false;
	}
}
