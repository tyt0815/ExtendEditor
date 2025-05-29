// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

class FSuperManagerModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	void FixUpRedirectors();

private:

	TArray<FString> FolderPathsSelected;

#pragma region ContentBrowserMenuExtention
	void InitCBMenuExtention();
	TSharedRef<FExtender> CustomCBMenuExtender(const TArray<FString>& SelectedPaths);
	void AddCBMenuEntry(FMenuBuilder& MenuBuilder);
	void OnDeleteUnusedAssetButtonClicked();
	void OnDeleteEmptyFolderButtonClicked();
	void OnAdvancedDeleteButtonClicked();
	bool IsRootFolderPath(const FString& FolderPath);
#pragma endregion

#pragma region CustomEditorTab
	void RegisterAdvancedDeletionTab();
	TSharedRef<SDockTab>  OnSpawnAdvanceDeletionTab(const FSpawnTabArgs& SpawnTabArgs);
	TArray<TSharedPtr<FAssetData>> GetAllAssetDataUnderSelectedFolder();
#pragma endregion

#pragma region ProcessDataForAdvanceDeletionTab
public:
	bool DeleteSingleAssetForAssetList(const FAssetData& AssetDataToDelete);
	bool DeleteMultipleAssetsForAssetList(const TArray<FAssetData>& AssetsToDelete);
	void ListUnusedAssetsForAssetList(
		const TArray<TSharedPtr<FAssetData>>& AssetsDataToFilter,
		TArray<TSharedPtr<FAssetData>>& OutUnusedAssetsData
	);
	void ListSameNameAssetsForAssetList(
		const TArray<TSharedPtr<FAssetData>>& AssetsDataToFilter,
		TArray<TSharedPtr<FAssetData>>& OutSameNameAssetsData
	);
#pragma endregion
};
