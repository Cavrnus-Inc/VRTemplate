// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "Core/Tick/CavrnusTickableObject.h"

UCavrnusTickableObject* UCavrnusTickableObject::Initialize(const TFunction<void(const float DeltaTick)>& TickFunc)
{
	Func = TickFunc;

	return this;
}

UCavrnusTickableObject* UCavrnusTickableObject::SetTickEnabled(const bool bEnabled)
{
	bIsEnabled = bEnabled;
	return this;
}

void UCavrnusTickableObject::Tick(float DeltaTime)
{
	if (Func)
		Func(DeltaTime);
}

TStatId UCavrnusTickableObject::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UTickableUObject, STATGROUP_Tickables);
}

bool UCavrnusTickableObject::IsTickable() const
{
	return bIsEnabled;
}

