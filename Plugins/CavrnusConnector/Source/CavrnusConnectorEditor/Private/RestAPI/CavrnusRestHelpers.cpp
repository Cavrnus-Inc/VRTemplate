#include "CavrnusRestHelpers.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

void FCavrnusRestHelpers::PostHasSeenSetupScreenMetric()
{
    // Prepare the JSON body
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
    JsonObject->SetStringField("metricId", "csc-setupPrompted");
    JsonObject->SetStringField("source", "unrealEditor");

    TSharedPtr<FJsonObject> TagsObject = MakeShareable(new FJsonObject);
    TagsObject->SetStringField("deviceGuid", GetCavrnusDeviceGuid());

    JsonObject->SetObjectField("tags", TagsObject);

    FString RequestBody;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
    
    // Create the HTTP request
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
    HttpRequest->SetURL("https://api.cavrn.us/api/app-metric");
    HttpRequest->SetVerb("POST");
    HttpRequest->SetHeader("Content-Type", "application/json");
    HttpRequest->SetContentAsString(RequestBody);

    /*UE_LOG(LogTemp, Log, TEXT("Posting Metric %s"), *RequestBody);

    HttpRequest->OnProcessRequestComplete().BindLambda([](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
        {
            if (bWasSuccessful && Response->GetResponseCode() == 200)
            {
                UE_LOG(LogTemp, Log, TEXT("Metric sent successfully: %s"), *Response->GetContentAsString());
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("Failed to send metric. Code: %d, Error: %s"),
                    Response.IsValid() ? Response->GetResponseCode() : -1,
                    *Request->GetURL());
            }
        });*/

    HttpRequest->ProcessRequest();
}

FString FCavrnusRestHelpers::GetCavrnusDeviceGuid()
{
    FString id;
    //Only do this once
    if (!FPlatformMisc::GetStoredValue(TEXT("Cavrnus"), TEXT("UE"), TEXT("DeviceGuid"), id))
    {
        id = FGuid::NewGuid().ToString();
        FPlatformMisc::SetStoredValue(TEXT("Cavrnus"), TEXT("UE"), TEXT("DeviceGuid"), id);
    }
    return id;
}
