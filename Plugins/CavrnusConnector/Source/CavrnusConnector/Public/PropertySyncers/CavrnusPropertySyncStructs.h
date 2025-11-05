// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Types/CavrnusPropertyValue.h"
#include "CavrnusPropertySyncStructs.generated.h"

UENUM()
enum class ECavrnusSyncResolvedPropertyType : uint8
{
	None,
	Bool,
	String,
	Float,
	Int,       // ✅ New: explicitly track integer properties
	Vector,
	Color,
	Transform,
	Struct     // ✅ New: catch-all for arbitrary USTRUCTs
};

USTRUCT(BlueprintType)
struct FCavrnusSyncInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cavrnus|Property")
	FString ContainerName = "";
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cavrnus|Property")
	FString PropertyName = "Transform";
};

USTRUCT(BlueprintType)
struct FCavrnusSyncOptions
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cavrnus|Sync|Property")
	bool bIsTransient = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cavrnus|Sync|Property")
	bool bSendValue = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cavrnus|Sync|Property")
	bool bSetDefaultValue = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cavrnus|Sync|Property")
	bool bReceiveValue = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cavrnus|Sync|Property")
	float MaxPollingTimeBeforeSend = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cavrnus|Sync|Property")
	float PollingTimeSeconds = 0.1f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cavrnus|Sync|Property")
	float SecondsToWaitBeforePosting = 0.2f;

};

USTRUCT(BlueprintType)
struct FCavrnusSyncTransformOptions : public FCavrnusSyncOptions
{
	GENERATED_BODY()

	/** Minimum position change (cm) before sending an update */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cavrnus|Sync|Transform")
	float PositionTolerance = 0.5f;

	/** Minimum scale change (percentage) before sending an update */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cavrnus|Sync|Transform")
	float ScaleTolerance = 0.01f;

	/** Minimum rotation change (degrees) before sending an update*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cavrnus|Sync|Transform")
	float RotationTolerance = 0.5f;
};


USTRUCT(BlueprintType)
struct FCavrnusSyncStructOptions : public FCavrnusSyncOptions
{
	GENERATED_BODY()

	/** Minimum position change (cm) before sending an update */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cavrnus|Sync|Struct")
	bool UseHashing = true;
};

USTRUCT(BlueprintType)
struct FCavrnusSyncVectorOptions : public FCavrnusSyncOptions
{
	GENERATED_BODY()

	/** Minimum position change (cm) before sending an update */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cavrnus|Sync|Vector")
	float LengthTolerance = 0.1f;
};

USTRUCT(BlueprintType)
struct FCavrnusSyncColorOptions : public FCavrnusSyncOptions
{
	GENERATED_BODY()

	/** Minimum position change (cm) before sending an update */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cavrnus|Sync|Color")
	float DeltaRGB = 0.02f;
};

USTRUCT(BlueprintType)
struct FCavrnusGenericStructSync
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Cavrnus|Sync|Struct")
	FString StructText;

	UPROPERTY(BlueprintReadOnly, Category = "Cavrnus|Sync|Struct")
	FName StructTypeName;

	UPROPERTY(BlueprintReadOnly, Category = "Cavrnus|Sync|Struct")
	FGuid SyncId;
};

struct FCavrnusUpdateTracker
{

	FGuid SentId;
	FGuid ReceivedId;

	static FGuid GenerateNewUpdateId()
	{
		return FGuid::NewGuid();
	}

	bool IsLocalSend() const
	{
		return SentId.IsValid() && SentId != ReceivedId;
	}

	void MarkSent(FGuid Id)
	{
		SentId = Id;
	}

	void MarkReceived()
	{
		ReceivedId = SentId;
		SentId.Invalidate();
	}
};

