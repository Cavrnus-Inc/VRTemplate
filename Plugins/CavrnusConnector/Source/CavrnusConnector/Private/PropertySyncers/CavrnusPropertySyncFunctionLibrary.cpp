// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "PropertySyncers/CavrnusPropertySyncFunctionLibrary.h"
#include "PropertySyncers/CavrnusPropertySyncer.h"
#include "Engine/World.h"
DEFINE_FUNCTION(UCavrnusPropertySyncFunctionLibrary::execCavrnusInternalSyncProperty)
{
    // 1) Fixed params
    P_GET_OBJECT(AActor, Actor);
    P_GET_STRUCT_REF(FCavrnusSyncInfo, SyncInfo);

    // 2) Step the Property param if present
    FProperty* PropertyProp = nullptr;
    void* PropertyPtr = nullptr;
    if (Stack.PeekCode() != EX_EndFunctionParms)
    {
        Stack.StepCompiledIn<FProperty>(nullptr);
        PropertyProp = Stack.MostRecentProperty;
        PropertyPtr = Stack.MostRecentPropertyAddress;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[Thunk] No Property param on stack"));
    }

    // 3) Step the SyncOptions param if present
    FProperty* OptionsProp = nullptr;
    void* OptionsPtr = nullptr;
    if (Stack.PeekCode() != EX_EndFunctionParms)
    {
        Stack.StepCompiledIn<FStructProperty>(nullptr);
        OptionsProp = Stack.MostRecentProperty;
        OptionsPtr = Stack.MostRecentPropertyAddress;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[Thunk] No SyncOptions param on stack"));
    }

    // 4) Finish parameter parsing
    P_FINISH;

    // … rest of your logic unchanged …
    if (PropertyProp != nullptr)
    {
        FString TypeName = PropertyProp->GetCPPType();
        UE_LOG(LogTemp, Warning, TEXT("Resolved ValueProp type: %s"), *TypeName);
        if (FStructProperty* StructProp = CastField<FStructProperty>(PropertyProp))
        {
            const UScriptStruct* ValueType = StructProp->Struct;
            UE_LOG(LogTemp, Warning, TEXT("Resolved struct type: %s"), *ValueType->GetName());
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("PropertyProp is null — no property resolved"));
    }

    FCavrnusSyncOptions SyncOptions{};
    if (OptionsProp && OptionsPtr)
    {
        if (FStructProperty* OptionsStructProp = CastField<FStructProperty>(OptionsProp))
        {
            const UScriptStruct* IncomingStruct = OptionsStructProp->Struct;
            const UScriptStruct* BaseStruct = FCavrnusSyncOptions::StaticStruct();

            if (IncomingStruct == BaseStruct)
            {
                BaseStruct->CopyScriptStruct(&SyncOptions, OptionsPtr);
            }
            else
            {
                for (TFieldIterator<FProperty> It(BaseStruct); It; ++It)
                {
                    FProperty* BaseProp = *It;
                    FProperty* DerivedProp = IncomingStruct->FindPropertyByName(BaseProp->GetFName());
                    if (!DerivedProp) continue;
                    if (DerivedProp->SameType(BaseProp))
                    {
                        void* DestPtr = BaseProp->ContainerPtrToValuePtr<void>(&SyncOptions);
                        void* SrcPtr = DerivedProp->ContainerPtrToValuePtr<void>(OptionsPtr);
                        BaseProp->CopyCompleteValue(DestPtr, SrcPtr);
                    }
                }
                UE_LOG(LogTemp, Warning,
                    TEXT("Promoted SyncOptions from incoming struct '%s' to base '%s'"),
                    *IncomingStruct->GetName(), *BaseStruct->GetName());
            }
        }
    }

    if (UCavrnusPropertySyncer* Syncer = NewObject<UCavrnusPropertySyncer>(Actor))
    {
        Syncer->Initialize(PropertyProp, Actor, SyncInfo, SyncOptions);
        *(UCavrnusPropertySyncer**)RESULT_PARAM = Syncer;
    }

    UE_LOG(LogTemp, Warning, TEXT("Container: %s"), *SyncInfo.ContainerName);
    UE_LOG(LogTemp, Warning, TEXT("Property: %s"), *SyncInfo.PropertyName);

}
