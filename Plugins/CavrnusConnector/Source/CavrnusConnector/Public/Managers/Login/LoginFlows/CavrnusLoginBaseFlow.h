// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CavrnusConnectorSettings.h"
#include "Managers/Login/CavrnusLoginConfig.h"
#include "Types/CavrnusSpaceConnection.h"
#include "UObject/Object.h"
#include "CavrnusLoginBaseFlow.generated.h"

UCLASS()
class CAVRNUSCONNECTOR_API UCavrnusLoginBaseFlow : public UObject
{
	GENERATED_BODY()
public:
	virtual void DoLogin(const FCavrnusLoginConfig& InInitialLoginConfig);

protected:
	FCavrnusLoginConfig LoginConfig = FCavrnusLoginConfig();
	
	void AwaitValidServer(const TFunction<void()>& OnSuccess, const TFunction<void(const FString&)>& OnFail = nullptr);
	
	void TryJoinSpace(const TFunction<void()>& OnSuccess = nullptr, const TFunction<void(const FString&)>& OnFail = nullptr);

	void TryMemberAuthWithRuntimeToken(const TFunction<void()>& OnSuccess = nullptr, const TFunction<void(const FString&)>& OnFail = nullptr);
	void TryMemberAuthWithPassword(const TFunction<void()>& OnSuccess = nullptr, const TFunction<void(const FString&)>& OnFail = nullptr);

	void TryGuestAuth(const TFunction<void()>& OnSuccess = nullptr, const TFunction<void(const FString&)>& OnFail = nullptr);
	void TryApiKeyAuth(const TFunction<void()>& OnSuccess = nullptr, const TFunction<void(const FString&)>& OnFail = nullptr);

	void PromptMemberLogin(const TFunction<void()>& OnSuccess = nullptr);
	void PromptMemberLoginAndSaveToken(const TFunction<void()>& OnSuccess = nullptr);
	void PromptGuestLogin(const TFunction<void()>& OnSuccess = nullptr);

	virtual void HandleConnectedSpace(const FCavrnusSpaceConnection& SpaceConn);

protected:
#pragma region UI Spawning
	void ShowMemberLoginWidget();
	void ShowGuestLoginWidget();
	void ShowJoinIdWidget();
	void ShowServerSelectionWidget();
	void ShowSpaceListWidget();
	void ShowAuthenticationProgressWidget(bool bShowWidget);
	void ShowLoadingProgressWidget(bool bShowWidget);

	TWeakObjectPtr<UUserWidget> AuthLoadingWidget = nullptr;
	TWeakObjectPtr<UUserWidget> LoadingWidget = nullptr;
	
#pragma endregion
};
