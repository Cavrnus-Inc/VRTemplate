// Copyright (c) 2025 Cavrnus. All rights reserved.

/**
 * @file CavrnusFileUtilityLibrary.h
 * @brief Declaration of the UCavrnusFileUtilityLibrary class, which provides a set of static utility functions
 * related to file system reads and writes for the Cavrnus Spatial Connector plugin in Unreal Engine 5.
 */

#pragma once

#include <CoreMinimal.h>
#include <Kismet/BlueprintFunctionLibrary.h>

#include "CavrnusFileUtilityLibrary.generated.h"	// Always last

UCLASS(Abstract)
class CAVRNUSCONNECTOR_API UCavrnusFileUtilityLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	// Functions
public:
	/**
	 * @brief Open a file browser to select one or more file(s)
	 * @param MultiSelect Set whether the user can select more than one target.
	 * @param FileTypes String that defines the file types that should appear.
	 * @return true if one or more valid targets was returned.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Content",
		meta = (ToolTip = "Selects one or more files for upload", ShortToolTip = "Open an OS File Picker"))
	static bool OpenFileDialog(bool MultiSelect, FString FileTypes, TArray<FString>& OutFileNames);

	/**
	 * @brief Open a file browser to select one or more folder(s)
	 * @param MultiSelect Set whether the user can select more than one target.
	 * @return true if one or more valid targets was returned.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Content",
		meta = (ToolTip = "Selects a folder for upload", ShortToolTip = "Open an OS Folder Picker"))
	static bool OpenFolderDialog(TArray<FString>& OutFolderNames);
};
