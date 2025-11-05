// Copyright (c) 2025 Cavrnus. All rights reserved.

#include "RelayModel/CavrnusContentModel.h"
#include <Misc/FileHelper.h>

namespace Cavrnus
{
	CavrnusContentModel::CavrnusContentModel()
	{
	}

	CavrnusContentModel::~CavrnusContentModel()
	{
		// Clean up ProgressBindings's callback arrays
		for (auto kvp : ProgressBindings)
		{
			for (auto callback : kvp.Value)
			{
				delete callback;
			}
			kvp.Value.Empty();
		}

		// Clean up CompletionBindings's callback arrays
		for (auto kvp : CompletionBindings)
		{
			for (auto callback : kvp.Value)
			{
				delete callback;
			}
			kvp.Value.Empty();
		}

		for (auto kvp : FileCompletionBindings)
		{
			for (auto callback : kvp.Value)
			{
				delete callback;
			}
			kvp.Value.Empty();
		}

		for (auto kvp : FailureBindings)
		{
			for (auto callback : kvp.Value)
			{
				delete callback;
			}
			kvp.Value.Empty();
		}
	}

	bool CavrnusContentModel::RegisterContentCallbacks(FString contentId, const CavrnusContentProgressFunction& prog, const CavrnusContentFunction& complete, const CavrnusError& failure)
	{
		if (CompletedContentPaths.Contains(contentId))
		{
			complete(CompletionToStream(CompletedContentPaths[contentId]), CompletedContentFileNames[contentId]);
			return true;
		}
		if (FailedContents.Contains(contentId))
		{
			failure(FailedContents[contentId]);
			return true;
		}

		using progFunction = const CavrnusContentProgressFunction;
		progFunction* ProgCallbackPtr = new progFunction(prog);
		ProgressBindings.FindOrAdd(contentId);
		ProgressBindings[contentId].Add(ProgCallbackPtr);

		using completedFunction = const CavrnusContentFunction;
		completedFunction* CompletedCallbackPtr = new completedFunction(complete);
		CompletionBindings.FindOrAdd(contentId);
		CompletionBindings[contentId].Add(CompletedCallbackPtr);

		using errorFunction = const CavrnusError;
		errorFunction* FailureCallbackPtr = new errorFunction(failure);
		FailureBindings.FindOrAdd(contentId);
		FailureBindings[contentId].Add(FailureCallbackPtr);

		return false;
	}

	bool CavrnusContentModel::RegisterFileContentCallbacks(FString contentId, const CavrnusContentProgressFunction& prog, const CavrnusFileContentFunction& complete, const CavrnusError& failure)
	{
		if (CompletedContentPaths.Contains(contentId))
		{
			complete(CompletedContentPaths[contentId], CompletedContentFileNames[contentId]);
			return true;
		}
		if (FailedContents.Contains(contentId))
		{
			failure(FailedContents[contentId]);
			return true;
		}

		using progFunction = const CavrnusContentProgressFunction;
		progFunction* ProgCallbackPtr = new progFunction(prog);
		ProgressBindings.FindOrAdd(contentId);
		ProgressBindings[contentId].Add(ProgCallbackPtr);

		using completedFunction = const CavrnusFileContentFunction;
		completedFunction* CompletedCallbackPtr = new completedFunction(complete);
		FileCompletionBindings.FindOrAdd(contentId);
		FileCompletionBindings[contentId].Add(CompletedCallbackPtr);

		using errorFunction = const CavrnusError;
		errorFunction* FailureCallbackPtr = new errorFunction(failure);
		FailureBindings.FindOrAdd(contentId);
		FailureBindings[contentId].Add(FailureCallbackPtr);

		return false;
	}

	void CavrnusContentModel::HandleProgressCallback(FString contentId, float progress, FString step)
	{
		if (!ProgressBindings.Contains(contentId))
			return;

		for (int i = 0; i < ProgressBindings[contentId].Num(); i++)
			(*ProgressBindings[contentId][i])(progress, step);
	}

	void CavrnusContentModel::HandleCompletionCallback(FString contentId, FString filePath, FString fileNameWithExt)
	{
		CompletedContentPaths.Add(contentId, filePath);
		CompletedContentFileNames.Add(contentId, fileNameWithExt);

		if (CompletionBindings.Contains(contentId))
		{
			// Call the CompletionBindings callbacks for this contentId now, and clean them up
			auto streambytes = CompletionToStream(filePath);
			for (auto callback : CompletionBindings[contentId])
			{
				(*callback)(streambytes, fileNameWithExt);
				delete callback;
			}
			CompletionBindings.Remove(contentId);
		}

		if (FileCompletionBindings.Contains(contentId))
		{
			// Also call the FileCompletionBindings callbacks for this contentId now, and clean them up
			for (auto callback : FileCompletionBindings[contentId])
			{
				(*callback)(filePath, fileNameWithExt);
				delete callback;
			}
			FileCompletionBindings.Remove(contentId);
		}

		if (ProgressBindings.Contains(contentId))
		{
			// Also discard the ProgressBindings callbacks for this contentId
			for (auto callback : ProgressBindings[contentId])
			{
				delete callback;
			}
			ProgressBindings.Remove(contentId);
		}

		if (FailureBindings.Contains(contentId))
		{
			for (auto callback : FailureBindings[contentId])
			{
				delete callback;
			}
			FailureBindings.Remove(contentId);
		}
	}

	void CavrnusContentModel::HandleFailureCallback(FString contentId, FString error)
	{
		FailedContents.Add(contentId, error);

		if (CompletionBindings.Contains(contentId))
		{
			// Call the CompletionBindings callbacks for this contentId now, and clean them up
			for (auto callback : CompletionBindings[contentId])
			{
				delete callback;
			}
			CompletionBindings.Remove(contentId);
		}

		if (FileCompletionBindings.Contains(contentId))
		{
			// Also call the FileCompletionBindings callbacks for this contentId now, and clean them up
			for (auto callback : FileCompletionBindings[contentId])
			{
				delete callback;
			}
			FileCompletionBindings.Remove(contentId);
		}

		if (ProgressBindings.Contains(contentId))
		{
			// Also discard the ProgressBindings callbacks for this contentId
			for (auto callback : ProgressBindings[contentId])
			{
				delete callback;
			}
			ProgressBindings.Remove(contentId);
		}

		if (FailureBindings.Contains(contentId))
		{
			for (auto callback : FailureBindings[contentId])
			{
				(*callback)(error);
				delete callback;
			}
			FailureBindings.Remove(contentId);
		}
	}


	TArray64<uint8> CavrnusContentModel::CompletionToStream(FString filePath)
	{
		TArray64<uint8> res;
		FFileHelper::LoadFileToArray(res, *filePath, 0);

		return res;
	}
} // namespace Cavrnus
