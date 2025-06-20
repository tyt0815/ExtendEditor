// Copyright Epic Games, Inc. All Rights Reserved.

#include "SuperManager.h"
#include "ContentBrowserModule.h"
#include "LevelEditor.h"
#include "EditorAssetLibrary.h"
#include "ObjectTools.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "Slate/AdvanceDeletionWidget.h"
#include "CustomStyle/SuperManagerStyle.h"
#include "Engine/Selection.h"
#include "Subsystems/EditorActorSubsystem.h"
#include "CustomUICommand/SuperManagerUICommands.h"
#include "SceneOutlinerModule.h"
#include "CustomOutlinerColumn/OutlinerSelectionLockColumn.h"
#include "DebugHeader.h"

#define LOCTEXT_NAMESPACE "FSuperManagerModule"

void FSuperManagerModule::StartupModule()
{
	FSuperManagerStyle::InitializeIcons();

	InitCBMenuExtention();

	RegisterAdvancedDeletionTab();

	InitCustomUICommands();

	InitLevelEditorExtention();

	InitCustomSelectionEvent();

	InitSceneOutlinerColumn();
}

void FSuperManagerModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(FName("AdvanceDeletion"));
	FSuperManagerStyle::ShutDown();
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
		FSlateIcon(FSuperManagerStyle::GetStyleSetName(), TEXT("ContentBrowser.DeleteUnusedAssets")),	// Custom icon
		FExecuteAction::CreateRaw(this, &FSuperManagerModule::OnDeleteUnusedAssetButtonClicked)	//The actual function to execute
	);
	MenuBuilder.AddMenuEntry(
		FText::FromString(TEXT("Delete empty folder")),
		FText::FromString(TEXT("Safely delete all empty folders")),	// MenuEntry에 대한 tooltip
		FSlateIcon(FSuperManagerStyle::GetStyleSetName(), TEXT("ContentBrowser.DeleteEmptyFolders")),	// Custom icon
		FExecuteAction::CreateRaw(this, &FSuperManagerModule::OnDeleteEmptyFolderButtonClicked)	//The actual function to execute
	);
	MenuBuilder.AddMenuEntry(
		FText::FromString(TEXT("Advance delete")),
		FText::FromString(TEXT("List assets by specific condition in a tab for deleting")),	// MenuEntry에 대한 tooltip
		FSlateIcon(FSuperManagerStyle::GetStyleSetName(), TEXT("ContentBrowser.AdvanceDeletion")),	// Custom icon
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
	)
		.SetDisplayName(FText::FromString(TEXT("Advance Deletion")))
		.SetIcon(FSlateIcon(FSuperManagerStyle::GetStyleSetName(), TEXT("ContentBrowser.AdvanceDeletion")));
}

TSharedRef<SDockTab> FSuperManagerModule::OnSpawnAdvanceDeletionTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab).TabRole(ETabRole::NomadTab)[
		SNew(SAdvanceDeletionWidget)
			.AssetsDataToStore(GetAllAssetDataUnderSelectedFolder())
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

#pragma region ProcessDataForAdvanceDeletionTab

bool FSuperManagerModule::DeleteSingleAssetForAssetList(const FAssetData& AssetDataToDelete)
{
	TArray<FAssetData> AssetsToDelete;
	AssetsToDelete.Add(AssetDataToDelete);
	return ObjectTools::DeleteAssets(AssetsToDelete) > 0 ? true : false;
}

bool FSuperManagerModule::DeleteMultipleAssetsForAssetList(const TArray<FAssetData>& AssetsToDelete)
{
	return ObjectTools::DeleteAssets(AssetsToDelete) > 0 ? true : false;
}

void FSuperManagerModule::ListUnusedAssetsForAssetList(
	const TArray<TSharedPtr<FAssetData>>& AssetsDataToFilter,
	TArray<TSharedPtr<FAssetData>>& OutUnusedAssetsData
)
{
	OutUnusedAssetsData.Empty();

	for (const TSharedPtr<FAssetData>& AssetData : AssetsDataToFilter)
	{
		TArray<FString> Referencers = UEditorAssetLibrary::FindPackageReferencersForAsset(AssetData->GetObjectPathString());
		if (Referencers.Num() == 0)
		{
			OutUnusedAssetsData.Add(AssetData);
		}
	}
}

void FSuperManagerModule::ListSameNameAssetsForAssetList(const TArray<TSharedPtr<FAssetData>>& AssetsDataToFilter, TArray<TSharedPtr<FAssetData>>& OutSameNameAssetsData)
{
	OutSameNameAssetsData.Empty();
	TMultiMap<FString, TSharedPtr<FAssetData>> AssetsDataMap;

	for (const TSharedPtr<FAssetData>& AssetData : AssetsDataToFilter)
	{
		AssetsDataMap.Add(AssetData->AssetName.ToString(), AssetData);
	}

	TArray<FString> Keys;
	AssetsDataMap.GetKeys(Keys);
	for (const FString& Key : Keys)
	{
		TArray<TSharedPtr<FAssetData>> AssetsDataArray;
		AssetsDataMap.MultiFind(Key, AssetsDataArray);
		if (AssetsDataArray.Num() > 1)
		{
			OutSameNameAssetsData.Append(AssetsDataArray);
		}
	}
}

#pragma endregion

void FSuperManagerModule::InitLevelEditorExtention()
{
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));
	
	TSharedRef<FUICommandList> GlobalLevelEditorActions = LevelEditorModule.GetGlobalLevelEditorActions();
	GlobalLevelEditorActions->Append(CustomUICommands.ToSharedRef());

	TArray<FLevelEditorModule::FLevelViewportMenuExtender_SelectedActors>& LevelEditorMenuExtenders =
		LevelEditorModule.GetAllLevelViewportContextMenuExtenders();
	LevelEditorMenuExtenders.Add(
		FLevelEditorModule::FLevelViewportMenuExtender_SelectedActors::CreateRaw(
			this,
			&FSuperManagerModule::CustomLevelEditorMenuExtender
		)
	);
}

TSharedRef<FExtender> FSuperManagerModule::CustomLevelEditorMenuExtender(const TSharedRef<FUICommandList> UICommandList, const TArray<AActor*> SelectedActors)
{
	TSharedRef<FExtender> MenuExtender = MakeShareable(new FExtender());
	if (!SelectedActors.IsEmpty())
	{
		MenuExtender->AddMenuExtension(
			FName("ActorOptions"),			// Extension hook, position to insert
			EExtensionHook::Before,		// Delete 라는 Extention Hook을 가진 extenion 밑에 추가
			TSharedPtr<FUICommandList>(),	// custom hot key
			FMenuExtensionDelegate::CreateRaw(this, &FSuperManagerModule::AddLevelEditorMenuEntry)	// Second binding, will define details for this menu entry
		);
	}
	return MenuExtender;
}

void FSuperManagerModule::AddLevelEditorMenuEntry(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.AddMenuEntry(
		FText::FromString(TEXT("Lock Actor Selection")),		// MenuEntry Title
		FText::FromString(TEXT("Prevent actor from being selected")),	// MenuEntry에 대한 tooltip
		FSlateIcon(FSuperManagerStyle::GetStyleSetName(), TEXT("LevelEditor.SelectionLock")),	// Custom icon
		FExecuteAction::CreateRaw(this, &FSuperManagerModule::OnLockActorSelectionButtonClicked)	//The actual function to execute
	);
	MenuBuilder.AddMenuEntry(
		FText::FromString(TEXT("Unlock All Actor Selection")),		// MenuEntry Title
		FText::FromString(TEXT("Remove the selection constranint on all actor")),	// MenuEntry에 대한 tooltip
		FSlateIcon(FSuperManagerStyle::GetStyleSetName(), TEXT("LevelEditor.SelectionUnlock")),	// Custom icon
		FExecuteAction::CreateRaw(this, &FSuperManagerModule::OnUnlockActorSelectionButtonClicked)	//The actual function to execute
	);
}

void FSuperManagerModule::OnLockActorSelectionButtonClicked()
{
	if (!GetEditorActorSubsystem())
	{
		return;
	}

	TArray<AActor*> SelectedLevelActors = EditorActorSubsystem->GetSelectedLevelActors();
	FString LockedActorList = TEXT("");
	int32 Counter = 0;
	for (AActor* SelectedActor : SelectedLevelActors)
	{
		if (!SelectedActor)
		{
			continue;
		}

		SetActorSelectionLockEnable(SelectedActor, true);
		LockedActorList.Append(TEXT("\n") + SelectedActor->GetActorLabel());
		++Counter;
	}
	DebugHeader::ShowNotifyInfo(TEXT("Lock ") + FString::FromInt(Counter) + TEXT(" Actors") + LockedActorList);
}

void FSuperManagerModule::OnUnlockActorSelectionButtonClicked()
{
	FString UnlockedActorList = TEXT("");
	int32 Counter = 0;
	for (AActor* LockedActor : SelectionLockedActors)
	{
		if (!LockedActor)
		{
			continue;
		}
		SetActorSelectionLockEnable(LockedActor, false);
		UnlockedActorList.Append(TEXT("\n") + LockedActor->GetActorLabel());
		++Counter;
	}
	DebugHeader::ShowNotifyInfo(TEXT("Unlock ") + FString::FromInt(Counter) + TEXT(" Actors") + UnlockedActorList);
}

void FSuperManagerModule::InitCustomSelectionEvent()
{
	USelection::SelectObjectEvent.AddRaw(this, &FSuperManagerModule::OnActorSelected);
}

void FSuperManagerModule::OnActorSelected(UObject* SelectedObject)
{
	AActor* SelectedActor = Cast<AActor>(SelectedObject);
	if (!GetEditorActorSubsystem() || !SelectedActor)
	{
		return;
	}
	
	if (IsActorSelectionLocked(SelectedActor))
	{
		EditorActorSubsystem->SetActorSelectionState(SelectedActor, false);
	}
}

void FSuperManagerModule::SetActorSelectionLockEnable(AActor* Actor, bool bEnable)
{
	if (!Actor || !GetEditorActorSubsystem())
	{
		return;
	}
	if (bEnable)
	{
		SelectionLockedActors.AddUnique(Actor);
		EditorActorSubsystem->SetActorSelectionState(Actor, false);
	}
	else
	{
		SelectionLockedActors.Remove(Actor);
	}
	RefreshSceneOutliner();
}

bool FSuperManagerModule::IsActorSelectionLocked(AActor* Actor)
{
	return Actor ? SelectionLockedActors.Find(Actor) >= 0 : false;
}

void FSuperManagerModule::InitCustomUICommands()
{
	FSuperManagerUICommands::Register();

	CustomUICommands = MakeShareable<FUICommandList>(new FUICommandList());
	if (CustomUICommands.IsValid())
	{
		CustomUICommands->MapAction(
			FSuperManagerUICommands::Get().LockActorSelection,
			FExecuteAction::CreateRaw(this, &FSuperManagerModule::OnSelectionLockHotKeyPressed)
		);
		CustomUICommands->MapAction(
			FSuperManagerUICommands::Get().UnlockActorSelection,
			FExecuteAction::CreateRaw(this, &FSuperManagerModule::OnUnlockSelectionHotKeyPressed)
		);
	}
}

void FSuperManagerModule::OnSelectionLockHotKeyPressed()
{
	OnLockActorSelectionButtonClicked();
}

void FSuperManagerModule::OnUnlockSelectionHotKeyPressed()
{
	OnUnlockActorSelectionButtonClicked();
}

void FSuperManagerModule::InitSceneOutlinerColumn()
{
	FSceneOutlinerModule& SceneOutlinerModule =
		FModuleManager::LoadModuleChecked<FSceneOutlinerModule>(TEXT("SceneOutliner"));

	FSceneOutlinerColumnInfo SelectionLockColumnInfo(
		ESceneOutlinerColumnVisibility::Visible,
		1,
		FCreateSceneOutlinerColumn::CreateRaw(this, &FSuperManagerModule::OnCreateSelectionLockColumn)
	);

	SceneOutlinerModule.RegisterDefaultColumnType<FOutlinerSelectionLockColumn>(SelectionLockColumnInfo);
}

TSharedRef<ISceneOutlinerColumn> FSuperManagerModule::OnCreateSelectionLockColumn(ISceneOutliner& SceneOutliner)
{
	return MakeShareable(new FOutlinerSelectionLockColumn(SceneOutliner));
}

void FSuperManagerModule::RefreshSceneOutliner()
{
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));
	TSharedPtr<ISceneOutliner> SceneOutliner = LevelEditorModule.GetFirstLevelEditor()->GetMostRecentlyUsedSceneOutliner();
	if (SceneOutliner.IsValid())
	{
		SceneOutliner->FullRefresh();
	}
}

UEditorActorSubsystem* FSuperManagerModule::GetEditorActorSubsystem()
{
	if (!EditorActorSubsystem)
	{
		EditorActorSubsystem = GEditor->GetEditorSubsystem<UEditorActorSubsystem>();
	}
	return EditorActorSubsystem;
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSuperManagerModule, SuperManager)