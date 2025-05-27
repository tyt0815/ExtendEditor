#include "AssetActions/QuickAssetAction.h"
#include "EditorUtilityLibrary.h"
#include "EditorAssetLibrary.h"
#include "DebugHeader.h"

void UQuickAssetAction::DuplicateAssets(int32 DuplicatesNum)
{
	if (DuplicatesNum <= 0)
	{
		Print(TEXT("Please enter a VALID number"), FColor::Red);
		return;
	}

	TArray<FAssetData> SelectedAssetsData = UEditorUtilityLibrary::GetSelectedAssetData();
	uint32 Counter = 0;
	for (const FAssetData& SelectedAssetData : SelectedAssetsData)
	{
		for (int32 i = 0; i < DuplicatesNum; ++i)
		{
			const FString SourceAssetPath = SelectedAssetData.ObjectPath.ToString();
			const FString NewDuplicatedAssetName = SelectedAssetData.AssetName.ToString() + TEXT("_") + FString::FromInt(i + 1);
			const FString NewPathName = FPaths::Combine(SelectedAssetData.PackagePath.ToString(), NewDuplicatedAssetName);

			if (UEditorAssetLibrary::DuplicateAsset(SourceAssetPath, NewPathName))
			{
				UEditorAssetLibrary::SaveAsset(NewPathName, false);
				++Counter;
			}
		}
	}

	if (Counter > 0)
	{
		Print(TEXT("Successfully duplikcated " + FString::FromInt(Counter) + " files"), FColor::Green);
	}
}
