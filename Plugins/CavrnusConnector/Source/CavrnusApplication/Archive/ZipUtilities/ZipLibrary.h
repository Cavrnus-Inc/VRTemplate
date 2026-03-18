

#pragma once

#include "Zipper.h"
#include "Unzipper.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ZipLibrary.generated.h"

/**
 * Library that uses the Zipper and Unzipper internally.
*/
UCLASS()
class UZipLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:

	/**
	 * Zip a whole directory into a zip file.
	 * @param FolderPath The Directory we want to zip.
	 * @param TargetLocation Where we want to zip it. (i.e. "C:/Folder/Archive.zip")
	 * @param Password The password of the archive. Blank means no password.
	 * @param CompressLevel The compression level.
	 * @param CreationFlag What to do in case of conflict.
	 * @param OnFilesZipped Callback when the archive is created.
	 * @param OnFileZipped Callback when a file has been zipped into the archive.
	*/
	static void ZipDirectory
	(
		const FString& DirectoryPath, 
		const FString& TargetLocation, 
		const FString& Password = TEXT(""), 
		const EZipCompressLevel CompressLevel = EZipCompressLevel::Level9, 
		const EZipCreationFlag  CreationFlag  = EZipCreationFlag::CancelIfExists, 
		const FOnFilesZipped&   OnFilesZipped = FOnFilesZipped(),
		const FOnFileZipped &   OnFileZipped  = FOnFileZipped()
	);

	/**
	 * Unzip a zip archive. 
	 * @param ArchivePath The path of the archive we want to extract. 
	 * @param TargetLocation Where we want to extract it.
	 * @param Password The password of the archive. Blank means no password.
	 * @param bForce If we want to overwrite existing files.
	 * @param OnFilesUnzipped Called when files are unzipped.
	 * @param OnFileUnzipped Called when a file has been unzipped from the archive.
	*/
	static void UnzipArchive
	(
		const FString& ArchivePath,
		const FString& TargetLocation,
		const FString& Password = TEXT(""),
		const bool bForce = false,
		const FOnFilesUnzipped& OnFilesUnzipped = FOnFilesUnzipped(),
		const FOnFileUnzipped & OnFileUnzipped  = FOnFileUnzipped()
	);
};

