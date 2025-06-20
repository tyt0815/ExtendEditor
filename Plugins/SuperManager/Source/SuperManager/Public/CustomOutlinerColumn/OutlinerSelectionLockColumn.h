// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ISceneOutlinerColumn.h"

class FOutlinerSelectionLockColumn : public ISceneOutlinerColumn
{
public:
	FOutlinerSelectionLockColumn(ISceneOutliner& SceneOutliner) {};

	virtual SHeaderRow::FColumn::FArguments ConstructHeaderRowColumn() override;

	virtual const TSharedRef<SWidget> ConstructRowWidget(FSceneOutlinerTreeItemRef TreeItem, const STableRow<FSceneOutlinerTreeItemPtr>& Row) override;

private:
	void OnSelectionLockCheckStateChanged(ECheckBoxState NewState, TWeakObjectPtr<AActor> Actor);


public:
	FORCEINLINE virtual FName GetColumnID() override
	{
		return TEXT("SelectionLock");
	}

	FORCEINLINE static FName GetID()
	{
		return TEXT("SelectionLock");
	}
};