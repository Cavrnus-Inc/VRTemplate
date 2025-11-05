// Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "ChatMenu/CavrnusChatEntry.h"
#include "Components/Button.h"
#include "Components/ScrollBox.h"
#include "UI/Menus/MinimalMenu/SidebarMenu/SidebarMenuBase.h"
#include "CavrnusMessageMenuBase.generated.h"

UCLASS(Abstract)
class CAVRNUSCONNECTOR_API UCavrnusMessageMenuBase : public USidebarMenuBase
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|ChatWindow", meta = (BindWidget))
	UScrollBox* ChatScrollBox = nullptr;

	UPROPERTY(EditAnywhere, Category = "Cavrnus|ChatWindow")
	TSubclassOf<UCavrnusChatEntry> ChatEntryWidget = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|ChatWindow", meta = (BindWidget))
	UButton* ResetPositionButton = nullptr;

	UFUNCTION()
	void OnScrollBoxScrolled(float CurrentOffset);

	UPROPERTY()
	FCavrnusSpaceConnection SpaceConnection = FCavrnusSpaceConnection();
	
	UPROPERTY()
	bool AutoScrollToEnd = false;

	UPROPERTY()
	TMap<FString, UCavrnusChatEntry*> AllWidgetsMap;

	UPROPERTY()
	TArray<UCavrnusChatEntry*> SortedChatWidgets;

	FTimerHandle TimerHandle = FTimerHandle();

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY()
	UCavrnusBinding* ChatEventsBinding;
	
	UFUNCTION(BlueprintCallable, Category = "Cavrnus|ChatWindow")
	virtual void ResetPositionButtonClicked();
	
	virtual void ScrollToEnd();
	virtual void ScrollToBottomAfterDelay();

	virtual void LocalUserArrived();
	void HookChatBindings();

	UFUNCTION()
	virtual void HandleChatAdded(FChatEntry ChatAdded);

	UFUNCTION()
	virtual void HandleChatUpdated(FChatEntry ChatUpdated);

	UFUNCTION()
	virtual void HandleChatRemoved(FString RemovalId);

	virtual void ForceChildrenRebuild() override;

	virtual void RebuildChildren();
};
