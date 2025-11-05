#pragma once
#include "Blueprint/UserWidget.h"

class CAVRNUSCONNECTOR_API FCavrnusWidgetFactory
{
public:
	static UClass* GetDefaultBlueprint(const FString& Path, UClass* BaseClass);

	template <typename T>
	static T* LoadAssetFromPath(const FString& InPath)
	{
		if (InPath.IsEmpty())
			return nullptr;

		const TSoftObjectPtr<T> AssetRef{FSoftObjectPath(InPath)};
		return AssetRef.LoadSynchronous();
	}
	
	template <typename T>
	static T* CreateUserWidget(const TSubclassOf<T>& InUserWidgetBlueprint, UWorld* InWorld)
	{
		static_assert(TIsDerivedFrom<T, UUserWidget>::IsDerived, "T must be a subclass of UUserWidget");

		if (!InUserWidgetBlueprint)
		{
			UE_LOG(LogTemp, Warning, TEXT("CreateUserWidget: InUserWidgetBlueprint is null!"));
			return nullptr;
		}

		if (!InWorld)
		{
			UE_LOG(LogTemp, Warning, TEXT("CreateUserWidget: TargetOwner is null!"));
			return nullptr;
		}

		if (T* UserWidgetInstance = CreateWidget<T>(InWorld, InUserWidgetBlueprint))
			return UserWidgetInstance;
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("CreateUserWidget: Failed to create UserWidget!"));
			return nullptr;
		}
	}

	template <typename T>
	static T* CreateUserWidgetFromPath(const FString& InPath, UWorld* InWorld)
	{
		if (InPath.IsEmpty())
		{
			UE_LOG(LogTemp, Warning, TEXT("CreateUserWidgetFromPath: InPath is empty!"));
			return nullptr;
		}

		if (!InWorld)
		{
			UE_LOG(LogTemp, Warning, TEXT("CreateUserWidgetFromPath: InWorld is null!"));
			return nullptr;
		}

		const TSoftClassPtr<UUserWidget> TargetWidget(InPath);
		if (UClass* LoadedClass = TargetWidget.LoadSynchronous())
		{
			if (UUserWidget* CreatedWidget = CreateWidget<UUserWidget>(InWorld, LoadedClass))
			{
				return Cast<T>(CreatedWidget);
			}
			else
				UE_LOG(LogTemp, Warning, TEXT("CreateWidget failed"));
		}
		else
			UE_LOG(LogTemp, Warning, TEXT("Failed to load class from path: %s"), *InPath);

		return nullptr;
	}
};