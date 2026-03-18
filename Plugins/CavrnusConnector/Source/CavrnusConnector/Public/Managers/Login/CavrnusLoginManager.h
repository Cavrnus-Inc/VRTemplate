// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Containers/Ticker.h"
#include "LoginFlows/CavrnusLoginBaseFlow.h"
#include "Managers/CavrnusService.h"
#include "UObject/Object.h"

#include "CavrnusLoginManager.generated.h"

UCLASS()
class CAVRNUSCONNECTOR_API UCavrnusLoginManager : public UCavrnusService
{
	GENERATED_BODY()
public:
	virtual void Dispose() override;
	virtual void Initialize() override;

	void DoLogin(FCavrnusLoginConfig InConfig);
	void DoPluginSettingsLogin();
	void DoPieLogin();

	bool HasLoginBeenInitiated() const { return bLoginInitiated; }
	void ResetLoginState();

	//LifeCycle Methods
	virtual void OnEndPIE(bool bIsSimulating) override;
	virtual void OnAppShutdown() override;

	static void ResolveServer(FString& Server);

private:
	bool bLoginInitiated = false;

	UPROPERTY()
	TObjectPtr<UCavrnusLoginBaseFlow> LoginFlow;

	/** Ticker runs until viewport/UI is ready, then we bind; cleared when bind runs or on Dispose. */
	FTSTicker::FDelegateHandle ViewportReadyTickerHandle;

	bool ApplyCommandLineArgs(FCavrnusLoginConfig* InConfig);
	void BindUIIsReadyWhenViewportReady(FCavrnusLoginConfig InConfig);

};
