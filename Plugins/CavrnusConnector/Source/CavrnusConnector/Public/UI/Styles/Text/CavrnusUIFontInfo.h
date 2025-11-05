// // Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include "CoreMinimal.h"

enum class ECavrnusTextBlockSize : uint8;
/**
 * 
 */
class CAVRNUSCONNECTOR_API FCavrnusUIFontInfo
{
public:
	static int32 GetFontSizeForTextBlockSize(const ECavrnusTextBlockSize& Size);
};
