// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "Managers/CavrnusService.h"
#include "Engine/Engine.h"
#include "Misc/CoreDelegates.h"
#if WITH_EDITOR
#include "Editor.h"
#endif

void UCavrnusService::Initialize()
{
	UE_LOG(LogTemp, Warning, TEXT("UCavrnusLifecycleManagerBase::InitializeManager() from %s"), *this->GetClass()->GetName());
	// Engine init
	EngineInitHandle = FCoreDelegates::OnFEngineLoopInitComplete.AddUObject(this, &UCavrnusService::OnAppInit);

	// Engine shutdown
	EnginePreExitHandle = FCoreDelegates::OnEnginePreExit.AddUObject(this, &UCavrnusService::OnAppShutdown);

#if WITH_EDITOR
	BeginPIEHandle = FEditorDelegates::BeginPIE.AddUObject(this, &UCavrnusService::OnBeginPIE);
	EndPIEHandle   = FEditorDelegates::EndPIE.AddUObject(this, &UCavrnusService::OnEndPIE);
#endif
}

void UCavrnusService::Deinitialize()
{
	UE_LOG(LogTemp, Warning, TEXT("UCavrnusLifecycleManagerBase::DeinitializeManager() from %s"),*this->GetClass()->GetName());
	if (EngineInitHandle.IsValid())
		FCoreDelegates::OnFEngineLoopInitComplete.Remove(EngineInitHandle);

	if (EnginePreExitHandle.IsValid())
		FCoreDelegates::OnEnginePreExit.Remove(EnginePreExitHandle);

#if WITH_EDITOR
	if (BeginPIEHandle.IsValid())
		FEditorDelegates::BeginPIE.Remove(BeginPIEHandle);
	if (EndPIEHandle.IsValid())
		FEditorDelegates::EndPIE.Remove(EndPIEHandle);
#endif

	EngineInitHandle.Reset();
	EnginePreExitHandle.Reset();
	
#if WITH_EDITOR
	BeginPIEHandle.Reset();
	EndPIEHandle.Reset();
#endif
}

void UCavrnusService::Teardown()
{
	UE_LOG(LogTemp, Warning, TEXT("UCavrnusLifecycleManagerBase::Teardown()"));
}
