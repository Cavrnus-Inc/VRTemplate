#pragma once

#include "CoreMinimal.h"
#include "Unzipper.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnFileUnzippedDynamic, const FString&, ArchiveLocation, const FString&, DiskLocation, const int64, FilesZippedCount, const int64, TotalFilesCount);
DECLARE_MULTICAST_DELEGATE_FourParams(FOnFileUnzipped, const FString&, const FString&, const int64, const int64);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnFilesUnzippedDynamic, const bool, bSuccess, const FString&, DirectoryPath, const int64, FilesZippedCount);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnFilesUnzipped, const bool, const FString&, const int64);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUnzipProgressDynamic, float, PercentComplete);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnUnzipProgress, float /*PercentComplete*/);
/**
 * Helper to unzip files.
*/
UCLASS(BlueprintType)
class UUnzipper : public UObject
{
	GENERATED_BODY()
public:

	UUnzipper();
	
	FOnUnzipProgress OnUnzipProgressEvent;
	FOnUnzipProgressDynamic OnUnzipProgressDynamicEvent;

	FOnUnzipProgress& OnProgress(); // Accessor
	typedef void(*FOnUnzipProgressPtr)(void*, float /*Percent*/);
	/**
	 * Creates a new Unzipper
	 * @return A new Unzipper
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Cavrnus|Unzip")
	static UPARAM(DisplayName = "Unzipper") UUnzipper* CreateUnzipper();

	/**
	 * Sets the archive we are going to extract.
	 * @param Path The path of the archive we are going to extract.
	*/
	UFUNCTION(BlueprintCallable, Category = "Cavrnus|Unzip")
	void SetArchive(FString Path);

	/**
	 * Tests if the archive exists.
	 * @return If the archive exists and extraction might begin.
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Cavrnus|Unzip")
	UPARAM(DisplayName = "Valid") bool IsValid() const;

	/**
	 * Unzip all files of the archive set with `SetArchive`.
	 * @param TargetLocation Where you want to extract the archive.
	 * @param Password This archive's password. Leave it blank if no password.
	 * @param bForce If we should overwrite existing files.
	 * @return If the extraction succeeded.
	*/
	bool UnzipAll(const FString& TargetLocation, const FString& Password = TEXT(""), const bool bForce = false);

	/**
	 * Unzip all files of the archive set with `SetArchive` asynchronously.
	 * @param TargetLocation Where you want to extract the archive.
	 * @param Password This archive's password. Leave it blank if no password.
	 * @param bForce If we should overwrite existing files.
	 * @return If the extraction succeeded.
	*/
	UFUNCTION(BlueprintCallable, Category = "Cavrnus|Unzip")
	void UnzipAllAsync(FString TargetLocation, FString Password = TEXT(""), const bool bForce = false);

	/**
	 * Change the size of the buffer used to read archives.
	 * A bigger buffer might increase perfomance when extracting large files.
	 * Default buffer size is 8192 bytes.
	 * @param NewBufferSize The new size of the buffer in bytes.
	*/
	UFUNCTION(BlueprintCallable, Category = "Cavrnus|Unzip")
	void SetBufferSize(const int64& NewBufferSize);

	/**
	 * Delegate called when unzipping is over.
	*/
	UPROPERTY(BlueprintAssignable, Category = "Cavrnus|Unzip", DisplayName = "OnFilesUnzipped")
	FOnFilesUnzippedDynamic OnFilesUnzippedDynamicEvent;

	/**
	 * Delegate called just after a file has been unzipped from the archive.
	*/
	UPROPERTY(BlueprintAssignable, Category = "Cavrnus|Unzip", DisplayName = "OnFileUnzipped")
	FOnFileUnzippedDynamic OnFileUnzippedDynamicEvent;


	/**
	 * Delegate called just after a file has been unzipped from the archive.
	*/
	FOnFileUnzipped & OnFileUnzipped();


	/**
	 * Delegate called when unzipping is over.
	*/
	FOnFilesUnzipped& OnFilesUnzipped();

private:
	typedef void(*FOnFileUnzippedPtr)(void*, const int64, const int64, const FString&, const FString&);

	/**
	 * Unzip a file inside an opened archive.
	 * @param TargetLocation The extraction location.
	 * @param File The opened archive pointer.
	 * @param Buffer The buffer to read the archive.
	 * @param InBufferSize The size of the buffer.
	 * @return If we should keep extracting the archive.
	*/
	static bool UnzipCurrentFile(const int64 FileIndex, const int64 TotalFileCount, const FString& TargetLocation, const FString& ArchivePassword, const bool bForceUnzip, void* const File, void* Buffer, const int64 InBufferSize, FOnFileUnzippedPtr Callback, void* CallbackData);

	/**
	 * Unzip all files of the archive set with `SetArchive`.
	 * @param TargetLocation Where you want to extract the archive.
	 * @param Password This archive's password. Leave it blank if no password.
	 * @param bForce If we should overwrite existing files.
	 * @return The number of files of the archive or 0 if extraction failed.
	*/
	static int64 UnzipAllInternal(
		const FString& TargetLocation,
		const FString& ArchivePath,
		const FString& Password,
		const bool bForce,
		const int64 BufferSize,
		FOnFileUnzippedPtr FileCallback,
		void* FileCallbackData,
		FOnUnzipProgressPtr ProgressCallback = nullptr,
		void* ProgressCallbackData = nullptr);
	/**
	 * Broadcast the event.
	 * @param bSuccess If the extraction succeeded.
	*/
	void OnUnzipCompleted(const int64 TotalFileCount, const FString& DirectoryPath, const bool bSuccess);

	/**
	 * Broadcast the event.
	 * @param ArchiveLocation The location of the file in the archive.
	 * @param DiskLocation The location of the file on the disk.
	*/
	void OnFileUnzipCompleted(const int64 ExtractedCount, const int64 TotalFileCount, const FString& ArchiveLocation, const FString& DiskLocation);

private:
	FOnFileUnzipped  OnFileUnzippedEvent;
	FOnFilesUnzipped OnFilesUnzippedEvent;

private:
	FString ArchivePath;
	int64 BufferSize;
};
