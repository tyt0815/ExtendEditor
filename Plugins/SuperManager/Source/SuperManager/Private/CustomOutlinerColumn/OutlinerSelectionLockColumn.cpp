// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomOutlinerColumn/OutlinerSelectionLockColumn.h"
#include "CustomStyle/SuperManagerStyle.h"
#include "ActorTreeItem.h"
#include "SuperManager.h"
#include "DebugHeader.h"

SHeaderRow::FColumn::FArguments FOutlinerSelectionLockColumn::ConstructHeaderRowColumn()
{
	SHeaderRow::FColumn::FArguments ConstructedHeaderRow =
		SHeaderRow::Column(GetColumnID())
		.FixedWidth(24.0f)
		.HAlignHeader(HAlign_Center)
		.VAlignHeader(VAlign_Center)
		.HAlignCell(HAlign_Center)
		.VAlignCell(VAlign_Center)
		.DefaultTooltip(FText::FromString(TEXT("Actor Selection Lock - Press icon to lock actor selection")))
		[
			SNew(SImage)
				.ColorAndOpacity(FSlateColor::UseForeground())
				.Image(FSuperManagerStyle::GetCreatedSlateStyleSet()->GetBrush(FName("LevelEditor.SelectionLockNew")))
		];

	return ConstructedHeaderRow;
}

const TSharedRef<SWidget> FOutlinerSelectionLockColumn::ConstructRowWidget(
	FSceneOutlinerTreeItemRef TreeItem,
	const STableRow<FSceneOutlinerTreeItemPtr>& Row
)
{
	FActorTreeItem* ActorTreeItem = TreeItem->CastTo<FActorTreeItem>();
	if (!ActorTreeItem || !ActorTreeItem->IsValid())
	{
		return SNullWidget::NullWidget;
	}

	FSuperManagerModule& SuperManager = FModuleManager::LoadModuleChecked<FSuperManagerModule>(TEXT("SuperManager"));
	bool bSelectionLocked = SuperManager.IsActorSelectionLocked(ActorTreeItem->Actor.Get());

	const FCheckBoxStyle& ToggleButtonStyle = 
		FSuperManagerStyle::GetCreatedSlateStyleSet()->GetWidgetStyle<FCheckBoxStyle>(TEXT("SceneOutliner.SelectionLock"));

	return SNew(SCheckBox)
		.Type(ESlateCheckBoxType::ToggleButton)
		.Style(&ToggleButtonStyle)
		.IsChecked(bSelectionLocked ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
		.HAlign(HAlign_Center)
		.Visibility(EVisibility::Visible)
		.OnCheckStateChanged_Lambda([this, ActorTreeItem](ECheckBoxState NewState)
			{
				OnSelectionLockCheckStateChanged(NewState, ActorTreeItem->Actor);
			}
		);
}

void FOutlinerSelectionLockColumn::OnSelectionLockCheckStateChanged(ECheckBoxState NewState, TWeakObjectPtr<AActor> Actor)
{
	FSuperManagerModule& SuperManager = FModuleManager::LoadModuleChecked<FSuperManagerModule>(TEXT("SuperManager"));
	if (!Actor.IsValid())
	{
		return;
	}
	switch (NewState)
	{
	case ECheckBoxState::Unchecked:
		SuperManager.SetActorSelectionLockEnable(Actor.Get(), false);
		DebugHeader::ShowNotifyInfo(TEXT("Selection Lock\n") + Actor->GetActorLabel());
		break;
	case ECheckBoxState::Checked:
		SuperManager.SetActorSelectionLockEnable(Actor.Get(), true);
		DebugHeader::ShowNotifyInfo(TEXT("Selection Unlock\n") + Actor->GetActorLabel());
		break;
	case ECheckBoxState::Undetermined:
		break;
	default:
		break;
	}
}
