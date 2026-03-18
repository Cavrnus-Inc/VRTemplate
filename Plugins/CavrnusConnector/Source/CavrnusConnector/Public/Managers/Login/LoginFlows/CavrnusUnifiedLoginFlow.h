// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CavrnusLoginBaseFlow.h"
#include "CavrnusUnifiedLoginFlow.generated.h"

UCLASS()
class CAVRNUSCONNECTOR_API UCavrnusUnifiedLoginFlow : public UCavrnusLoginBaseFlow
{
	GENERATED_BODY()
public:
	virtual void DoLogin(const FCavrnusLoginConfig& InLoginConfig) override;

private:
	void HandleAuthDispatch();
	void HandlePieAuth();
	void HandleMemberAuth();
	void HandleGuestAuth();
	void HandleAllowBothAuth();
	void HandleAllowBothShowCombinedWidget();
	void HandleSpaceJoinForMember();
	void HandleSpaceJoinForGuest();

	void LogFlowStep(const FString& StepName, const FString& Reason) const;
};
