// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "Core/DisposableUObject.h"

void UDisposableUObject::Dispose()
{
	for (auto& Obj : UObjectDisposables)
		Obj->Dispose();
	UObjectDisposables.Empty();

	for (auto& Obj : NativeDisposables)
		Obj->Dispose();
	NativeDisposables.Empty();
}
