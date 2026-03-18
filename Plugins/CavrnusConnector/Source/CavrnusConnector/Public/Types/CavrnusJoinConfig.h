// Copyright (c) 2025 Cavrnus. All rights reserved.

/**
 * @file CavrnusJoinConfig.h
 * @brief Defines the FCavrnusJoinConfig and related structs for the Cavrnus join config REST API.
 */

#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"

#include "CavrnusJoinConfig.generated.h"		// Always last

/**
 * @brief Guest access options for a join config.
 */
USTRUCT(BlueprintType)
struct CAVRNUSCONNECTOR_API FCavrnusJoinConfigGuestOptions
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|JoinConfig")
	bool bEnabled = false;

	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|JoinConfig")
	TArray<FString> Roles;

	FCavrnusJoinConfigGuestOptions() = default;

	FCavrnusJoinConfigGuestOptions(bool bInEnabled, const TArray<FString>& InRoles)
		: bEnabled(bInEnabled), Roles(InRoles)
	{
	}

	static FCavrnusJoinConfigGuestOptions FromJson(const TSharedPtr<FJsonObject>& Json)
	{
		FCavrnusJoinConfigGuestOptions Result;
		if (!Json.IsValid()) return Result;

		Json->TryGetBoolField(TEXT("enabled"), Result.bEnabled);

		const TArray<TSharedPtr<FJsonValue>>* RolesArray;
		if (Json->TryGetArrayField(TEXT("roles"), RolesArray))
		{
			for (const auto& Val : *RolesArray)
			{
				FString Role;
				if (Val->TryGetString(Role))
				{
					Result.Roles.Add(Role);
				}
			}
		}

		return Result;
	}

	TSharedPtr<FJsonObject> ToJson() const
	{
		TSharedPtr<FJsonObject> Json = MakeShareable(new FJsonObject);
		Json->SetBoolField(TEXT("enabled"), bEnabled);

		TArray<TSharedPtr<FJsonValue>> RolesArray;
		for (const FString& Role : Roles)
		{
			RolesArray.Add(MakeShareable(new FJsonValueString(Role)));
		}
		Json->SetArrayField(TEXT("roles"), RolesArray);

		return Json;
	}
};

/**
 * @brief Instance options for a join config.
 */
USTRUCT(BlueprintType)
struct CAVRNUSCONNECTOR_API FCavrnusJoinConfigInstanceOptions
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|JoinConfig")
	bool bEnabled = false;

	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|JoinConfig")
	int32 MaxActiveUsers = 0;

	/** Strategy: "custom" or "fill" */
	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|JoinConfig")
	FString Strategy = "";

	/** Time-to-live in seconds */
	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|JoinConfig")
	int32 TTL = 0;

	FCavrnusJoinConfigInstanceOptions() = default;

	FCavrnusJoinConfigInstanceOptions(bool bInEnabled, int32 InMaxActiveUsers, const FString& InStrategy, int32 InTTL)
		: bEnabled(bInEnabled), MaxActiveUsers(InMaxActiveUsers), Strategy(InStrategy), TTL(InTTL)
	{
	}

	static FCavrnusJoinConfigInstanceOptions FromJson(const TSharedPtr<FJsonObject>& Json)
	{
		FCavrnusJoinConfigInstanceOptions Result;
		if (!Json.IsValid()) return Result;

		Json->TryGetBoolField(TEXT("enabled"), Result.bEnabled);
		Json->TryGetStringField(TEXT("strategy"), Result.Strategy);

		double MaxUsers = 0;
		if (Json->TryGetNumberField(TEXT("maxActiveUsers"), MaxUsers))
		{
			Result.MaxActiveUsers = static_cast<int32>(MaxUsers);
		}

		double Ttl = 0;
		if (Json->TryGetNumberField(TEXT("ttl"), Ttl))
		{
			Result.TTL = static_cast<int32>(Ttl);
		}

		return Result;
	}

	TSharedPtr<FJsonObject> ToJson() const
	{
		TSharedPtr<FJsonObject> Json = MakeShareable(new FJsonObject);
		Json->SetBoolField(TEXT("enabled"), bEnabled);
		Json->SetNumberField(TEXT("maxActiveUsers"), MaxActiveUsers);
		Json->SetStringField(TEXT("strategy"), Strategy);
		Json->SetNumberField(TEXT("ttl"), TTL);

		return Json;
	}
};

/**
 * @brief Represents a Cavrnus join config, used to configure how users join a space.
 */
USTRUCT(BlueprintType)
struct CAVRNUSCONNECTOR_API FCavrnusJoinConfig
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|JoinConfig")
	FString JoinConfigId = "";

	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|JoinConfig")
	bool bActive = true;

	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|JoinConfig")
	FString CustomSlug = "";

	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|JoinConfig")
	FDateTime ExpiresAt = FDateTime::MinValue();

	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|JoinConfig")
	FString Name = "";

	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|JoinConfig")
	FString SpaceId = "";

	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|JoinConfig")
	FString Password = "";

	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|JoinConfig")
	FCavrnusJoinConfigGuestOptions GuestOptions;

	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|JoinConfig")
	FCavrnusJoinConfigInstanceOptions InstanceOptions;

	FCavrnusJoinConfig() = default;

	static FCavrnusJoinConfig FromJson(const TSharedPtr<FJsonObject>& Json)
	{
		FCavrnusJoinConfig Result;
		if (!Json.IsValid()) return Result;

		Json->TryGetStringField(TEXT("id"), Result.JoinConfigId);
		Json->TryGetBoolField(TEXT("active"), Result.bActive);
		Json->TryGetStringField(TEXT("customSlug"), Result.CustomSlug);
		Json->TryGetStringField(TEXT("name"), Result.Name);
		Json->TryGetStringField(TEXT("room"), Result.SpaceId);
		Json->TryGetStringField(TEXT("password"), Result.Password);

		FString ExpiresAtStr;
		if (Json->TryGetStringField(TEXT("expiresAt"), ExpiresAtStr) && !ExpiresAtStr.IsEmpty())
		{
			FDateTime::ParseIso8601(*ExpiresAtStr, Result.ExpiresAt);
		}

		const TSharedPtr<FJsonObject>* GuestObj;
		if (Json->TryGetObjectField(TEXT("guestOptions"), GuestObj))
		{
			Result.GuestOptions = FCavrnusJoinConfigGuestOptions::FromJson(*GuestObj);
		}

		const TSharedPtr<FJsonObject>* InstanceObj;
		if (Json->TryGetObjectField(TEXT("instanceOptions"), InstanceObj))
		{
			Result.InstanceOptions = FCavrnusJoinConfigInstanceOptions::FromJson(*InstanceObj);
		}

		return Result;
	}

	TSharedPtr<FJsonObject> ToJson() const
	{
		TSharedPtr<FJsonObject> Json = MakeShareable(new FJsonObject);

		if (!JoinConfigId.IsEmpty())
		{
			Json->SetStringField(TEXT("id"), JoinConfigId);
		}

		Json->SetBoolField(TEXT("active"), bActive);

		if (!CustomSlug.IsEmpty())
		{
			Json->SetStringField(TEXT("customSlug"), CustomSlug);
		}

		if (ExpiresAt != FDateTime::MinValue())
		{
			Json->SetStringField(TEXT("expiresAt"), ExpiresAt.ToIso8601());
		}

		if (!Name.IsEmpty())
		{
			Json->SetStringField(TEXT("name"), Name);
		}

		if (!SpaceId.IsEmpty())
		{
			Json->SetStringField(TEXT("roomId"), SpaceId);
		}

		if (!Password.IsEmpty())
		{
			Json->SetStringField(TEXT("password"), Password);
		}

		Json->SetObjectField(TEXT("guestOptions"), GuestOptions.ToJson());
		Json->SetObjectField(TEXT("instanceOptions"), InstanceOptions.ToJson());

		return Json;
	}
};

/**
 * @brief Response from listing join configs (paginated).
 */
USTRUCT(BlueprintType)
struct CAVRNUSCONNECTOR_API FCavrnusJoinConfigListResponse
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|JoinConfig")
	int32 Page = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|JoinConfig")
	int32 Pages = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|JoinConfig")
	int32 Total = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|JoinConfig")
	TArray<FCavrnusJoinConfig> JoinConfigs;

	FCavrnusJoinConfigListResponse() = default;

	static FCavrnusJoinConfigListResponse FromJson(const TSharedPtr<FJsonObject>& Json)
	{
		FCavrnusJoinConfigListResponse Result;
		if (!Json.IsValid()) return Result;

		double PageVal = 0;
		if (Json->TryGetNumberField(TEXT("page"), PageVal))
		{
			Result.Page = static_cast<int32>(PageVal);
		}

		double PagesVal = 0;
		if (Json->TryGetNumberField(TEXT("pages"), PagesVal))
		{
			Result.Pages = static_cast<int32>(PagesVal);
		}

		double TotalVal = 0;
		if (Json->TryGetNumberField(TEXT("total"), TotalVal))
		{
			Result.Total = static_cast<int32>(TotalVal);
		}

		const TArray<TSharedPtr<FJsonValue>>* ItemsArray;
		if (Json->TryGetArrayField(TEXT("joinConfigs"), ItemsArray))
		{
			for (const auto& Val : *ItemsArray)
			{
				const TSharedPtr<FJsonObject>* ItemObj;
				if (Val->TryGetObject(ItemObj))
				{
					Result.JoinConfigs.Add(FCavrnusJoinConfig::FromJson(*ItemObj));
				}
			}
		}

		return Result;
	}
};
