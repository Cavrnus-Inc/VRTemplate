#include "Unzipper.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "Async/Async.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"

THIRD_PARTY_INCLUDES_START
#pragma warning( push )
#pragma warning( disable : 4668)
#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#include "ThirdParty/iowin32.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif
#include "ThirdParty/unzip.h"
#pragma warning( pop )
THIRD_PARTY_INCLUDES_END

UUnzipper* UUnzipper::CreateUnzipper()
{
    return NewObject<UUnzipper>();
}

UUnzipper::UUnzipper()
    : BufferSize(8192)
{
}

void UUnzipper::SetBufferSize(const int64& NewBufferSize)
{
    ensureMsgf(NewBufferSize > 0, TEXT("Buffer Size must be bigger than 0."));
    BufferSize = NewBufferSize;
}

void UUnzipper::SetArchive(FString Path)
{
    ArchivePath = FPaths::ConvertRelativePathToFull(MoveTemp(Path));
}

bool UUnzipper::IsValid() const
{
    return FPaths::FileExists(ArchivePath);
}

bool UUnzipper::UnzipAll(const FString& TargetLocation, const FString& Password, const bool bForce)
{
    const auto Callback = [](void* Data, const int64 FileIndex, const int64 TotalFileCount, const FString& ArchiveLocation, const FString& DiskLocation)
        {
            UUnzipper* const This = (UUnzipper*)Data;
            This->OnFileUnzipCompleted(FileIndex, TotalFileCount, ArchiveLocation, DiskLocation);
        };

    const int64 bUnzipSuccess = UnzipAllInternal(TargetLocation, ArchivePath, Password, bForce, BufferSize, Callback, this);
    OnUnzipCompleted(bUnzipSuccess, TargetLocation, (bool)bUnzipSuccess);
    return (bool)bUnzipSuccess;
}

void UUnzipper::UnzipAllAsync(FString TargetLocation, FString Password, const bool bForce)
{
    if (!IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("File \"%s\" doesn't exist."), *ArchivePath);
        OnUnzipCompleted(0, TargetLocation, false);
        return;
    }

    AddToRoot();

    FString Archive = FPaths::ConvertRelativePathToFull(ArchivePath);
    const int64 Buffer = BufferSize;
    const bool bCallFileCallback = OnFileUnzippedDynamicEvent.IsBound() || OnFileUnzippedEvent.IsBound();

    TWeakObjectPtr<UUnzipper> This = TWeakObjectPtr<UUnzipper>(this);

    AsyncTask(ENamedThreads::AnyThread, [This, TargetLocation = MoveTemp(TargetLocation),
        Archive = MoveTemp(Archive), Password = MoveTemp(Password), bForce, Buffer, bCallFileCallback]() mutable
        {
            FOnFileUnzippedPtr FileCallback = [](void* Data, const int64 FileIndex, const int64 TotalFileCount, const FString& ArchiveLocation, const FString& DiskLocation)
                {
                    TWeakObjectPtr<UUnzipper>& Self = *(TWeakObjectPtr<UUnzipper>*)Data;
                    AsyncTask(ENamedThreads::GameThread, [Self, ArchiveLocation, DiskLocation, TotalFileCount, FileIndex]()
                        {
                            if (Self.IsValid())
                            {
                                Self->OnFileUnzipCompleted(FileIndex, TotalFileCount, ArchiveLocation, DiskLocation);
                            }
                        });
                };

            auto ProgressCallback = [](void* Data, float Percent)
                {
                    TWeakObjectPtr<UUnzipper>& Self = *(TWeakObjectPtr<UUnzipper>*)Data;
                    AsyncTask(ENamedThreads::GameThread, [Self, Percent]()
                        {
                            if (Self.IsValid())
                            {
                                Self->OnUnzipProgressEvent.Broadcast(Percent);
                                Self->OnUnzipProgressDynamicEvent.Broadcast(Percent);
                            }
                        });
                };

            const int64 bUnzipSuccess = UnzipAllInternal(TargetLocation, Archive, Password, bForce, Buffer,
                bCallFileCallback ? FileCallback : nullptr, bCallFileCallback ? (void*)&This : nullptr,
                ProgressCallback, (void*)&This);

            AsyncTask(ENamedThreads::GameThread, [This, bUnzipSuccess, TargetLocation]()
                {
                    if (This.IsValid())
                    {
                        This->OnUnzipCompleted(bUnzipSuccess, TargetLocation, (bool)bUnzipSuccess);
                        This->RemoveFromRoot();
                    }
                    else
                    {
                        UE_LOG(LogTemp, Error, TEXT("Unzipper got garbage collected while unzipping."));
                    }
                });
        });
}

int64 UUnzipper::UnzipAllInternal(const FString& TargetLocation, const FString& ArchivePath, const FString& Password, const bool bForce, const int64 BufferSize,
    FOnFileUnzippedPtr FileCallback, void* FileCallbackData,
    FOnUnzipProgressPtr ProgressCallback, void* ProgressCallbackData)
{
#if PLATFORM_WINDOWS
    zlib_filefunc64_def Ffunc;
    fill_win32_filefunc64W(&Ffunc);
    unzFile File = unzOpen2_64(TCHAR_TO_WCHAR(*ArchivePath), &Ffunc);
#else
    unzFile File = unzOpen64(TCHAR_TO_UTF8(*ArchivePath));
    if (!File)
    {
        File = unzOpen64(TCHAR_TO_WCHAR(*ArchivePath));
    }
#endif

    if (!File)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to open archive \"%s\"."), *ArchivePath);
        return 0;
    }

    unz_global_info64 GlobalInfo;
    if (unzGetGlobalInfo64(File, &GlobalInfo) != UNZ_OK)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get archive info for \"%s\"."), *ArchivePath);
        unzClose(File);
        return 0;
    }

    const int64 FileCount = (int64)GlobalInfo.number_entry;
    void* Buffer = FMemory::Malloc(BufferSize);
    float LastBroadcastedPercent = -1.0f;

    const bool bSuccess = ([&]() -> bool
        {
            for (int64 i = 0; i < FileCount; ++i)
            {
                if (!UnzipCurrentFile(i + 1, FileCount, TargetLocation, Password, bForce, File, Buffer, BufferSize, FileCallback, FileCallbackData))
                {
                    return false;
                }

                if (i < FileCount - 1 && unzGoToNextFile(File) != UNZ_OK)
                {
                    UE_LOG(LogTemp, Error, TEXT("Failed to advance to next file in archive."));
                    return false;
                }

                if (ProgressCallback)
                {
                    const float Percent = (float)(i + 1) / (float)FileCount;
                    if (Percent - LastBroadcastedPercent >= 0.01f)
                    {
                        LastBroadcastedPercent = Percent;
                        ProgressCallback(ProgressCallbackData, Percent);
                    }
                }
            }
            return true;
        })();

    FMemory::Free(Buffer);
    unzClose(File);

    if (!bSuccess)
    {
        UE_LOG(LogTemp, Error, TEXT("Unzip failed for \"%s\"."), *ArchivePath);
        return 0;
    }

    return FileCount;
}

bool UUnzipper::UnzipCurrentFile(const int64 FileIndex, const int64 TotalFileCount, const FString& TargetLocation, const FString& ArchivePassword, const bool bForceUnzip,
    void* const File, void* Buffer, const int64 InBufferSize, FOnFileUnzippedPtr Callback, void* CallbackData)
{
    unz_file_info64 FileInfo;
    char RawFileName[512];
    if (unzGetCurrentFileInfo64(File, &FileInfo, RawFileName, sizeof(RawFileName), nullptr, 0, nullptr, 0) != UNZ_OK)
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to get file info."));
        return false;
    }

    FString FileName = UTF8_TO_TCHAR(RawFileName);
    FString CleanFileName = FPaths::GetCleanFilename(FileName);
    if (CleanFileName.IsEmpty())
    {
        FPlatformFileManager::Get().GetPlatformFile().CreateDirectory(*(TargetLocation / FileName));
        return true;
    }

    FString WriteLocation = TargetLocation / FileName;
    if (FPaths::FileExists(WriteLocation) && !bForceUnzip)
    {
        UE_LOG(LogTemp, Warning, TEXT("File \"%s\" exists. Skipping."), *WriteLocation);
        return true;
    }

    if (bForceUnzip && FPaths::FileExists(WriteLocation))
    {
        FPlatformFileManager::Get().GetPlatformFile().DeleteFile(*WriteLocation);
    }

    FTCHARToUTF8 PasswordUTF8(*ArchivePassword);
    const char* PasswordChr = ArchivePassword.IsEmpty() ? nullptr : PasswordUTF8.Get();

    if (unzOpenCurrentFilePassword(File, PasswordChr) != UNZ_OK)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to open file \"%s\" with password."), *FileName);
        return false;
    }

    int32 Error = 1;
    {
        TUniquePtr<FArchive> Ar = TUniquePtr<FArchive>(IFileManager::Get().CreateFileWriter(*WriteLocation));

        if (!Ar)
        {
            Error = -254;
            UE_LOG(LogTemp, Error, TEXT("Failed to create archive to write to \"%s\""), *WriteLocation);
        }

        while (Error > 0)
        {
            Error = unzReadCurrentFile(File, Buffer, InBufferSize);

            if (Error < 0)
            {
                break;
            }

            if (Error > 0)
            {
                Ar->Serialize(Buffer, Error);
            }
        }

        if (Ar)
        {
            Ar->Close();
        }

        if (Error != UNZ_OK)
        {
            UE_LOG(LogTemp, Warning, TEXT("Error while reading file \"%s\". Code: %d."), *FileName, Error);
        }
    }

    {
        const int32 CloseError = unzCloseCurrentFile(File);
        if (CloseError != UNZ_OK)
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to close file \"%s\". Code: %d."), *FileName, CloseError);
        }
    }

    if (Callback)
    {
        Callback(CallbackData, FileIndex, TotalFileCount, FileName, WriteLocation);
    }

    return Error == UNZ_OK;
}

FOnUnzipProgress& UUnzipper::OnProgress()
{
    return OnUnzipProgressEvent;
}

void UUnzipper::OnUnzipCompleted(const int64 TotalFileCount, const FString& DirectoryPath, const bool bSuccess)
{
    check(IsInGameThread());

    if (IsRooted())
    {
        RemoveFromRoot();
    }

    OnFilesUnzippedEvent.Broadcast(bSuccess, DirectoryPath, TotalFileCount);
    OnFilesUnzippedDynamicEvent.Broadcast(bSuccess, DirectoryPath, TotalFileCount);
}

void UUnzipper::OnFileUnzipCompleted(const int64 FileIndex, const int64 TotalFileCount, const FString& ArchiveLocation, const FString& DiskLocation)
{
    check(IsInGameThread());

    OnFileUnzippedEvent.Broadcast(ArchiveLocation, DiskLocation, FileIndex, TotalFileCount);
    OnFileUnzippedDynamicEvent.Broadcast(ArchiveLocation, DiskLocation, FileIndex, TotalFileCount);
}

FOnFileUnzipped& UUnzipper::OnFileUnzipped()
{
    return OnFileUnzippedEvent;
}

FOnFilesUnzipped& UUnzipper::OnFilesUnzipped()
{
    return OnFilesUnzippedEvent;
}
