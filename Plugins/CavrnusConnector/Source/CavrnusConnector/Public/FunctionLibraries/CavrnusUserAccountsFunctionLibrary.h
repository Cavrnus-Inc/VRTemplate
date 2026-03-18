// Copyright (c) 2025 Cavrnus. All rights reserved.

/**
 * @file CavrnusUserAccountsFunctionLibrary.h
 * @brief Blueprint function library for fetching Cavrnus user accounts via REST API.
 */

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Types/CavrnusUserAccount.h"

#include "CavrnusUserAccountsFunctionLibrary.generated.h"		// Always last

// ============================================
// Response Types
// ============================================

/**
 * @brief Paginated response from listing user accounts.
 */
USTRUCT(BlueprintType)
struct CAVRNUSCONNECTOR_API FCavrnusUserAccountListResponse
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|Users|Advanced")
	int32 Page = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|Users|Advanced")
	int32 Pages = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|Users|Advanced")
	int32 Total = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Cavrnus|Users|Advanced")
	TArray<FCavrnusUserAccount> Users;

	FCavrnusUserAccountListResponse() = default;

	static FCavrnusUserAccountListResponse FromJson(const TSharedPtr<FJsonObject>& Json)
	{
		FCavrnusUserAccountListResponse Result;
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

		const TArray<TSharedPtr<FJsonValue>>* DocsArray;
		if (Json->TryGetArrayField(TEXT("docs"), DocsArray))
		{
			for (const auto& Val : *DocsArray)
			{
				const TSharedPtr<FJsonObject>* ItemObj;
				if (Val->TryGetObject(ItemObj))
				{
					Result.Users.Add(FCavrnusUserAccount::FromJson(*ItemObj));
				}
			}
		}

		return Result;
	}
};

// ============================================
// Delegate Declarations
// ============================================

DECLARE_DYNAMIC_DELEGATE_OneParam(FCavrnusUserAccountListReceived, FCavrnusUserAccountListResponse, Response);
DECLARE_DYNAMIC_DELEGATE_OneParam(FCavrnusUserAccountReceived, FCavrnusUserAccount, UserAccount);
DECLARE_DYNAMIC_DELEGATE_OneParam(FCavrnusUserAccountError, FString, Error);

// ============================================
// C++ Callback Types
// ============================================

typedef TFunction<void(const FCavrnusUserAccountListResponse&)> CavrnusUserAccountListCallback;
typedef TFunction<void(const FCavrnusUserAccount&)> CavrnusUserAccountCallback;
typedef TFunction<void(const FString&)> CavrnusUserAccountErrorCallback;

// ============================================
// Class Definition
// ============================================

UCLASS(Abstract)
class CAVRNUSCONNECTOR_API UCavrnusUserAccountsFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	// ============================================
	// Fetch Users (paginated)
	// ============================================

	/**
	 * @brief Fetch a paginated list of user accounts from the server.
	 * @param Limit Max results per page (default 25)
	 * @param Page Page number (default 1)
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Users|Advanced",
		meta = (ToolTip = "Fetch a paginated list of user accounts from the server via the REST API"))
	static void FetchUsers(int32 Limit, int32 Page, FCavrnusUserAccountListReceived OnSuccess, FCavrnusUserAccountError OnFailure);

	static void FetchUsers(int32 Limit, int32 Page, CavrnusUserAccountListCallback OnSuccess, CavrnusUserAccountErrorCallback OnFailure);

	// ============================================
	// Fetch All Users (auto-paginated)
	// ============================================

	/**
	 * @brief Fetch all user accounts from the server, automatically handling pagination.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Users|Advanced",
		meta = (ToolTip = "Fetch all user accounts from the server via the REST API (auto-paginates)"))
	static void FetchAllUsers(FCavrnusUserAccountListReceived OnSuccess, FCavrnusUserAccountError OnFailure);

	static void FetchAllUsers(CavrnusUserAccountListCallback OnSuccess, CavrnusUserAccountErrorCallback OnFailure);

	// ============================================
	// Fetch User by ID
	// ============================================

	/**
	 * @brief Fetch a single user account by ID.
	 * @param UserId The user ID to look up
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Users|Advanced",
		meta = (ToolTip = "Fetch a single user account by ID via the REST API"))
	static void FetchUserById(const FString& UserId, FCavrnusUserAccountReceived OnSuccess, FCavrnusUserAccountError OnFailure);

	static void FetchUserById(const FString& UserId, CavrnusUserAccountCallback OnSuccess, CavrnusUserAccountErrorCallback OnFailure);

private:
	/** Internal helper for auto-pagination. Accumulates results across pages. */
	static void FetchUsersPage(int32 Limit, int32 Page, TSharedPtr<FCavrnusUserAccountListResponse> Accumulator, CavrnusUserAccountListCallback OnSuccess, CavrnusUserAccountErrorCallback OnFailure);
};
