// Copyright (c) 2025 Cavrnus. All rights reserved.

#pragma once

#include <Containers/Map.h>
#include "Types/CavrnusBinding.h"
#include "Types/CavrnusCallbackTypes.h"
#include "SpacePropertyModel.h"
#include <fstream>

namespace Cavrnus
{
	class CavrnusContentModel
	{
	public:
		CavrnusContentModel();
		virtual ~CavrnusContentModel();

		bool RegisterContentCallbacks(FString contentId, const CavrnusContentProgressFunction& prog, const CavrnusContentFunction& complete, const CavrnusError& onFail);
		bool RegisterFileContentCallbacks(FString contentId, const CavrnusContentProgressFunction& prog, const CavrnusFileContentFunction& complete, const CavrnusError& onFail);

		void HandleProgressCallback(FString contentId, float progress, FString step);
		void HandleCompletionCallback(FString contentId, FString filePath, FString fileNameWithExt);
		void HandleFailureCallback(FString contentId, FString error);

	private:

		CaseSensitiveMap<FString, FString> CompletedContentPaths;
		CaseSensitiveMap<FString, FString> CompletedContentFileNames;
		CaseSensitiveMap<FString, FString> FailedContents;

		CaseSensitiveMap<FString, TArray<const CavrnusContentProgressFunction*>> ProgressBindings;
		CaseSensitiveMap<FString, TArray<const CavrnusContentFunction*>> CompletionBindings;
		CaseSensitiveMap<FString, TArray<const CavrnusError*>> FailureBindings;
		CaseSensitiveMap<FString, TArray<const CavrnusFileContentFunction*>> FileCompletionBindings;

		TArray64<uint8> CompletionToStream(FString filePath);
	};
} // namespace Cavrnus
