// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "CavrnusKeyValueStore.h"
#include "Misc/App.h"

FString FCavrnusKeyValueStore::GetPrefixedProjectKey(const FString& Key)
{
	const FString ProjectName = FApp::GetProjectName();
	return FString::Printf(TEXT("%s_%s"), *ProjectName, *Key);
}

bool FCavrnusKeyValueStore::KeyEntryExists(const FString& Key)
{
	FString OutValue;
	const FString ProjectKey = GetPrefixedProjectKey(Key);
	return FPlatformMisc::GetStoredValue(TEXT("Cavrnus"), TEXT("UE"), ProjectKey, OutValue);
}

bool FCavrnusKeyValueStore::DeleteKey(const FString& Key)
{
	const FString ProjectKey = GetPrefixedProjectKey(Key);
	return FPlatformMisc::DeleteStoredValue(TEXT("Cavrnus"), TEXT("UE"), ProjectKey);
}

FString FCavrnusKeyValueStore::GetStoredStringValue(const FString& Key)
{
	FString OutValue;
	const FString ProjectKey = GetPrefixedProjectKey(Key);
	FPlatformMisc::GetStoredValue(TEXT("Cavrnus"), TEXT("UE"), ProjectKey, OutValue);

	return OutValue;
}

bool FCavrnusKeyValueStore::GetStoredBoolValue(const FString& Key, const bool DefaultValue)
{
	if (!KeyEntryExists(Key))
		SetStoredBoolValue(Key, DefaultValue);
	
	FString OutValue;
	const FString ProjectKey = GetPrefixedProjectKey(Key);
	FPlatformMisc::GetStoredValue(TEXT("Cavrnus"), TEXT("UE"), ProjectKey, OutValue);

	return OutValue.ToBool();
}

bool FCavrnusKeyValueStore::SetStoredStringValue(const FString& Key, const FString& Value)
{
	const FString ProjectKey = GetPrefixedProjectKey(Key);
	return FPlatformMisc::SetStoredValue(TEXT("Cavrnus"), TEXT("UE"), ProjectKey, Value);
}

bool FCavrnusKeyValueStore::SetStoredBoolValue(const FString& Key, const bool Value)
{
	const auto StringBoolVal = Value ? TEXT("True") : TEXT("False");
	const FString ProjectKey = GetPrefixedProjectKey(Key);
	return FPlatformMisc::SetStoredValue(TEXT("Cavrnus"), TEXT("UE"), ProjectKey, StringBoolVal);
}
