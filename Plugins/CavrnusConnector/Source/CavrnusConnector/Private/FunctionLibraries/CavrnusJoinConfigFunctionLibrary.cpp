// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "FunctionLibraries/CavrnusJoinConfigFunctionLibrary.h"
#include "CavrnusConnectorModule.h"
#include "RestAPI/CavrnusRestApiClient.h"

// ============================================
// List Join Configs
// ============================================

void UCavrnusJoinConfigFunctionLibrary::ListJoinConfigs(const FString& SpaceId, int32 Limit, int32 Page, FCavrnusJoinConfigListReceived OnSuccess, FCavrnusJoinConfigError OnFailure)
{
	CavrnusJoinConfigListCallback SuccessCallback = [OnSuccess](const FCavrnusJoinConfigListResponse& Response)
	{
		OnSuccess.ExecuteIfBound(Response);
	};
	CavrnusJoinConfigErrorCallback ErrorCallback = [OnFailure](const FString& Error)
	{
		OnFailure.ExecuteIfBound(Error);
	};
	ListJoinConfigs(SpaceId, Limit, Page, SuccessCallback, ErrorCallback);
}

void UCavrnusJoinConfigFunctionLibrary::ListJoinConfigs(const FString& SpaceId, int32 Limit, int32 Page, CavrnusJoinConfigListCallback OnSuccess, CavrnusJoinConfigErrorCallback OnFailure)
{
	const FString Path = FString::Printf(TEXT("join-configs?roomId=%s&limit=%d&page=%d"), *SpaceId, Limit, Page);

	FCavrnusRestApiClient::Get(Path, [OnSuccess, OnFailure](const FCavrnusRestApiResponse& Response)
	{
		if (Response.bSuccess && Response.JsonBody.IsValid())
		{
			FCavrnusJoinConfigListResponse ListResponse = FCavrnusJoinConfigListResponse::FromJson(Response.JsonBody);
			OnSuccess(ListResponse);
		}
		else
		{
			OnFailure(Response.ErrorMessage);
		}
	});
}

// ============================================
// Create Join Config
// ============================================

void UCavrnusJoinConfigFunctionLibrary::CreateJoinConfig(FCavrnusJoinConfig Config, FCavrnusJoinConfigReceived OnSuccess, FCavrnusJoinConfigError OnFailure, ECavrnusApiNotifyAction NotifyAction)
{
	CavrnusJoinConfigCallback SuccessCallback = [OnSuccess](const FCavrnusJoinConfig& JoinConfig)
	{
		OnSuccess.ExecuteIfBound(JoinConfig);
	};
	CavrnusJoinConfigErrorCallback ErrorCallback = [OnFailure](const FString& Error)
	{
		OnFailure.ExecuteIfBound(Error);
	};
	CreateJoinConfig(Config, SuccessCallback, ErrorCallback, NotifyAction);
}

void UCavrnusJoinConfigFunctionLibrary::CreateJoinConfig(const FCavrnusJoinConfig& Config, CavrnusJoinConfigCallback OnSuccess, CavrnusJoinConfigErrorCallback OnFailure, ECavrnusApiNotifyAction NotifyAction)
{
	TSharedPtr<FJsonObject> JsonBody = Config.ToJson();
	// Remove Id field — server assigns it
	JsonBody->RemoveField(TEXT("id"));

	FCavrnusRestApiClient::Post(TEXT("join-configs"), JsonBody, [OnSuccess, OnFailure, NotifyAction](const FCavrnusRestApiResponse& Response)
	{
		if (Response.bSuccess && Response.JsonBody.IsValid())
		{
			FCavrnusJoinConfig Result = FCavrnusJoinConfig::FromJson(Response.JsonBody);
			FCavrnusRestApiClient::Notify(NotifyAction, TEXT("Create Join Config"), true, TEXT("Join config created successfully."));
			OnSuccess(Result);
		}
		else
		{
			FCavrnusRestApiClient::Notify(NotifyAction, TEXT("Create Join Config"), false, Response.ErrorMessage);
			OnFailure(Response.ErrorMessage);
		}
	});
}

// ============================================
// Fetch Join Config
// ============================================

void UCavrnusJoinConfigFunctionLibrary::FetchJoinConfig(const FString& Id, FCavrnusJoinConfigReceived OnSuccess, FCavrnusJoinConfigError OnFailure)
{
	CavrnusJoinConfigCallback SuccessCallback = [OnSuccess](const FCavrnusJoinConfig& JoinConfig)
	{
		OnSuccess.ExecuteIfBound(JoinConfig);
	};
	CavrnusJoinConfigErrorCallback ErrorCallback = [OnFailure](const FString& Error)
	{
		OnFailure.ExecuteIfBound(Error);
	};
	FetchJoinConfig(Id, SuccessCallback, ErrorCallback);
}

void UCavrnusJoinConfigFunctionLibrary::FetchJoinConfig(const FString& Id, CavrnusJoinConfigCallback OnSuccess, CavrnusJoinConfigErrorCallback OnFailure)
{
	const FString Path = FString::Printf(TEXT("join-configs/%s"), *Id);

	FCavrnusRestApiClient::Get(Path, [OnSuccess, OnFailure](const FCavrnusRestApiResponse& Response)
	{
		if (Response.bSuccess && Response.JsonBody.IsValid())
		{
			FCavrnusJoinConfig Result = FCavrnusJoinConfig::FromJson(Response.JsonBody);
			OnSuccess(Result);
		}
		else
		{
			OnFailure(Response.ErrorMessage);
		}
	});
}

// ============================================
// Update Join Config
// ============================================

void UCavrnusJoinConfigFunctionLibrary::UpdateJoinConfig(FCavrnusJoinConfig Config, FCavrnusJoinConfigReceived OnSuccess, FCavrnusJoinConfigError OnFailure, ECavrnusApiNotifyAction NotifyAction)
{
	CavrnusJoinConfigCallback SuccessCallback = [OnSuccess](const FCavrnusJoinConfig& JoinConfig)
	{
		OnSuccess.ExecuteIfBound(JoinConfig);
	};
	CavrnusJoinConfigErrorCallback ErrorCallback = [OnFailure](const FString& Error)
	{
		OnFailure.ExecuteIfBound(Error);
	};
	UpdateJoinConfig(Config, SuccessCallback, ErrorCallback, NotifyAction);
}

void UCavrnusJoinConfigFunctionLibrary::UpdateJoinConfig(const FCavrnusJoinConfig& Config, CavrnusJoinConfigCallback OnSuccess, CavrnusJoinConfigErrorCallback OnFailure, ECavrnusApiNotifyAction NotifyAction)
{
	if (Config.JoinConfigId.IsEmpty())
	{
		const FString ErrorMsg = TEXT("Cannot update join config: JoinConfigId is empty.");
		FCavrnusRestApiClient::Notify(NotifyAction, TEXT("Update Join Config"), false, ErrorMsg);
		OnFailure(ErrorMsg);
		return;
	}

	const FString Path = FString::Printf(TEXT("join-configs/%s"), *Config.JoinConfigId);
	TSharedPtr<FJsonObject> JsonBody = Config.ToJson();

	// POST not PUT per API spec
	FCavrnusRestApiClient::Post(Path, JsonBody, [OnSuccess, OnFailure, NotifyAction](const FCavrnusRestApiResponse& Response)
	{
		if (Response.bSuccess && Response.JsonBody.IsValid())
		{
			FCavrnusJoinConfig Result = FCavrnusJoinConfig::FromJson(Response.JsonBody);
			FCavrnusRestApiClient::Notify(NotifyAction, TEXT("Update Join Config"), true, TEXT("Join config updated successfully."));
			OnSuccess(Result);
		}
		else
		{
			FCavrnusRestApiClient::Notify(NotifyAction, TEXT("Update Join Config"), false, Response.ErrorMessage);
			OnFailure(Response.ErrorMessage);
		}
	});
}

// ============================================
// Delete Join Config
// ============================================

void UCavrnusJoinConfigFunctionLibrary::DeleteJoinConfig(const FString& Id, FCavrnusJoinConfigDeleted OnSuccess, FCavrnusJoinConfigError OnFailure, ECavrnusApiNotifyAction NotifyAction)
{
	CavrnusJoinConfigDeletedCallback SuccessCallback = [OnSuccess]()
	{
		OnSuccess.ExecuteIfBound();
	};
	CavrnusJoinConfigErrorCallback ErrorCallback = [OnFailure](const FString& Error)
	{
		OnFailure.ExecuteIfBound(Error);
	};
	DeleteJoinConfig(Id, SuccessCallback, ErrorCallback, NotifyAction);
}

void UCavrnusJoinConfigFunctionLibrary::DeleteJoinConfig(const FString& Id, CavrnusJoinConfigDeletedCallback OnSuccess, CavrnusJoinConfigErrorCallback OnFailure, ECavrnusApiNotifyAction NotifyAction)
{
	// Singular path per API spec: join-config/{id}
	const FString Path = FString::Printf(TEXT("join-config/%s"), *Id);

	FCavrnusRestApiClient::Delete(Path, [OnSuccess, OnFailure, NotifyAction](const FCavrnusRestApiResponse& Response)
	{
		if (Response.bSuccess)
		{
			FCavrnusRestApiClient::Notify(NotifyAction, TEXT("Delete Join Config"), true, TEXT("Join config deleted successfully."));
			OnSuccess();
		}
		else
		{
			FCavrnusRestApiClient::Notify(NotifyAction, TEXT("Delete Join Config"), false, Response.ErrorMessage);
			OnFailure(Response.ErrorMessage);
		}
	});
}

// ============================================
// Fetch Join Config by Slug
// ============================================

void UCavrnusJoinConfigFunctionLibrary::FetchJoinConfigBySlug(const FString& Slug, FCavrnusJoinConfigReceived OnSuccess, FCavrnusJoinConfigError OnFailure)
{
	CavrnusJoinConfigCallback SuccessCallback = [OnSuccess](const FCavrnusJoinConfig& JoinConfig)
	{
		OnSuccess.ExecuteIfBound(JoinConfig);
	};
	CavrnusJoinConfigErrorCallback ErrorCallback = [OnFailure](const FString& Error)
	{
		OnFailure.ExecuteIfBound(Error);
	};
	FetchJoinConfigBySlug(Slug, SuccessCallback, ErrorCallback);
}

void UCavrnusJoinConfigFunctionLibrary::FetchJoinConfigBySlug(const FString& Slug, CavrnusJoinConfigCallback OnSuccess, CavrnusJoinConfigErrorCallback OnFailure)
{
	const FString Path = FString::Printf(TEXT("join-configs/custom-slug/%s"), *Slug);

	FCavrnusRestApiClient::Get(Path, [OnSuccess, OnFailure](const FCavrnusRestApiResponse& Response)
	{
		if (Response.bSuccess && Response.JsonBody.IsValid())
		{
			FCavrnusJoinConfig Result = FCavrnusJoinConfig::FromJson(Response.JsonBody);
			OnSuccess(Result);
		}
		else
		{
			OnFailure(Response.ErrorMessage);
		}
	});
}
