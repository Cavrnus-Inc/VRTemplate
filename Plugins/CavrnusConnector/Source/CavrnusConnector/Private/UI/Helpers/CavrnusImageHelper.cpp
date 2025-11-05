// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Helpers/CavrnusImageHelper.h"
#include "CavrnusConnectorModule.h"
#include "Engine/Texture2DDynamic.h"
#include "UI/Helpers/CavrnusAsyncTaskDownloadImage.h"

void UCavrnusImageHelper::FetchTexture(const FString& InUrl, TFunction<void(UTexture2DDynamic* OutTexture, const FVector2D& OutAspectRatio)> OnFetchTexture)
{
	OnFetchTextureCallback = MoveTemp(OnFetchTexture);

	DownloadTask = NewObject<UCavrnusAsyncTaskDownloadImage>();
	DownloadTask->OnSuccess.AddDynamic(this, &UCavrnusImageHelper::OnFetchTexture);
	DownloadTask->Start(InUrl);
}

void UCavrnusImageHelper::Cancel()
{
	if (DownloadTask)
		DownloadTask->Cancel();
}

void UCavrnusImageHelper::BeginDestroy()
{
	UObject::BeginDestroy();

	if (DownloadTask)
	{
		DownloadTask->OnSuccess.RemoveDynamic(this, &UCavrnusImageHelper::OnFetchTexture);
		DownloadTask = nullptr;
	}
}

void UCavrnusImageHelper::OnFetchTexture(UTexture2DDynamic* Texture)
{
	if (Texture)
	{
		if (OnFetchTextureCallback)
		{
			const int32 TextureWidth = Texture->SizeX;
			const int32 TextureHeight = Texture->SizeY;

			FVector2D OutAspectRatio = FVector2D(1,1);

			if (TextureHeight != 0)
			{
				// Trying to ensure that the texture ratio is something like 16:9, for instance
				const int32 GCD = GetGCD(TextureWidth, TextureHeight);
				const int32 RatioNumerator = TextureWidth / GCD;
				const int32 RatioDenominator = TextureHeight / GCD;

				OutAspectRatio.X = RatioNumerator;
				OutAspectRatio.Y = RatioDenominator;
			}
			else
				UE_LOG(LogCavrnusConnector, Error, TEXT("Texture height is invalid! Texture height is ZERO."))
			
			OnFetchTextureCallback(Texture, OutAspectRatio);
		}
	} else
		UE_LOG(LogCavrnusConnector, Error, TEXT("Attempt to fetch texture failed!: Texture is null"))
}

int32 UCavrnusImageHelper::GetGCD(int32 A, int32 B)
{
	while (B != 0)
	{
		const int32 Temp = B;
		B = A % B;
		A = Temp;
	}
	return A;
}
