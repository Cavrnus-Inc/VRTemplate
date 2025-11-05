// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once
#include "CoreMinimal.h"

/**
 * Wrapper for UE persistent values, akin to Unity PlayerRefs
 */
class CAVRNUSCONNECTOR_API FCavrnusKeyValueStore
{
public:
	static bool KeyEntryExists(const FString& Key);
	static bool DeleteKey(const FString& Key);
	
	static FString GetStoredStringValue(const FString& Key);
	static bool GetStoredBoolValue(const FString& Key, const bool DefaultValue = false);
	
	static bool SetStoredStringValue(const FString& Key, const FString& Value);
	static bool SetStoredBoolValue(const FString& Key, const bool Value);

private:
	static FString GetPrefixedProjectKey(const FString& Key); 
};
