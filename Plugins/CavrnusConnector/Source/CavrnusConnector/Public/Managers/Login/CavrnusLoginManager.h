// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "LoginFlows/CavrnusLoginBaseFlow.h"
#include "Managers/CavrnusService.h"
#include "UObject/Object.h"

#include "CavrnusLoginManager.generated.h"

UCLASS()
class CAVRNUSCONNECTOR_API UCavrnusLoginManager : public UCavrnusService
{
	GENERATED_BODY()
public:
	virtual void Initialize() override;
	
	void DoLogin(const FString& InLoginFlowType, const FCavrnusLoginConfig InConfig);
	void DoPluginSettingsLogin();
	void DoPieLogin();

	//LifeCycle Methods
	virtual void OnEndPIE(bool bIsSimulating) override;
	virtual void OnAppShutdown() override;

	static void ResolveServer(FString& Server);
	
private:
	bool HasAttemptedToLoginAlready = false;
	
	UPROPERTY()
	TObjectPtr<UCavrnusLoginBaseFlow> LoginFlow;
	
	bool ApplyCommandLineArgs(FCavrnusLoginConfig* InConfig);

};
