// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Helpers/CavrnusAsyncTaskDownloadImage.h"

#include "HttpModule.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Engine/Texture2DDynamic.h"
#include "Rendering/Texture2DResource.h"
#include "RenderGraphUtils.h"
#include "RenderCommandFence.h"
#include "RHIResources.h"
#include "RenderingThread.h"
#include "RHICommandList.h"
#include "Modules/ModuleManager.h"
#include "IImageWrapperModule.h"   

#include "Interfaces/IHttpResponse.h"

#if !UE_SERVER
static void WriteRawToTexture_RenderThread(FTexture2DDynamicResource* TextureResource, TArray64<uint8>* RawData, bool bUseSRGB = true)
{
	check(IsInRenderingThread());

	if (TextureResource)
	{
		FRHITexture2D* TextureRHI = TextureResource->GetTexture2DRHI();

		int32 Width = TextureRHI->GetSizeX();
		int32 Height = TextureRHI->GetSizeY();

		uint32 DestStride = 0;
		uint8* DestData = reinterpret_cast<uint8*>(RHILockTexture2D(TextureRHI, 0, RLM_WriteOnly, DestStride, false, false));

		for (int32 y = 0; y < Height; y++)
		{
			uint8* DestPtr = &DestData[((int64)Height - 1 - y) * DestStride];
			const FColor* SrcPtr = &((FColor*)(RawData->GetData()))[((int64)Height - 1 - y) * Width];
			for (int32 x = 0; x < Width; x++)
			{
				*DestPtr++ = SrcPtr->B;
				*DestPtr++ = SrcPtr->G;
				*DestPtr++ = SrcPtr->R;
				*DestPtr++ = SrcPtr->A;
				SrcPtr++;
			}
		}

		RHIUnlockTexture2D(TextureRHI, 0, false, false);
	}

	delete RawData;
}
#endif

UCavrnusAsyncTaskDownloadImage* UCavrnusAsyncTaskDownloadImage::DownloadImage(FString URL)
{
	UCavrnusAsyncTaskDownloadImage* DownloadTask = NewObject<UCavrnusAsyncTaskDownloadImage>();
	DownloadTask->Start(URL);
	return DownloadTask;
}

void UCavrnusAsyncTaskDownloadImage::Start(FString URL)
{
#if !UE_SERVER
	CurrentRequest = FHttpModule::Get().CreateRequest();
	CurrentRequest->OnProcessRequestComplete().BindUObject(this, &UCavrnusAsyncTaskDownloadImage::HandleImageRequest);
	CurrentRequest->SetURL(URL);
	CurrentRequest->SetVerb(TEXT("GET"));
	CurrentRequest->ProcessRequest();
#else
	RemoveFromRoot();
#endif
}

void UCavrnusAsyncTaskDownloadImage::Cancel()
{
	if (CurrentRequest.IsValid())
	{
		bCancelled = true;
		CurrentRequest->OnProcessRequestComplete().Unbind();
		CurrentRequest->CancelRequest();
		CurrentRequest = nullptr;

		// We can choose whether to broadcast fail here or stay silent
		OnFail.Broadcast(nullptr);

		RemoveFromRoot();
	}
}

void UCavrnusAsyncTaskDownloadImage::HandleImageRequest(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded)
{
#if !UE_SERVER
	RemoveFromRoot();

	// Bail out if user cancelled
	if (bCancelled)
	{
		return;
	}

	if (bSucceeded && HttpResponse.IsValid() && HttpResponse->GetContentLength() > 0)
	{
		IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
		TSharedPtr<IImageWrapper> ImageWrappers[3] =
		{
			ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG),
			ImageWrapperModule.CreateImageWrapper(EImageFormat::JPEG),
			ImageWrapperModule.CreateImageWrapper(EImageFormat::BMP),
		};

		for (auto& ImageWrapper : ImageWrappers)
		{
			if (ImageWrapper.IsValid() && ImageWrapper->SetCompressed(HttpResponse->GetContent().GetData(), HttpResponse->GetContentLength()))
			{
				TArray64<uint8>* RawData = new TArray64<uint8>();
				if (ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, *RawData))
				{
					if (UTexture2DDynamic* Texture = UTexture2DDynamic::Create(ImageWrapper->GetWidth(), ImageWrapper->GetHeight()))
					{
						Texture->SRGB = true;
						Texture->UpdateResource();

						if (FTexture2DDynamicResource* TextureResource = static_cast<FTexture2DDynamicResource*>(Texture->GetResource()))
						{
							ENQUEUE_RENDER_COMMAND(MyUniqueRenderCommand)(
							[TextureResource, RawData](FRHICommandListImmediate&)
							{
								WriteRawToTexture_RenderThread(TextureResource, RawData);
							});
						}
						else
						{
							delete RawData;
						}

						OnSuccess.Broadcast(Texture);
						return;
					}
				}
			}
		}
	}

	OnFail.Broadcast(nullptr);
#endif
}