// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "PropertySyncers/CavrnusPropertySyncer.h"
#include "CavrnusConnectorModule.h"
#include "CavrnusFunctionLibrary.h"
#include "CavrnusSubsystem.h"
#include "Helpers/CavrnusMathHelpers.h"
#include "UObject/UnrealType.h"  
#include "PropertySyncers/CavrnusPropertyPoller.h"
#include "PropertySyncers/CavrnusPropertySyncManager.h"
#include "Runtime/Launch/Resources/Version.h"

void UCavrnusPropertySyncer::Initialize(
	FProperty* InUnrealProperty,
	UObject* InContainerObject,
	const FCavrnusSyncInfo& InSyncInfo,
	const FCavrnusSyncOptions& InSyncOptions)
{
	SyncInfo = InSyncInfo;
	SyncOptions = InSyncOptions;
	bHasTornDown = false;

	if (!TryGetPropertyType(InUnrealProperty, InContainerObject))
		return;
	
	UCavrnusFunctionLibrary::AwaitAnySpaceConnection([this](const FCavrnusSpaceConnection& Sc)
	{
		if (bHasTornDown || !IsValid(this))
			return;
		
		SpaceConnection = Sc;

		if (SyncOptions.bSendValue)
            SetupPolling();

		if (SyncOptions.bSetDefaultValue)
			UCavrnusFunctionLibrary::DefineGenericPropertyDefaultValue(SpaceConnection, SyncInfo.ContainerName, SyncInfo.PropertyName, GetLocalValue(), false);
		
		if (SyncOptions.bReceiveValue)
		{
			// Set up Lambda for Received events from the Bind
			auto ReceivePropertyLambda = [this](const Cavrnus::FPropertyValue& Value, const FString&, const FString&)
			{
				SetLocalValue(Value);

				if (UpdateTracker.IsLocalSend())
				{
					UpdateTracker.MarkReceived();
					OnPropertyReceiveBroadcast(true);
				}
				else
				{
					OnPropertyReceiveBroadcast(false);
				}
			};
			PropertyBinding = UCavrnusFunctionLibrary::BindGenericPropertyValue(
				SpaceConnection,
				SyncInfo.ContainerName,
				SyncInfo.PropertyName,
				ReceivePropertyLambda);
			
			BindingId = PropertyBinding->BindingId;
		}
	});
	
	Id = FGuid::NewGuid();
	UCavrnusSubsystem::Get()->Services->Get<UCavrnusPropertySyncManager>()->Register(Id, this);
}

void UCavrnusPropertySyncer::SetupPolling()
{
	Poller = NewObject<UCavrnusPropertyPoller>(this);
	Poller->Initialize(SpaceConnection, SyncInfo, SyncOptions);

	Poller->SetSendGuidCallback(
		[this](FGuid value)
		{
			UpdateTracker.MarkSent(value);
		}
	);

	Poller->GetLocalValueFunc([this]
		{
			return GetLocalValue();
		});

	switch (ResolvedPropertyType)
	{
	case ECavrnusSyncResolvedPropertyType::Bool:
		Poller->GetAreValuesEqualFunc([](const auto& A, const auto& B)
			{
				return A.BoolValue == B.BoolValue;
			});
		break;

	case ECavrnusSyncResolvedPropertyType::String:
		Poller->GetAreValuesEqualFunc([](const auto& A, const auto& B)
			{
				return A.StringValue == B.StringValue;
			});
		break;

	case ECavrnusSyncResolvedPropertyType::Float:
		Poller->GetAreValuesEqualFunc([](const auto& A, const auto& B)
			{
				return FMath::IsNearlyEqual(A.FloatValue, B.FloatValue, 0.001f);
			});
		break;

	case ECavrnusSyncResolvedPropertyType::Color:
		Poller->GetAreValuesEqualFunc([](const auto& A, const auto& B)
			{
				return A.ColorValue.Equals(B.ColorValue, 0.001f);
			});
		break;

	case ECavrnusSyncResolvedPropertyType::Vector:
		Poller->GetAreValuesEqualFunc([](const auto& A, const auto& B)
			{
				return A.VectorValue.Equals(B.VectorValue, 0.001f);
			});
		break;

	case ECavrnusSyncResolvedPropertyType::Transform:
	{
		FCavrnusSyncTransformOptions FoundOptions;
		TryGetSyncOptionType<FCavrnusSyncTransformOptions>(SyncOptions, FoundOptions);

		Poller->GetAreValuesEqualFunc([FoundOptions](const auto& A, const auto& B)
			{
				return FCavrnusMathHelpers::AreTransformsApproximatelyEqual(
					A.TransformValue,
					B.TransformValue,
					FoundOptions.PositionTolerance,
					FoundOptions.RotationTolerance,
					FoundOptions.ScaleTolerance);
			});
	}
	break;

	case ECavrnusSyncResolvedPropertyType::Struct:
		Poller->GetAreValuesEqualFunc([](const auto& A, const auto& B)
			{
				// Compare serialized text representations
				return A.StringValue == B.StringValue;
			});
		break;

	default:
		UE_LOG(LogCavrnusConnector, Warning,
			TEXT("[UCavrnusPropertySyncer::SetupPolling] Unsupported ResolvedPropertyType: %d"),
			static_cast<uint8>(ResolvedPropertyType));
		break;
	}
}


bool UCavrnusPropertySyncer::SetSyncedStructFromText(const FString& StructText)
{
	if (ResolvedPropertyType != ECavrnusSyncResolvedPropertyType::Struct || !StructProperty || !KnownValuePtr)
	{
		UE_LOG(LogCavrnusConnector, Warning,
			TEXT("[SetSyncedStructFromText] Invalid state — not a struct or missing property/value"));
		return false;
	}

	bool bImported = false;

#if (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION == 0)
	// UE5.0: ImportText returns bool
	bImported = StructProperty->ImportText(*StructText, KnownValuePtr, PPF_None, nullptr);
#elif (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1)
	// UE5.1–5.6+: ImportText_InContainer returns const TCHAR* (nullptr on failure).
	const TCHAR* Result = StructProperty->ImportText_InContainer(*StructText, KnownValuePtr, nullptr, PPF_None);
	bImported = (Result != nullptr);
#endif

	if (!bImported)
	{
		UE_LOG(LogCavrnusConnector, Warning,
			TEXT("[SetSyncedStructFromText] Failed to import struct text for '%s'"),
			*StructProperty->GetName());
		return false;
	}

	return true;
}


Cavrnus::FPropertyValue UCavrnusPropertySyncer::GetLocalValue() const
{
	switch (ResolvedPropertyType)
	{
	case ECavrnusSyncResolvedPropertyType::Bool:
		return Cavrnus::FPropertyValue::BoolPropValue(BoolProperty->GetPropertyValue(KnownValuePtr));

	case ECavrnusSyncResolvedPropertyType::String:
		return Cavrnus::FPropertyValue::StringPropValue(StringProperty->GetPropertyValue(KnownValuePtr));

	case ECavrnusSyncResolvedPropertyType::Float:
		return Cavrnus::FPropertyValue::FloatPropValue(NumericProperty->GetFloatingPointPropertyValue(KnownValuePtr));

	case ECavrnusSyncResolvedPropertyType::Int:
		return Cavrnus::FPropertyValue::FloatPropValue(static_cast<float>(NumericProperty->GetSignedIntPropertyValue(KnownValuePtr)));

	case ECavrnusSyncResolvedPropertyType::Transform:
		return Cavrnus::FPropertyValue::TransformPropValue(*static_cast<FTransform*>(KnownValuePtr));

	case ECavrnusSyncResolvedPropertyType::Vector:
		return Cavrnus::FPropertyValue::VectorPropValue(*static_cast<FVector*>(KnownValuePtr));

	case ECavrnusSyncResolvedPropertyType::Color:
		return Cavrnus::FPropertyValue::ColorPropValue(*static_cast<FLinearColor*>(KnownValuePtr));

	case ECavrnusSyncResolvedPropertyType::Struct:
	{
		FString TextValue;
		if (StructProperty && KnownValuePtr)
		{
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION == 0
			// UE5.0: ExportTextItem is still available
			StructProperty->ExportTextItem(
				TextValue,
				KnownValuePtr,
				nullptr,
				nullptr,
				PPF_None,
				nullptr);
#else
			// UE5.1+ (including UE5.6): use ExportText_InContainer
			StructProperty->ExportText_InContainer(
				/*Index*/ 0,
				/*OutText*/ TextValue,
				/*Container*/ KnownValuePtr,
				/*Default*/ nullptr,
				/*Owner*/ nullptr,
				/*PortFlags*/ PPF_None,
				/*ExportRootScope*/ nullptr);
#endif

			return Cavrnus::FPropertyValue::StringPropValue(TextValue);
		}
		break;
	}

	default:
		break;
	}

	return Cavrnus::FPropertyValue();
}


void UCavrnusPropertySyncer::SetLocalValue(const Cavrnus::FPropertyValue& PropertyValue) const
{
	if (!KnownValuePtr)
	{
		UE_LOG(LogCavrnusConnector, Warning, TEXT("[SetLocalValue] KnownValuePtr is null"));
		return;
	}

	switch (ResolvedPropertyType)
	{
	case ECavrnusSyncResolvedPropertyType::Bool:
		if (BoolProperty)
			BoolProperty->SetPropertyValue(KnownValuePtr, PropertyValue.BoolValue);
		break;

	case ECavrnusSyncResolvedPropertyType::String:
		if (StringProperty)
			StringProperty->SetPropertyValue(KnownValuePtr, PropertyValue.StringValue);
		break;

	case ECavrnusSyncResolvedPropertyType::Float:
		if (NumericProperty)
			NumericProperty->SetFloatingPointPropertyValue(KnownValuePtr, PropertyValue.FloatValue);
		break;

	case ECavrnusSyncResolvedPropertyType::Int:
		if (NumericProperty)
			NumericProperty->SetIntPropertyValue(KnownValuePtr, static_cast<int64>(PropertyValue.FloatValue));
		break;

	case ECavrnusSyncResolvedPropertyType::Transform:
		*static_cast<FTransform*>(KnownValuePtr) = PropertyValue.TransformValue;
		break;

	case ECavrnusSyncResolvedPropertyType::Vector:
		*static_cast<FVector*>(KnownValuePtr) = PropertyValue.VectorValue;
		break;

	case ECavrnusSyncResolvedPropertyType::Color:
		*static_cast<FLinearColor*>(KnownValuePtr) = PropertyValue.ColorValue;
		break;

	case ECavrnusSyncResolvedPropertyType::Struct:
		if (StructProperty && !PropertyValue.StringValue.IsEmpty())
		{
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION == 0
			// UE5.0: ImportText is still available
			if (!StructProperty->ImportText(*PropertyValue.StringValue, KnownValuePtr, PPF_None, nullptr))
			{
				UE_LOG(LogCavrnusConnector, Warning,
					TEXT("[SetLocalValue] Failed to import struct text for '%s'"),
					*StructProperty->Struct->GetName());
			}
#elif ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1
			// UE5.1–5.6+: Use ImportText_InContainer (omit ErrorText to avoid GWarn type mismatch)
			if (!StructProperty->ImportText_InContainer(*PropertyValue.StringValue, KnownValuePtr, nullptr, PPF_None))
			{
				UE_LOG(LogCavrnusConnector, Warning,
					TEXT("[SetLocalValue] Failed to import struct text for '%s'"),
					*StructProperty->Struct->GetName());
			}
#endif
		}
		break;

	default:
		UE_LOG(LogCavrnusConnector, Warning,
			TEXT("[SetLocalValue] Unsupported ResolvedPropertyType: %d"),
			static_cast<uint8>(ResolvedPropertyType));
		break;
	}
}


void UCavrnusPropertySyncer::OnPropertyReceiveBroadcast(bool bLocal)
{
	FString LocalStr = "False";
	if (bLocal)
		LocalStr = "True";
	CavrnusPropertyReceived.Broadcast(bLocal);
}

void UCavrnusPropertySyncer::SendCurrentValue(bool Transient)
{
	UE_LOG(LogTemp, Error, TEXT("Send Current Value"));
	Cavrnus::FPropertyValue PropVal = UCavrnusPropertySyncer::GetLocalValue();
	if (Transient)
	{
		UE_LOG(LogCavrnusConnector, Warning, TEXT("UCavrnusPropertySyncer::SendCurrentValue : No Transient Support Yet"));
		UpdateTracker.MarkSent(FCavrnusUpdateTracker::GenerateNewUpdateId());
		// Make sure the transient starts after the guid is set.
	}
	else
	{
		UpdateTracker.MarkSent(FCavrnusUpdateTracker::GenerateNewUpdateId());
		UCavrnusFunctionLibrary::PostGenericPropertyUpdate(SpaceConnection, SyncInfo.ContainerName, SyncInfo.PropertyName, PropVal);
		//CavrnusPropertyReceived.Broadcast(true);
	}
}

bool UCavrnusPropertySyncer::TryGetPropertyType(FProperty* Prop, void* InContainerObject)
{
	if (!Prop || !InContainerObject)
	{
		UE_LOG(LogCavrnusConnector, Warning, TEXT("[TryGetPropertyType] Null property or container object"));
		return false;
	}

	// Bool
	if (FBoolProperty* FoundProp = CastField<FBoolProperty>(Prop))
	{
		ResolvedPropertyType = ECavrnusSyncResolvedPropertyType::Bool;
		BoolProperty = FoundProp;
		KnownValuePtr = FoundProp->ContainerPtrToValuePtr<void>(InContainerObject);
		return true;
	}

	// String
	if (FStrProperty* FoundProp = CastField<FStrProperty>(Prop))
	{
		ResolvedPropertyType = ECavrnusSyncResolvedPropertyType::String;
		StringProperty = FoundProp;
		KnownValuePtr = FoundProp->ContainerPtrToValuePtr<void>(InContainerObject);
		return true;
	}

	// Numeric
	if (FNumericProperty* NumProp = CastField<FNumericProperty>(Prop))
	{
		NumericProperty = NumProp;
		KnownValuePtr = NumProp->ContainerPtrToValuePtr<void>(InContainerObject);

		if (NumProp->IsFloatingPoint())
		{
			ResolvedPropertyType = ECavrnusSyncResolvedPropertyType::Float;
			return true;
		}
		else if (NumProp->IsInteger())
		{
			ResolvedPropertyType = ECavrnusSyncResolvedPropertyType::Int;
			return true;
		}
		else
		{
			UE_LOG(LogCavrnusConnector, Warning, TEXT("[TryGetPropertyType] Numeric property is neither float nor int"));
			return false;
		}
	}

	// Structs
	if (FStructProperty* FoundStruct = CastField<FStructProperty>(Prop))
	{
		StructProperty = FoundStruct;

		if (FoundStruct->Struct == TBaseStructure<FTransform>::Get())
		{
			ResolvedPropertyType = ECavrnusSyncResolvedPropertyType::Transform;
			KnownValuePtr = FoundStruct->ContainerPtrToValuePtr<FTransform>(InContainerObject);
			return true;
		}
		if (FoundStruct->Struct == TBaseStructure<FLinearColor>::Get())
		{
			ResolvedPropertyType = ECavrnusSyncResolvedPropertyType::Color;
			KnownValuePtr = FoundStruct->ContainerPtrToValuePtr<FLinearColor>(InContainerObject);
			return true;
		}
		if (FoundStruct->Struct == TBaseStructure<FVector>::Get())
		{
			ResolvedPropertyType = ECavrnusSyncResolvedPropertyType::Vector;
			KnownValuePtr = FoundStruct->ContainerPtrToValuePtr<FVector>(InContainerObject);
			return true;
		}

		// ✅ Fallback for arbitrary USTRUCTs
		ResolvedPropertyType = ECavrnusSyncResolvedPropertyType::Struct;
		KnownValuePtr = FoundStruct->ContainerPtrToValuePtr<void>(InContainerObject);
		return true;
	}

	UE_LOG(LogCavrnusConnector, Warning,
		TEXT("[UCavrnusPropertySyncer::TryGetPropertyType] Unable to resolve property type for '%s'"),
		*Prop->GetName());
	return false;
}


void UCavrnusPropertySyncer::Teardown()
{
	if (!BindingId.IsEmpty())
		UCavrnusFunctionLibrary::UnbindWithId(BindingId);

	BindingId = FString();
	PropertyBinding = nullptr;

	bHasTornDown = true;
	UCavrnusSubsystem::Get()->Services->Get<UCavrnusPropertySyncManager>()->Unregister(Id);
}

