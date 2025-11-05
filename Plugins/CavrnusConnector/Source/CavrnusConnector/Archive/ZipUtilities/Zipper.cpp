

#include "Zipper.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "Async/Async.h"
#include "Async/TaskGraphInterfaces.h"
#include "HAL/PlatformFilemanager.h"

#ifndef _FILE_OFFSET_BITS
#	define _FILE_OFFSET_BITS 64
#endif // _FILE_OFFSET_BITS

THIRD_PARTY_INCLUDES_START
#if defined(_MSVC_VER)
#	pragma warning( push )
#	pragma warning( disable : 4668)
#endif
#	include "ThirdParty/zip.h"
#if defined(_MSVC_VER)
#	pragma warning( pop )
#endif
THIRD_PARTY_INCLUDES_END

struct FZipDirectoryVisitor : public IPlatformFile::FDirectoryVisitor
{
	FZipDirectoryVisitor(TMap<FString, FString>* InFiles, const FString& InPath)
		: Files(InFiles)
		, Path(InPath)
	{}

	virtual bool Visit(const TCHAR* FilenameOrDirectory, bool bIsDirectory)
	{
		if (!bIsDirectory)
		{
			const FString CurrentPath(FilenameOrDirectory);
			Files->Add(CurrentPath.Replace(*(Path / TEXT("")), TEXT("")), FilenameOrDirectory);
		}
		return true;
	}

private:
	TMap<FString, FString>* Files;
	FString Path;
};

UZipper* UZipper::CreateZipper()
{
	return NewObject<UZipper>();
}

UZipper::UZipper()
	: ConcurrentialAsyncZips(0)
{
}

UZipper::~UZipper()
{
}

FOnZipProgress& UZipper::OnProgress()
{
	return OnZipProgressEvent;
}

void UZipper::AddFile(const FString& FilePath, const FString& ArchiveLocation)
{	
	const FString FilePathAbs = FPaths::ConvertRelativePathToFull(FilePath);
	if (ArchiveLocation.Len() <= 0)
	{
		Files.Add(FPaths::GetCleanFilename(FilePath), FilePathAbs);
	}
	else
	{
		if (ArchiveLocation[0] == TEXT('/'))
		{
			const FString SanitizedPath = ArchiveLocation.Mid(1);
			Files.Add(SanitizedPath, FilePath);
		}
		else
		{
			Files.Add(ArchiveLocation, FilePathAbs);
		}
	}
}

void UZipper::AddFilesInDirectoryWithExtension(const FString& Path, const FString& MatchingExtension)
{
	if (!FPaths::DirectoryExists(Path))
	{
		UE_LOG(LogTemp, Error, TEXT("AddFilesInDirectoryWithExtension(): Directory %s does not exist."), *Path);
		return;
	}

	struct FZipVisitor : public IPlatformFile::FDirectoryVisitor
	{
		FZipVisitor(TMap<FString, FString>& InFiles, const FString& Extension, const FString& Path)
			: LocalFiles(InFiles)
			, LocalExtension(Extension)
			, LocalPath(Path)
		{}

		virtual bool Visit(const TCHAR* FilenameOrDirectory, bool bIsDirectory)
		{
			if (!bIsDirectory)
			{
				const FString CurrentPath(FilenameOrDirectory);
				if (FPaths::GetExtension(CurrentPath) == LocalExtension)
				{
					LocalFiles.Add(CurrentPath.Replace(*(LocalPath / TEXT("")), TEXT("")), FilenameOrDirectory);
				}
			}
			return true;
		}

	private:
		TMap<FString, FString>& LocalFiles;
		const FString& LocalExtension;
		const FString& LocalPath;
	} Visitor(Files, MatchingExtension, Path);

	const int32 FilesCount = Files.Num();

	FPlatformFileManager::Get().GetPlatformFile().IterateDirectoryRecursively(*Path, Visitor);

	UE_LOG(LogTemp, Log, TEXT("Added all files with extension \"%s\" in directory \"%s\". Total files added: %d."), 
		*MatchingExtension, *Path, Files.Num() - FilesCount);
}

void UZipper::ClearDirectories()
{
	Directories.Empty();
}

void UZipper::AddDirectory(const FString& Path)
{
	Directories.Add(FPaths::ConvertRelativePathToFull(Path));
}

bool UZipper::RemoveFile(const FString& ArchivePath)
{
	return Files.Remove(ArchivePath) > 0;
}

bool UZipper::Zip(const FString& TargetLocation, const FString& Password, const EZipCompressLevel CompressLevel, const EZipCreationFlag CreationFlag)
{
	const auto FileCallback = [](void* Data, const int64 A, const int64 B, const FString& ArchivePath, const FString& DiskPath) -> void
	{
		static_cast<UZipper*>(Data)->CallOnFileZippedEvent(ArchivePath, DiskPath, A, B);
	};

	const int64 bZipSuccess = ZipInternal(TargetLocation, Password, CompressLevel, CreationFlag, Files, Directories, FileCallback, this);
	
	ConcurrentialAsyncZips++;
	CallOnFilesZippedEvent((bool)bZipSuccess, TargetLocation, bZipSuccess);
	
	return (bool)bZipSuccess;
}

// Method doesn't exist in 4.23
namespace FCustomFileHelper
{
	bool LoadFileToArray(TArray64<uint8>& Result, const TCHAR* Filename, uint32 Flags)
	{
		FScopedLoadingState ScopedLoadingState(Filename);

		FArchive* const Reader = IFileManager::Get().CreateFileReader(Filename, Flags);
		if (!Reader)
		{
			if (!(Flags & FILEREAD_Silent))
			{
				UE_LOG(LogStreaming, Warning, TEXT("Failed to read file '%s' error."), Filename);
			}
			return false;
		}
		int64 TotalSize = Reader->TotalSize();
		// Allocate slightly larger than file size to avoid re-allocation when caller null terminates file buffer
		Result.Reset(TotalSize + 2);
		Result.AddUninitialized(TotalSize);
		Reader->Serialize(Result.GetData(), Result.Num());
		const bool Success = Reader->Close();
		delete Reader;
		return Success;
	}
}


int64 UZipper::ZipInternal(
	const FString& TargetLocation,
	const FString& Password,
	const EZipCompressLevel CompressLevel,
	const EZipCreationFlag CreationFlag,
	TMap<FString, FString>& Files,
	TArray<FString>& Directories,
	FOnFileZippedPtr FileCallback,
	void* FileCallbackData,
	FOnZipProgressPtr ProgressCallback,
	void* ProgressCallbackData)
{
	const uint8 Compression = static_cast<uint8>(CompressLevel);

	UE_LOG(LogTemp, Log, TEXT("Zipping: Target: \"%s\", %d file(s), With Password: %d, Compression Level: %d."), *TargetLocation, Files.Num(), !Password.IsEmpty(), Compression);

	zipFile ZipFile;

	int OpenFlags = APPEND_STATUS_CREATE;

	if (FPaths::FileExists(TargetLocation))
	{
		switch (CreationFlag)
		{
		case EZipCreationFlag::Append:
			UE_LOG(LogTemp, Log, TEXT("Target archive exists, files will be added to the archive."));
			OpenFlags = APPEND_STATUS_ADDINZIP;
			break;
		case EZipCreationFlag::Overwrite:
			if (!FPlatformFileManager::Get().GetPlatformFile().DeleteFile(*TargetLocation))
			{
				UE_LOG(LogTemp, Error, TEXT("Failed to overwrite existing archive."));
				return false;
			}
			UE_LOG(LogTemp, Log, TEXT("Target archive exists, overwriting old archive."))
			break;
		case EZipCreationFlag::CancelIfExists:
			UE_LOG(LogTemp, Warning, TEXT("Target archive exists. Canceling."))
			return false;
		}
	}

	ZipFile = zipOpen(TCHAR_TO_UTF8(*TargetLocation), OpenFlags);

	if (ZipFile == NULL)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to zip files: can't create or open file \"%s\"."), *TargetLocation);
		return false;
	}

	zip_fileinfo ZipInfo;
	FMemory::Memzero(ZipInfo);

	AddDirectoriesToFiles(Directories, Files);
	
	int32 FilesZippedCount = 0;

	FTCHARToUTF8 PasswordUTF8(*Password);
	const char* const PasswordChr = Password.IsEmpty() ? nullptr : PasswordUTF8.Get();

	for (const auto& File : Files)
	{
		TArray64<uint8> FileData;
		if (!FCustomFileHelper::LoadFileToArray(FileData, *File.Value, 0))
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to open file \"%s\" for reading."), *FPaths::ConvertRelativePathToFull(File.Value));
			continue;
		}

		unsigned long CrcFile = 0;
		if (!Password.IsEmpty())
		{
			CrcFile = GetFileCrc(FileData);
		}

		const bool bUseZip64 = IsLargeFile(File.Value);


		const int32 Err = zipOpenNewFileInZip3_64
		(
			ZipFile, 
			TCHAR_TO_UTF8(*File.Key), 
			&ZipInfo, 
			nullptr, 0, nullptr, 0, nullptr, 
			(Compression != 0 ? Z_DEFLATED : 0),
			Compression, 0,
			-MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY, 
			PasswordChr,
			CrcFile, (int)bUseZip64
		);

		if (Err != ZIP_OK)
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to open file \"%s\" with zipOpenNewFileInZip3_64(). Code: %d."), *File.Key, Err);
			continue;
		}

		if (zipWriteInFileInZip(ZipFile, FileData.GetData(), FileData.Num()) != ZIP_OK)
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to write data into archive for file \"%s\"."), *File.Key);
			continue;
		}

		UE_LOG(LogTemp, Log, TEXT("File \"%s\" added to the archive with path \"%s\"."), *File.Value, *File.Key);

		FilesZippedCount++;

		float LastBroadcastedPercent = -1.0f;
		if (ProgressCallback)
		{
			const float Percent = (float)(FilesZippedCount) / (float)Files.Num();
			if (Percent - LastBroadcastedPercent >= 0.01f)
			{
				LastBroadcastedPercent = Percent;
				ProgressCallback(ProgressCallbackData, Percent);
			}
		}
		if (FileCallback)
		{
			FileCallback(FileCallbackData, FilesZippedCount + 1, Files.Num(), File.Key, File.Value);
		}
	}

	if (zipClose(ZipFile, NULL) != ZIP_OK)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to close \"%s\"."), *TargetLocation);
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("Archive \"%s\" created with %d file(s). %d/%d file(s) zipped."), *TargetLocation, FilesZippedCount, FilesZippedCount, Files.Num());

	return FilesZippedCount;
}

static void BroadcastFileZipped(void* Data, const int64 FilesZipped, const int64 TotalFiles, const FString& ArchivePath, const FString& DiskPath)
{
	TWeakObjectPtr<UZipper>& Self = *(TWeakObjectPtr<UZipper>*)Data;
	AsyncTask(ENamedThreads::GameThread, [Self, FilesZipped, TotalFiles, ArchivePath, DiskPath]()
		{
			if (Self.IsValid())
			{
				Self->CallOnFileZippedEvent(ArchivePath, DiskPath, FilesZipped, TotalFiles);
			}
		});
}

static void BroadcastZipProgress(void* Data, float Percent)
{
	TWeakObjectPtr<UZipper>& Self = *(TWeakObjectPtr<UZipper>*)Data;
	AsyncTask(ENamedThreads::GameThread, [Self, Percent]()
		{
			if (Self.IsValid())
			{
				Self->OnZipProgressEvent.Broadcast(Percent);
				Self->OnZipProgressDynamicEvent.Broadcast(Percent);
			}
		});
}

void UZipper::ZipAsync(FString TargetLocation, FString Password, const EZipCompressLevel CompressLevel, const EZipCreationFlag CreationFlag)
{
	ConcurrentialAsyncZips++;

	AddToRoot();

	TMap<FString, FString>& Files_ = Files;
	TArray<FString>& Directories_ = Directories;

	const bool bCallFileCallback = OnFileZippedEvent.IsBound();
	const bool bCallProgressCallback = OnZipProgressEvent.IsBound() || OnZipProgressDynamicEvent.IsBound();

	TWeakObjectPtr<UZipper> This = this;

	// Static helpers defined at file scope in Zipper.cpp
	extern void BroadcastFileZipped(void* Data, const int64 FilesZipped, const int64 TotalFiles, const FString & ArchivePath, const FString & DiskPath);
	extern void BroadcastZipProgress(void* Data, float Percent);

	FOnFileZippedPtr FileCallbackPtr = bCallFileCallback ? &BroadcastFileZipped : nullptr;
	void* FileCallbackData = bCallFileCallback ? (void*)&This : nullptr;

	FOnZipProgressPtr ProgressCallbackPtr = bCallProgressCallback ? &BroadcastZipProgress : nullptr;
	void* ProgressCallbackData = bCallProgressCallback ? (void*)&This : nullptr;

	AsyncTask(ENamedThreads::AnyThread, [This, TargetLocation = MoveTemp(TargetLocation), Password = MoveTemp(Password),
		CompressLevel, CreationFlag, Files_, Directories_,
		FileCallbackPtr, FileCallbackData,
		ProgressCallbackPtr, ProgressCallbackData]() mutable
		{
			const int64 bSuccess = ZipInternal(
				TargetLocation,
				Password,
				CompressLevel,
				CreationFlag,
				Files_,
				Directories_,
				FileCallbackPtr,
				FileCallbackData,
				ProgressCallbackPtr,
				ProgressCallbackData
			);

			AsyncTask(ENamedThreads::GameThread, [This, bSuccess, TargetLocation]()
				{
					if (This.IsValid())
					{
						This->CallOnFilesZippedEvent((bool)bSuccess, TargetLocation, bSuccess);
					}
				});
		});
}



void UZipper::AddDirectoriesToFiles(const TArray<FString>& Directories, TMap<FString, FString>& Files)
{
	if (Directories.Num() <= 0)
	{
		return;
	}

	for (const FString& Directory : Directories)
	{
		FZipDirectoryVisitor Visitor(&Files, Directory);
		FPlatformFileManager::Get().GetPlatformFile().IterateDirectoryRecursively(*Directory, Visitor);
	}
}

bool UZipper::IsLargeFile(const FString& Path)
{
	return IFileManager::Get().FileSize(*Path) >= 0xffffffff;
}

unsigned long UZipper::GetFileCrc(const TArray64<uint8>& Data)
{
	unsigned long Crc = 0;
	
	Crc = crc32(Crc, Data.GetData(), Data.Num());

	return Crc;
}

int32 UZipper::GetFilesCount() const
{
	return Files.Num();
}


FOnFileZipped& UZipper::OnFileZipped()
{
	return OnFileZippedEvent;
}

FOnFilesZipped& UZipper::OnFilesZipped()
{
	return OnFilesZippedEvent;
}

void UZipper::CallOnFileZippedEvent(const FString& ArchivePath, const FString& FilePath, const int64 FilesZipped, const int64 TotalFiles)
{
	check(IsInGameThread());

	OnFileZippedEventDynamic.Broadcast(ArchivePath, FilePath, FilesZipped, TotalFiles);
	OnFileZippedEvent.Broadcast(ArchivePath, FilePath, FilesZipped, TotalFiles);
}

void UZipper::CallOnFilesZippedEvent(const bool bSuccess, const FString& ArchivePath, const int64 TotalFilesCount)
{
	check(IsInGameThread());

	if ((--ConcurrentialAsyncZips) <= 0)
	{
		RemoveFromRoot();
	}

	OnFilesZippedEventDynamic.Broadcast(bSuccess, ArchivePath, TotalFilesCount);
	OnFilesZippedEvent.Broadcast(bSuccess, ArchivePath, TotalFilesCount);
}


