

#include "ZipLibrary.h"


void UZipLibrary::ZipDirectory(const FString& FolderPath, const FString& TargetLocation, const FString& Password, const EZipCompressLevel CompressLevel, const EZipCreationFlag CreationFlag, const FOnFilesZipped& OnFilesZipped, const FOnFileZipped& OnFileZipped)
{
	UZipper* const Zipper = UZipper::CreateZipper();
	
	if (OnFilesZipped.IsBound())
	{
		Zipper->OnFilesZipped().AddLambda([OnFilesZipped](const bool bSuccess, const FString& A, const int64 B) -> void
		{
			OnFilesZipped.Broadcast(bSuccess, A, B);
		});
	}

	if (OnFileZipped.IsBound())
	{
		Zipper->OnFileZipped().AddLambda([OnFileZipped](const FString& A, const FString& B, const int64 C, const int64 D) -> void
		{
			OnFileZipped.Broadcast(A, B, C, D);
		});
	}

	Zipper->AddDirectory(FolderPath);

	Zipper->ZipAsync(TargetLocation, Password, CompressLevel, CreationFlag);
}


void UZipLibrary::UnzipArchive(const FString& ArchivePath, const FString& TargetLocation, const FString& Password, const bool bForce, const FOnFilesUnzipped& OnFilesUnzipped, const FOnFileUnzipped& OnFileUnzipped)
{
	UUnzipper* const Unzipper = UUnzipper::CreateUnzipper();

	Unzipper->SetArchive(ArchivePath);

	if (OnFilesUnzipped.IsBound())
	{
		Unzipper->OnFilesUnzipped().AddLambda([OnFilesUnzipped](const bool A, const FString& B, const int64 C) -> void
		{
			OnFilesUnzipped.Broadcast(A, B, C);
		});
	}

	if (OnFileUnzipped.IsBound())
	{
		Unzipper->OnFileUnzipped().AddLambda([OnFileUnzipped](const FString& A, const FString& B, const int64 C, const int64 D) -> void
		{
			OnFileUnzipped.Broadcast(A, B, C, D);
		});
	}

	Unzipper->UnzipAllAsync(TargetLocation, Password, bForce);
}
