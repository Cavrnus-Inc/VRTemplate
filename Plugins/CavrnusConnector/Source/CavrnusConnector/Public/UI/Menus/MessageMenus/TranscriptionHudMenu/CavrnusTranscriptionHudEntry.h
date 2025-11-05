// Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Types/ChatEntry.h"
#include "CavrnusTranscriptionHudEntry.generated.h"

class UCavrnusTranscriptionHudEntry;

DECLARE_DELEGATE_OneParam(FOnKilledDelegate, UCavrnusTranscriptionHudEntry*);

UCLASS(Abstract)
class CAVRNUSCONNECTOR_API UCavrnusTranscriptionHudEntry : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void Setup(const FChatEntry& InChatEntry, float InDisplayDuration);

	FChatEntry ChatEntry = FChatEntry();

	FOnKilledDelegate OnKilled;
	
protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|TranscriptionEntry")
	float DisplayDuration = 0;
	
	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|TranscriptionEntry", meta = (BindWidgetOptional))
	UTextBlock* CreatorName = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|TranscriptionEntry", meta = (BindWidgetOptional))
	UTextBlock* CreationTime = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|TranscriptionEntry", meta = (BindWidgetOptional))
	UTextBlock* Message = nullptr;

	UPROPERTY(EditAnywhere, Category = "Cavrnus|TranscriptionEntry")
	FLinearColor LocalUserColor = FLinearColor();

	UFUNCTION(BlueprintImplementableEvent, Category = "Cavrnus|TranscriptionEntry")
	void SetupComplete(FChatEntry InChatEntry);

	UFUNCTION(BlueprintImplementableEvent, Category = "Cavrnus|TranscriptionEntry")
	void TranscriptionInProgress();

	UFUNCTION(BlueprintImplementableEvent, Category = "Cavrnus|TranscriptionEntry")
	void TranscriptionComplete();

	UFUNCTION(BlueprintCallable, Category = "Cavrnus|TranscriptionEntry")
	void DurationExpired();

private:
	bool CanTick = false;
	
	FString FormatTime(const FChatEntry& ChatEntry);
};
