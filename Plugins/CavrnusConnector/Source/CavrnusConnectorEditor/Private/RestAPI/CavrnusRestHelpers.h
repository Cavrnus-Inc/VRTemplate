#pragma once

#include "CoreMinimal.h"

class FCavrnusRestHelpers
{
public:
	static void PostHasSeenSetupScreenMetric();

private:
	static FString GetCavrnusDeviceGuid();
};