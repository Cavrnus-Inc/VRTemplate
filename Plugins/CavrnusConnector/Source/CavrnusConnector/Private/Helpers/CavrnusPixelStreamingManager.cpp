#include "Helpers/CavrnusPixelStreamingManager.h"
#include "Engine/Engine.h"

void UCavrnusPixelStreamingManager::Initialize()
{
    Super::Initialize();

#if WITH_PIXELSTREAMING
    if (FModuleManager::Get().IsModuleLoaded("PixelStreaming"))
    {
        UPixelStreamingDelegates* PSDelegate = UPixelStreamingDelegates::GetPixelStreamingDelegates();
        if (PSDelegate)
        {
#if (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION <= 1)
            NewConnectionDelegate = PSDelegate->OnNewConnectionNative.AddUObject(this, &UCavrnusPixelStreamingManager::HandleConnectedClient);
            ClosedConnectionDelegate = PSDelegate->OnClosedConnectionNative.AddUObject(this, &UCavrnusPixelStreamingManager::HandleDisconnectedClient);
            ConnectedToSignallingServerDelegate = PSDelegate->OnConnectedToSignallingServerNative.AddUObject(this, &UCavrnusPixelStreamingManager::HandleConnectedServer);
            DisconnectedFromSignallingServerDelegate = PSDelegate->OnDisconnectedFromSignallingServerNative.AddUObject(this, &UCavrnusPixelStreamingManager::HandleDisconnectedServer);
            DataChannelOpenDelegate = PSDelegate->OnDataChannelOpenNative.AddUObject(this, &UCavrnusPixelStreamingManager::HandleDataChannelOpen);
#else
            NewConnectionDelegate = PSDelegate->OnNewConnectionNative.AddUObject(this, &UCavrnusPixelStreamingManager::HandleConnectedClient_PSv2);
            ClosedConnectionDelegate = PSDelegate->OnClosedConnectionNative.AddUObject(this, &UCavrnusPixelStreamingManager::HandleDisconnectedClient_PSv2);
            ConnectedToSignallingServerDelegate = PSDelegate->OnConnectedToSignallingServerNative.AddUObject(this, &UCavrnusPixelStreamingManager::HandleConnectedServer);
            DisconnectedFromSignallingServerDelegate = PSDelegate->OnDisconnectedFromSignallingServerNative.AddUObject(this, &UCavrnusPixelStreamingManager::HandleDisconnectedServer);
            DataChannelOpenDelegate = PSDelegate->OnDataChannelOpenNative.AddUObject(this, &UCavrnusPixelStreamingManager::HandleDataChannelOpen_PSv2);
#endif
            UE_LOG(LogTemp, Log, TEXT("PixelStreamingManager bound to UPixelStreamingDelegates"));
        }
    }

#endif
}

void UCavrnusPixelStreamingManager::Deinitialize()
{
#if WITH_PIXELSTREAMING
    if (UPixelStreamingDelegates* PSDelegate = UPixelStreamingDelegates::GetPixelStreamingDelegates())
    {
        if (NewConnectionDelegate.IsValid())
        {
            PSDelegate->OnNewConnectionNative.Remove(NewConnectionDelegate);
            NewConnectionDelegate.Reset();
        }

        if (ConnectedToSignallingServerDelegate.IsValid())
        {
            PSDelegate->OnConnectedToSignallingServerNative.Remove(ConnectedToSignallingServerDelegate);
            ConnectedToSignallingServerDelegate.Reset();
        }

        if (DisconnectedFromSignallingServerDelegate.IsValid())
        {
            PSDelegate->OnDisconnectedFromSignallingServerNative.Remove(DisconnectedFromSignallingServerDelegate);
            DisconnectedFromSignallingServerDelegate.Reset();
        }

        if (DataChannelOpenDelegate.IsValid())
        {
            PSDelegate->OnDataChannelOpenNative.Remove(DataChannelOpenDelegate);
            DataChannelOpenDelegate.Reset();
        }

        UE_LOG(LogTemp, Log, TEXT("PixelStreamingManager unbound from UPixelStreamingDelegates"));
    }

    bIsConnected = false;
#endif

    Super::Deinitialize();
}


void UCavrnusPixelStreamingManager::Teardown()
{
    UE_LOG(LogTemp, Log, TEXT("UCavrnusPixelStreamingManager::Teardown"));
}

bool UCavrnusPixelStreamingManager::IsStreamingAvailable() const
{
#if WITH_PIXELSTREAMING
	return bIsAvailable;
#else
    return false;
#endif
}

bool UCavrnusPixelStreamingManager::IsStreamingActive() const
{
#if WITH_PIXELSTREAMING
    return bIsConnected;
#else
    return false;
#endif
}


TArray<FString> UCavrnusPixelStreamingManager::GetKnownBrowserCommands() const
{
    return {
        TEXT("play"),
        TEXT("pause"),
        TEXT("fullscreen"),
        TEXT("custom_event"),
        TEXT("set_quality"),
        TEXT("input_event"),
        TEXT("charInput"),
        TEXT("keyDown")
    };
}

#if WITH_PIXELSTREAMING
#if (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION > 1)

void UCavrnusPixelStreamingManager::HandleDataChannelOpen_PSv2(FString value, FPixelStreamingPlayerId PlayerId, FPixelStreamingDataChannel* DataChannel)
{

}

void UCavrnusPixelStreamingManager::HandleDisconnectedClient_PSv2(FString StreamerId, FString PlayerId, bool WasQualityController)
{
    bIsConnected = false;
    UE_LOG(LogTemp, Log, TEXT("Detected Pixel Streaming Client Disconnection"));
}

void UCavrnusPixelStreamingManager::HandleConnectedClient_PSv2(FString StreamerId, FString PlayerId, bool QualityController)
{
    bIsConnected = true;
    UE_LOG(LogTemp, Log, TEXT("Pixel Streaming Connected"));
}
#endif

void UCavrnusPixelStreamingManager::HandleDataChannelOpen(FPixelStreamingPlayerId PlayerId, webrtc::DataChannelInterface* DCI)
{

}

void UCavrnusPixelStreamingManager::HandleConnectedClient(FString PlayerId, bool QualityController)
{
    bIsConnected = true;
    UE_LOG(LogTemp, Log, TEXT("Pixel Streaming Connected"));
}

void UCavrnusPixelStreamingManager::HandleDisconnectedClient(FPixelStreamingPlayerId PlayerId, bool WasQualityController)
{
    bIsConnected = false;
    UE_LOG(LogTemp, Log, TEXT("Detected Pixel Streaming Client Disconnection"));
}

void UCavrnusPixelStreamingManager::HandleConnectedServer()
{
    bIsAvailable = true;
    UE_LOG(LogTemp, Log, TEXT("Pixel Streaming Connected"));
}

void UCavrnusPixelStreamingManager::HandleDisconnectedServer()
{
    bIsAvailable = false;
    bIsConnected = false;
    UE_LOG(LogTemp, Log, TEXT("Pixel Streaming Disconnected"));
}
#endif