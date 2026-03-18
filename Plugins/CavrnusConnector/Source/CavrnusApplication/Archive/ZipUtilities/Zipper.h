

#pragma once

#include "CoreMinimal.h"
#include "Zipper.generated.h"

UENUM(BlueprintType)
enum class EZipCompressLevel : uint8
{
	// Compression Level 0. No compression at all. Faster but bigger .zip file.
	Level0 = 0 UMETA(DisplayName = "Level 0"),

	Level1 = 1 UMETA(DisplayName = "Level 1", Tooltip = "Compression Level 1"),
	Level2 = 2 UMETA(DisplayName = "Level 2", Tooltip = "Compression Level 2"),
	Level3 = 3 UMETA(DisplayName = "Level 3", Tooltip = "Compression Level 3"),
	Level4 = 4 UMETA(DisplayName = "Level 4", Tooltip = "Compression Level 4"),
	Level5 = 5 UMETA(DisplayName = "Level 5", Tooltip = "Compression Level 5"),
	Level6 = 6 UMETA(DisplayName = "Level 6", Tooltip = "Compression Level 6"),
	Level7 = 7 UMETA(DisplayName = "Level 7", Tooltip = "Compression Level 7"),
	Level8 = 8 UMETA(DisplayName = "Level 8", Tooltip = "Compression Level 8"),

	// Compression Level 9. Highest compression level. Slower but smaller .zip file.
	Level9 = 9 UMETA(DisplayName = "Level 9")
};

UENUM(BlueprintType)
enum class EZipCreationFlag : uint8
{
	// Cancel the extraction if a file causes conflict.
	CancelIfExists,
	// Replace existing files.
	Overwrite,
	// Append the files to the existing archive.
	Append
};
typedef void(*FOnZipProgressPtr)(void*, float /*Percent*/);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnFileZippedDynamic, const FString&, ArchivePath, const FString&, FilePath, const int64, FilesZippedCount, const int64, TotalFilesCount);
DECLARE_MULTICAST_DELEGATE_FourParams(FOnFileZipped, const FString&, const FString&, const int64, const int64);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnFilesZippedDynamic, const bool, bSuccess, const FString&, ArchivePath, const int64, TotalFilesZipped);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnFilesZipped, const bool, const FString&, const int64);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnZipProgressDynamic, float, PercentComplete);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnZipProgress, float /*PercentComplete*/);
/**
 * Helper to zip files.
*/
UCLASS(BlueprintType)
class UZipper : public UObject
{
	GENERATED_BODY()
private:
	typedef void(*FOnFileZippedPtr)(void* Data, const int64, const int64, const FString&, const FString&);

public:
	UZipper();
	~UZipper();

	/**
	 * Creates a new Zipper
	 * @return A new Zipper
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Cavrnus|Zip")
	static UPARAM(DisplayName = "Zipper") UZipper* CreateZipper();

	/**
	 * Adds a file to the archive.
	 * @param FilePath The path of the file we want to add.
	 * @param ArchivePath The full path (name included) of the file in the archive.
	*/
	UFUNCTION(BlueprintCallable, Category = "Cavrnus|Zip")
	void AddFile(const FString& FilePath, const FString& ArchivePath = TEXT(""));

	/**
	 * Adds a full directory to the archive.
	 * @param Path The path of the directory to add.
	*/
	UFUNCTION(BlueprintCallable, Category = "Cavrnus|Zip")
	void AddDirectory(const FString& Path);

	/**
	 * Adds all the files in the directory matching the extension to the zip archive.
	 * @param Path The path of the directory.
	 * @param MatchingExtension The extension of the files to be added.
	*/
	UFUNCTION(BlueprintCallable, Category = "Cavrnus|Zip")
	void AddFilesInDirectoryWithExtension(const FString& Path, const FString& MatchingExtension);

	/**
	 * Clears all full directories added to the archive.
	*/
	UFUNCTION(BlueprintCallable, Category = "Cavrnus|Zip")
	void ClearDirectories();

	/**
	 * Remove a file that was added previously.
	 * @param ArchivePath The path of the file that was added previously we want to delete.
	 * @return If the file was there and was removed.
	*/
	UFUNCTION(BlueprintCallable, Category = "Cavrnus|Zip")
	UPARAM(DisplayName = "Success") bool RemoveFile(const FString& ArchivePath);

	/**
	 * Zip the files added previously into a single file.
	 * This function is asynchronous, it runs on another thread.
	 * @param TargetLocation The target location of the zip archive.
	 * @param Password The password of the zip archive. No password if blank.
	 * @param CompressLevel The compression level of the archive. 0 means no compression.
	 * @param CreationFlag What we want to do with this archive.
	 * @return If the file has been zipped.
	*/
	UFUNCTION(BlueprintCallable, Category = "Cavrnus|Zip")
	UPARAM(DisplayName = "Success") bool Zip(const FString& TargetLocation, const FString& Password = TEXT(""), const EZipCompressLevel CompressLevel = EZipCompressLevel::Level9, const EZipCreationFlag CreationFlag = EZipCreationFlag::CancelIfExists);

	/**
	 * Zip the files added previously into a single file.
	 * This function is asynchronous, it runs on another thread.
	 * @param TargetLocation The target location of the zip archive.
	 * @param Password The password of the zip archive. No password if blank.
	 * @param CompressLevel The compression level of the archive. 0 means no compression.
	 * @param CreationFlag What we want to do with this archive.
	*/
	UFUNCTION(BlueprintCallable, Category = "Cavrnus|Zip", DisplayName = "Zip Asynchronously")
	void ZipAsync(FString TargetLocation, FString Password = TEXT(""), const EZipCompressLevel CompressLevel = EZipCompressLevel::Level9, const EZipCreationFlag CreationFlag = EZipCreationFlag::CancelIfExists);

	/**
	 * @return The number of files previously added in the zipper. The directories
	 *         and their content are not included.
	*/
	UFUNCTION(BlueprintCallable, Category = "Cavrnus|Zip", BlueprintPure)
	UPARAM(DisplayName = "Count") int32 GetFilesCount() const;

	// Delegate called just after a file has been successfully zipped.
	UPROPERTY(BlueprintAssignable, Category = "Cavrnus|Zip", DisplayName = "OnFileZippedEvent")
	FOnFileZippedDynamic OnFileZippedEventDynamic;

	// Called when the zipping task is over.
	UPROPERTY(BlueprintAssignable, Category = "Cavrnus|Zip", DisplayName = "OnFilesZippedEvent")
	FOnFilesZippedDynamic OnFilesZippedEventDynamic;

	// Delegate called just after a file has been successfully zipped.
	FOnFileZipped& OnFileZipped();

	// Called when the zipping task is over.
	FOnFilesZipped& OnFilesZipped();


	FOnZipProgressDynamic OnZipProgressDynamicEvent;
	FOnZipProgress OnZipProgressEvent;

	FOnZipProgress& OnProgress(); // Accessor

	FOnFileZipped  OnFileZippedEvent;
	FOnFilesZipped OnFilesZippedEvent;

private:
	/**
	 * Check if this file is large.
	 * @param Path The path of the file to check.
	 * @return true if the file is bigger than 0xffffffff.
	*/
	static bool	IsLargeFile(const FString& Path);

	/**
	 * Get the Cyclic Redundancy Checksum of the file.
	 * @param Data The data of the file to get CRC.
	 * @return The Cyclic Redundancy Checksum.
	*/
	static unsigned long GetFileCrc(const TArray64<uint8>& Data);

	/**
	 * Add the content of the directories as files.
	*/
	static void AddDirectoriesToFiles(const TArray<FString>& Directories, TMap<FString, FString>& Files);

	/**
	 * Zip the files.
	*/
	static int64 ZipInternal(
		const FString& TargetLocation,
		const FString& Password,
		const EZipCompressLevel CompressLevel,
		const EZipCreationFlag CreationFlag,
		TMap<FString, FString>& Files,
		TArray<FString>& Directories,
		FOnFileZippedPtr FileCallback,
		void* FileCallbackData,
		FOnZipProgressPtr ProgressCallback = nullptr,
		void* ProgressCallbackData = nullptr
	);

public:
	// Call the delegate and take care of being on the good thread.
	void CallOnFileZippedEvent(const FString& ArchivePath, const FString& FilePath, const int64 FilesZipped, const int64 TotalFiles);
	// Call the delegate and take care of being on the good thread.
	void CallOnFilesZippedEvent(const bool bSuccess, const FString& ArchivePath, const int64 TotalFilesCount);

private:
	// The files we have to include in the archive.
	// Stored as { TargetFile, InputFile }.
	// Using a map directly removes duplicates.
	TMap<FString, FString> Files;

	// The list of directories to add into the archive.
	TArray<FString> Directories;

	// Number of concurrent sends.
	// Used to know if we can remove the zipper from the root.
	int64 ConcurrentialAsyncZips;
};


