// Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CavrnusTranscriptionHudEntry.h"
#include "UI/Menus/MessageMenus/CavrnusMessageMenuBase.h"
#include "CavrnusTranscriptionHud.generated.h"

UCLASS(Abstract)
class CAVRNUSCONNECTOR_API UCavrnusTranscriptionHud : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
	UPROPERTY()
	TMap<FString, UCavrnusTranscriptionHudEntry*> AllWidgetsMap;

	UPROPERTY()
	TArray<UCavrnusTranscriptionHudEntry*> VisibleChatWidgets;
	
	UPROPERTY()
	FCavrnusSpaceConnection SpaceConnection = FCavrnusSpaceConnection();

	UPROPERTY()
	UCavrnusBinding* ChatEventsBinding;
	
	UPROPERTY(EditAnywhere, Category = "Cavrnus|TranscriptionWindow")
	int MaxEntriesVisible = 10;

	UPROPERTY(EditAnywhere, Category = "Cavrnus|TranscriptionWindow")
	float DurationVisible = 5;
	
	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|TranscriptionWindow", meta = (BindWidget))
	UScrollBox* ScrollBox = nullptr;

	UPROPERTY(EditAnywhere, Category = "Cavrnus|TranscriptionWindow")
	TSubclassOf<UCavrnusTranscriptionHudEntry> TranscriptionHudEntryWidget = nullptr;

	UFUNCTION()
	void HandleChatAdded(FChatEntry ChatAdded);

	UFUNCTION()
	void HandleChatUpdated(FChatEntry ChatUpdated);

	UFUNCTION()
	void HandleChatRemoved(FString RemovalId);

	void HookChatBindings();

	void RebuildChildren();

	void SortVisibleWidgets();
};
