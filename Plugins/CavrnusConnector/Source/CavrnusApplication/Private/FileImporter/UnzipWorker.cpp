
#include "FileImporter/UnzipWorker.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Async/Async.h"

// This is the critical one:
#include "unzip.h"

// If you want verbose logging macros:
#include "FileImporter/CavrnusBaseLoader.h" // for LOG_CAVRNUS_VERBOSE

//////////////////////////////////////////////////////////////////////////
// Simple synchronous helpers used by workers (non-UObject code)

// Prefer using ZipLibrary's sync APIs on worker threads to avoid UObjects in workers.
// Here we expose a thin wrapper used by background tasks.
// Worker_UnzipAll: synchronous unzip using MiniZip (unz* API)
// Safe to run on a worker thread. Returns success flag, extracted file list, and error string.
bool Worker_UnzipAll(const FString& ZipPath,
    const FString& DestFolder,
    TArray<FString>& OutFiles,
    FString& OutError,
    TFunction<void(const FString& FileName, float Progress)> OnFileExtracted)
{
    OutFiles.Reset();

    if (!FPaths::FileExists(ZipPath))
    {
        OutError = FString::Printf(TEXT("Zip not found: %s"), *ZipPath);
        return false;
    }

    IFileManager::Get().MakeDirectory(*DestFolder, true);

    // Open archive (minizip)
    unzFile ZipHandle = unzOpen(TCHAR_TO_UTF8(*ZipPath));
    if (!ZipHandle)
    {
        OutError = FString::Printf(TEXT("Failed to open archive: %s"), *ZipPath);
        UE_LOG(LogTemp, Error, TEXT("%s"), *OutError);
        return false;
    }

    auto CleanupAndReturn = [&](bool bSuccess)
        {
            unzClose(ZipHandle);
            return bSuccess;
        };

    bool bOk = true;

    // Count files
    int32 TotalFiles = 0;
    if (unzGoToFirstFile(ZipHandle) == UNZ_OK)
    {
        do { ++TotalFiles; } while (unzGoToNextFile(ZipHandle) == UNZ_OK);
        unzGoToFirstFile(ZipHandle);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Worker_UnzipAll: unzGoToFirstFile returned non-OK for '%s'"), *ZipPath);
    }

    int32 CurrentIndex = 0;
    const int32 ChunkSize = 256 * 1024;
    TArray<uint8> Chunk;
    Chunk.SetNumUninitialized(ChunkSize);

    if (unzGoToFirstFile(ZipHandle) == UNZ_OK)
    {
        do
        {
            // Get file info + name (minizip returns UTF-8 in this buffer)
            char FileNameUtf8[2048];
            unz_file_info FileInfo;
            if (unzGetCurrentFileInfo(ZipHandle, &FileInfo,
                FileNameUtf8, sizeof(FileNameUtf8),
                nullptr, 0, nullptr, 0) != UNZ_OK)
            {
                bOk = false;
                OutError = TEXT("Failed to get file info");
                break;
            }

            // Convert and normalize path
            FString RelPath = UTF8_TO_TCHAR(FileNameUtf8);
            FPaths::NormalizeFilename(RelPath);

            // Detect directory entries (common pattern: trailing slash)
            bool bEntryLooksLikeDirectory = false;
            if (!RelPath.IsEmpty())
            {
                const TCHAR LastChar = RelPath[RelPath.Len() - 1];
                if (LastChar == TEXT('/') || LastChar == TEXT('\\'))
                {
                    bEntryLooksLikeDirectory = true;
                    RelPath = RelPath.LeftChop(1); // trim trailing slash
                }
            }

            // Compute output paths and ensure directories exist
            const FString OutPath = FPaths::Combine(DestFolder, RelPath);
            const FString OutDir = FPaths::GetPath(OutPath);
            IFileManager::Get().MakeDirectory(*OutDir, true);

            // If this entry is a directory, we've created it; continue
            if (bEntryLooksLikeDirectory || RelPath.IsEmpty())
            {
                LOG_CAVRNUS_VERBOSE("Worker_UnzipAll: directory entry (created): %s", *OutDir);
                // Make sure to still count it toward file count semantics if desired — original behavior skipped directories implicitly
                continue;
            }

            // Open the current file inside the zip
            if (unzOpenCurrentFile(ZipHandle) != UNZ_OK)
            {
                bOk = false;
                OutError = FString::Printf(TEXT("Failed to open file in zip: %s"), *RelPath);
                break;
            }

            // Handle zero-length files explicitly (create empty file, add to list, callback)
            if (FileInfo.uncompressed_size == 0)
            {
                TArray<uint8> Empty;
                if (!FFileHelper::SaveArrayToFile(Empty, *OutPath))
                {
                    UE_LOG(LogTemp, Error, TEXT("Worker_UnzipAll: Failed to write zero-length file: %s"), *OutPath);
                    unzCloseCurrentFile(ZipHandle);
                    bOk = false;
                    OutError = FString::Printf(TEXT("Failed to write file: %s"), *OutPath);
                    break;
                }

                const FString FullOut = FPaths::ConvertRelativePathToFull(OutPath);
                OutFiles.Add(FullOut);

                ++CurrentIndex;
                const float Percent = (TotalFiles > 0) ? (float)CurrentIndex / (float)TotalFiles : 1.0f;
                const FString DisplayName = FPaths::GetCleanFilename(RelPath);
                LOG_CAVRNUS_VERBOSE("Worker_UnzipAll: extracted zero-byte '%s' (%d/%d) -> Percent=%.3f", *DisplayName, CurrentIndex, TotalFiles, Percent);

                if (OnFileExtracted)
                {
					OnFileExtracted(DisplayName, Percent);
                }

                unzCloseCurrentFile(ZipHandle);
                continue;
            }

            // Create writer (FArchive) and stream chunks from the zip into disk
            FArchive* Ar = IFileManager::Get().CreateFileWriter(*OutPath);
            if (!Ar)
            {
                UE_LOG(LogTemp, Error, TEXT("Worker_UnzipAll: Failed to create output file: %s"), *OutPath);
                unzCloseCurrentFile(ZipHandle);
                bOk = false;
                OutError = FString::Printf(TEXT("Failed to create output file: %s"), *OutPath);
                break;
            }

            int64 Remaining = (int64)FileInfo.uncompressed_size;
            while (Remaining > 0)
            {
                const int32 ToRead = FMath::Min<int64>(ChunkSize, Remaining);
                const int BytesRead = unzReadCurrentFile(ZipHandle, Chunk.GetData(), ToRead);
                if (BytesRead < 0)
                {
                    UE_LOG(LogTemp, Error, TEXT("Worker_UnzipAll: Error reading file from zip: %s"), *RelPath);
                    bOk = false;
                    OutError = FString::Printf(TEXT("Error reading file: %s"), *RelPath);
                    break;
                }
                if (BytesRead > 0)
                {
                    Ar->Serialize(Chunk.GetData(), BytesRead);
                    Remaining -= BytesRead;
                }
                else
                {
                    // EOF for this entry
                    break;
                }
            }

            Ar->Flush();
            Ar->Close();
            delete Ar;

            // Close current file entry in zip
            unzCloseCurrentFile(ZipHandle);

            if (!bOk)
            {
                break;
            }

            // Add extracted file to list and call callback
            const FString FullOut = FPaths::ConvertRelativePathToFull(OutPath);
            OutFiles.Add(FullOut);

            ++CurrentIndex;
            const float Percent = (TotalFiles > 0) ? (float)CurrentIndex / (float)TotalFiles : 1.0f;
            const FString DisplayName = FPaths::GetCleanFilename(RelPath);
            LOG_CAVRNUS_VERBOSE("Worker_UnzipAll: extracted '%s' (% d / % d)->Percent = % .3f", *DisplayName, CurrentIndex, TotalFiles, Percent);

            // If you want updates to happen in the GameThread, you gotta make sure they're executing there.
            if (OnFileExtracted)
            {
                OnFileExtracted(DisplayName, Percent);
            }

        } while (unzGoToNextFile(ZipHandle) == UNZ_OK);
    }
    return CleanupAndReturn(bOk);
}