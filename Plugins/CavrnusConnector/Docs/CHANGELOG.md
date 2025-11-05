# Changelog

## [2025.3.2] - 2025-9-29

### Added
- Added new SpaceList with support for live updates
- Various relay improvements including tag fixes.

## [2025.1.8] - 2025-8-1

### Added
- Add improved UI input fields for logging in with validation.
- Add server message api binds and corresponding UserWidget

### Fixed
- Fix video frame copy overflow by limiting to reported resolution size
- Fix welcome modal not showing when starting app.

## [2025.1.6] - 2025-5-30

### Added
- Large file support in cloud storage
- RenameSpace, DeleteSpace, UpdateThumbnailForSpace methods

### Changed
- Improved pawn color support
- CavrnusRelayNet.exe is now CollaborationCommunicationSystem.exe

### Fixed
- Fixed CavrnusAvatarManager to correctly track local pawn destruction and recreation.
- Local pawn mesh no longer visible to local camera
- Local pawn component clean up on space exit
- Performance improvements related to avoiding C++ container copying
- Stability improvements related to pointer value checking, smart pointer usage

## [2025.1.5] - 2025-4-14

### Added
- Cavrnus GC Manager to own object lifespans
- Cavrnus UI Flow Manager to handle login and space join flows
- GetRemoteContent & AwaitContentPredicate: improved asset reference streaming support
- Upload progress and error: support and callbacks

### Changed
- Improved FAbsolutePropertyId performance w/ combined name hash
- Improved Space exit cleanup / class shutdowns
- Moved CavrnusSpatialConnector actor fields to plugin settings
- Remote Avatar class and SpawnObject class registeration moved (again) to OverrideRemoteAvatarClass, RegisterSpawnableObjectType, and UnregisterSpawnableObjectType functions, rather than the CavrnusSpatialConnector actor or plugin settings

### Removed
- Dependencies requiring a Cavrnus Space Connector actor to be found in a level
- Duplicated AvatarVis transient

### Fixed
- WBP_CustomSlider errors with null Updater
- Create and Join space UI flow

## [2025.1.4] - 2025-3-22

### Added
- Invite/Uninvite functionality
- Non-user account filtering
- First and Last name fields added to FCavrnusUserAccount.
- FCavrnusSpaceConnectionInfo added to hold fields removed from FCavrnusSpaceConnection. Use GetCavrnusSpaceConnectionInfo to access.

### Changed
- Allowed BindingID to be accessible outside the plugin
- CavrnusRelayNet execution transitioned from std::thread to ProcessHandle
- FCavrnusSpaceConnection now contains only the space ID in order to improve copy struct performance when querying property values.
- Welcome screen update.

### Fixed
- Performance improvements in SpacePropertyModel GetPropValue / GetCurrentPropValue
- Server property value will be selected over property's default value

## [2025.1.0] - 2025-1-14

### Added
- Added option to Unbind CavrnusBindings using a string Id, allowing users to avoid Unreal garbage collection problems related to UObjects.

### Changed
- CavrnusRelayNet subfolder changed from "net6.0-windows" to "Win64"
- Library callbacks related to Space Connection and Authorization results now public

### Removed
- Unused proto files
- CavrnusSpatialConnectorSubSystem class
- Sample level removed from plugin, will be moved to sample project at future date

### Fixed
- CavrnusConnector no longer adds the entire module to PublicIncludePaths
- CavrnusSpatialConnector object creation callbacks
- Fab compatibility - Plugin code no longer assumes root folder name "CavrnusConnector"
- CavrnusAvatarManager remote pawn visibility management

## [2024.3.1] - 2025-1-2

### Added
- SpaceInfo data added to CavrnusSpaceConnection
- Added JoinIdLogin widget to join space directly.
- Added ability to force/unforce RelayNet to Keep Alive while Unreal is frozen
- Changelog.md now copied to Docs subdirectory
- SpaceInformation UI button and window
- Server Selection Widget for case where Developer does not input server.
- Space Tags/Keywords
- Journal/local storage folder infrastructure 
- Token Caching to save Cavrnus login credentials
- CommandLine arguments for various Cavrnus server login fields: "Server", "GuestName", "UserEmail", "UserPassword", "GuestCavrnusAuthToken", "MemberCavrnusAuthToken", "SpaceJoinId"

### Changed
- The "None" options in the CavrnusSpatialConnector menu now say "Custom", to reduce confusion.
- Plugin friendly name now shows as "Cavrnus Spatial Connector"
- Silent mode flag fixed and set to be True by default. Hides the Cavrnus Relay console window. Unhidden in project settings under Plugins / Cavrnus Spatial Connector.
- Cleanup of pawn sync components. Renamed several blueprint classes/components
- Google Protobuf files and CavrnusRelayNet moved to approved ThirdParty subfolder location
- UCavrnusSeverSelectWidget is now UCavrnusServerSelectionWidget

### Removed
- Depedency on ProceduralMeshComponent plugin
- Unused pawn classes and blueprint assets
- Copy of SpaceId stored in CavrnusSpatialConnector

### Fixed
- Errors in SK_Avatar_CollabViewer_PhysicsAsset.uasset
- Miscellaneous Server Select menu fixes
- CSC no longer destroys the local user's pawn on possession change
- Input action binding warnings
- Misc. avatar material issues

## [2024.3.0] - 2024-10-11

### Added
- Create Space methods added to CavrnusFunctionLibrary and UI.
- New CavrnusSyncHeadRotation component for pawns.
- New Animation synching components - CavrnusPawnAnimationComponent and AC_CavrnusAnimationAccessor.uasset

### Changed
- Set EnabledByDefault to false in CavrnusConnector.uplugin. User needs to enable the plugin in the project, even when plugin is located in `<Project>/Plugins`.
- Animation accessor added to BP_BasePawn_Remote.uasset.
- Local property handling logic in CavrnusRelayNet.
- Increased transient property timeout period in CavrnusRelayNet.
- Performance improvements to interop batching.
- CavrnusRelayNet performance improvements - reduced logging, fixed scheduler.
- CavrnusConnectorSettings improvements.
  - Settings that apply to the project (e.g. RelayNetDisableRTC) should now go in DefaultCavrnus.ini. Copy the instance found in the plugin to `<Project>/Config`, and modify settings there to apply values in packaged builds.
  - Settings that are meant for local development (e.g. RelayNetVerboseLogging) can be set in the Unreal Editor menu Project | Plugins. These get stored in the `<Project>/Saved/Config/WindowsEditor` subdirectory.
- Moved CavrnusConnector loading to PreDefault loading phase. Fixes issues where adding CavrnusConnector uasset references to a C++ project's assets could result in failures on subsequent editor starts.

### Fixed
- Widget cleanup issues; moved one-time initialization from event Construct to OnInitialized. Added cleanup to NativeDestruct.
- Properties handling improvements to eliminate unnecessary callback execution.
- Bugs where property smoothing resulted in missing property updates.
- Data invalidation fix due to incorrect property change ID caching.
- Asset dependency errors in SK_Hand.uasset and SK_Hand_Skeleton.uasset.
- AC_Cavrnus_SyncAvatarTransform - IsValid checking to fix error on shutdown.
- Bug where multiple copies of UserWidget could appear in UserPanel.
- Sample Scene fixes and improvements.
- Bug where initial journal value callbacks would not fire.
