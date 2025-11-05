// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "PropertySyncers/CavrnusPropertyPoller.h"
#include "CavrnusFunctionLibrary.h"

void UCavrnusPropertyPoller::Initialize(
	const FCavrnusSpaceConnection& InSpaceConnection,
	const FCavrnusSyncInfo& InSyncInfo,
	const FCavrnusSyncOptions& InSyncOptions)
{
	SpaceConn = InSpaceConnection;
	SyncOptions = InSyncOptions;
	SyncInfo = InSyncInfo;

	MaxPollingTimeBeforeSendRemaining = SyncOptions.MaxPollingTimeBeforeSend;

	bShouldTick = true;
}

void UCavrnusPropertyPoller::Teardown()
{
	if (IsValid(LiveUpdater))
		LiveUpdater->Cancel();

	LiveUpdater = nullptr;
}

UCavrnusPropertyPoller* UCavrnusPropertyPoller::GetLocalValueFunc(const TFunction<Cavrnus::FPropertyValue()>& GetLocalValueFunc)
{
	LocalValueFunc = GetLocalValueFunc;
	return this;
}

UCavrnusPropertyPoller* UCavrnusPropertyPoller::GetAreValuesEqualFunc(
	const TFunction<bool(Cavrnus::FPropertyValue LocalPropValue,Cavrnus::FPropertyValue ServerPropValue)>&InAreValuesEqualFunc)
{
	AreValuesEqualFunc = InAreValuesEqualFunc;
	return this;
}

UCavrnusPropertyPoller* UCavrnusPropertyPoller::SetSendGuidCallback(const TFunction<void(FGuid)>& InSetGuidCallback)
{
	SetInFlightGuid = InSetGuidCallback;
	return this;
}

void UCavrnusPropertyPoller::Tick(float DeltaTime)
{
	if (!UCavrnusFunctionLibrary::IsConnectedToAnySpace())
		return;

	MaxPollingTimeBeforeSendRemaining -= DeltaTime;
	PollingTimeRemaining -= DeltaTime;

	// Basically lets force send if the max time has elapsed even if the poll interval hasn't
	if (MaxPollingTimeBeforeSendRemaining <= 0.0f)
	{
		// Reset the main poller (PollingTimeRemaining) IF we actually have new,different value to post
		if (TryFinalizeValue())
			PollingTimeRemaining = SyncOptions.PollingTimeSeconds;
		
		MaxPollingTimeBeforeSendRemaining = SyncOptions.MaxPollingTimeBeforeSend;
		return;
	}

	// Send when the regular polling time hits zero, as normal...
	if (PollingTimeRemaining <= 0.0f)
	{
		PollingTimeRemaining = SyncOptions.PollingTimeSeconds;
		TrySendingNewValue();
	}
}

bool UCavrnusPropertyPoller::TryFinalizeValue()
{
	if (!IsValid(LiveUpdater))
		return false;

	const Cavrnus::FPropertyValue LocalValue = LocalValueFunc();
	const Cavrnus::FPropertyValue ServerValue = UCavrnusFunctionLibrary::GetGenericPropertyValue(SpaceConn, SyncInfo.ContainerName, SyncInfo.PropertyName);

	// Only finalize if value actually changed...
	if (AreValuesEqualFunc(LocalValue, ServerValue))
		return false;
	
	if (!SyncOptions.bIsTransient)
	{
		LastSentPropValue = LocalValue;
		if (SetInFlightGuid)
			SetInFlightGuid(FCavrnusUpdateTracker::GenerateNewUpdateId());
		LiveUpdater->FinalizeGeneric(LocalValue);
		LiveUpdater = nullptr;

		return true;
	}

	return false;
}

void UCavrnusPropertyPoller::TrySendingNewValue()
{
	const Cavrnus::FPropertyValue LocalValue = LocalValueFunc();
	const Cavrnus::FPropertyValue ServerValue = UCavrnusFunctionLibrary::GetGenericPropertyValue(SpaceConn, SyncInfo.ContainerName, SyncInfo.PropertyName);

	const double GameTimeSeconds = FPlatformTime::Seconds();

	// Prevent spam
	if (IsValid(LiveUpdater))
	{
		if (GameTimeSeconds <= LiveUpdater->GetLastUpdatedTimeSeconds() + SyncOptions.SecondsToWaitBeforePosting)
			return;
	}

	// Has the value changed? 
	if (AreValuesEqualFunc(LocalValue, ServerValue))
	{
		if (IsValid(LiveUpdater) && !SyncOptions.bIsTransient)
		{
			LastSentPropValue = LocalValue;
			if (SetInFlightGuid)
				SetInFlightGuid(FCavrnusUpdateTracker::GenerateNewUpdateId());
			LiveUpdater->FinalizeGeneric(LocalValue);
			LiveUpdater = nullptr;
		}
		
		return;
	}

	// Oh, hey we have a new value!
	LastSentPropValue = LocalValue;
	
	if (SetInFlightGuid)
		SetInFlightGuid(FCavrnusUpdateTracker::GenerateNewUpdateId());
	if (!IsValid(LiveUpdater))
	{
		LiveUpdater = UCavrnusFunctionLibrary::BeginTransientGenericPropertyUpdate(
			SpaceConn,
			SyncInfo.ContainerName,
			SyncInfo.PropertyName,
			LastSentPropValue);
	}
	else
		LiveUpdater->UpdateWithNewDataGeneric(LastSentPropValue);

}

TStatId UCavrnusPropertyPoller::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UCavrnusPropertySyncBase, STATGROUP_Tickables);
}

bool UCavrnusPropertyPoller::IsTickable() const
{
	return bShouldTick;
}