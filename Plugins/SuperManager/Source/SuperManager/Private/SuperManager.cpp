// Copyright Epic Games, Inc. All Rights Reserved.

#include "SuperManager.h"
#include "ContentBrowserModule.h"
#include "EditorAssetLibrary.h"
#include "ObjectTools.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "Slate/AdvanceDeletionWidget.h"
#include "DebugHeader.h"

#define LOCTEXT_NAMESPACE "FSuperManagerModule"

void FSuperManagerModule::StartupModule()
{
	InitCBMenuExtention();

	RegisterAdvancedDeletionTab();
}

void FSuperManagerModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}
void FSuperManagerModule::FixUpRedirectors()
{
	TArray<UObjectRedirector*> RedirectorsToFixArray;

	FAssetRegistryModule& AssetRegistryModule =
		FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	FARFilter Filter;
	Filter.bRecursivePaths = true;
	Filter.PackagePaths.Emplace("/Game");
	Filter.ClassPaths.Emplace(UObjectRedirector::StaticClass());

	TArray<FAssetData> OutRedirectors;
	AssetRegistryModule.Get().GetAssets(Filter, OutRedirectors);

	for (const FAssetData& RedirectorData : OutRedirectors)
	{
		UObjectRedirector* RedirectorToFix = Cast<UObjectRedirector>(RedirectorData.GetAsset());
		if (RedirectorToFix)
		{
			RedirectorsToFixArray.Add(RedirectorToFix);
		}

	}

	FAssetToolsModule& AssettoolsModule =
		FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));

	AssettoolsModule.Get().FixupReferencers(RedirectorsToFixArray);
	
}
#pragma region ContentBrowserMenuExtention
void FSuperManagerModule::InitCBMenuExtention()
{
	FContentBrowserModule& ContentBrowserModule = 
		FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));

	TArray<FContentBrowserMenuExtender_SelectedPaths>& ContentBrowserModuleMenuExtenders = 
		ContentBrowserModule.GetAllPathViewContextMenuExtenders();

	FContentBrowserMenuExtender_SelectedPaths CustomCBMenuDelegate;
	CustomCBMenuDelegate.BindRaw(this, &FSuperManagerModule::CustomCBMenuExtender);
	ContentBrowserModuleMenuExtenders.Add(CustomCBMenuDelegate);

	// 델리게이트를 바인딩하는 또다른 방법
	//ContentBrowserModuleMenuExtenders.Add(FContentBrowserMenuExtender_SelectedPaths::CreateRaw(this, &FSuperManagerModule::CustomCBMenuExtender));
}

TSharedRef<FExtender> FSuperManagerModule::CustomCBMenuExtender(const TArray<FString>& SelectedPaths)
{
	TSharedRef<FExtender> MenuExtender(new FExtender());
	if (SelectedPaths.Num() > 0)
	{
		MenuExtender->AddMenuExtension(
			FName("Delete"),			// Extension hook, position to insert
			EExtensionHook::After,		// Delete 라는 Extention Hook을 가진 extenion 밑에 추가
			TSharedPtr<FUICommandList>(),	// custom hot key
			FMenuExtensionDelegate::CreateRaw(this, &FSuperManagerModule::AddCBMenuEntry)	// Second binding, will define details for this menu entry
		);

		FolderPathsSelected = SelectedPaths;
	}
	return MenuExtender;
}

// Define details for the custom menu entry
void FSuperManagerModule::AddCBMenuEntry(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.AddMenuEntry(
		FText::FromString(TEXT("Delete unused assets")),		// MenuEntry Title
		FText::FromString(TEXT("Safely delete all unused assets under folder")),	// MenuEntry에 대한 tooltip
		FSlateIcon(),	// Custom icon
		FExecuteAction::CreateRaw(this, &FSuperManagerModule::OnDeleteUnusedAssetButtonClicked)	//The actual function to execute
	);
	MenuBuilder.AddMenuEntry(
		FText::FromString(TEXT("Delete empty folder")),
		FText::FromString(TEXT("Safely delete all empty folders")),	// MenuEntry에 대한 tooltip
		FSlateIcon(),	// Custom icon
		FExecuteAction::CreateRaw(this, &FSuperManagerModule::OnDeleteEmptyFolderButtonClicked)	//The actual function to execute
	);
	MenuBuilder.AddMenuEntry(
		FText::FromString(TEXT("Advance delete")),
		FText::FromString(TEXT("List assets by specific condition in a tab for deleting")),	// MenuEntry에 대한 tooltip
		FSlateIcon(),	// Custom icon
		FExecuteAction::CreateRaw(this, &FSuperManagerModule::OnAdvancedDeleteButtonClicked)	//The actual function to execute
	);
}

void FSuperManagerModule::OnDeleteUnusedAssetButtonClicked()
{
	if (FolderPathsSelected.Num() > 1)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("You can only do this to one folder"));
		return;
	}

	FixUpRedirectors();
	
	TArray<FString> AssetsPathNames = UEditorAssetLibrary::ListAssets(FolderPathsSelected[0]);
	if (AssetsPathNames.Num() == 0)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("No assets found under selected folder"), false);
		return;
	}

	TArray<FAssetData> UnusedAssetsData;
	
	for (const FString& AssetPathName : AssetsPathNames)
	{
		// root folder는 건디리지 않는다
		if (IsRootFolderPath(AssetPathName))
		{
			continue;
		}

		if (!UEditorAssetLibrary::DoesAssetExist(AssetPathName))
		{
			continue;
		}
		TArray<FString> Referencers = UEditorAssetLibrary::FindPackageReferencersForAsset(AssetPathName);
		if (Referencers.Num() == 0)
		{
			UnusedAssetsData.Add(UEditorAssetLibrary::FindAssetData(AssetPathName));
		}
	}

	if (UnusedAssetsData.Num() > 0)
	{
		EAppReturnType::Type ConfirmResult = DebugHeader::ShowMsgDialog(
			EAppMsgType::YesNo,
			TEXT("A total of ") + FString::FromInt(UnusedAssetsData.Num()) + TEXT(" found.\
			\nWould you like to proceed"), false
		);

		if (ConfirmResult == EAppReturnType::No)
		{
			return;
		}

		ObjectTools::DeleteAssets(UnusedAssetsData);
	}
	else
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("No unused assets found under selected folder"));
	}
}

void FSuperManagerModule::OnDeleteEmptyFolderButtonClicked()
{
	FixUpRedirectors();
	
	FString EmptyFolderPathsNames;
	TArray<FString> EmptyFoldersPathArray;
	for (const FString& FolderPathSelected : FolderPathsSelected)
	{
		if (IsRootFolderPath(FolderPathSelected))
		{
			continue;
		}

		TArray<FString> FolderPathsArray = UEditorAssetLibrary::ListAssets(FolderPathSelected, true, true);
		for (const FString FolderPath : FolderPathsArray)
		{
			if (IsRootFolderPath(FolderPath) || !UEditorAssetLibrary::DoesDirectoryExist(FolderPath))
			{
				continue;
			}

			TArray<FString> AssetsInFolder = UEditorAssetLibrary::ListAssets(FolderPath, true);
			// TODO: 이유는 모르겠지만 DoesDirectoryHaveAssets 함수가 무조건 false 반환함.
			// if(!UEditorAssetLibrary::DoesDirectoryHaveAssets(FolderPath, true))
			if (AssetsInFolder.Num() < 1)
			{
				EmptyFolderPathsNames.Append(FolderPath);
				EmptyFolderPathsNames.Append("\n");

				EmptyFoldersPathArray.Add(FolderPath);
			}
		}
	}

	if (EmptyFoldersPathArray.Num() == 0)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("No empty folder"), false);
		return;
	}

	EAppReturnType::Type ConfirmResult = DebugHeader::ShowMsgDialog(
		EAppMsgType::YesNo,
		TEXT("Empty folders found in:\n") +
		EmptyFolderPathsNames +
		TEXT("Would you like to delete all?"),
		false
	);

	int32 Counter = 0;
	if (ConfirmResult == EAppReturnType::Type::No)
	{
		return;
	}

	for (const FString& EmptyFolderPath : EmptyFoldersPathArray)
	{
		if (UEditorAssetLibrary::DeleteDirectory(EmptyFolderPath))
		{
			++Counter;
		}
		else
		{
			DebugHeader::Print(TEXT("Failed to delete: ") + EmptyFolderPath, FColor::Red);
		}
	}
	
	DebugHeader::ShowMsgDialog(
		EAppMsgType::Ok,
		TEXT("Delete: ") + FString::FromInt(Counter) +
		TEXT("\nFailed: " + FString::FromInt(EmptyFoldersPathArray.Num() - Counter)),
		false
	);
}

void FSuperManagerModule::OnAdvancedDeleteButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(FName("AdvanceDeletion"));
}

bool FSuperManagerModule::IsRootFolderPath(const FString& FolderPath)
{
	return 
		FolderPath.Contains(TEXT("Developers")) ||
		FolderPath.Contains(TEXT("Collections")) ||
		FolderPath.Contains(TEXT("__ExternalActors__")) ||
		FolderPath.Contains(TEXT("__ExternalObjects__"));
}

#pragma endregion
#pragma region CustomEditorTab

void FSuperManagerModule::RegisterAdvancedDeletionTab()
{
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		FName("AdvanceDeletion"),
		FOnSpawnTab::CreateRaw(this, &FSuperManagerModule::OnSpawnAdvanceDeletionTab)
	).SetDisplayName(FText::FromString(TEXT("Advance Deletion")));
}

TSharedRef<SDockTab> FSuperManagerModule::OnSpawnAdvanceDeletionTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab).TabRole(ETabRole::NomadTab)[
		SNew(SAdvanceDeletionWidget)
			.AssetsDataArray(GetAllAssetDataUnderSelectedFolder())
	];
}

TArray<TSharedPtr<FAssetData>> FSuperManagerModule::GetAllAssetDataUnderSelectedFolder()
{
	TArray<TSharedPtr<FAssetData>> AvailableAssetsData;

	for (const FString& FolderPathSelected : FolderPathsSelected)
	{
		TArray<FString> AssetPathsName = UEditorAssetLibrary::ListAssets(FolderPathSelected);
		for (const FString& AssetPath : AssetPathsName)
		{
			if (IsRootFolderPath(AssetPath) || ! UEditorAssetLibrary::DoesAssetExist(AssetPath))
			{
				continue;
			}

			const FAssetData AssetData = UEditorAssetLibrary::FindAssetData(AssetPath);
			AvailableAssetsData.Add(MakeShared<FAssetData>(AssetData));
		}
	}
	return AvailableAssetsData;
}

#pragma endregion

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSuperManagerModule, SuperManager)