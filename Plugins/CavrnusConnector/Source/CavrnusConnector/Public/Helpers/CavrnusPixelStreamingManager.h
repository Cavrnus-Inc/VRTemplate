// CavrnusPixelStreamingManager.h

#pragma once
#ifndef WITH_PIXELSTREAMING
#define WITH_PIXELSTREAMING 0
#endif

#include "CoreMinimal.h"
#include "Managers/CavrnusService.h"
#include "Components/EditableTextBox.h"
#include "CavrnusSubsystem.h"

#if WITH_PIXELSTREAMING
#include "PixelStreamingDelegates.h"   // Correct include for UPixelStreamingDelegates
#if (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION > 1)

#include "PixelStreamingDataChannel.h"
#endif
#endif
#include "CavrnusPixelStreamingManager.generated.h"

UCLASS()
class CAVRNUSCONNECTOR_API UCavrnusPixelStreamingManager : public UCavrnusService
{
    GENERATED_BODY()

public:
    virtual void Initialize() override;
    virtual void Deinitialize() override;
    virtual void Teardown() override;

    UFUNCTION(BlueprintPure, Category = "Cavrnus|PixelStreaming", meta = (DisplayName = "Get Pixel Streaming Manager"))
    static UCavrnusPixelStreamingManager* GetPixelStreamingManager()
    {
#if WITH_PIXELSTREAMING
        return UCavrnusSubsystem::Get()->Services->Get<UCavrnusPixelStreamingManager>();
#else
        return nullptr;
#endif
    }

    UFUNCTION(BlueprintCallable, Category = "Cavrnus|PixelStreaming")
    bool IsStreamingActive() const;
    UFUNCTION(BlueprintCallable, Category = "Cavrnus|PixelStreaming")
    bool IsStreamingAvailable() const;

    UFUNCTION(BlueprintCallable, Category = "Cavrnus|PixelStreaming")
    TArray<FString> GetKnownBrowserCommands() const;

private:
#if WITH_PIXELSTREAMING
    FDelegateHandle NewConnectionDelegate;
    FDelegateHandle ClosedConnectionDelegate;
    FDelegateHandle ConnectedToSignallingServerDelegate;
    FDelegateHandle DisconnectedFromSignallingServerDelegate;
	FDelegateHandle DataChannelOpenDelegate;

    bool bIsConnected = false;
    bool bIsAvailable = false;
#
    void HandleDataChannelOpen(FPixelStreamingPlayerId PlayerId, webrtc::DataChannelInterface* DCI);
    void HandleConnectedClient(FString PlayerId, bool QualityController);
    void HandleDisconnectedClient(FPixelStreamingPlayerId PlayerId, bool WasQualityController);
    void HandleConnectedServer();
    void HandleDisconnectedServer();

#if (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION > 1)
    void HandleConnectedClient_PSv2(FString StreamerId, FString PlayerId, bool QualityController);
    void HandleDisconnectedClient_PSv2(FString StreamerId, FString PlayerId, bool WasQualityController);
    void HandleDataChannelOpen_PSv2(FString value, FPixelStreamingPlayerId PlayerId, FPixelStreamingDataChannel* DataChannel);
#endif

#endif // WITH_CAVRNUS_PIXELSTREAMING
};
