// // Copyright (c) 2025 Cavrnus. All rights reserved.

#include "UI/Helpers/CavrnusUIHelpers.h"

FString FCavrnusUIHelpers::GetAbbreviatedDate(const FDateTime& Date)
{
	const FString Months[] = {
		TEXT("JAN"), TEXT("FEB"), TEXT("MAR"), TEXT("APR"),
		TEXT("MAY"), TEXT("JUN"), TEXT("JUL"), TEXT("AUG"),
		TEXT("SEP"), TEXT("OCT"), TEXT("NOV"), TEXT("DEC")
	};
	
	FDateTime UtcNow = FDateTime::UtcNow();
	FDateTime LocalNow = FDateTime::Now();
	FTimespan TimeZoneOffset = LocalNow - UtcNow; // LocalTime - UTCTime = TimeZone Offset
	FDateTime LocalDate = Date + TimeZoneOffset;
	
	const int32 MonthIndex = LocalDate.GetMonth() - 1;
	const FString AbbrevMonth = Months[MonthIndex];
	
	const int32 Day = LocalDate.GetDay();
	const FTimespan TimeOfDay = LocalDate.GetTimeOfDay();
    
	int32 Hour = TimeOfDay.GetHours();
	const int32 Minute = TimeOfDay.GetMinutes();
	FString Ampm = TEXT("AM");

	// Convert to 12-hour format and determine AM/PM
	if (Hour >= 12)
	{
		Ampm = TEXT("PM");
		if (Hour > 12)
		{
			Hour -= 12;  // Convert to 12-hour format if PM
		}
	}
	else if (Hour == 0)
		Hour = 12;  // Midnight case (12:00 AM)

	// Format the date as "FEB 05 3:09 PM"
	FString CustomFormattedDate = FString::Printf(TEXT("%s %02d %d:%02d %s"), *AbbrevMonth, Day, Hour, Minute, *Ampm);

	return CustomFormattedDate;
}
