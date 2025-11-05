// Copyright (c) 2025 Cavrnus. All rights reserved.

/**
 * @file CavrnusFunctionLibrary.h
 * @brief Declaration of the UCavrnusFunctionLibrary class, which provides a set of static utility functions
 * for interacting with the Cavrnus Spatial Connector plugin in Unreal Engine 5.
 *
 * The UCavrnusFunctionLibrary class includes various functions for authentication, managing spaces, properties,
 * permissions, and handling user connections within the Cavrnus platform. It serves as a blueprint function library
 * allowing developers to easily integrate Cavrnus functionalities into their Unreal Engine projects.
 */

#pragma once

// Includes
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include <Containers/Map.h>
#include <Delegates/Delegate.h>
#include "Types/CavrnusInputDevice.h"
#include "Types/CavrnusOutputDevice.h"
#include "Types/CavrnusVideoInputDevice.h"
#include "Types/CavrnusAuthentication.h"
#include "Types/CavrnusApiKeyData.h"
#include "Types/CavrnusSpaceConnection.h"
#include "Types/CavrnusSpaceConnectionInfo.h"
#include "Types/CavrnusSpaceInfo.h"
#include "Types/CavrnusUser.h"
#include "Types/CavrnusSpawnedObject.h"
#include "Types/CavrnusBinding.h"
#include "Types/CavrnusCallbackTypes.h"
#include "Types/PropertyPostOptions.h"
#include "Types/PropertiesContainer.h"
#include "Types/CavrnusServerStatus.h"
#include "Types/CavrnusConnectionStatusEnum.h"
#include "Types/CavrnusServerMessage.h"
#include "LivePropertyUpdates\CavrnusLivePropertyUpdate.h"

#include <algorithm>

#include "CavrnusFunctionLibrary.generated.h"	// Always last

struct FCavrnusLoginConfig;
// ============================================
// Support enums
// ============================================
UENUM()
enum class EValidInvalidOutputExecPins : uint8
{
	IsValid,
	IsInvalid
};

// ============================================
// Class Definition
// ============================================
UCLASS(Abstract)
class CAVRNUSCONNECTOR_API UCavrnusFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	// Functions
public:

	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus")
	static void SetForceKeepAlive();

	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus")
	static void EndForceKeepAlive();

#pragma region Authentication

	// ============================================
	// Authentication Functions
	// ============================================

	DECLARE_DYNAMIC_DELEGATE(FCavrnusSuccess);
	DECLARE_DYNAMIC_DELEGATE_OneParam(FCavrnusError, FString, Error);

	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Login", meta=(ToolTip="Disable ConnectOnStart in Settings if using this Blueprint Function"))
	static void CavrnusLoginMemberFlow(const FString& OptionalServer);

	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Login", meta=(ToolTip="Disable ConnectOnStart in Settings if using this Blueprint Function"))
	static void CavrnusLoginGuestFlow(const FString& OptionalServer);

	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Login", meta=(ToolTip="Disable ConnectOnStart in Settings if using this Blueprint Function"))
	static void CavrnusLoginMemberFlowWithConfig(const FString& OptionalServer, const FString& OptionalEmail, const FString& OptionalPassword, const FString& OptionalSpaceToAutoJoin);

	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Login", meta=(ToolTip="Disable ConnectOnStart in Settings if using this Blueprint Function"))
	static void CavrnusLoginGuestFlowWithConfig(const FString& OptionalServer, const FString& OptionalGuestName, const FString& OptionalSpaceToAutoJoin);

	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Login", meta=(ToolTip="Disable ConnectOnStart in Settings if using this Blueprint Function"))
	static void CavrnusLoginGlobalSettingsFlow();
	
	static void SetupCavrnusEventHooks();

	/**
	 * @brief Checks if the user is logged in.
	 * @return True if the user is logged in, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Authentication",
		meta = (ToolTip = "Checks if you are logged in", ShortToolTip = "Checks if you are logged in"))
	static bool IsLoggedIn();


	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Authentication",
		meta = (ToolTip = "Sets the server address you will be connecting to", ShortToolTip = "Sets the server address"))
	static void SetServer(const FString& Server);

	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Authentication",
		meta = (ToolTip = "Returns the selected Server string", ShortToolTip = "Returns the selected Server string"))
	static const FString& GetServer();

	DECLARE_DYNAMIC_DELEGATE_OneParam(FCavrnusServerSet, FString, Server);
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Authentication",
		meta = (ToolTip = "Throws an event when server has been finalized (usually this will happen instantly if you have pre-defined the server)", ShortToolTip = "Throws an event when server is finalized"))
	static void AwaitServerSet(FCavrnusServerSet OnServerSet);

	static void AwaitServerSet(CavrnusServerSet OnServerSet);


	DECLARE_DYNAMIC_DELEGATE_OneParam(FCavrnusAuthRecv, FCavrnusAuthentication, Auth);
	/** 
	 * @brief Authenticates the user with a password.
	 * @param Server The server address.
	 * @param Email The user's email.
	 * @param Password The user's password.
	 * @param OnSuccess Delegate called on successful authentication.
	 * @param OnFailure Delegate called on authentication failure.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Authentication",
		meta = (ToolTip = "Logs in the Cavrnus using a username and password", ShortToolTip = "Login with credentials"))
	static void AuthenticateWithPassword(const FString& Server, const FString& Email, const FString& Password, FCavrnusAuthRecv OnSuccess, FCavrnusError OnFailure);

	static void AuthenticateWithPassword(const FString& Server, const FString& Email, const FString& Password, CavrnusAuthRecv OnSuccess, CavrnusError OnFailure);

	/**
	 * @brief Authenticates the user as a guest.
	 * @param Server The server address.
	 * @param UserName The user's name.
	 * @param OnSuccess Delegate called on successful authentication.
	 * @param OnFailure Delegate called on authentication failure.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Authentication",
		meta = (ToolTip = "Logs in to Cavrnus as a guest user", ShortToolTip = "Login as a guest"))
	static void AuthenticateAsGuest(const FString& Server, const FString& UserName, FCavrnusAuthRecv OnSuccess, FCavrnusError OnFailure);

	static void AuthenticateAsGuest(const FString& Server, const FString& UserName, CavrnusAuthRecv OnSuccess, CavrnusError OnFailure);

	/**
	 * @brief Authenticates the user using an apikey.
	 * @param Server The server address. Empty implies Cavrnus cloud servers, with domain to be determined by the key provided
	 * @param AccessKey The access key.
	 * @param AccessToken The (secret) access token.
	 * @param OnSuccess Delegate called on successful authentication.
	 * @param OnFailure Delegate called on authentication failure.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Authentication",
		meta = (ToolTip = "Login using a revokable and storable api-key", ShortToolTip = "Login using an api-key"))
	static void AuthenticateWithApiKey(const FString& Server, const FString& AccessKey, const FString& AccessToken, FCavrnusAuthRecv OnSuccess, FCavrnusError OnFailure);

	static void AuthenticateWithApiKey(const FString& Server, const FString& AccessKey, const FString& AccessToken, CavrnusAuthRecv OnSuccess, CavrnusError OnFailure);

	
	DECLARE_DYNAMIC_DELEGATE_OneParam(FCavrnusDeviceCodeBeginRecv, FCavrnusDeviceCodeData, DeviceCode);
	/**
	 * @brief Request a device code to log in the user via their system web browser, or manually using a second authenticated device.
	 * @param Server The optional server address. Empty implies Cavrnus cloud servers, with domain to be determined by the key provided
	 * @param Source An identifier representing the origin of this request (e.g. unreal-editor, your-app-name, etc)
	 * @param CustomActivatedMessage An optional message that will be presented to the user when authentication completes within their authenticated browser.
	 * @param AutoOpenUrl If true, the system will automatically launch the system web browser with the needed url. If false your integration will need to do this itself, using the DeviceCodeData provided to OnSuccess.
	 * @param OnSuccess Delegate called on successful devicecode initiation. This should be followed with a call to AuthenticateWithDeviceCodeComplete to await authorization, but this gives a chance to update the UI, or manually present the url or entry code when it is necessary (e.g. running on a system with no browser available)
	 * @param OnFailure Delegate called on failure.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Authentication",
		meta = (ToolTip = "Begin a login using the system browser to provide authentication back to the application.", ShortToolTip = "Begin Login using a device-code"))
	static void AuthenticateWithDeviceCodeBegin(const FString& Server, const FString& Source, const FString& CustomActivatedMessage, bool AutoOpenUrl, FCavrnusDeviceCodeBeginRecv OnSuccess, FCavrnusError OnFailure);

	static void AuthenticateWithDeviceCodeBegin(const FString& Server, const FString& Source, const FString& CustomActivatedMessage, bool AutoOpenUrl, CavrnusDeviceCodeRecv OnSuccess, CavrnusError OnFailure);

	/**
	 * @brief Authenticates the user using a requested device code. This call will not fire until the device code times out, or the user authorizes the device code in their browser or alternative device.
	 * @param Server The server address. Empty implies Cavrnus cloud servers, with domain to be determined by the key provided
	 * @param DeviceCode The device code provided by AuthenticateWithDeviceCodeBegin
	 * @param UserCode The user code provided by AuthenticateWithDeviceCodeBegin
	 * @param OnSuccess Delegate called on successful authentication.
	 * @param OnFailure Delegate called on authentication failure.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Authentication",
		meta = (ToolTip = "Await device-code authorization initiated from AuthenticateWithDeviceCodeBegin", ShortToolTip = "Conclude Login using a device-code"))
	static void AuthenticateWithDeviceCodeConclude(const FString& Server, const FString& DeviceCode, const FString& UserCode, FCavrnusAuthRecv OnSuccess, FCavrnusError OnFailure);

	static void AuthenticateWithDeviceCodeConclude(const FString& Server, const FString& DeviceCode, const FString& UserCode, CavrnusAuthRecv OnSuccess, CavrnusError OnFailure);



	DECLARE_DYNAMIC_DELEGATE_OneParam(FCavrnusApiKeyRecv, FCavrnusApiKeyData, ApiKey);
	/**
	 * @brief Create an Api-Key for the current authorized user. Api-Keys can be used to authenticate a user later, and can be revoked or deleted using the Cavrnus Console.
	 * @param Name A name to identify the apikey. This does not have to be unique. If left empty a generic name will be generated.
	 * @param OnSuccess Delegate called on successful apikey creation, with the apikey information provided. NOTE that the accessToken can ONLY be obtained within this callback, and not retrieved by any means at a later date.
	 * @param OnFailure Delegate called on failure, for example, you were not authenticated, or your authenticated user does not have permission to create apikeys.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Authentication",
		meta = (ToolTip = "Create a new ApiKey to later use as a login method", ShortToolTip = "Create an api-key for later login"))
	static void CreateApiKey(const FString& Name, FCavrnusApiKeyRecv OnSuccess, FCavrnusError OnFailure);

	static void CreateApiKey(const FString& Name, CavrnusApiKeyRecv OnSuccess, CavrnusError OnFailure);



	/**
	 * @brief Wait for user authentication to finish. Since there is a potential network delay during authentication, this is used to prevent errors that would occur if commands are issued before authorization is complete.
	 * @param OnAuth Delegate called when authentication is complete.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Authentication",
		meta = (ToolTip = "Throws an event when user authentication is complete", ShortToolTip = "Throws an event when user authentication is complete"))
	static void AwaitAuthentication(FCavrnusAuthRecv OnAuth);

	static void AwaitAuthentication(CavrnusAuthRecv OnAuth);

	DECLARE_DYNAMIC_DELEGATE_OneParam(FCavrnusServerStatusRecv, FCavrnusServerStatus, Status);

	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Server",
		meta = (ToolTip = "Checks the validity and status of a server endpoint and domain, such as 'domain.cavrn.us'", ShortToolTip = "Checks Server Validity"))
	static void CheckServerStatus(const FString& Server, FCavrnusServerStatusRecv OnStatus);
	static void CheckServerStatus(const FString& Server, CavrnusServerStatusRecv OnStatus);

#pragma endregion

#pragma region Spaces

	// ============================================
	// Space Functions
	// ============================================

	DECLARE_DYNAMIC_DELEGATE_OneParam(FCavrnusAllSpacesInfoEvent, const TArray<FCavrnusSpaceInfo>&, SpacesInfo);
	/**
	 * @brief Fetches a list of all joinable spaces.
	 * @param OnRecvCurrentJoinableSpaces Delegate called with the list of joinable spaces.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Spaces",
		meta = (ToolTip = "Gets a list of all current spaces which can be joined", ShortToolTip = "Gets a list of all current spaces which can be joined"))
	static void FetchJoinableSpaces(FCavrnusAllSpacesInfoEvent OnRecvCurrentJoinableSpaces);

	static void FetchJoinableSpaces(CavrnusAllSpacesInfoEvent OnRecvCurrentJoinableSpaces);

	DECLARE_DYNAMIC_DELEGATE_OneParam(FCavrnusSpaceInfoEvent, const FCavrnusSpaceInfo&, SpaceInfo);
	/**
	 * @brief Fetches the SpaceInfo of a specific space
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Spaces",
		meta = (ToolTip = "Fetches the SpaceInfo of a specific space, using either it's ID or it's Join Code", ShortToolTip = "Fetches the SpaceInfo of a specific space"))
	static void FetchSpaceInfo(FString spaceId, FCavrnusSpaceInfoEvent OnRecvSpaceInfo);

	static void FetchSpaceInfo(FString spaceId, CavrnusSpaceInfoEvent OnRecvSpaceInfo);

	/**
	 * @brief Updates the current list of Spaces that can be joined.  Triggers when the metadata of a Space changes, or when the user's access to join it has changed.
	 * @param SpaceAdded Delegate called when a space is added.
	 * @param SpaceUpdated Delegate called when a space is updated.
	 * @param SpaceRemoved Delegate called when a space is removed.
	 * @return A disposable UCavrnusBinding instance.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Spaces",
		meta = (ToolTip = "Triggers when spaces become available to you that you can join, or when their metadata changes", ShortToolTip = "Maintains a list of spaces"))
	static UPARAM(DisplayName = "Disposable") UCavrnusBinding* BindJoinableSpaces(FCavrnusSpaceInfoEvent SpaceAdded, FCavrnusSpaceInfoEvent SpaceUpdated, FCavrnusSpaceInfoEvent SpaceRemoved);

	static UCavrnusBinding* BindJoinableSpaces(CavrnusSpaceInfoEvent SpaceAdded, CavrnusSpaceInfoEvent SpaceUpdated, CavrnusSpaceInfoEvent SpaceRemoved);
	
	/**
	 * @brief Checks if there is any active connection to a space.
	 * @return True if connected to any space, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Spaces",
		meta = (ToolTip = "Checks if there is any active connection to a space", ShortToolTip = "Checks if you are connected to a space"))
	static bool IsConnectedToAnySpace();

	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Spaces",
		meta = (ToolTip = "Gets currents space connections", ShortToolTip = "Gets all space connections"))
	static TArray<FCavrnusSpaceConnectionInfo>& GetCurrentSpaceConnections();
	
	/**
	 * @brief Finds and returns a FCavrnusSpaceConnectionInfo that matches a FCavrnusSpaceConnection.
	 * @param SpaceConn The ID of the space to get the FCavrnusSpaceConnectionInfo of the space requested.
	 * @param Execs Blueprint executable pin that fires on node exit.
	 * @param Info the FCavrnusSpaceConnectionInfo if found.
	 */
	// C++ users should use the other version of this function.
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Spaces",
		meta = (ToolTip = "Retrieves the FCavrnusSpaceConnectionInfo for a space", ShortToolTip = "Retrieve FCavrnusSpaceConnectionInfo", ExpandEnumAsExecs = "Execs"))
	static void GetCavrnusSpaceConnectionInfo(const FCavrnusSpaceConnection& SpaceConn, EValidInvalidOutputExecPins& Execs, FCavrnusSpaceConnectionInfo& SpaceConnectionInfo);

	/**
	 * @brief Finds and returns a FCavrnusSpaceConnectionInfo that matches a FCavrnusSpaceConnection. C++ users should use this function rather than the previous one.
	 * @param SpaceConn The ID of the space to get the FCavrnusSpaceConnectionInfo of the space requested.
	 * @return The FCavrnusSpaceConnectionInfo if found, nullptr if not.
	 */
	static FCavrnusSpaceConnectionInfo* GetCavrnusSpaceConnectionInfo(const FCavrnusSpaceConnection& SpaceConn);

	DECLARE_DYNAMIC_DELEGATE_OneParam(FCavrnusSpaceConnected, FCavrnusSpaceConnection, SpaceConnection);
	/**
	 * @brief Connects to a Space, joining voice & video, chat, and receiving/processing Journal updates.
	 * @param SpaceId The ID of the space to join.
	 * @param OnConnected Delegate called on successful connection.
	 * @param OnFailure Delegate called on connection failure.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Spaces",
		meta = (ToolTip = "Connects to a Space; joining voice & video and recieving/processing the journal", ShortToolTip = "Connects to a Space"))
	static void JoinSpace(FString SpaceId, FCavrnusSpaceConnected OnConnected, FCavrnusError OnFailure);

	static void JoinSpace(FString SpaceId, CavrnusSpaceConnected OnConnected, CavrnusError OnFailure);


	DECLARE_DYNAMIC_DELEGATE_OneParam(FCavrnusSpaceStatusChanged, FCavrnusConnectionStatus, status);

	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Spaces",
		meta = (ToolTip = "Invokes a callback with the status of a joined space, whenever that status changes.", ShortToolTip = "Hook Space Status"))
	static void BindSpaceStatus(FCavrnusSpaceConnection space, FCavrnusSpaceStatusChanged OnStatus);

	static void BindSpaceStatus(FCavrnusSpaceConnection space, CavrnusSpaceStatusChanged OnStatus);


	DECLARE_DYNAMIC_DELEGATE_OneParam(FCavrnusServerMessageEvent, FCavrnusServerMessage, msg);

	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Spaces",
		meta = (ToolTip = "Invokes a callback whenever the server sends a message to the connected space.", ShortToolTip = "Hook Space Server Messaging"))
	static void BindSpaceServerMessages(FCavrnusSpaceConnection space, FCavrnusServerMessageEvent OnStatus);

	static void BindSpaceServerMessages(FCavrnusSpaceConnection space, CavrnusServerMessageEvent OnStatus);


	DECLARE_DYNAMIC_DELEGATE_OneParam(FCavrnusSpaceCreated, FCavrnusSpaceInfo, CreatedSpaceInfo);
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Spaces",
		meta = (ToolTip = "Creates a new Cavrnus Space", ShortToolTip = "Creates a new Cavrnus Space"))
	static void CreateSpace(FString SpaceName, FCavrnusSpaceCreated OnCreation, FCavrnusError OnFailure);

	static void CreateSpace(FString SpaceName, CavrnusSpaceCreated OnCreation, CavrnusError OnFailure);

	static void CreateSpace(FString SpaceName, TArray<FString> Keywords, CavrnusSpaceCreated OnCreation, CavrnusError OnFailure);

	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Spaces",
		meta = (ToolTip = "Permanently Deletes a Cavrnus Space", ShortToolTip = "Permanently Deletes a Cavrnus Space"))
	static void DeleteSpace(FString SpaceId, FCavrnusSuccess OnSuccess, FCavrnusError OnFailure);

	static void DeleteSpace(FString SpaceId, CavrnusSuccess OnSuccess, CavrnusError OnFailure);

	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Spaces",
		meta = (ToolTip = "Renames a Cavrnus Space", ShortToolTip = "Renames a Cavrnus Space"))
	static void RenameSpace(FString SpaceId, FString NewName, FCavrnusSuccess OnSuccess, FCavrnusError OnFailure);

	static void RenameSpace(FString SpaceId, FString NewName, CavrnusSuccess OnSuccess, CavrnusError OnFailure);

	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Spaces",
		meta = (ToolTip = "Uploads a new Thumbnail for a Cavrnus Space", ShortToolTip = "Uploads a Thumbnail for a Cavrnus Space"))
	static void UploadSpaceThumbnail(FString SpaceId, FString ThumbnailFilePath, FCavrnusSuccess OnSuccess, FCavrnusError OnFailure);

	static void UploadSpaceThumbnail(FString SpaceId, FString ThumbnailFilePath, CavrnusSuccess OnSuccess, CavrnusError OnFailure);

	DECLARE_DYNAMIC_DELEGATE_OneParam(FCavrnusUserAccountsFetched, const TArray<FCavrnusUserAccount>&, CavrnusUserAccounts);
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Spaces",
		meta = (ToolTip = "Fetch all User Accounts on the server", ShortToolTip = "Fetch all User Accounts on the server"))
	static void FetchAllUserAccounts(FCavrnusUserAccountsFetched OnAccountsFetched);

	static void FetchAllUserAccounts(CavrnusUserAccountsFetched OnAccountsFetched);

	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Spaces",
		meta = (ToolTip = "Invite the user to a given space", ShortToolTip = "Invite the user to a given space"))
	static void InviteUserToSpace(FCavrnusSpaceInfo SpaceInfo, FCavrnusUserAccount UserAccount);

	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Spaces",
		meta = (ToolTip = "Invite the user to a given space", ShortToolTip = "Invite the user to a given space with callbacks"))
	static void InviteUserToSpaceWithCallbacks(FCavrnusSpaceInfo SpaceInfo, FCavrnusUserAccount UserAccount, FCavrnusSuccess OnSuccess, FCavrnusError OnFailure);

	static void InviteUserToSpace(FCavrnusSpaceInfo SpaceInfo, FCavrnusUserAccount UserAccount, CavrnusSuccess OnSuccess, CavrnusError OnFailure);

	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Spaces",
		meta = (ToolTip = "Invite the user to a given space", ShortToolTip = "Invite the user to a given space with a given role"))
	static void InviteUserToSpaceWithRole(FCavrnusSpaceInfo SpaceInfo, FCavrnusUserAccount UserAccount, FString roleId);

	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Spaces",
		meta = (ToolTip = "Invite the user to a given space", ShortToolTip = "Invite the user to a given space with a given role, with callbacks"))
	static void InviteUserToSpaceWithRoleWithCallbacks(FCavrnusSpaceInfo SpaceInfo, FCavrnusUserAccount UserAccount, FString roleId, FCavrnusSuccess OnSuccess, FCavrnusError OnFailure);

	static void InviteUserToSpaceWithRole(FCavrnusSpaceInfo SpaceInfo, FCavrnusUserAccount UserAccount, FString roleId, CavrnusSuccess OnSuccess, CavrnusError OnFailure);

	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Spaces",
		meta = (ToolTip = "Remove the user from a given space", ShortToolTip = "Remove the user from a given space"))
	static void RemoveUserFromSpace(FCavrnusSpaceInfo SpaceInfo, FCavrnusUserAccount UserAccount);

	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Spaces",
		meta = (ToolTip = "Remove the user from a given space", ShortToolTip = "Remove the user from a given space"))
	static void RemoveUserFromSpaceWithCallbacks(FCavrnusSpaceInfo SpaceInfo, FCavrnusUserAccount UserAccount, FCavrnusSuccess OnSuccess, FCavrnusError OnFailure);
	static void RemoveUserFromSpace(FCavrnusSpaceInfo SpaceInfo, FCavrnusUserAccount UserAccount, CavrnusSuccess OnSuccess, CavrnusError OnFailure);

	/**
	 * @brief Delegate triggered when the process of joining a Space begins.
	 * @param SpaceIdBeingLoaded The ID of the space being joined.
	 */
	DECLARE_DYNAMIC_DELEGATE_OneParam(FCavrnusSpaceBeginLoading, FString, SpaceIdBeingLoaded);
	
	/**
	 * @brief Awaits the beginning of any Space loading process. This triggers a special delegate to trace the identity of the Space.
	 * @param OnConnected Delegate to call when a space begins loading.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Spaces",
		meta = (ToolTip = "Triggers when you begin attempting to join a space, returning the ID of the Space being joined", ShortToolTip = "Triggers when you begin attempting to join a space"))
	static void AwaitAnySpaceBeginLoading(FCavrnusSpaceBeginLoading OnConnected);

	/**
	 * @brief Awaits the beginning of any Space loading process. This triggers a special delegate to trace the identity of the Space.
	 * @param OnConnected Delegate to call when a space begins loading.
	 */
	static void AwaitAnySpaceBeginLoading(CavrnusSpaceBeginLoading OnConnected);

	DECLARE_DYNAMIC_DELEGATE(FCavrnusSpaceEndLoading);

	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Spaces",
		meta = (ToolTip = "Triggers when you begin attempting to join a space, returning the ID of the Space being joined", ShortToolTip = "Triggers when you begin attempting to join a space"))
	static void AwaitAnySpaceEndLoading(FCavrnusSpaceEndLoading OnEndLoading);

	static void AwaitAnySpaceEndLoading(CavrnusSpaceEndLoading OnEndLoading);

	/**
	 * @brief Awaits connection to any Space. This triggers the provided delegate immediately if already connected, or as soon as a connection is made.
	 * @param OnConnected Delegate to call upon successful connection to a space.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Spaces",
		meta = (ToolTip = "Triggers immediately if you are already in a space, otherwise triggers as soon as you connect", ShortToolTip = "Triggers as soon as you connect to a space"))
	static void AwaitAnySpaceConnection(FCavrnusSpaceConnected OnConnected);

	/**
	 * @brief Awaits connection to any space, triggering the provided delegate immediately if already connected, or as soon as a connection is made.
	 * @param OnConnected Delegate to call upon successful connection to a space.
	 */
	static void AwaitAnySpaceConnection(CavrnusSpaceConnected OnConnected);

	/**
	 * @brief Disconnects from the specified space, stopping property updates and losing user & voice connections.
	 * @param SpaceConnection The space connection to disconnect from.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Spaces",
		meta = (ToolTip = "Disconnects you from a given space.  You will stop recieving property updates, and lose user & voice connections", ShortToolTip = "Disconnects you from a given space"))
	static void ExitSpace(FCavrnusSpaceConnection SpaceConnection);

	DECLARE_DYNAMIC_DELEGATE(FCavrnusSpaceExited);
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Spaces",
		meta = (ToolTip = "Triggers when you exit a space", ShortToolTip = "Triggers when you exit a space"))
	static void AwaitAnySpaceExited(FCavrnusSpaceExited OnExit);


	static void AwaitAnySpaceExited(CavrnusSpaceExited OnExit);

#pragma endregion

	// ============================================
	// Property Functions
	// ============================================

#pragma region Generic Prop Functions

	/**
	 * @brief Defines a default value for a generic property within a specified container.
	 * @param SpaceConnection The space connection containing the property.
	 * @param ContainerName The name of the container holding the property.
	 * @param PropertyName The name of the property to define the default value for.
	 * @param PropertyValue The default value to assign to the property.
	 * @param overrideExistingDefault Whether to override an existing default value, defaults to true.
	 */
	static void DefineGenericPropertyDefaultValue(FCavrnusSpaceConnection SpaceConnection, const FPropertiesContainer& ContainerName, const FString& PropertyName, Cavrnus::FPropertyValue PropertyValue, bool overrideExistingDefault = true);


	/**
	 * @brief Retrieves the current value of a generic property within a specified container.
	 * @param SpaceConnection The space connection containing the property.
	 * @param ContainerName The name of the container holding the property.
	 * @param PropertyName The name of the property to retrieve the value from.
	 * @return The current value of the property.
	 */
	static Cavrnus::FPropertyValue GetGenericPropertyValue(FCavrnusSpaceConnection SpaceConnection, const FPropertiesContainer& ContainerName, const FString& PropertyName);

	/**
	 * @brief Binds a callback to updates of a generic property in the Journal.
	 * @param SpaceConnection The space connection containing the property.
	 * @param ContainerName The name of the container holding the property.
	 * @param PropertyName The name of the property to bind the update callback to.
	 * @param OnPropertyUpdated Callback to trigger when the property's value is updated.
	 * @return A disposable binding instance.
	 */
	static UCavrnusBinding* BindGenericPropertyValue(FCavrnusSpaceConnection SpaceConnection, const FPropertiesContainer& ContainerName, const FString& PropertyName, const CavrnusPropertyFunction& OnPropertyUpdated);

	static UCavrnusLivePropertyUpdate* BeginTransientGenericPropertyUpdate(FCavrnusSpaceConnection SpaceConnection, const FPropertiesContainer& ContainerName, const FString& PropertyName, Cavrnus::FPropertyValue PropertyValue);

	/**
	 * @brief Posts an update to a generic property in the Journal.
	 * @param SpaceConnection The space connection containing the property.
	 * @param ContainerName The name of the container holding the property.
	 * @param PropertyName The name of the property to update.
	 * @param PropertyValue The new value to assign to the property.
	 */
	static void PostGenericPropertyUpdate(FCavrnusSpaceConnection SpaceConnection, const FPropertiesContainer& ContainerName, const FString& PropertyName, Cavrnus::FPropertyValue PropertyValue, const FPropertyPostOptions& options = FPropertyPostOptions());

	/**
	 * @brief Test whether a property exists in the journal for the space.
	 * @param SpaceConnection The space connection containing the property.
	 * @param ContainerName The name of the container holding the property.
	 * @param PropertyName The name of the property to query.
	 */
	static bool PropertyValueExists(FCavrnusSpaceConnection SpaceConnection, const FPropertiesContainer& ContainerName, const FString& PropertyName);

#pragma endregion

#pragma region Color Prop Functions
	
	/**
	 * @brief Defines a default value for a color property within a specified container.
	 * @param SpaceConnection The space connection containing the property.
	 * @param ContainerName The name of the container holding the property.
	 * @param PropertyName The name of the property to define the default value for.
	 * @param PropertyValue The default color value to assign to the property.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Properties",
		meta = (ToolTip = "Defines what the application will show if a new prop value has not been assigned", ShortToolTip = "Defines the Propertie's initial value"))
	static void DefineColorPropertyDefaultValue(FCavrnusSpaceConnection SpaceConnection, const FString& ContainerName, const FString& PropertyName, FLinearColor PropertyValue);

	static void DefineColorPropertyDefaultValue(FCavrnusSpaceConnection SpaceConnection, const FPropertiesContainer& ContainerName, const FString& PropertyName, FLinearColor PropertyValue);


	/**
	 * @brief Retrieves the current value of a color property within a specified container.
	 * @param SpaceConnection The space connection containing the property.
	 * @param ContainerName The name of the container holding the property.
	 * @param PropertyName The name of the property to retrieve the value from.
	 * @return The current color value of the property.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Properties",
		meta = (ToolTip = "Gets the current property value, whether the default or the one currently set", ShortToolTip = "Gets the current value"))
	static FLinearColor GetColorPropertyValue(FCavrnusSpaceConnection SpaceConnection, const FString& ContainerName, const FString& PropertyName);

	static FLinearColor GetColorPropertyValue(FCavrnusSpaceConnection SpaceConnection, const FPropertiesContainer& ContainerName, const FString& PropertyName);

	/**
	 * @brief Delegate triggered when a color property is updated.
	 * @param Value The new value of the property.
	 * @param ContainerName The name of the container holding the property.
	 * @param PropertyName The name of the updated property.
	 */
	DECLARE_DYNAMIC_DELEGATE_ThreeParams(FColorPropertyUpdated, FLinearColor, Value, FString, ContainerName, FString, PropertyName);
	/**
	 * @brief Binds a callback to updates of a color property's value. There is an initial event that occurs when the callback is first bound.
	 * @param SpaceConnection The space connection containing the property.
	 * @param ContainerName The name of the container holding the property.
	 * @param PropertyName The name of the property to bind the update callback to.
	 * @param OnPropertyUpdated Callback to trigger when the property's value is updated.
	 * @return A disposable binding instance.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Properties",
		meta = (ToolTip = "Triggers an Event when the property changes, plus an inital event when first bound", ShortToolTip = "Triggers an Event when the property changes"))
	static UPARAM(DisplayName = "Disposable") UCavrnusBinding* BindColorPropertyValue(FCavrnusSpaceConnection SpaceConnection, const FString& ContainerName, const FString& PropertyName, FColorPropertyUpdated OnPropertyUpdated);

	static UCavrnusBinding* BindColorPropertyValue(FCavrnusSpaceConnection SpaceConnection, const FPropertiesContainer& ContainerName, const FString& PropertyName, const CavrnusColorFunction& OnPropertyUpdated);

	/**
	 * @brief Begins a temporary update to a color property's value. The update will show for everyone in the space but will not be saved unless finalized with PostColorPropertyUpdate.
	 * @param SpaceConnection The space connection containing the property.
	 * @param ContainerName The name of the container holding the property.
	 * @param PropertyName The name of the property to update.
	 * @param PropertyValue The temporary color value to assign to the property.
	 * @return A pointer to the live color property update instance.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Properties",
		meta = (ToolTip = "Begins a temporary property update.  This will show for everyone in the space, but will not be saved unless finalized with a PostPropertyUpdate.", ShortToolTip = "Sends a temporary property update to the server"))
	static UCavrnusLiveColorPropertyUpdate* BeginTransientColorPropertyUpdate(FCavrnusSpaceConnection SpaceConnection, const FString& ContainerName, const FString& PropertyName, FLinearColor PropertyValue);

	static UCavrnusLiveColorPropertyUpdate* BeginTransientColorPropertyUpdate(FCavrnusSpaceConnection SpaceConnection, const FPropertiesContainer& ContainerName, const FString& PropertyName, FLinearColor PropertyValue);

	/**
	 * @brief Updates the color property value in the given container and synchronizes the data to the Journal.
	 * @param SpaceConnection The space connection containing the property.
	 * @param ContainerName The name of the container holding the property.
	 * @param PropertyName The name of the property to update.
	 * @param PropertyValue The new color value to assign to the property.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Properties",
		meta = (ToolTip = "Updates the property value at the given path and synchronizes the data to the server", ShortToolTip = "Updates the property value at the given path"))
	static void PostColorPropertyUpdate(FCavrnusSpaceConnection SpaceConnection, const FString& ContainerName, const FString& PropertyName, FLinearColor PropertyValue);

	static void PostColorPropertyUpdate(FCavrnusSpaceConnection SpaceConnection, const FPropertiesContainer& ContainerName, const FString& PropertyName, FLinearColor PropertyValue);

#pragma endregion

#pragma region Bool Prop Functions

	/**
	 * @brief Defines a default value for a boolean property within a specified container.
	 * @param SpaceConnection The space connection containing the property.
	 * @param ContainerName The name of the container holding the property.
	 * @param PropertyName The name of the property to define the default value for.
	 * @param PropertyValue The default boolean value to assign to the property.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Properties",
		meta = (ToolTip = "Defines what the application will show if a new prop value has not been assigned", ShortToolTip = "Defines the Propertie's initial value"))
	static void DefineBoolPropertyDefaultValue(FCavrnusSpaceConnection SpaceConnection, const FString& ContainerName, const FString& PropertyName, bool PropertyValue);

	static void DefineBoolPropertyDefaultValue(FCavrnusSpaceConnection SpaceConnection, const FPropertiesContainer& ContainerName, const FString& PropertyName, bool PropertyValue);


	/**
	 * @brief Retrieves the current value of a boolean property within a specified container.
	 * @param SpaceConnection The space connection containing the property.
	 * @param ContainerName The name of the container holding the property.
	 * @param PropertyName The name of the property to retrieve the value from.
	 * @return The current boolean value of the property.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Properties",
		meta = (ToolTip = "Gets the current property value, whether the default or the one currently set", ShortToolTip = "Gets the current value"))
	static bool GetBoolPropertyValue(FCavrnusSpaceConnection SpaceConnection, const FString& ContainerName, const FString& PropertyName);

	static bool GetBoolPropertyValue(FCavrnusSpaceConnection SpaceConnection, const FPropertiesContainer& ContainerName, const FString& PropertyName);


	/**
	 * @brief Delegate triggered when a boolean property is updated.
	 * @param Value The new value of the property.
	 * @param ContainerName The name of the container holding the property.
	 * @param PropertyName The name of the updated property.
	 */
	DECLARE_DYNAMIC_DELEGATE_ThreeParams(FBoolPropertyUpdated, bool, Value, FString, ContainerName, FString, PropertyName);
	
	/**
	 * @brief Binds a callback to updates of a boolean property's value.
	 * @param SpaceConnection The space connection containing the property.
	 * @param ContainerName The name of the container holding the property.
	 * @param PropertyName The name of the property to bind the update callback to.
	 * @param OnPropertyUpdated Callback to trigger when the property's value is updated.
	 * @return A disposable binding instance.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Properties",
		meta = (ToolTip = "Triggers an Event when the property changes, plus an inital event when first bound", ShortToolTip = "Triggers an Event when the property changes"))
	static UPARAM(DisplayName = "Disposable") UCavrnusBinding* BindBooleanPropertyValue(FCavrnusSpaceConnection SpaceConnection, const FString& ContainerName, const FString& PropertyName, FBoolPropertyUpdated OnPropertyUpdated);

	static UCavrnusBinding* BindBooleanPropertyValue(FCavrnusSpaceConnection SpaceConnection, const FPropertiesContainer& ContainerName, const FString& PropertyName, const CavrnusBoolFunction& OnPropertyUpdated);

	/**
	 * @brief Begins a temporary update to a boolean property's value. The update will show for everyone in the space but will not be saved unless finalized with PostBoolPropertyUpdate.
	 * @param SpaceConnection The space connection containing the property.
	 * @param ContainerName The name of the container holding the property.
	 * @param PropertyName The name of the property to update.
	 * @param PropertyValue The temporary boolean value to assign to the property.
	 * @return A pointer to the live boolean property update instance.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Properties",
		meta = (ToolTip = "Begins a temporary property update.  This will show for everyone in the space, but will not be saved unless finalized with a PostPropertyUpdate.", ShortToolTip = "Sends a temporary property update to the server"))
	static UCavrnusLiveBoolPropertyUpdate* BeginTransientBoolPropertyUpdate(FCavrnusSpaceConnection SpaceConnection, const FString& ContainerName, const FString& PropertyName, bool PropertyValue);

	static UCavrnusLiveBoolPropertyUpdate* BeginTransientBoolPropertyUpdate(FCavrnusSpaceConnection SpaceConnection, const FPropertiesContainer& ContainerName, const FString& PropertyName, bool PropertyValue);

	/**
	 * @brief Posts an update to a boolean property's value, synchronizing the data to the server.
	 * @param SpaceConnection The space connection containing the property.
	 * @param ContainerName The name of the container holding the property.
	 * @param PropertyName The name of the property to update.
	 * @param PropertyValue The new boolean value to assign to the property.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Properties",
		meta = (ToolTip = "Updates the property value at the given path and synchronizes the data to the server", ShortToolTip = "Updates the property value at the given path"))
	static void PostBoolPropertyUpdate(FCavrnusSpaceConnection SpaceConnection, const FString& ContainerName, const FString& PropertyName, bool PropertyValue);

	static void PostBoolPropertyUpdate(FCavrnusSpaceConnection SpaceConnection, const FPropertiesContainer& ContainerName, const FString& PropertyName, bool PropertyValue);

#pragma endregion

#pragma region Float Prop Functions

	/**
	 * @brief Defines a default value for a float property within a specified container.
	 * @param SpaceConnection The space connection containing the property.
	 * @param ContainerName The name of the container holding the property.
	 * @param PropertyName The name of the property to define the default value for.
	 * @param PropertyValue The default float value to assign to the property.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Properties",
		meta = (ToolTip = "Defines what the application will show if a new prop value has not been assigned", ShortToolTip = "Defines the Propertie's initial value"))
	static void DefineFloatPropertyDefaultValue(FCavrnusSpaceConnection SpaceConnection, const FString& ContainerName, const FString& PropertyName, float PropertyValue);

	static void DefineFloatPropertyDefaultValue(FCavrnusSpaceConnection SpaceConnection, const FPropertiesContainer& ContainerName, const FString& PropertyName, float PropertyValue);

	/**
	 * @brief Retrieves the current value of a float property within a specified container, either set or default.
	 * @param SpaceConnection The space connection containing the property.
	 * @param ContainerName The name of the container holding the property.
	 * @param PropertyName The name of the property to retrieve the value from.
	 * @return The current float value of the property.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Properties",
		meta = (ToolTip = "Gets the current property value, whether the default or the one currently set", ShortToolTip = "Gets the current value"))
	static float GetFloatPropertyValue(FCavrnusSpaceConnection SpaceConnection, const FString& ContainerName, const FString& PropertyName);

	static float GetFloatPropertyValue(FCavrnusSpaceConnection SpaceConnection, const FPropertiesContainer& ContainerName, const FString& PropertyName);

	/**
	 * @brief Delegate triggered when a float property is updated.
	 * @param Value The new value of the property.
	 * @param ContainerName The name of the container holding the property.
	 * @param PropertyName The name of the updated property.
	 */
	DECLARE_DYNAMIC_DELEGATE_ThreeParams(FFloatPropertyUpdated, float, Value, FString, ContainerName, FString, PropertyName);
	
	/**
	 * @brief Binds a callback to when the property changes, plus an inital event when first bound.
	 * @param SpaceConnection The space connection containing the property.
	 * @param ContainerName The name of the container holding the property.
	 * @param PropertyName The name of the property to bind the update callback to.
	 * @param OnPropertyUpdated Callback to trigger when the property's value is updated.
	 * @return A disposable binding instance.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Properties",
		meta = (ToolTip = "Triggers an Event when the property changes, plus an inital event when first bound", ShortToolTip = "Triggers an Event when the property changes"))
	static UPARAM(DisplayName = "Disposable") UCavrnusBinding* BindFloatPropertyValue(FCavrnusSpaceConnection SpaceConnection, const FString& ContainerName, const FString& PropertyName, FFloatPropertyUpdated OnPropertyUpdated);
	
	static UCavrnusBinding* BindFloatPropertyValue(FCavrnusSpaceConnection SpaceConnection, const FPropertiesContainer& ContainerName, const FString& PropertyName, const CavrnusFloatFunction& OnPropertyUpdated);

	/**
	 * @brief Begins a temporary update to a float property's value.
	 *        This update will be visible to everyone in the space but will not be saved unless finalized with PostFloatPropertyUpdate.
	 * @param SpaceConnection The space connection containing the property.
	 * @param ContainerName The name of the container holding the property.
	 * @param PropertyName The name of the property to update.
	 * @param PropertyValue The temporary float value to assign to the property.
	 * @return A pointer to the live float property update instance.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Properties",
		meta = (ToolTip = "Begins a temporary property update.  This will show for everyone in the space, but will not be saved unless finalized with a PostPropertyUpdate.", ShortToolTip = "Sends a temporary property update to the server"))
	static UCavrnusLiveFloatPropertyUpdate* BeginTransientFloatPropertyUpdate(FCavrnusSpaceConnection SpaceConnection, const FString& ContainerName, const FString& PropertyName, float PropertyValue);

	static UCavrnusLiveFloatPropertyUpdate* BeginTransientFloatPropertyUpdate(FCavrnusSpaceConnection SpaceConnection, const FPropertiesContainer& ContainerName, const FString& PropertyName, float PropertyValue);

	/**
	 * @brief Posts an update to a float property's value, synchronizing the data to the server.
	 * @param SpaceConnection The space connection containing the property.
	 * @param ContainerName The name of the container holding the property.
	 * @param PropertyName The name of the property to update.
	 * @param PropertyValue The new float value to assign to the property.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Properties",
		meta = (ToolTip = "Updates the property value at the given path and synchronizes the data to the server", ShortToolTip = "Updates the property value at the given path"))
	static void PostFloatPropertyUpdate(FCavrnusSpaceConnection SpaceConnection, const FString& ContainerName, const FString& PropertyName, float PropertyValue);

	static void PostFloatPropertyUpdate(FCavrnusSpaceConnection SpaceConnection, const FPropertiesContainer& ContainerName, const FString& PropertyName, float PropertyValue);

#pragma endregion

#pragma region String Prop Functions

	/**
	 * @brief Defines a default value for a string property within a specified container.
	 * @param SpaceConnection The space connection containing the property.
	 * @param ContainerName The name of the container holding the property.
	 * @param PropertyName The name of the property to define the default value for.
	 * @param PropertyValue The default string value to assign to the property.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Properties",
		meta = (ToolTip = "Defines what the application will show if a new prop value has not been assigned", ShortToolTip = "Defines the Propertie's initial value"))
	static void DefineStringPropertyDefaultValue(FCavrnusSpaceConnection SpaceConnection, const FString& ContainerName, const FString& PropertyName, FString PropertyValue);

	static void DefineStringPropertyDefaultValue(FCavrnusSpaceConnection SpaceConnection, const FPropertiesContainer& ContainerName, const FString& PropertyName, FString PropertyValue);

	/**
	 * @brief Retrieves the current value of a string property within a specified container.
	 * @param SpaceConnection The space connection containing the property.
	 * @param ContainerName The name of the container holding the property.
	 * @param PropertyName The name of the property to retrieve the value from.
	 * @return The current string value of the property.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Properties",
		meta = (ToolTip = "Gets the current property value, whether the default or the one currently set", ShortToolTip = "Gets the current value"))
	static FString GetStringPropertyValue(FCavrnusSpaceConnection SpaceConnection, const FString& ContainerName, const FString& PropertyName);

	static FString GetStringPropertyValue(FCavrnusSpaceConnection SpaceConnection, const FPropertiesContainer& ContainerName, const FString& PropertyName);

	/**
	 * @brief Delegate triggered when a string property is updated.
	 * @param Value The new value of the property.
	 * @param ContainerName The name of the container holding the property.
	 * @param PropertyName The name of the updated property.
	 */
	DECLARE_DYNAMIC_DELEGATE_ThreeParams(FStringPropertyUpdated, FString, Value, FString, ContainerName, FString, PropertyName);
	
	/**
	 * @brief Binds a callback to updates of a string property's value.
	 * @param SpaceConnection The space connection containing the property.
	 * @param ContainerName The name of the container holding the property.
	 * @param PropertyName The name of the property to bind the update callback to.
	 * @param OnPropertyUpdated Callback to trigger when the property's value is updated.
	 * @return A disposable binding instance.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Properties",
		meta = (ToolTip = "Triggers an Event when the property changes, plus an inital event when first bound", ShortToolTip = "Triggers an Event when the property changes"))
	static UPARAM(DisplayName = "Disposable") UCavrnusBinding* BindStringPropertyValue(FCavrnusSpaceConnection SpaceConnection, const FString& ContainerName, const FString& PropertyName, FStringPropertyUpdated OnPropertyUpdated);
	
	static UCavrnusBinding* BindStringPropertyValue(FCavrnusSpaceConnection SpaceConnection, const FPropertiesContainer& ContainerName, const FString& PropertyName, const CavrnusStringFunction& OnPropertyUpdated);

	/**
	 * @brief Begins a temporary update to a string property's value.
	 *        This update will be visible to everyone in the space but will not be saved unless finalized with PostStringPropertyUpdate.
	 * @param SpaceConnection The space connection containing the property.
	 * @param ContainerName The name of the container holding the property.
	 * @param PropertyName The name of the property to update.
	 * @param PropertyValue The temporary string value to assign to the property.
	 * @return A pointer to the live string property update instance.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Properties",
		meta = (ToolTip = "Begins a temporary property update.  This will show for everyone in the space, but will not be saved unless finalized with a PostPropertyUpdate.", ShortToolTip = "Sends a temporary property update to the server"))
	static UCavrnusLiveStringPropertyUpdate* BeginTransientStringPropertyUpdate(FCavrnusSpaceConnection SpaceConnection, const FString& ContainerName, const FString& PropertyName, FString PropertyValue);

	static UCavrnusLiveStringPropertyUpdate* BeginTransientStringPropertyUpdate(FCavrnusSpaceConnection SpaceConnection, const FPropertiesContainer& ContainerName, const FString& PropertyName, FString PropertyValue);

	/**
	 * @brief Posts an update to a string property's value, synchronizing the data to the server.
	 * @param SpaceConnection The space connection containing the property.
	 * @param ContainerName The name of the container holding the property.
	 * @param PropertyName The name of the property to update.
	 * @param PropertyValue The new string value to assign to the property.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Properties",
		meta = (ToolTip = "Updates the property value at the given path and synchronizes the data to the server", ShortToolTip = "Updates the property value at the given path"))
	static void PostStringPropertyUpdate(FCavrnusSpaceConnection SpaceConnection, const FString& ContainerName, const FString& PropertyName, FString PropertyValue);
	
	static void PostStringPropertyUpdate(FCavrnusSpaceConnection SpaceConnection, const FPropertiesContainer& ContainerName, const FString& PropertyName, FString PropertyValue);

#pragma endregion

#pragma region Vector Prop Functions

	/**
	 * @brief Defines a default value for a vector property within a specified container.
	 * @param SpaceConnection The space connection containing the property.
	 * @param ContainerName The name of the container holding the property.
	 * @param PropertyName The name of the property to define the default value for.
	 * @param PropertyValue The default vector value (FVector4) to assign to the property.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Properties",
		meta = (ToolTip = "Defines what the application will show if a new prop value has not been assigned", ShortToolTip = "Defines the Propertie's initial value"))
	static void DefineVectorPropertyDefaultValue(FCavrnusSpaceConnection SpaceConnection, const FString& ContainerName, const FString& PropertyName, FVector4 PropertyValue);

	static void DefineVectorPropertyDefaultValue(FCavrnusSpaceConnection SpaceConnection, const FPropertiesContainer& ContainerName, const FString& PropertyName, FVector4 PropertyValue);

	/**
	 * @brief Retrieves the current value of a vector property within a specified container.
	 * @param SpaceConnection The space connection containing the property.
	 * @param ContainerName The name of the container holding the property.
	 * @param PropertyName The name of the property to retrieve the value from.
	 * @return The current vector value (FVector4) of the property.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Properties",
		meta = (ToolTip = "Gets the current property value, whether the default or the one currently set", ShortToolTip = "Gets the current value"))
	static FVector4 GetVectorPropertyValue(FCavrnusSpaceConnection SpaceConnection, const FString& ContainerName, const FString& PropertyName);

	static FVector4 GetVectorPropertyValue(FCavrnusSpaceConnection SpaceConnection, const FPropertiesContainer& ContainerName, const FString& PropertyName);

	/**
	 * @brief Delegate triggered when a vector property is updated.
	 * @param Value The new value of the property.
	 * @param ContainerName The name of the container holding the property.
	 * @param PropertyName The name of the updated property.
	 */
	DECLARE_DYNAMIC_DELEGATE_ThreeParams(FVectorPropertyUpdated, FVector4, Value, FString, ContainerName, FString, PropertyName);
	/**
	 * @brief Binds a callback to updates of a vector property's value.
	 * @param SpaceConnection The space connection containing the property.
	 * @param ContainerName The name of the container holding the property.
	 * @param PropertyName The name of the property to bind the update callback to.
	 * @param OnPropertyUpdated Callback to trigger when the property's value is updated.
	 * @return A disposable binding instance.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Properties",
		meta = (ToolTip = "Triggers an Event when the property changes, plus an inital event when first bound", ShortToolTip = "Triggers an Event when the property changes"))
	static UPARAM(DisplayName = "Disposable") UCavrnusBinding* BindVectorPropertyValue(FCavrnusSpaceConnection SpaceConnection, const FString& ContainerName, const FString& PropertyName, FVectorPropertyUpdated OnPropertyUpdated);
	
	static UCavrnusBinding* BindVectorPropertyValue(FCavrnusSpaceConnection SpaceConnection, const FPropertiesContainer& ContainerName, const FString& PropertyName, const CavrnusVectorFunction& OnPropertyUpdated);

	/**
	 * @brief Begins a temporary update to a vector property's value.
	 *        This update will be visible to everyone in the space but will not be saved unless finalized with PostVectorPropertyUpdate.
	 * @param SpaceConnection The space connection containing the property.
	 * @param ContainerName The name of the container holding the property.
	 * @param PropertyName The name of the property to update.
	 * @param PropertyValue The temporary vector value (FVector4) to assign to the property.
	 * @return A pointer to the live vector property update instance.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Properties",
		meta = (ToolTip = "Begins a temporary property update.  This will show for everyone in the space, but will not be saved unless finalized with a PostPropertyUpdate.", ShortToolTip = "Sends a temporary property update to the server"))
	static UCavrnusLiveVectorPropertyUpdate* BeginTransientVectorPropertyUpdate(FCavrnusSpaceConnection SpaceConnection, const FString& ContainerName, const FString& PropertyName, FVector4 PropertyValue);

	static UCavrnusLiveVectorPropertyUpdate* BeginTransientVectorPropertyUpdate(FCavrnusSpaceConnection SpaceConnection, const FPropertiesContainer& ContainerName, const FString& PropertyName, FVector4 PropertyValue);

	/**
	 * @brief Posts an update to a vector property's value, synchronizing the data to the server.
	 * @param SpaceConnection The space connection containing the property.
	 * @param ContainerName The name of the container holding the property.
	 * @param PropertyName The name of the property to update.
	 * @param PropertyValue The new vector value (FVector4) to assign to the property.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Properties",
		meta = (ToolTip = "Updates the property value at the given path and synchronizes the data to the server", ShortToolTip = "Updates the property value at the given path"))
	static void PostVectorPropertyUpdate(FCavrnusSpaceConnection SpaceConnection, const FString& ContainerName, const FString& PropertyName, FVector4 PropertyValue);

	static void PostVectorPropertyUpdate(FCavrnusSpaceConnection SpaceConnection, const FPropertiesContainer& ContainerName, const FString& PropertyName, FVector4 PropertyValue);

#pragma endregion

#pragma region Transform Prop Functions

	/**
	 * @brief Defines a default value for a transform property within a specified container.
	 * @param SpaceConnection The space connection containing the property.
	 * @param ContainerName The name of the container holding the property.
	 * @param PropertyName The name of the property to define the default value for.
	 * @param PropertyValue The default transform value (FTransform) to assign to the property.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Properties",
		meta = (ToolTip = "Defines what the application will show if a new prop value has not been assigned", ShortToolTip = "Defines the Propertie's initial value"))
	static void DefineTransformPropertyDefaultValue(FCavrnusSpaceConnection SpaceConnection, const FString& ContainerName, const FString& PropertyName, FTransform PropertyValue);

	static void DefineTransformPropertyDefaultValue(FCavrnusSpaceConnection SpaceConnection, const FPropertiesContainer& ContainerName, const FString& PropertyName, FTransform PropertyValue);


	/**
	 * @brief Retrieves the current value of a transform property within a specified container.
	 * @param SpaceConnection The space connection containing the property.
	 * @param ContainerName The name of the container holding the property.
	 * @param PropertyName The name of the property to retrieve the value from.
	 * @return The current transform value (FTransform) of the property.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Internal",
		meta = (ToolTip = "Gets the current property value, whether the default or the one currently set", ShortToolTip = "Gets the current value"))
	static FTransform GetTransformPropertyValue(FCavrnusSpaceConnection SpaceConnection, const FString& ContainerName, const FString& PropertyName);

	static FTransform GetTransformPropertyValue(FCavrnusSpaceConnection SpaceConnection, const FPropertiesContainer& ContainerName, const FString& PropertyName);

	/**
	 * @brief Delegate triggered when a transform property is updated.
	 * @param Value The new value of the property.
	 * @param ContainerName The name of the container holding the property.
	 * @param PropertyName The name of the updated property.
	 */
	DECLARE_DYNAMIC_DELEGATE_ThreeParams(FTransformPropertyUpdated, FTransform, Value, FString, ContainerName, FString, PropertyName);
	/**
	 * @brief Binds a callback to updates of a transform property's value.
	 * @param SpaceConnection The space connection containing the property.
	 * @param ContainerName The name of the container holding the property.
	 * @param PropertyName The name of the property to bind the update callback to.
	 * @param OnPropertyUpdated Callback to trigger when the property's value is updated.
	 * @return A disposable binding instance.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Properties",
		meta = (ToolTip = "Triggers an Event when the property changes, plus an inital event when first bound", ShortToolTip = "Triggers an Event when the property changes"))
	static UPARAM(DisplayName = "Disposable") UCavrnusBinding* BindTransformPropertyValue(FCavrnusSpaceConnection SpaceConnection, const FString& ContainerName, const FString& PropertyName, FTransformPropertyUpdated OnPropertyUpdated);
	
	static UCavrnusBinding* BindTransformPropertyValue(FCavrnusSpaceConnection SpaceConnection, const FPropertiesContainer& ContainerName, const FString& PropertyName, const CavrnusTransformFunction& OnPropertyUpdated);

	/**
	 * @brief Begins a temporary update to a transform property's value.
	 *        This update will be visible to everyone in the space but will not be saved unless finalized with PostTransformPropertyUpdate.
	 * @param SpaceConnection The space connection containing the property.
	 * @param ContainerName The name of the container holding the property.
	 * @param PropertyName The name of the property to update.
	 * @param PropertyValue The temporary transform value (FTransform) to assign to the property.
	 * @return A pointer to the live transform property update instance.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Properties",
		meta = (ToolTip = "Begins a temporary property update.  This will show for everyone in the space, but will not be saved unless finalized with a PostPropertyUpdate.", ShortToolTip = "Sends a temporary property update to the server", AutoCreateRefTerm = "options"))
	static UCavrnusLiveTransformPropertyUpdate* BeginTransientTransformPropertyUpdate(FCavrnusSpaceConnection SpaceConnection, const FString& ContainerName, const FString& PropertyName, FTransform PropertyValue, const FPropertyPostOptions& options);

	static UCavrnusLiveTransformPropertyUpdate* BeginTransientTransformPropertyUpdate(FCavrnusSpaceConnection SpaceConnection, const FPropertiesContainer& ContainerName, const FString& PropertyName, FTransform PropertyValue, const FPropertyPostOptions& options);

	/**
	 * @brief Posts an update to a transform property's value, synchronizing the data to the server.
	 * @param SpaceConnection The space connection containing the property.
	 * @param ContainerName The name of the container holding the property.
	 * @param PropertyName The name of the property to update.
	 * @param PropertyValue The new transform value (FTransform) to assign to the property.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Properties",
		meta = (ToolTip = "Updates the property value at the given path and synchronizes the data to the server", ShortToolTip = "Updates the property value at the given path", AutoCreateRefTerm = "options"))
	static void PostTransformPropertyUpdate(FCavrnusSpaceConnection SpaceConnection, const FString& ContainerName, const FString& PropertyName, FTransform PropertyValue, const FPropertyPostOptions& options);

	static void PostTransformPropertyUpdate(FCavrnusSpaceConnection SpaceConnection, const FPropertiesContainer& ContainerName, const FString& PropertyName, FTransform PropertyValue, const FPropertyPostOptions& options);

#pragma endregion

#pragma region Permissions

	// ============================================
	// Permissions
	// ============================================

	/**
	 * @brief Delegate triggered when a policy is updated.
	 * @param Policy The name of the policy that was updated.
	 * @param IsAllowed Boolean indicating whether the policy is allowed or not.
	 */
	DECLARE_DYNAMIC_DELEGATE_TwoParams(FCavrnusPolicyUpdated, FString, Policy, bool, IsAllowed);
	
	/**
	 * @brief Binds a global policy update event for the user.
	 * @param Policy The name of the policy to bind to.
	 * @param OnPolicyUpdated The callback to trigger when the policy is updated.
	 * @return A disposable binding instance.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Internal",
		meta = (ToolTip = "Binds an event to throw when a policy is/isn't allowed for the user (returns false until policies are fetched & resolved)", ShortToolTip = "Throws an event if a policy is allowed"))
	static UPARAM(DisplayName = "Disposable") UCavrnusBinding* BindGlobalPolicy(const FString& Policy, FCavrnusPolicyUpdated OnPolicyUpdated);

	static UCavrnusBinding* BindGlobalPolicy(FString Policy, CavrnusPolicyUpdated OnPolicyUpdated);

	/**
	 * @brief Binds a space-specific policy update event for the user.
	 * @param SpaceConnection The space connection containing the policy.
	 * @param Policy The name of the policy to bind to.
	 * @param OnPolicyUpdated The callback to trigger when the policy is updated.
	 * @return A disposable binding instance.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Internal",
		meta = (ToolTip = "Binds an event to throw when a policy is/isn't allowed for the user in a given space (returns false until policies are fetched & resolved)", ShortToolTip = "Throws an event if a policy is allowed"))
	static UPARAM(DisplayName = "Disposable") UCavrnusBinding* BindSpacePolicy(FCavrnusSpaceConnection SpaceConnection, const FString& Policy, FCavrnusPolicyUpdated OnPolicyUpdated);

	static UCavrnusBinding* BindSpacePolicy(FCavrnusSpaceConnection SpaceConnection, const FString& Policy, CavrnusPolicyUpdated OnPolicyUpdated);

#pragma endregion
	
#pragma region Spawned Objects

	// ============================================
	// Spawned Objects
	// ============================================

	/**
	 * @brief Instantiates the given object with no set properties.
	 *
	 * Note: You will need to pull the Container ID out of the Spawned Object and assign property values to it.
	 *
	 * @param SpaceConnection The space connection in which to spawn the object.
	 * @param UniqueIdentifier A unique identifier for the object.
	 * @param spawnedObjectArrived Callback triggered when the object is successfully spawned.
	 * @return The container name of the spawned object.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Objects",
		meta = (AutoCreateRefTerm = "spawnedObjectArrived",	ToolTip = "Instantiates the given object with no set properties (note you will need to pull the Container ID out of the Spawned Object and assign property values to it)", ShortToolTip = "Instantiates the given object"))
	static const FCavrnusSpawnedObject& SpawnObject(FCavrnusSpaceConnection SpaceConnection, const FString& UniqueIdentifier);

	/**
	 * @brief Destroys the given object.
	 *
	 * @param SpawnedObject The object to be destroyed.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Objects",
		meta = (ToolTip = "Destroys the given object", ShortToolTip = "Destroys the given object"))
	static void DestroyObject(const FCavrnusSpawnedObject& SpawnedObject);

	/**
	 * @brief Register Spawnable Object class type.
	 * @param Identifier The name to match when SpawnObject is called.
	 * @param ClassType The AActor class that will be instantiated.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Objects",
		meta = (ToolTip = "Adds class type to the list", ShortToolTip = "Register Spawnable object type"))
	static void RegisterSpawnableObjectType(const FString& Identifier, TSubclassOf<AActor> ClassType);

	/**
	 * @brief Unregister Spawnable Object class type.
	 * @param Identifier The name to match when SpawnObject is called.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Objects",
		meta = (ToolTip = "Removes class type from the list", ShortToolTip = "Unregister Spawnable object type"))
	static void UnregisterSpawnableObjectType(const FString& Identifier);

#pragma endregion

#pragma region Space Users

	// ============================================
	// Space Users
	// ============================================

	/**
	 * @brief Delegate triggered when a space user event occurs.
	 * @param User The user associated with the event.
	 */
	DECLARE_DYNAMIC_DELEGATE_OneParam(FCavrnusSpaceUserEvent, FCavrnusUser, User);
	
	/**
	 * @brief Awaits the local user in the specified space connection.
	 *
	 * This function returns all users currently in the space.
	 *
	 * @param SpaceConnection The space connection to check for users.
	 * @param LocalUserArrived Callback triggered when the local user is found.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Users",
		meta = (ToolTip = "Returns all users currently in the space", ShortToolTip = "Returns all users currently in the space"))
	static void AwaitLocalUser(FCavrnusSpaceConnection SpaceConnection, FCavrnusSpaceUserEvent LocalUserArrived);

	static void AwaitLocalUser(FCavrnusSpaceConnection SpaceConnection, CavrnusSpaceUserEvent LocalUserArrived);

	/**
	 * @brief Gets all users currently in the specified space connection.
	 *
	 * @param SpaceConnection The space connection to check for users.
	 * @return An array of all users currently in the space.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Users",
		meta = (ToolTip = "Returns all users currently in the space", ShortToolTip = "Returns all users currently in the space"))
	static TArray<FCavrnusUser> GetCurrentSpaceUsers(FCavrnusSpaceConnection SpaceConnection);

	/**
	 * @brief Binds events for when users join or leave the specified space connection.
	 *
	 * This function triggers whenever users join or leave the given space.
	 *
	 * @param SpaceConnection The space connection to bind the events to.
	 * @param UserAdded Callback triggered when a user joins the space.
	 * @param UserRemoved Callback triggered when a user leaves the space.
	 * @return A disposable binding to manage the event.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Users",
		meta = (ToolTip = "Triggers whenever users join or leave a given space", ShortToolTip = "Triggers whenever users join or leave a given space"))
	static UPARAM(DisplayName = "Disposable") UCavrnusBinding* BindSpaceUsers(FCavrnusSpaceConnection SpaceConnection, FCavrnusSpaceUserEvent UserAdded, FCavrnusSpaceUserEvent UserRemoved);

	/**
	 * @brief Binds events for when users join or leave the specified space connection.
	 *
	 * This function triggers whenever users join or leave the given space.
	 *
	 * @param SpaceConnection The space connection to bind the events to.
	 * @param UserAdded Callback triggered when a user joins the space.
	 * @param UserRemoved Callback triggered when a user leaves the space.
	 * @return A disposable binding to manage the event.
	 */
	static UCavrnusBinding* BindSpaceUsers(FCavrnusSpaceConnection SpaceConnection, CavrnusSpaceUserEvent UserAdded, CavrnusSpaceUserEvent UserRemoved);

	/**
	 * @brief Delegate triggered when a user's video frame is updated.
	 * @param UserVideoFrame The updated video frame texture.
	 */
	DECLARE_DYNAMIC_DELEGATE_OneParam(FCavrnusUserVideoFrameEvent, UTexture2D*, UserVideoFrame);

	/**
	 * @brief Binds an event for when a user's video frame updates.
	 *
	 * This function triggers an event when the property changes, plus an initial event when first bound.
	 *
	 * @param SpaceConnection The space connection to bind the event to.
	 * @param User The user whose video frames are being tracked.
	 * @param OnVideoFrameUpdate Callback triggered when the video frame updates.
	 * @return A disposable binding to manage the event.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Properties",
		meta = (ToolTip = "Triggers an Event when the property changes, plus an inital event when first bound", ShortToolTip = "Triggers an Event when the property changes"))
	static UPARAM(DisplayName = "Disposable") UCavrnusBinding* BindUserVideoFrames(FCavrnusSpaceConnection SpaceConnection, const FCavrnusUser& User, FCavrnusUserVideoFrameEvent OnVideoFrameUpdate);

	/**
	 * @brief Binds an event for when a user's video frame updates.
	 *
	 * This function triggers an event when the property changes, plus an initial event when first bound.
	 *
	 * @param SpaceConnection The space connection to bind the event to.
	 * @param User The user whose video frames are being tracked.
	 * @param UpdateFrameCallback Callback triggered when the video frame updates.
	 * @return A disposable binding to manage the event.
	 */
	static UCavrnusBinding* BindUserVideoFrames(FCavrnusSpaceConnection SpaceConnection, const FCavrnusUser& User, const VideoFrameUpdateFunction& UpdateFrameCallback);

#pragma endregion

#pragma region Voice and Video

	// ============================================
	// Voice and Video
	// ============================================

	/**
	 * @brief Sets the muted state for the local user.
	 *
	 * @param SpaceConnection The space connection for the local user.
	 * @param bIsMuted Whether the user should be muted or not.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Voice and Video",
		meta = (ToolTip = "Sets muted state for local user", ShortToolTip = "Set my muted state"))
	static void SetLocalUserMutedState(FCavrnusSpaceConnection SpaceConnection, bool bIsMuted);

	/**
	 * @brief Sets the streaming state for the local user.
	 *
	 * @param SpaceConnection The space connection for the local user.
	 * @param bIsStreaming Whether the user should be streaming or not.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Voice and Video",
		meta = (ToolTip = "Sets streaming state for local user", ShortToolTip = "Set my streaming state"))
	static void SetLocalUserStreamingState(FCavrnusSpaceConnection SpaceConnection, bool bIsStreaming);

	/**
	 * @brief Delegate for receiving the saved input device.
	 *
	 * @param InputDevice The input device that was saved.
	 */
	DECLARE_DYNAMIC_DELEGATE_OneParam(FCavrnusSavedInputDevice, const FCavrnusInputDevice&, InputDevice);
	
	/**
	 * @brief Fetches the last saved audio input device or a default if none is set.
	 *
	 * @param OnReceiveDevice Callback triggered with the saved input device.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Voice and Video",
		meta = (ToolTip = "Gets the last microphone used, or a default if none is set", ShortToolTip = "Gets the last microphone used"))
	static void FetchSavedAudioInput(FCavrnusSavedInputDevice OnReceiveDevice);

	/**
	 * @brief Fetches the last saved audio input device or a default if none is set.
	 *
	 * @param OnReceiveDevice Callback triggered with the saved input device.
	 */
	static void FetchSavedAudioInput(CavrnusSavedInputDevice OnReceiveDevice);

	/**
	 * @brief Delegate for receiving available input devices.
	 *
	 * @param InputDevices The list of available input devices.
	 */
	DECLARE_DYNAMIC_DELEGATE_OneParam(FCavrnusAvailableInputDevices, const TArray<FCavrnusInputDevice>&, InputDevices);
	
	/**
	 * @brief Fetches the available audio input devices.
	 *
	 * @param OnReceiveDevices Callback triggered with the list of available input devices.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Voice and Video",
		meta = (ToolTip = "Gets available microphones", ShortToolTip = "Gets available microphones"))
	static void FetchAudioInputs(FCavrnusAvailableInputDevices OnReceiveDevices);

	/**
	 * @brief Fetches the available audio input devices.
	 *
	 * @param OnReceiveDevices Callback triggered with the list of available input devices.
	 */
	static void FetchAudioInputs(CavrnusAvailableInputDevices OnReceiveDevices);

	/**
	 * @brief Sets the audio input device to use.
	 *
	 * @param Device The audio input device to set.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Voice and Video",
		meta = (ToolTip = "Sets which microphone to use", ShortToolTip = "Sets which microphone to use"))
	static void UpdateAudioInput(FCavrnusInputDevice Device);

	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Voice and Video",
		meta = (ToolTip = "Sets which microphone to use", ShortToolTip = "Sets which microphone to use"))
	static void UpdateAudioInputWithCallback(FCavrnusInputDevice Device, FCavrnusSuccess OnSuccess, FCavrnusError OnFailure);

	static void UpdateAudioInput(FCavrnusInputDevice Device, CavrnusSuccess OnSuccess, CavrnusError OnFailure);


	/**
	 * @brief Delegate for receiving the saved output device.
	 *
	 * @param OutputDevice The output device that was saved.
	 */
	DECLARE_DYNAMIC_DELEGATE_OneParam(FCavrnusSavedOutputDevice, const FCavrnusOutputDevice&, OutputDevice);
	
	/**
	 * @brief Fetches the last saved audio output device or a default if none is set.
	 *
	 * @param OnReceiveDevice Callback triggered with the saved output device.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Voice and Video",
		meta = (ToolTip = "Gets the last speakers used, or a default if none is set", ShortToolTip = "Gets the last speakers used"))
	static void FetchSavedAudioOutput(FCavrnusSavedOutputDevice OnReceiveDevice);

	/**
	 * @brief Fetches the last saved audio output device or a default if none is set.
	 *
	 * @param OnReceiveDevice Callback triggered with the saved output device.
	 */
	static void FetchSavedAudioOutput(CavrnusSavedOutputDevice OnReceiveDevice);

	/**
	 * @brief Delegate for receiving available output devices.
	 *
	 * @param OutputDevices The list of available output devices.
	 */
	DECLARE_DYNAMIC_DELEGATE_OneParam(FCavrnusAvailableOutputDevices, const TArray<FCavrnusOutputDevice>&, OutputDevices);
	
	/**
	 * @brief Fetches the available audio output devices.
	 *
	 * @param OnReceiveDevices Callback triggered with the list of available output devices.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Voice and Video",
		meta = (ToolTip = "Gets available speakers", ShortToolTip = "Gets available speakers"))
	static void FetchAudioOutputs(FCavrnusAvailableOutputDevices OnReceiveDevices);

	/**
	 * @brief Fetches the available audio output devices.
	 *
	 * @param OnReceiveDevices Callback triggered with the list of available output devices.
	 */
	static void FetchAudioOutputs(CavrnusAvailableOutputDevices OnReceiveDevices);

	/**
	 * @brief Sets the audio output device to use.
	 *
	 * @param Device The audio output device to set.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Voice and Video",
		meta = (ToolTip = "Sets which speaker to use", ShortToolTip = "Sets which speaker to use"))
	static void UpdateAudioOutput(FCavrnusOutputDevice Device);

	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Voice and Video",
		meta = (ToolTip = "Sets which speaker to use", ShortToolTip = "Sets which speaker to use"))
	static void UpdateAudioOutputWithCallback(FCavrnusOutputDevice Device, FCavrnusSuccess OnSuccess, FCavrnusError OnFailure);

	static void UpdateAudioOutput(FCavrnusOutputDevice Device, CavrnusSuccess OnSuccess, CavrnusError OnFailure);

	/**
	 * @brief Delegate for receiving available video input devices.
	 *
	 * @param VideoInputDevices The list of available video input devices.
	 */
	DECLARE_DYNAMIC_DELEGATE_OneParam(FCavrnusAvailableVideoInputDevices, const TArray<FCavrnusVideoInputDevice>&, VideoInputDevices);
	
	/**
	 * @brief Fetches the available video input devices.
	 *
	 * @param OnReceiveDevices Callback triggered with the list of available video input devices.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Voice and Video",
		meta = (ToolTip = "Gets available camera/stream sources", ShortToolTip = "Gets available camera/stream sources"))
	static void FetchVideoInputs(FCavrnusAvailableVideoInputDevices OnReceiveDevices);

	/**
	 * @brief Fetches the available video input devices.
	 *
	 * @param OnReceiveDevices Callback triggered with the list of available video input devices.
	 */
	static void FetchVideoInputs(CavrnusAvailableVideoInputDevices OnReceiveDevices);

	/**
	 * @brief Sets the video input device to use.
	 *
	 * @param Device The video input device to set.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Voice and Video",
		meta = (ToolTip = "Sets which camera/stream source to use", ShortToolTip = "Sets which camera/stream source to use"))
	static void UpdateVideoInput(FCavrnusVideoInputDevice Device);

	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Voice and Video",
		meta = (ToolTip = "Sets which camera/stream source to use", ShortToolTip = "Sets which camera/stream source to use"))
	static void UpdateVideoInputWithCallback(FCavrnusVideoInputDevice Device, FCavrnusSuccess OnSuccess, FCavrnusError OnFailure);

	static void UpdateVideoInput(FCavrnusVideoInputDevice Device, CavrnusSuccess OnSuccess, CavrnusError OnFailure);

#pragma endregion

#pragma region Remote Content

	DECLARE_DYNAMIC_DELEGATE_OneParam(FCavrnusRemoteContentInfoFunction, const FCavrnusRemoteContent&, uploadedContent);

	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Content",
		meta = (ToolTip = "Downloads the given content on your device and returns an array of its bytes", ShortToolTip = "Downloads the given content"))
	static void FetchFileInfoById(FString ContentId, FCavrnusRemoteContentInfoFunction OnRecvContentInfo);

	static void FetchFileInfoById(FString ContentId, const CavrnusRemoteContentInfoFunction& OnRecvContentInfo);

	/**
	 * @brief Delegate for handling content download progress.
	 *
	 * @param Progress The current progress of the download, as a percentage.
	 * @param Step The current step in the download process.
	 */
	DECLARE_DYNAMIC_DELEGATE_TwoParams(FCavrnusContentProgressFunction, float, Progress, FString, Step);
	/**
	 * @brief Delegate for handling the downloaded content bytes.
	 *
	 * @param bytes The array of bytes representing the downloaded content.
	 */
	//DECLARE_DYNAMIC_DELEGATE_OneParam(FCavrnusContentFunction, const TArray64<uint8>&, bytes);
	typedef TFunction<void(const TArray64<uint8>&)> CavrnusContentFunctionLargeFile;
	/**
	 * @brief Downloads the given content by its ID and returns an array of its bytes.
	 *
	 * @param ContentId The ID of the content to download.
	 * @param OnProgress The delegate to call with progress updates.
	 * @param OnContentLoaded The delegate to call when the content is loaded.
	 */
	/*
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Content",
		meta = (ToolTip = "Downloads the given content on your device and returns an array of its bytes", ShortToolTip = "Downloads the given content"))
	*/
	static void FetchFileById(FString ContentId, FCavrnusContentProgressFunction OnProgress, CavrnusContentFunctionLargeFile OnContentLoaded, FCavrnusError OnFailure);
	
	/**
	 * @brief Downloads the given content by its ID and returns an array of its bytes.
	 *
	 * @param ContentId The ID of the content to download.
	 * @param OnProgress The delegate to call with progress updates.
	 * @param OnContentLoaded The delegate to call when the content is loaded.
	 */
	static void FetchFileById(FString ContentId, const CavrnusContentProgressFunction& OnProgress, const CavrnusContentFunction& OnContentLoaded, const CavrnusError& OnFailure);

	/**
	 * @brief Delegate for handling the file path of the downloaded content.
	 *
	 * @param fileDest The file path where the content was downloaded.
	 */
	DECLARE_DYNAMIC_DELEGATE_OneParam(FCavrnusContentFileFunction, FString, fileDest);
	/**
	 * @brief Downloads the given content by its ID to a specified disk location and returns the file path.
	 *
	 * @param ContentId The ID of the content to download.
	 * @param FolderDestination The folder path where the content should be saved.
	 * @param OnProgress The delegate to call with progress updates.
	 * @param OnContentLoaded The delegate to call when the content is loaded.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Content",
		meta = (ToolTip = "Downloads the given content to your disk and returns the filepath", ShortToolTip = "Downloads the given content"))
	static void FetchFileByIdToDisk(FString ContentId, FString FolderDestination, FCavrnusContentProgressFunction OnProgress, FCavrnusContentFileFunction OnContentLoaded, FCavrnusError OnFailure);
	
	/**
	 * @brief Downloads the given content by its ID to a specified disk location and returns the file path.
	 *
	 * @param ContentId The ID of the content to download.
	 * @param FileDestination The file path where the content should be saved.
	 * @param OnProgress The delegate to call with progress updates.
	 * @param OnContentLoaded The function to call when the content is loaded.
	 */
	static void FetchFileByIdToDisk(FString ContentId, FString FolderDestination, const CavrnusContentProgressFunction& OnProgress, const TFunction<void(FString)>& OnContentLoaded, const CavrnusError& OnFailure);

	/**
	 * @brief Delegate for handling the available remote content.
	 *
	 * @param availableContent The list of available remote content.
	 */
	DECLARE_DYNAMIC_DELEGATE_OneParam(FCavrnusRemoteContentFunction, const TArray<FCavrnusRemoteContent>&, availableContent);
	
	/**
	 * @brief Fetches metadata on all uploaded content stored on the Cavrnus server.
	 *
	 * @param OnAvailableContentFetched The delegate to call when the available content is fetched.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Content",
		meta = (ToolTip = "Fetch metadata on all uploaded content stored on the Cavrnus server", ShortToolTip = "Fetch metadata on all uploaded content"))
	static void FetchAllUploadedContent(FCavrnusRemoteContentFunction OnAvailableContentFetched);
	
	/**
	 * @brief Fetches metadata on all uploaded content stored on the Cavrnus server.
	 *
	 * @param OnAvailableContentFetched The delegate to call when the available content is fetched.
	 */
	static void FetchAllUploadedContent(const CavrnusRemoteContentFunction& OnAvailableContentFetched);

	static const TMap<FString, FCavrnusRemoteContent>& GetRemoteContent();

	static void AwaitRemoteContentByPredicate(TFunction<bool(const FCavrnusRemoteContent& content)> predicate, TFunction<void(const FCavrnusRemoteContent& content)> onContentArrived);

	/**
	 * @brief Delegate for handling the uploaded content information.
	 *
	 * @param uploadedContent The information of the uploaded content.
	 */
	DECLARE_DYNAMIC_DELEGATE_OneParam(FCavrnusUploadCompleteFunction, const FCavrnusRemoteContent&, uploadedContent);
	
	/**
	 * @brief Encrypts and uploads a file from your disk to your Cavrnus Server.
	 *
	 * @param FilePath The file path of the content to upload.
	 * @param OnUploadComplete The delegate to call when the upload is complete.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Content",
		meta = (ToolTip = "Uploads the given file path to the Cavrnus server", ShortToolTip = "Uploads the given file path"))
	static void UploadContent(FString FilePath, FCavrnusUploadCompleteFunction OnUploadComplete);
	
	/**
	 * @brief Encrypts and uploads a file from your disk to your Cavrnus Server.
	 *
	 * @param FilePath The file path of the content to upload.
	 * @param OnUploadComplete The delegate to call when the upload is complete.
	 */
	static void UploadContent(FString FilePath, const CavrnusUploadCompleteFunction& OnUploadComplete);

	DECLARE_DYNAMIC_DELEGATE_TwoParams(FCavrnusUploadProgressFunction, const FString&, progressStep, float, progress);

	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Content",
		meta = (ToolTip = "Uploads the given file path to the Cavrnus server", ShortToolTip = "Uploads the given file path"))
	static void UploadContentWithTags(FString FilePath, TMap<FString, FString> Tags, FCavrnusUploadCompleteFunction OnUploadComplete, FCavrnusError OnError, FCavrnusUploadProgressFunction OnProgress);

	static void UploadContentWithTags(FString FilePath, TMap<FString, FString> Tags, const CavrnusUploadCompleteFunction& OnUploadComplete, const CavrnusError& OnError, const CavrnusUploadProgressFunction& OnProgress);


	/**
	 * @brief Delegate for handling the request for a folder to be fetched/created
	 *
	 * @param fullFolderPath The full path to the folder
	 */
	DECLARE_DYNAMIC_DELEGATE_OneParam(FCavrnusFolderCallback, const FString&, fullFolderPath);

	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Content",
		meta = (ToolTip = "Gets or Creates a folder at a valid path with read/write permissions", ShortToolTip = "Gets a valid folder path"))
	static void RequestContentDestinationFolder(FString FolderName, FCavrnusFolderCallback OnRecvFullFolderPath);

	/**
	 * @brief Creates a new folder if doesn't exist, that has valid read/write permissions.  Useful for getting a folder to download content to that is valid on multiple platforms.
	 *
	 * @param FolderName The name of the specific folder.  This will be placed in the hidden Cavrnus Content Folder.
	 * @param OnRecvFolder Gives back the full folder path.
	 */
	static void RequestContentDestinationFolder(FString FolderName, const TFunction<void(FString)>& OnRecvFullFolderPath);

#pragma endregion

#pragma region Chat

	/**
	 * @brief Delegate for handling the local Chat entry functions.
	 *
	 * @param chat The current Chat entry to be handled.
	 */
	DECLARE_DYNAMIC_DELEGATE_OneParam(FCavrnusChatFunction, const FChatEntry&, chat);
	/**
	 * @brief Delegate for handling removal of Chat entr(ies).
	 *
	 * @param chat The current Chat entry to be handled.
	 */
	DECLARE_DYNAMIC_DELEGATE_OneParam(FCavrnusChatRemovedFunction, const FString&, chatId);

	/**
	 * @brief Binds a callback to chat message updates in a given Cavrnus Space.
	 * @param spaceConn Identity of the current Cavrnus Space.
	 * @param ChatAdded Function pointer option to specify the function to add a chat message
	 * @param ChatUpdated Function pointer option to specify the function to udpate a chat message
	 * @param ChatRemoved Function pointer option to specify the function to remove a chat message
	 * @return A disposable binding instance.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Chat",
		meta = (ToolTip = "Triggers events when recieving chat messages/updates", ShortToolTip = "Triggers events when recieving chats"))
	static UPARAM(DisplayName = "Disposable") UCavrnusBinding* BindChatMessages(const FCavrnusSpaceConnection& spaceConn, const FCavrnusChatFunction& ChatAdded, const FCavrnusChatFunction& ChatUpdated, const FCavrnusChatRemovedFunction& ChatRemoved);

	static UCavrnusBinding* BindChatMessages(const FCavrnusSpaceConnection& spaceConn, const CavrnusChatFunction& ChatAdded, const CavrnusChatFunction& ChatUpdated, const CavrnusChatRemovedFunction& ChatRemoved);

	/**
	 * @brief Binds a callback to post an updated message to a given Cavrnus Space.
	 * @param spaceConn Identity of the current Cavrnus Space.
	 * @param Chat Chat message text to post
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Chat",
		meta = (ToolTip = "Send a new chat message", ShortToolTip = "Send a new chat message"))
	static void PostChatMessage(const FCavrnusSpaceConnection& spaceConn, const FString& Chat);

#pragma endregion

#pragma region Ai

	DECLARE_DYNAMIC_DELEGATE_OneParam(FCavrnusAiResponseFunction, const FString&, response);

	UFUNCTION(BlueprintCallable, CallInEditor, Exec, Category = "Cavrnus|Ai",
		meta = (ToolTip = "Asks ChatGPT a question about the current space journal", ShortToolTip = "Asks ChatGPT a question"))
	static void QueryAiAboutCurrentSpace(const FCavrnusSpaceConnection& spaceConn, const FString& Question, const FCavrnusAiResponseFunction& onSuccess, const FCavrnusError& onFailure);

	static void QueryAiAboutCurrentSpace(const FCavrnusSpaceConnection& spaceConn, const FString& Question, const TFunction<void(FString)>& onSuccess, const CavrnusError& onFailure);

#pragma endregion

	static FString CreateBindingId(Cavrnus::CavrnusUnbind bindingCallback);

	static void UnbindWithId(FString bindingId);

private:

	/**
	 * Checks for errors in the provided space connection.
	 *
	 * @param SpaceConnection The space connection to check.
	 * @return Returns true if the space connection is valid; otherwise, logs an error and returns false.
	 */
	static bool HooksSetUp;

	static bool CheckErrors(FCavrnusSpaceConnection SpaceConnection);

	static FDelegateHandle UiFlowTeardownHandle;
	static bool HasLiveUiFlowManager;

	static FDelegateHandle CavrnusShutdownHandle;
	static bool ShutdownHooked;
	static void ShutdownCavrnusSystems(UWorld* World, bool bSessionEnded, bool bCleanupResources);

	static void BindSpaceJoin();

	static void SetupJoinedSpace(const FCavrnusSpaceConnection& spaceConn);

	static void HookCavrnusShutdown();

	static void SetupUiFlowManager(const FCavrnusLoginConfig& LoginConfig);
};
