// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "FunctionLibraries/CavrnusSpaceTagsFunctionLibrary.h"
#include "CavrnusConnectorModule.h"
#include "RestAPI/CavrnusRestApiClient.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"

// ============================================
// Fetch Space Tags
// ============================================

void UCavrnusSpaceTagsFunctionLibrary::FetchSpaceTags(const FString& SpaceId, FCavrnusSpaceTagsReceived OnSuccess, FCavrnusSpaceTagsError OnFailure)
{
	CavrnusSpaceTagsCallback SuccessCallback = [OnSuccess](const FCavrnusSpaceTagMap& Tags)
	{
		OnSuccess.ExecuteIfBound(Tags);
	};
	CavrnusSpaceTagsErrorCallback ErrorCallback = [OnFailure](const FString& Error)
	{
		OnFailure.ExecuteIfBound(Error);
	};
	FetchSpaceTags(SpaceId, SuccessCallback, ErrorCallback);
}

void UCavrnusSpaceTagsFunctionLibrary::FetchSpaceTags(const FString& SpaceId, CavrnusSpaceTagsCallback OnSuccess, CavrnusSpaceTagsErrorCallback OnFailure)
{
	const FString Path = FString::Printf(TEXT("rooms/%s/tags"), *SpaceId);

	FCavrnusRestApiClient::Get(Path, [OnSuccess, OnFailure](const FCavrnusRestApiResponse& Response)
	{
		if (Response.bSuccess && Response.JsonBody.IsValid())
		{
			FCavrnusSpaceTagMap Result;

			// API returns {"tags": TagMap} — parse from nested "tags" field
			const TSharedPtr<FJsonObject>* TagsObject;
			if (Response.JsonBody->TryGetObjectField(TEXT("tags"), TagsObject))
			{
				for (const auto& Pair : (*TagsObject)->Values)
				{
					FString Value;
					if (Pair.Value->TryGetString(Value))
					{
						Result.Tags.Add(Pair.Key, Value);
					}
				}
			}
			else
			{
				// Fallback: response is the tag map directly
				for (const auto& Pair : Response.JsonBody->Values)
				{
					FString Value;
					if (Pair.Value->TryGetString(Value))
					{
						Result.Tags.Add(Pair.Key, Value);
					}
				}
			}

			OnSuccess(Result);
		}
		else
		{
			OnFailure(Response.ErrorMessage);
		}
	});
}

// ============================================
// Fetch Space Tag (single)
// ============================================

void UCavrnusSpaceTagsFunctionLibrary::FetchSpaceTag(const FString& SpaceId, const FString& Key, FCavrnusSpaceTagValueReceived OnSuccess, FCavrnusSpaceTagsError OnFailure)
{
	TFunction<void(const FString&)> SuccessCallback = [OnSuccess](const FString& Value)
	{
		OnSuccess.ExecuteIfBound(Value);
	};
	CavrnusSpaceTagsErrorCallback ErrorCallback = [OnFailure](const FString& Error)
	{
		OnFailure.ExecuteIfBound(Error);
	};
	FetchSpaceTag(SpaceId, Key, SuccessCallback, ErrorCallback);
}

void UCavrnusSpaceTagsFunctionLibrary::FetchSpaceTag(const FString& SpaceId, const FString& Key, TFunction<void(const FString&)> OnSuccess, CavrnusSpaceTagsErrorCallback OnFailure)
{
	FetchSpaceTags(SpaceId,
		[Key, OnSuccess, OnFailure](const FCavrnusSpaceTagMap& Tags)
		{
			if (const FString* Found = Tags.Tags.Find(Key))
			{
				OnSuccess(*Found);
			}
			else
			{
				OnFailure(FString::Printf(TEXT("Tag key \"%s\" not found on space."), *Key));
			}
		},
		OnFailure);
}

// ============================================
// Add Space Tag
// ============================================

void UCavrnusSpaceTagsFunctionLibrary::AddSpaceTag(const FString& SpaceId, const FString& Key, const FString& Value, FCavrnusSpaceTagsReceived OnSuccess, FCavrnusSpaceTagsError OnFailure, ECavrnusApiNotifyAction NotifyAction)
{
	CavrnusSpaceTagsCallback SuccessCallback = [OnSuccess](const FCavrnusSpaceTagMap& Tags)
	{
		OnSuccess.ExecuteIfBound(Tags);
	};
	CavrnusSpaceTagsErrorCallback ErrorCallback = [OnFailure](const FString& Error)
	{
		OnFailure.ExecuteIfBound(Error);
	};
	AddSpaceTag(SpaceId, Key, Value, SuccessCallback, ErrorCallback, NotifyAction);
}

void UCavrnusSpaceTagsFunctionLibrary::AddSpaceTag(const FString& SpaceId, const FString& Key, const FString& Value, CavrnusSpaceTagsCallback OnSuccess, CavrnusSpaceTagsErrorCallback OnFailure, ECavrnusApiNotifyAction NotifyAction)
{
	FCavrnusSpaceTagMap SingleTag;
	SingleTag.Tags.Add(Key, Value);
	SetSpaceTags(SpaceId, SingleTag, /*bReplaceAll=*/ false, OnSuccess, OnFailure, NotifyAction);
}

// ============================================
// Set Space Tags
// ============================================

void UCavrnusSpaceTagsFunctionLibrary::SetSpaceTags(const FString& SpaceId, FCavrnusSpaceTagMap Tags, bool bReplaceAll, FCavrnusSpaceTagsReceived OnSuccess, FCavrnusSpaceTagsError OnFailure, ECavrnusApiNotifyAction NotifyAction)
{
	CavrnusSpaceTagsCallback SuccessCallback = [OnSuccess](const FCavrnusSpaceTagMap& ResultTags)
	{
		OnSuccess.ExecuteIfBound(ResultTags);
	};
	CavrnusSpaceTagsErrorCallback ErrorCallback = [OnFailure](const FString& Error)
	{
		OnFailure.ExecuteIfBound(Error);
	};
	SetSpaceTags(SpaceId, Tags, bReplaceAll, SuccessCallback, ErrorCallback, NotifyAction);
}

void UCavrnusSpaceTagsFunctionLibrary::SetSpaceTags(const FString& SpaceId, const FCavrnusSpaceTagMap& Tags, bool bReplaceAll, CavrnusSpaceTagsCallback OnSuccess, CavrnusSpaceTagsErrorCallback OnFailure, ECavrnusApiNotifyAction NotifyAction)
{
	const FString Path = FString::Printf(TEXT("rooms/%s/tags"), *SpaceId);

	TSharedPtr<FJsonObject> JsonBody = MakeShareable(new FJsonObject);

	TSharedPtr<FJsonObject> TagsObj = MakeShareable(new FJsonObject);
	for (const auto& Pair : Tags.Tags)
	{
		TagsObj->SetStringField(Pair.Key, Pair.Value);
	}
	JsonBody->SetObjectField(TEXT("tags"), TagsObj);
	JsonBody->SetBoolField(TEXT("replace"), bReplaceAll);

	FCavrnusRestApiClient::Put(Path, JsonBody, [OnSuccess, OnFailure, NotifyAction, Tags](const FCavrnusRestApiResponse& Response)
	{
		if (Response.bSuccess)
		{
			FCavrnusSpaceTagMap Result;

			// Try to parse tags from the response body
			if (Response.JsonBody.IsValid())
			{
				const TSharedPtr<FJsonObject>* TagsObject;
				if (Response.JsonBody->TryGetObjectField(TEXT("tags"), TagsObject))
				{
					for (const auto& Pair : (*TagsObject)->Values)
					{
						FString Value;
						if (Pair.Value->TryGetString(Value))
						{
							Result.Tags.Add(Pair.Key, Value);
						}
					}
				}
				else
				{
					// Fallback: response is the tag map directly
					for (const auto& Pair : Response.JsonBody->Values)
					{
						FString Value;
						if (Pair.Value->TryGetString(Value))
						{
							Result.Tags.Add(Pair.Key, Value);
						}
					}
				}
			}

			// If server didn't return tags, echo back what was sent
			if (Result.Tags.Num() == 0)
			{
				Result = Tags;
			}

			FCavrnusRestApiClient::Notify(NotifyAction, TEXT("Set Space Tags"), true, TEXT("Space tags updated successfully."));
			OnSuccess(Result);
		}
		else
		{
			FCavrnusRestApiClient::Notify(NotifyAction, TEXT("Set Space Tags"), false, Response.ErrorMessage);
			OnFailure(Response.ErrorMessage);
		}
	});
}

// ============================================
// Delete Space Tags
// ============================================

void UCavrnusSpaceTagsFunctionLibrary::DeleteSpaceTags(const FString& SpaceId, const TArray<FString>& TagKeys, FCavrnusSpaceTagsDeleted OnSuccess, FCavrnusSpaceTagsError OnFailure, ECavrnusApiNotifyAction NotifyAction)
{
	CavrnusSpaceTagsDeletedCallback SuccessCallback = [OnSuccess]()
	{
		OnSuccess.ExecuteIfBound();
	};
	CavrnusSpaceTagsErrorCallback ErrorCallback = [OnFailure](const FString& Error)
	{
		OnFailure.ExecuteIfBound(Error);
	};
	DeleteSpaceTags(SpaceId, TagKeys, SuccessCallback, ErrorCallback, NotifyAction);
}

void UCavrnusSpaceTagsFunctionLibrary::DeleteSpaceTags(const FString& SpaceId, const TArray<FString>& TagKeys, CavrnusSpaceTagsDeletedCallback OnSuccess, CavrnusSpaceTagsErrorCallback OnFailure, ECavrnusApiNotifyAction NotifyAction)
{
	const FString Path = FString::Printf(TEXT("rooms/%s/tags"), *SpaceId);

	TSharedPtr<FJsonObject> JsonBody = MakeShareable(new FJsonObject);
	TArray<TSharedPtr<FJsonValue>> KeysArray;
	for (const FString& Key : TagKeys)
	{
		KeysArray.Add(MakeShareable(new FJsonValueString(Key)));
	}
	JsonBody->SetArrayField(TEXT("tag"), KeysArray);

	FCavrnusRestApiClient::Delete(Path, [OnSuccess, OnFailure, NotifyAction](const FCavrnusRestApiResponse& Response)
	{
		if (Response.bSuccess)
		{
			FCavrnusRestApiClient::Notify(NotifyAction, TEXT("Delete Space Tags"), true, TEXT("Space tags deleted successfully."));
			OnSuccess();
		}
		else
		{
			FCavrnusRestApiClient::Notify(NotifyAction, TEXT("Delete Space Tags"), false, Response.ErrorMessage);
			OnFailure(Response.ErrorMessage);
		}
	}, JsonBody);
}
