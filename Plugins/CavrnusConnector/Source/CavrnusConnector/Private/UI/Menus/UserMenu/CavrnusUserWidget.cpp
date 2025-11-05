// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Menus/UserMenu/CavrnusUserWidget.h"

#include <Engine/Texture2D.h>
#include <Components/Image.h>
#include "CavrnusConnectorModule.h"
#include "CavrnusFunctionLibrary.h"
#include "Components/Button.h"

void UCavrnusUserWidget::NativeDestruct()
{
	Super::NativeDestruct();

	UnbindUserVideo();

	RtcStreamImage = nullptr;
}

void UCavrnusUserWidget::InitializeUserConnection(const FCavrnusUser& InUser)
{
	User = InUser;

	UnbindUserVideo();
	BindUserVideo();
	
	InitializeComplete();
}

void UCavrnusUserWidget::TriggerMaximizeUserSelected()
{
	MaximizeUserSelected(User);
}

void UCavrnusUserWidget::BindUserVideo()
{
	VideoFrameUpdateFunction UserVideoFrameUpdate = [this](UTexture2D* InTexture) {
		if (!RtcStreamImage)
		{
			UE_LOG(LogCavrnusConnector, Warning, TEXT("RtcStreamImage is not valid"));
			return;
		}

		if (!InTexture)
		{
			UE_LOG(LogCavrnusConnector, Warning, TEXT("InTexture is null"));
			return;
		}

		const int32 TextureSizeX = InTexture->GetSizeX();
		const int32 TextureSizeY = InTexture->GetSizeY();

		if (TextureSizeX <= 0 || TextureSizeY <= 0)
		{
			UE_LOG(LogCavrnusConnector, Warning, TEXT("InTexture has invalid dimensions: %d x %d"), TextureSizeX, TextureSizeY);
			return;
		}

		if (TextureSizeX > 0 && TextureSizeY > 0)
		{
			if (TextureSizeChanged(FVector2D(TextureSizeX, TextureSizeY)))
			{
				RtcStreamImage->Brush.SetImageSize(FVector2D(TextureSizeX, TextureSizeY));
			}

			RtcStreamImage->SetBrushFromTexture(InTexture);
		}
		else
		{
			UE_LOG(LogCavrnusConnector, Warning, TEXT("InTexture has invalid size: %d x %d"), TextureSizeX, TextureSizeY);
		}
	};

	UserVideoFrameBinding = UCavrnusFunctionLibrary::BindUserVideoFrames(User.SpaceConn, User, UserVideoFrameUpdate);
}

bool UCavrnusUserWidget::TextureSizeChanged(const FVector2D NewSize) const
{
	return CurrentStreamImageSize != NewSize;
}

void UCavrnusUserWidget::UnbindUserVideo()
{
	if (UserVideoFrameBinding)
		UserVideoFrameBinding->Unbind();
}
