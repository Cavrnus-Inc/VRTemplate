// Copyright (c) 2025 Cavrnus. All rights reserved.

/**
 * @file CavrnusRestApiClient.h
 * @brief Generic authenticated REST API client for making calls to the Cavrnus backend.
 *
 * Stateless static utility that reads auth token + server from DataState on each call.
 * Supports GET, POST, PUT, DELETE with JSON body and async callbacks.
 */

#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"
#include "Interfaces/IHttpRequest.h"
#include "Types/CavrnusApiNotifyAction.h"

/**
 * @brief Response from a Cavrnus REST API call.
 */
struct CAVRNUSCONNECTOR_API FCavrnusRestApiResponse
{
	/** Whether the HTTP request succeeded (2xx status code). */
	bool bSuccess = false;

	/** HTTP status code from the response. */
	int32 StatusCode = 0;

	/** Parsed JSON body of the response (may be null on failure). */
	TSharedPtr<FJsonObject> JsonBody;

	/** Error message if the request failed. */
	FString ErrorMessage = "";
};

/** Callback type for REST API responses. */
typedef TFunction<void(const FCavrnusRestApiResponse&)> CavrnusRestApiCallback;

/**
 * @brief Stateless static REST API client for Cavrnus backend calls.
 *
 * Reads auth token and server from DataState on each call.
 * Base URL: https://api.<BaseDomain>/api/  (e.g. cavrnus.cavrn.us → api.cavrn.us)
 * Headers: Authorization: Bearer <token>, X-Customer-Domain: <server>, Content-Type: application/json
 */
class CAVRNUSCONNECTOR_API FCavrnusRestApiClient
{
public:
	/**
	 * @brief Perform an HTTP GET request.
	 * @param RelativePath Path relative to the base API URL (e.g., "join-configs?roomId=abc")
	 * @param Callback Async callback with the response
	 */
	static void Get(const FString& RelativePath, CavrnusRestApiCallback Callback);

	/**
	 * @brief Perform an HTTP POST request with a JSON body.
	 * @param RelativePath Path relative to the base API URL
	 * @param JsonBody JSON object to send as the request body
	 * @param Callback Async callback with the response
	 */
	static void Post(const FString& RelativePath, const TSharedPtr<FJsonObject>& JsonBody, CavrnusRestApiCallback Callback);

	/**
	 * @brief Perform an HTTP PUT request with a JSON body.
	 * @param RelativePath Path relative to the base API URL
	 * @param JsonBody JSON object to send as the request body
	 * @param Callback Async callback with the response
	 */
	static void Put(const FString& RelativePath, const TSharedPtr<FJsonObject>& JsonBody, CavrnusRestApiCallback Callback);

	/**
	 * @brief Perform an HTTP DELETE request.
	 * @param RelativePath Path relative to the base API URL
	 * @param Callback Async callback with the response
	 * @param JsonBody Optional JSON body for the DELETE request
	 */
	static void Delete(const FString& RelativePath, CavrnusRestApiCallback Callback, const TSharedPtr<FJsonObject>& JsonBody = nullptr);

	/**
	 * @brief Cancel all in-flight HTTP requests.
	 * Call during shutdown to prevent stale callbacks from firing after teardown.
	 */
	static void CancelPendingRequests();

private:
	/** Tracked in-flight HTTP requests for clean cancellation on shutdown. */
	static TArray<TWeakPtr<IHttpRequest, ESPMode::ThreadSafe>> PendingRequests;
	/**
	 * @brief Internal method that performs the actual HTTP request.
	 * @param Verb HTTP verb (GET, POST, PUT, DELETE)
	 * @param RelativePath Path relative to the base API URL
	 * @param JsonBody Optional JSON body
	 * @param Callback Async callback with the response
	 */
	static void SendRequest(const FString& Verb, const FString& RelativePath, const TSharedPtr<FJsonObject>& JsonBody, CavrnusRestApiCallback Callback);

	/** @brief Construct the base URL from the current server. */
	static FString GetBaseUrl(const FString& Server);

public:
	/**
	 * @brief Fire a notification (toast or log) for an API operation result.
	 * @param Action The notification action to perform
	 * @param OperationName Human-readable name of the operation (e.g., "Create Join Config")
	 * @param bSuccess Whether the operation succeeded
	 * @param Message Success or error message to display
	 */
	static void Notify(ECavrnusApiNotifyAction Action, const FString& OperationName, bool bSuccess, const FString& Message);
};
