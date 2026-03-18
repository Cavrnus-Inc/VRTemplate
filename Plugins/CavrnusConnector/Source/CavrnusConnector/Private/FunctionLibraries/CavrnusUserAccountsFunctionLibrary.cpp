// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "FunctionLibraries/CavrnusUserAccountsFunctionLibrary.h"
#include "CavrnusConnectorModule.h"
#include "RestAPI/CavrnusRestApiClient.h"

// ============================================
// Fetch Users (paginated)
// ============================================

void UCavrnusUserAccountsFunctionLibrary::FetchUsers(int32 Limit, int32 Page, FCavrnusUserAccountListReceived OnSuccess, FCavrnusUserAccountError OnFailure)
{
	CavrnusUserAccountListCallback SuccessCallback = [OnSuccess](const FCavrnusUserAccountListResponse& Response)
	{
		OnSuccess.ExecuteIfBound(Response);
	};
	CavrnusUserAccountErrorCallback ErrorCallback = [OnFailure](const FString& Error)
	{
		OnFailure.ExecuteIfBound(Error);
	};
	FetchUsers(Limit, Page, SuccessCallback, ErrorCallback);
}

void UCavrnusUserAccountsFunctionLibrary::FetchUsers(int32 Limit, int32 Page, CavrnusUserAccountListCallback OnSuccess, CavrnusUserAccountErrorCallback OnFailure)
{
	const FString Path = FString::Printf(TEXT("users?limit=%d&page=%d"), Limit, Page);

	FCavrnusRestApiClient::Get(Path, [OnSuccess, OnFailure](const FCavrnusRestApiResponse& Response)
	{
		if (Response.bSuccess && Response.JsonBody.IsValid())
		{
			FCavrnusUserAccountListResponse ListResponse = FCavrnusUserAccountListResponse::FromJson(Response.JsonBody);
			UE_LOG(LogCavrnusConnector, Log, TEXT("FetchUsers response: %d users (page %d/%d, total %d)"), ListResponse.Users.Num(), ListResponse.Page, ListResponse.Pages, ListResponse.Total);
			OnSuccess(ListResponse);
		}
		else
		{
			OnFailure(Response.ErrorMessage);
		}
	});
}

// ============================================
// Fetch All Users (auto-paginated)
// ============================================

void UCavrnusUserAccountsFunctionLibrary::FetchAllUsers(FCavrnusUserAccountListReceived OnSuccess, FCavrnusUserAccountError OnFailure)
{
	CavrnusUserAccountListCallback SuccessCallback = [OnSuccess](const FCavrnusUserAccountListResponse& Response)
	{
		OnSuccess.ExecuteIfBound(Response);
	};
	CavrnusUserAccountErrorCallback ErrorCallback = [OnFailure](const FString& Error)
	{
		OnFailure.ExecuteIfBound(Error);
	};
	FetchAllUsers(SuccessCallback, ErrorCallback);
}

void UCavrnusUserAccountsFunctionLibrary::FetchAllUsers(CavrnusUserAccountListCallback OnSuccess, CavrnusUserAccountErrorCallback OnFailure)
{
	TSharedPtr<FCavrnusUserAccountListResponse> Accumulator = MakeShareable(new FCavrnusUserAccountListResponse());
	FetchUsersPage(100, 1, Accumulator, OnSuccess, OnFailure);
}

void UCavrnusUserAccountsFunctionLibrary::FetchUsersPage(int32 Limit, int32 Page, TSharedPtr<FCavrnusUserAccountListResponse> Accumulator, CavrnusUserAccountListCallback OnSuccess, CavrnusUserAccountErrorCallback OnFailure)
{
	FetchUsers(Limit, Page,
		[Limit, Accumulator, OnSuccess, OnFailure](const FCavrnusUserAccountListResponse& PageResponse)
		{
			// Accumulate users from this page
			Accumulator->Users.Append(PageResponse.Users);
			Accumulator->Total = PageResponse.Total;
			Accumulator->Pages = PageResponse.Pages;
			Accumulator->Page = PageResponse.Page;

			if (PageResponse.Page < PageResponse.Pages)
			{
				// More pages available — fetch the next one
				FetchUsersPage(Limit, PageResponse.Page + 1, Accumulator, OnSuccess, OnFailure);
			}
			else
			{
				// All pages fetched
				UE_LOG(LogCavrnusConnector, Log, TEXT("FetchAllUsers complete: %d total users across %d pages"), Accumulator->Users.Num(), Accumulator->Pages);
				OnSuccess(*Accumulator);
			}
		},
		OnFailure);
}

// ============================================
// Fetch User by ID
// ============================================

void UCavrnusUserAccountsFunctionLibrary::FetchUserById(const FString& UserId, FCavrnusUserAccountReceived OnSuccess, FCavrnusUserAccountError OnFailure)
{
	CavrnusUserAccountCallback SuccessCallback = [OnSuccess](const FCavrnusUserAccount& UserAccount)
	{
		OnSuccess.ExecuteIfBound(UserAccount);
	};
	CavrnusUserAccountErrorCallback ErrorCallback = [OnFailure](const FString& Error)
	{
		OnFailure.ExecuteIfBound(Error);
	};
	FetchUserById(UserId, SuccessCallback, ErrorCallback);
}

void UCavrnusUserAccountsFunctionLibrary::FetchUserById(const FString& UserId, CavrnusUserAccountCallback OnSuccess, CavrnusUserAccountErrorCallback OnFailure)
{
	const FString Path = FString::Printf(TEXT("users/%s"), *UserId);

	FCavrnusRestApiClient::Get(Path, [OnSuccess, OnFailure](const FCavrnusRestApiResponse& Response)
	{
		if (Response.bSuccess && Response.JsonBody.IsValid())
		{
			FCavrnusUserAccount Result = FCavrnusUserAccount::FromJson(Response.JsonBody);
			OnSuccess(Result);
		}
		else
		{
			OnFailure(Response.ErrorMessage);
		}
	});
}
