// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "UObject/UObjectGlobals.h"

class CAVRNUSCONNECTOR_API FCavrnusUIHelpers
{
public:
	template <typename T>
	static T* GetComponentInParent(UObject* Obj)
	{
		if (Obj == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("Object is null! Unable to find outer component!"))

			return nullptr;
		}
		UObject* CurrentOuter = Obj->GetOuter();
		while (CurrentOuter)
		{
			if (T* FoundObject = Cast<T>(CurrentOuter))
				return FoundObject;
			
			CurrentOuter = CurrentOuter->GetOuter();
		}

		UE_LOG(LogTemp, Warning, TEXT("Target component not found in parents!"))
		return nullptr;
	}

	static FString GetAbbreviatedDate(const FDateTime& Date);
};
