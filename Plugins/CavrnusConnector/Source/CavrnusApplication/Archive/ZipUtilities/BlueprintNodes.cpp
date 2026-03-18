
#include "BlueprintNodes.h"
#include "Zipper.h"
#include "Unzipper.h"
#include <Misc/Paths.h>

UZipAsyncBase::UZipAsyncBase()
	: Super()
	, Zipper(NewObject<UZipper>())
{}


UUnzipAsyncBase::UUnzipAsyncBase()
	: Super()
	, Unzipper(NewObject<UUnzipper>())
{}

void UZipDirectoryProxy::Activate()
{
	Zipper->AddDirectory(Directory);
	
	Zipper->OnFilesZipped().AddUObject(this, &UZipDirectoryProxy::OnZipOver);

	if (OnFileZipped.IsBound())
	{
		Zipper->OnFileZipped().AddUObject(this, &UZipDirectoryProxy::OnArchiveFileZipped);
	}

	if (OnProgress.IsBound())
	{
		Zipper->OnProgress().AddUObject(this, &UZipDirectoryProxy::OnZipProgress); // or UZipFilesProxy
	}
	Zipper->ZipAsync(TargetLocation, Password, Compression, Flag);
}

void UZipDirectoryProxy::OnZipProgress(float InPercent)
{
	Percent = InPercent;
	OnProgress.Broadcast(InPercent);
}

UZipDirectoryProxy* UZipDirectoryProxy::ZipDirectory(const FString& DirectoryToZip, const FString& ResultLocation, const FString& Password, const EZipCreationFlag CreationFlag, const EZipCompressLevel Compression)
{
	UZipDirectoryProxy* const Proxy = NewObject<UZipDirectoryProxy>();

	Proxy->Directory		= DirectoryToZip;
	Proxy->TargetLocation = ResultLocation;
	Proxy->Password		= Password;
	Proxy->Flag			= CreationFlag;
	Proxy->Compression	= Compression;

	return Proxy;
}

void UZipDirectoryProxy::OnZipOver(const bool bSuccess, const FString& ArchivePath, const int64 FilesUnzipped)
{
	(bSuccess ? OnDirectoryZipped : OnError).Broadcast(ArchivePath, TEXT(""), FilesUnzipped, FilesUnzipped, Percent);
	SetReadyToDestroy();
}

void UZipDirectoryProxy::OnArchiveFileZipped(const FString& ArchivePath, const FString& DiskPath, const int64 FilesUnzipped, const int64 TotalFiles)
{
	OnFileZipped.Broadcast(ArchivePath, DiskPath, FilesUnzipped, TotalFiles, Percent);
}

void UZipFilesProxy::Activate()
{
	for (const auto& File : Files)
	{
		Zipper->AddFile(File.FileSource, File.ArchiveFullPath);
	}

	Zipper->OnFilesZipped().AddUObject(this, &UZipFilesProxy::OnZipOver);

	if (OnFileZipped.IsBound())
	{
		Zipper->OnFileZipped().AddUObject(this, &UZipFilesProxy::OnArchiveFileZipped);
	}

	Zipper->ZipAsync(TargetLocation, Password, Compression, Flag);
}

UZipFilesProxy* UZipFilesProxy::ZipFiles(const TArray<FFileToZip>& FilesToZip, const FString& ResultLocation, const FString& Password, const EZipCreationFlag CreationFlag, const EZipCompressLevel Compression)
{
	UZipFilesProxy* const Proxy = NewObject<UZipFilesProxy>();

	Proxy->Files = FilesToZip;
	Proxy->TargetLocation = ResultLocation;
	Proxy->Password = Password;
	Proxy->Flag = CreationFlag;
	Proxy->Compression = Compression;

	return Proxy;
}

void UZipFilesProxy::OnZipOver(const bool bSuccess, const FString& ArchivePath, const int64 FilesUnzipped)
{
	(bSuccess ? OnDirectoryZipped : OnError).Broadcast(ArchivePath, FPaths::GetPath(ArchivePath), FilesUnzipped, FilesUnzipped, Percent);
	SetReadyToDestroy();
}

void UZipFilesProxy::OnArchiveFileZipped(const FString& ArchivePath, const FString& DiskPath, const int64 FilesUnzipped, const int64 TotalFiles)
{
	OnFileZipped.Broadcast(ArchivePath, DiskPath, FilesUnzipped, TotalFiles, Percent);
}

void UUnzipArchiveProxy::OnUnzipProgress(float percent)
{
	this->Percent = percent;
	UE_LOG(LogTemp, Log, TEXT("Unzip progress updated: %f"), percent);
	OnProgress.Broadcast(percent);
}

UUnzipArchiveProxy* UUnzipArchiveProxy::UnzipZipArchive(const FString& ZipArchiveLocation, const FString& ResultLocation, const FString& Password, const bool bOverwrite)
{
	UUnzipArchiveProxy* const Proxy = NewObject<UUnzipArchiveProxy>();

	Proxy->Target = ResultLocation;
	Proxy->Source = ZipArchiveLocation;
	Proxy->Password = Password;
	Proxy->bOverwrite = bOverwrite;
	Proxy->Percent = 0;

	return Proxy;
}

void UUnzipArchiveProxy::Activate()
{
	if (OnFileUnzipped.IsBound())
	{
		Unzipper->OnFileUnzipped().AddUObject(this, &UUnzipArchiveProxy::OnArchiveFileUnzipped);
	}

	Unzipper->OnFilesUnzipped().AddUObject(this, &UUnzipArchiveProxy::OnUnzipOver);

	// Add this block to bind progress delegate
	if (OnProgress.IsBound())
	{
		UE_LOG(LogTemp, Log, TEXT("Progress delegate is bound and ready."));
		Unzipper->OnProgress().AddUObject(this, &UUnzipArchiveProxy::OnUnzipProgress);
	}

	Unzipper->SetArchive(Source);
	Unzipper->UnzipAllAsync(Target, Password, bOverwrite);
}



void UUnzipArchiveProxy::OnUnzipOver(const bool bSuccess, const FString& DirectoryPath, const int64 FileCount)
{
	(bSuccess ? OnArchiveUnzipped : OnError).Broadcast(DirectoryPath, DirectoryPath, FileCount, FileCount, Percent);
	SetReadyToDestroy();
}

void UUnzipArchiveProxy::OnArchiveFileUnzipped(const FString& ArchivePath, const FString& DiskPath, const int64 A, const int64 B)
{
	OnFileUnzipped.Broadcast(ArchivePath, DiskPath, A, B, Percent);
}
