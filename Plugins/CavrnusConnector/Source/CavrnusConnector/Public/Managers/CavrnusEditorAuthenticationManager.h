// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include <Types/CavrnusDeviceCodeData.h>

#include "CoreMinimal.h"
#include "CavrnusConnectorSettings.h"

#include "Types/CavrnusAuthentication.h"
#include "UObject/Object.h"
#include "Managers/CavrnusService.h"

#include "CavrnusEditorAuthenticationManager.generated.h"

enum ECavrnusEditorConnectedStateEnum
{
	Connected,
	Connecting,
	Disconnected,
};

USTRUCT(BlueprintType)
struct FCavrnusEditorLoginInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Connection")
	FString Server;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Connection")
	FString AccessKey;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Connection")
	FString AccessToken;

	FCavrnusEditorLoginInfo()
		: Server(TEXT("")), AccessKey(TEXT("")), AccessToken(TEXT(""))
	{
	}

	bool IsValid() const
	{
		return !AccessKey.IsEmpty() && !AccessToken.IsEmpty();
	}
};

UCLASS()
class CAVRNUSCONNECTOR_API UCavrnusEditorAuthenticationManager : public UCavrnusService
{
	GENERATED_BODY()
public:
	TMulticastDelegate<void()> OnAuthStarted;
	TMulticastDelegate<void(const ECavrnusAuthMethodForPIE& EditorAuthMethod)> OnEditorAuthMethodChanged;
	TMulticastDelegate<void(const FCavrnusDeviceCodeData& CodeData)> OnDeviceCodeDataReceived;
	TMulticastDelegate<void(const FCavrnusAuthentication& Auth)> OnAuthCompleted;
	TMulticastDelegate<void(const ECavrnusEditorConnectedStateEnum& CurrentState)> OnConnStateChanged;

	virtual void Initialize() override;
	virtual void Deinitialize() override;
	void Teardown();

	bool HasEditorAuthenticated() const { return HasAuthenticated; }
	ECavrnusAuthMethodForPIE GetCurrentAuthMethod();
	void SetLoginAuthMethod(const ECavrnusAuthMethodForPIE& AuthMethod);

	void SetRuntimeToken(const FString& InToken);
	FString GetRuntimeToken();
	FString GetPIEAuthedServer();
	
	void DoConnect();

	bool TryGetEditorLoginInfo(FCavrnusEditorLoginInfo& OutInfo);
	FDelegateHandle BindConnectedState(const TFunction<void(const ECavrnusEditorConnectedStateEnum&)>& OnConnCallback);
	
private:
	bool HasAuthenticated = false;
	ECavrnusAuthMethodForPIE CurrentEditorAuthMethod = ECavrnusAuthMethodForPIE::JoinAsPIE;

	static FString RuntimeToken;
	static FString EditorAuthMethod;
	static FString ServerKey;
	static FString ApiEditorKey;
	static FString ApiEditorToken;

	ECavrnusEditorConnectedStateEnum CurrentState = Disconnected;

	void UpdateConnectedState(const ECavrnusEditorConnectedStateEnum& NewState);

	void StoreAuthData(const FCavrnusAuthentication& Auth, const TFunction<void()>& OnFinished, const TFunction<void(const FString&)>& OnFailed);
	void ClearEditorAuthData();
};
