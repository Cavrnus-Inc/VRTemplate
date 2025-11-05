#include "UI/Helpers/CavrnusWidgetFactory.h"
#include "CavrnusConnectorModule.h"

UClass* FCavrnusWidgetFactory::GetDefaultBlueprint(const FString& Path, UClass* BaseClass)
{
	if (Path.IsEmpty() || !BaseClass)
		return nullptr;

	// Safer load
	UClass* LoadedBlueprintClass = Cast<UClass>(StaticLoadObject(UClass::StaticClass(), nullptr, *Path));
	if (!LoadedBlueprintClass)
	{
		UE_LOG(LogCavrnusConnector, Warning,
			TEXT("Blueprint failed to load: %s (base class: %s)"), 
			*Path, *BaseClass->GetName());
	}
	return LoadedBlueprintClass;
}
