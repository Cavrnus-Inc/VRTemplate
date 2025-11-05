// ReflectionHelpers.h
#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "ReflectionHelpers.generated.h"

UCLASS()
class CAVRNUSCONNECTOR_API UReflectionHelpers : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintPure, Category="Reflection")
    static FString MakeLiteralString(FString Value) { return Value; }

    UFUNCTION(BlueprintPure, Category="Reflection")
    static bool MakeLiteralBool(bool Value) { return Value; }
};
