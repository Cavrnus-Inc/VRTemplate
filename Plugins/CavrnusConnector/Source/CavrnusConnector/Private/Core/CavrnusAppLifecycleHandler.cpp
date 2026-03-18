// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "Core/CavrnusAppLifecycleHandler.h"
#if WITH_EDITOR
#include "Editor.h"
#endif

void UCavrnusAppLifecycleHandler::AwaitAppStart(const TFunction<void(const bool IsEditor, UWorld* World)>& InStartCallback)
{
	StartCallback = InStartCallback;

	// Use OnPreWorldInitialization for both editor and runtime so the
	// RuntimeContext is always created before BeginPlay.  This lets
	// Blueprint login calls in level BeginPlay work in PIE.
	WorldHandle = FWorldDelegates::OnPreWorldInitialization.AddUObject(
		this,
		&UCavrnusAppLifecycleHandler::HandleWorldInit
	);

#if WITH_EDITOR
	FEditorDelegates::EndPIE.AddUObject(this, &UCavrnusAppLifecycleHandler::HandlePIEEnd);
#endif
}

void UCavrnusAppLifecycleHandler::AwaitAppEnd(const TFunction<void(const bool IsEditor)>& InEndCallback)
{
	EndCallback = InEndCallback;
}

void UCavrnusAppLifecycleHandler::Dispose()
{
	Super::Dispose();

	FWorldDelegates::OnPreWorldInitialization.Remove(WorldHandle);
	WorldHandle.Reset();
}

void UCavrnusAppLifecycleHandler::HandleWorldInit(UWorld* World, const UWorld::InitializationValues InitializationValues)
{
	if (!World || !World->IsGameWorld())
	{
		return;
	}

	UGameInstance* GameInstance = World->GetGameInstance();
	if (!GameInstance)
	{
		UE_LOG(LogTemp, Log, TEXT("CavrnusAppLifecycleHandler::HandleWorldInit - No GameInstance, skipping"));
		return;
	}

	const bool bIsEditor = (World->WorldType == EWorldType::PIE);

	UE_LOG(LogTemp, Log, TEXT("CavrnusAppLifecycleHandler::HandleWorldInit - IsEditor: %s, World: %s"),
		bIsEditor ? TEXT("true") : TEXT("false"),
		*World->GetName());

	if (StartCallback)
		StartCallback(bIsEditor, World);
}

#if WITH_EDITOR
void UCavrnusAppLifecycleHandler::HandlePIEEnd(bool bIsSim)
{
	if (EndCallback)
		EndCallback(true);
}
#endif
