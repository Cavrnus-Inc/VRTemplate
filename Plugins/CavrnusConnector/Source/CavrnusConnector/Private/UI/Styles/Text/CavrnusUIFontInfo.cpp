// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Styles/Text/CavrnusUIFontInfo.h"

#include "UI/Components/Text/CavrnusUITextBlock.h"

int32 FCavrnusUIFontInfo::GetFontSizeForTextBlockSize(const ECavrnusTextBlockSize& Size)
{
	switch (Size)
	{
		case ECavrnusTextBlockSize::H1:      return 16;  // Top-level group or panel title
		case ECavrnusTextBlockSize::H2:      return 14;  // Sub-section headers
		case ECavrnusTextBlockSize::Button:  return 12;  // Button text (default)
		case ECavrnusTextBlockSize::Body:    return 12;  // Standard body/information
		case ECavrnusTextBlockSize::Small:   return 10;  // Hints, metadata, annotations
		case ECavrnusTextBlockSize::Custom:
		default:                             return 0;
	}
}
