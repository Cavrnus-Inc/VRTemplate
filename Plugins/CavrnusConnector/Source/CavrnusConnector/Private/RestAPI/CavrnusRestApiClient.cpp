// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "RestAPI/CavrnusRestApiClient.h"
#include "CavrnusConnectorModule.h"
#include "RelayModel/CavrnusRelayModel.h"
#include "RelayModel/DataState.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Async/Async.h"
#include "UI/CavrnusUI.h"
#include "UI/CavrnusUISystems.h"
#include "UI/Systems/Messages/CavrnusScopedMessages.h"
#include "UI/Systems/Messages/ToastMessages/CavrnusToastMessageUISystem.h"
#include "UI/Systems/Messages/ToastMessages/Info/CavrnusInfoToastMessageWidget.h"

TArray<TWeakPtr<IHttpRequest, ESPMode::ThreadSafe>> FCavrnusRestApiClient::PendingRequests;

void FCavrnusRestApiClient::Get(const FString& RelativePath, CavrnusRestApiCallback Callback)
{
	SendRequest(TEXT("GET"), RelativePath, nullptr, MoveTemp(Callback));
}

void FCavrnusRestApiClient::Post(const FString& RelativePath, const TSharedPtr<FJsonObject>& JsonBody, CavrnusRestApiCallback Callback)
{
	SendRequest(TEXT("POST"), RelativePath, JsonBody, MoveTemp(Callback));
}

void FCavrnusRestApiClient::Put(const FString& RelativePath, const TSharedPtr<FJsonObject>& JsonBody, CavrnusRestApiCallback Callback)
{
	SendRequest(TEXT("PUT"), RelativePath, JsonBody, MoveTemp(Callback));
}

void FCavrnusRestApiClient::Delete(const FString& RelativePath, CavrnusRestApiCallback Callback, const TSharedPtr<FJsonObject>& JsonBody)
{
	SendRequest(TEXT("DELETE"), RelativePath, JsonBody, MoveTemp(Callback));
}

void FCavrnusRestApiClient::SendRequest(const FString& Verb, const FString& RelativePath, const TSharedPtr<FJsonObject>& JsonBody, CavrnusRestApiCallback Callback)
{
	// Layer 2: Early-out if the relay system has been shut down.
	// Silently drop — no callback, no AsyncTask — to avoid touching dead state.
	if (!Cavrnus::CavrnusRelayModel::IsAlive())
	{
		UE_LOG(LogCavrnusConnector, Log, TEXT("[RestApiClient] System is shutting down — dropping %s %s"), *Verb, *RelativePath);
		return;
	}

	// Get auth state
	Cavrnus::DataState* State = Cavrnus::CavrnusRelayModel::GetDataModel()->GetDataState();

	if (!State->CurrentAuthentication)
	{
		FCavrnusRestApiResponse ErrorResponse;
		ErrorResponse.bSuccess = false;
		ErrorResponse.StatusCode = 0;
		ErrorResponse.ErrorMessage = TEXT("Not authenticated. Please log in before making API calls.");

		AsyncTask(ENamedThreads::GameThread, [Callback, ErrorResponse]()
		{
			Callback(ErrorResponse);
		});
		return;
	}

	const FString Server = State->CurrentServer;
	const FString Token = State->CurrentAuthentication->Token;

	if (Server.IsEmpty())
	{
		FCavrnusRestApiResponse ErrorResponse;
		ErrorResponse.bSuccess = false;
		ErrorResponse.StatusCode = 0;
		ErrorResponse.ErrorMessage = TEXT("Server not set. Cannot make API calls without a configured server.");

		AsyncTask(ENamedThreads::GameThread, [Callback, ErrorResponse]()
		{
			Callback(ErrorResponse);
		});
		return;
	}

	// Build URL
	const FString Url = GetBaseUrl(Server) + RelativePath;

	// Create HTTP request
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetURL(Url);
	HttpRequest->SetVerb(Verb);
	HttpRequest->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *Token));
	HttpRequest->SetHeader(TEXT("X-Customer-Domain"), Server);
	HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

	// Set body if provided
	if (JsonBody.IsValid())
	{
		FString RequestBody;
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
		FJsonSerializer::Serialize(JsonBody.ToSharedRef(), Writer);
		HttpRequest->SetContentAsString(RequestBody);
	}

	UE_LOG(LogCavrnusConnector, Log, TEXT("[RestApiClient] %s %s"), *Verb, *Url);

	// Track in-flight request for cancellation on shutdown
	TWeakPtr<IHttpRequest, ESPMode::ThreadSafe> WeakRequest(HttpRequest);
	PendingRequests.Add(WeakRequest);

	// Bind response handler
	HttpRequest->OnProcessRequestComplete().BindLambda(
		[Callback, WeakRequest](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
		{
			// Remove from pending tracking
			PendingRequests.RemoveAll([&WeakRequest](const TWeakPtr<IHttpRequest, ESPMode::ThreadSafe>& Pending)
			{
				return !Pending.IsValid() || Pending.Pin() == WeakRequest.Pin();
			});

			// Layer 2: Drop response if system has shut down since the request was sent
			if (!Cavrnus::CavrnusRelayModel::IsAlive())
			{
				UE_LOG(LogCavrnusConnector, Log, TEXT("[RestApiClient] System shut down — dropping late HTTP response."));
				return;
			}

			FCavrnusRestApiResponse ApiResponse;

			if (!bConnectedSuccessfully || !Response.IsValid())
			{
				ApiResponse.bSuccess = false;
				ApiResponse.StatusCode = 0;
				ApiResponse.ErrorMessage = TEXT("HTTP request failed: connection error.");
			}
			else
			{
				ApiResponse.StatusCode = Response->GetResponseCode();
				ApiResponse.bSuccess = (ApiResponse.StatusCode >= 200 && ApiResponse.StatusCode < 300);

				const FString ResponseBody = Response->GetContentAsString();

				if (!ResponseBody.IsEmpty())
				{
					TSharedPtr<FJsonObject> ParsedJson;
					TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseBody);
					if (FJsonSerializer::Deserialize(Reader, ParsedJson))
					{
						ApiResponse.JsonBody = ParsedJson;
					}
				}

				if (!ApiResponse.bSuccess)
				{
					// Try to extract server-provided error message from JSON body
					FString ServerMessage;
					if (ApiResponse.JsonBody.IsValid())
					{
						// Try common error message fields
						if (!ApiResponse.JsonBody->TryGetStringField(TEXT("message"), ServerMessage))
						{
							ApiResponse.JsonBody->TryGetStringField(TEXT("error"), ServerMessage);
						}
					}

					if (!ServerMessage.IsEmpty())
					{
						ApiResponse.ErrorMessage = FString::Printf(TEXT("%s (Error Code: %d)"), *ServerMessage, ApiResponse.StatusCode);
					}
					else
					{
						// No server message — provide a human-readable fallback based on status code
						switch (ApiResponse.StatusCode)
						{
						case 401:
							ApiResponse.ErrorMessage = TEXT("Authentication expired or invalid. Please log in again. (Error Code: 401)");
							break;
						case 403:
							ApiResponse.ErrorMessage = TEXT("Permission denied. You do not have access to perform this operation. (Error Code: 403)");
							break;
						case 404:
							ApiResponse.ErrorMessage = TEXT("Resource not found. The requested item may have been deleted or the ID is invalid. (Error Code: 404)");
							break;
						case 429:
							ApiResponse.ErrorMessage = TEXT("Rate limited. Please try again later. (Error Code: 429)");
							break;
						default:
							ApiResponse.ErrorMessage = FString::Printf(TEXT("%s (Error Code: %d)"),
								ResponseBody.IsEmpty() ? TEXT("Unknown error") : *ResponseBody,
								ApiResponse.StatusCode);
							break;
						}
					}

					UE_LOG(LogCavrnusConnector, Warning, TEXT("[RestApiClient] Request failed: %s"), *ApiResponse.ErrorMessage);
				}
			}

			// Dispatch callback on game thread with a final IsAlive guard.
			// The outer lambda's check can pass before shutdown, but this
			// AsyncTask may run after KillDataModel has already fired.
			AsyncTask(ENamedThreads::GameThread, [Callback, ApiResponse]()
			{
				if (!Cavrnus::CavrnusRelayModel::IsAlive())
				{
					UE_LOG(LogCavrnusConnector, Log, TEXT("[RestApiClient] System shut down — dropping queued REST callback."));
					return;
				}
				Callback(ApiResponse);
			});
		});

	HttpRequest->ProcessRequest();
}

void FCavrnusRestApiClient::CancelPendingRequests()
{
	int32 CancelledCount = 0;
	for (const TWeakPtr<IHttpRequest, ESPMode::ThreadSafe>& WeakReq : PendingRequests)
	{
		TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> Req = WeakReq.Pin();
		if (Req.IsValid())
		{
			Req->CancelRequest();
			CancelledCount++;
		}
	}
	PendingRequests.Empty();

	if (CancelledCount > 0)
	{
		UE_LOG(LogCavrnusConnector, Log, TEXT("[RestApiClient] Cancelled %d in-flight HTTP request(s) during shutdown."), CancelledCount);
	}
}

FString FCavrnusRestApiClient::GetBaseUrl(const FString& Server)
{
	// Server is e.g. "cavrnus.cavrn.us" — API lives at "api.cavrn.us" (root domain).
	// Extract base domain (last two segments) so we don't create an invalid subdomain
	// like "api.cavrnus.cavrn.us" that won't match the SSL wildcard cert.
	TArray<FString> Parts;
	Server.ParseIntoArray(Parts, TEXT("."));
	const FString BaseDomain = Parts.Num() >= 2
		? FString::Printf(TEXT("%s.%s"), *Parts[Parts.Num() - 2], *Parts[Parts.Num() - 1])
		: Server;

	return FString::Printf(TEXT("https://api.%s/api/"), *BaseDomain);
}

void FCavrnusRestApiClient::Notify(ECavrnusApiNotifyAction Action, const FString& OperationName, bool bSuccess, const FString& Message)
{
	if (Action == ECavrnusApiNotifyAction::None)
	{
		return;
	}

	if (Action == ECavrnusApiNotifyAction::LogOnly)
	{
		if (bSuccess)
		{
			UE_LOG(LogCavrnusConnector, Log, TEXT("[%s] Success: %s"), *OperationName, *Message);
		}
		else
		{
			UE_LOG(LogCavrnusConnector, Warning, TEXT("[%s] Failed: %s"), *OperationName, *Message);
		}
		return;
	}

	if (Action == ECavrnusApiNotifyAction::Toast)
	{
		// UI systems may have been torn down (PIE end, shutdown) by the time async callbacks fire
		UCavrnusUISystems* UI = UCavrnusUI::Get();
		if (!UI || !UI->Messages() || !UI->Messages()->Toast())
		{
			UE_LOG(LogCavrnusConnector, Warning, TEXT("[%s] Toast requested but UI system unavailable: %s"), *OperationName, *Message);
			return;
		}

		UCavrnusInfoToastMessageWidget* Toast = UI->Messages()->Toast()->CreateAutoClose<UCavrnusInfoToastMessageWidget>();
		if (Toast)
		{
			Toast->SetPrimaryText(OperationName);
			Toast->SetSecondaryText(Message);
			Toast->SetType(bSuccess ? ECavrnusInfoToastMessageEnum::Success : ECavrnusInfoToastMessageEnum::Error);
		}
	}
}
