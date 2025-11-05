// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "Types/CavrnusBinding.h"
#include "CavrnusFunctionLibrary.h"

//===========================================================


void UCavrnusBinding::Setup(FString bindingid)
{
	BindingId = bindingid;
}

void UCavrnusBinding::Unbind_Implementation()
{
	UCavrnusFunctionLibrary::UnbindWithId(BindingId);
}