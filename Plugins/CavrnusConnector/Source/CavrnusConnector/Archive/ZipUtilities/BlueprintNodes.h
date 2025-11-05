#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "BlueprintNodes.generated.h"

class UZipper;
class UUnzipper;

enum class EZipCompressLevel : uint8;
enum class EZipCreationFlag  : uint8;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUnzipProgressEvent, float, PercentComplete);

USTRUCT(BlueprintType)
struct FFileToZip
{
	GENERATED_BODY()
public:
	/**
	 * The path to the file to include in the archive.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cavrnus|Zip")
	FString FileSource;
	
	/**
	 * The path of the file in the resulting archive.
	 * Must include the file name. ("Dir/file.ext")
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cavrnus|Zip")
	FString ArchiveFullPath;
};

UCLASS(Abstract)
class UZipAsyncBase : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:
	UZipAsyncBase();

protected:

	UPROPERTY()
	UZipper* Zipper;
};

UCLASS(Abstract)
class UUnzipAsyncBase : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:
	UUnzipAsyncBase();

protected:
	UPROPERTY()
	UUnzipper* Unzipper;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FOnZipEvent, const FString&, ArchiveFileLocation, const FString&, DiskFileLocation, const int64, FilesExtractedCount, const int64, TotalFilesCount, const float, PercentComplete);

UCLASS()
class UZipDirectoryProxy : public UZipAsyncBase
{
	GENERATED_BODY()
public:

	/**
	 * Called when the directory has been zipped.
	*/
	UPROPERTY(BlueprintAssignable)
	FOnZipEvent OnDirectoryZipped;

	/**
	 * Called when an error occured and the zip file hasn't been created.
	*/
	UPROPERTY(BlueprintAssignable)
	FOnZipEvent OnError;

	/**
	 * Called when a file has been zipped.
	*/
	UPROPERTY(BlueprintAssignable)
	FOnZipEvent OnFileZipped;

	virtual void Activate();

	/** Called to report progress of the zip operation. */
	UPROPERTY(BlueprintAssignable, Category = "Cavrnus|Zip")
	FOnUnzipProgressEvent OnProgress;

	UFUNCTION()
	void OnZipProgress(float InPercent);
	/**
	 * Zip the files in the directory into a single zip file.
	 * @param DirectoryToZip The path to the directory to zip.
	 * @param ResultLocation The target location of the zip archive.
	 * @param Password The password of the zip archive. No password if blank.
	 * @param Flag 
	 * @param Compression The compression level of the archive.
	*/
	UFUNCTION(BlueprintCallable, Category = "Cavrnus|Zip", meta = (BlueprintInternalUseOnly = "true", DisplayName = "Zip Directory"))
	static UZipDirectoryProxy* ZipDirectory(const FString& DirectoryToZip, const FString& ResultLocation, const FString& Password, const EZipCreationFlag CreationFlag, const EZipCompressLevel Compression);

private:
	void OnZipOver(const bool bSuccess, const FString& ArchivePath, const int64 FilesUnzipped);
	void OnArchiveFileZipped(const FString& ArchivePath, const FString& DiskPath, const int64 FilesUnzipped, const int64 TotalFiles);

private:
	FString				Directory;
	FString				TargetLocation;
	FString				Password;
	EZipCreationFlag	Flag;
	EZipCompressLevel	Compression;
	float				Percent;
};

UCLASS()
class UZipFilesProxy : public UZipAsyncBase
{
	GENERATED_BODY()
public:
	/**
	 * Called when the directory has been zipped.
	*/
	UPROPERTY(BlueprintAssignable)
	FOnZipEvent OnDirectoryZipped;

	/**
	 * Called when an error occured and the zip file hasn't been created.
	*/
	UPROPERTY(BlueprintAssignable)
	FOnZipEvent OnError;

	/**
	 * Called when a file has been zipped.
	*/
	UPROPERTY(BlueprintAssignable)
	FOnZipEvent OnFileZipped;


	/** 
	 *Called to report progress of the zip operation. 
	 */
	UPROPERTY(BlueprintAssignable, Category = "Cavrnus|Zip")
	FOnUnzipProgressEvent OnProgress;

	virtual void Activate();

	/**
	 * Zip the files passed into a single zip file.
	 * @param FilesToZip The files we want to zip.
	 * @param ResultLocation The target location of the zip archive.
	 * @param Password The password of the zip archive. No password if blank.
	 * @param Flag 
	 * @param Compression The compression level of the archive.
	*/
	UFUNCTION(BlueprintCallable, Category = "Cavrnus|Zip", meta = (BlueprintInternalUseOnly = "true", DisplayName = "Zip Files"))
	static UZipFilesProxy* ZipFiles(const TArray<FFileToZip>& FilesToZip, const FString& ResultLocation, const FString& Password, const EZipCreationFlag CreationFlag, const EZipCompressLevel Compression);

private:
	void OnZipOver(const bool bSuccess, const FString& ArchivePath, const int64 FilesUnzipped);
	void OnArchiveFileZipped(const FString& ArchivePath, const FString& DiskPath, const int64 FilesUnzipped, const int64 TotalFiles);

private:
	TArray<FFileToZip>	Files;
	FString				TargetLocation;
	FString				Password;
	EZipCreationFlag	Flag;
	EZipCompressLevel	Compression;
	float				Percent;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FOnUnzipArchiveEvent, const FString&, ArchiveFileLocation, const FString&, DiskFileLocation, const int64, FilesExtractedCount, const int64, TotalFilesCount, const float, PercentComplete);

UCLASS()
class UUnzipArchiveProxy : public UUnzipAsyncBase
{
	GENERATED_BODY()
public:

	/**
	 * Called when the archive has been unzipped
	*/
	UPROPERTY(BlueprintAssignable)
	FOnUnzipArchiveEvent OnArchiveUnzipped;

	/**
	 * Called when an error occured.
	*/
	UPROPERTY(BlueprintAssignable)
	FOnUnzipArchiveEvent OnError;

	/**
	 * Called when a file has been unzipped.
	*/
	UPROPERTY(BlueprintAssignable)
	FOnUnzipArchiveEvent OnFileUnzipped;

	/**
	 * Called to report progress of the unzip operation.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Cavrnus|Unzip")
	FOnUnzipProgressEvent OnProgress;


	UFUNCTION()
	void OnUnzipProgress(float Percent);
public:

	/**
	 * Unzip a zip archive.
	 * @param ZipArchiveLocation The path of the archive we want to extract.
	 * @param ResultLocation Where we want to extract it.
	 * @param Password The password of the archive. Blank means no password.
	 * @param bOverwrite If we want to overwrite existing files.
	*/
	UFUNCTION(BlueprintCallable, Category = "Cavrnus|Zip", meta = (BlueprintInternalUseOnly = "true", DisplayName = "Unzip Zip Archive"))
	static UUnzipArchiveProxy* UnzipZipArchive(const FString& ZipArchiveLocation, const FString& ResultLocation, const FString& Password = TEXT(""), const bool bOverwrite = false);

	virtual void Activate();

private:
	void OnUnzipOver(const bool bSuccess, const FString& DirectoryPath, const int64 FileCount);
	void OnArchiveFileUnzipped(const FString& ArchivePath, const FString& DiskPath, const int64 FileIndex, const int64 FileCount);

private:
	FString Target;
	FString Source;
	FString Password;
	float   Percent;
	bool	bOverwrite;
};


