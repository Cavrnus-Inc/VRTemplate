// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Systems/Loaders/Types/CavrnusIndeterminateLoader.h"
#include "UI/Components/Text/CavrnusUITextBlock.h"

UCavrnusIndeterminateLoader* UCavrnusIndeterminateLoader::SetMessage(const FString& Msg)
{
	if (MessageTextBlock)
		MessageTextBlock->SetText(FText::FromString(Msg));

	return this;
}
