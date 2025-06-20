// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomStyle/SuperManagerStyle.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleRegistry.h"

FName FSuperManagerStyle::StyleSetName = FName("SuperManagerStyle");
TSharedPtr<FSlateStyleSet> FSuperManagerStyle::CreatedSlateStyleSet = nullptr;

void FSuperManagerStyle::InitializeIcons()
{
	if (!CreatedSlateStyleSet.IsValid())
	{
		CreatedSlateStyleSet = CreateSlateStyleSet();
		FSlateStyleRegistry::RegisterSlateStyle(*CreatedSlateStyleSet);
	}
}

TSharedRef<FSlateStyleSet> FSuperManagerStyle::CreateSlateStyleSet()
{
	TSharedRef<FSlateStyleSet> CustomStyleSet = MakeShareable(new FSlateStyleSet(StyleSetName));

	const FString IconDirectory =
		IPluginManager::Get().FindPlugin(TEXT("SuperManager"))->GetBaseDir() / "Resources";

	CustomStyleSet->SetContentRoot(IconDirectory);

	static const FVector2D Icon16x16(16.f, 16.f);

	CustomStyleSet->Set(
		TEXT("ContentBrowser.DeleteUnusedAssets"),
		new FSlateImageBrush(IconDirectory / "DeleteUnusedAsset.png", Icon16x16)
	);
	CustomStyleSet->Set(
		TEXT("ContentBrowser.DeleteEmptyFolders"),
		new FSlateImageBrush(IconDirectory / "DeleteEmptyFolders.png", Icon16x16)
	);
	CustomStyleSet->Set(
		TEXT("ContentBrowser.AdvanceDeletion"),
		new FSlateImageBrush(IconDirectory / "AdvanceDeletion.png", Icon16x16)
	);
	CustomStyleSet->Set(
		TEXT("LevelEditor.SelectionLock"),
		new FSlateImageBrush(IconDirectory / "SelectionLock.png", Icon16x16)
	);
	CustomStyleSet->Set(
		TEXT("LevelEditor.SelectionUnlock"),
		new FSlateImageBrush(IconDirectory / "SelectionUnlock.png", Icon16x16)
	);
	CustomStyleSet->Set(
		TEXT("LevelEditor.SelectionLockNew"),
		new FSlateImageBrush(IconDirectory / "SelectionLockNew.png", Icon16x16)
	);

	// 토글 버튼 스타일
	const FCheckBoxStyle ToggleButtonStyle = FCheckBoxStyle()
		.SetCheckBoxType(ESlateCheckBoxType::ToggleButton)
		.SetPadding(FMargin(10.0f))
		// Unchecked image
		.SetUncheckedImage(FSlateImageBrush(IconDirectory / "SelectionLockNew.png", Icon16x16, FStyleColors::White25))
		.SetUncheckedHoveredImage(FSlateImageBrush(IconDirectory / "SelectionLockNew.png", Icon16x16, FStyleColors::AccentBlue))
		.SetUncheckedPressedImage(FSlateImageBrush(IconDirectory / "SelectionLockNew.png", Icon16x16, FStyleColors::Foreground))

		.SetCheckedImage(FSlateImageBrush(IconDirectory / "SelectionLockNew.png", Icon16x16, FStyleColors::Foreground))
		.SetCheckedHoveredImage(FSlateImageBrush(IconDirectory / "SelectionLockNew.png", Icon16x16, FStyleColors::AccentBlue))
		.SetCheckedPressedImage(FSlateImageBrush(IconDirectory / "SelectionLockNew.png", Icon16x16, FStyleColors::AccentGray));

	CustomStyleSet->Set(
		TEXT("SceneOutliner.SelectionLock"),
		ToggleButtonStyle
	);

	return CustomStyleSet;
}

void FSuperManagerStyle::ShutDown()
{
	if (CreatedSlateStyleSet.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*CreatedSlateStyleSet);
		CreatedSlateStyleSet.Reset();
	}
}
