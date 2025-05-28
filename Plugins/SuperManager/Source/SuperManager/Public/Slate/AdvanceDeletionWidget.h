// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "Widgets/SCompoundWidget.h"

class SAdvanceDeletionWidget : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SAdvanceDeletionWidget) {}
		SLATE_ARGUMENT(TArray<TSharedPtr<FAssetData>>, AssetsDataArray)
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs);

private:
	TArray<TSharedPtr<FAssetData>> AssetsDataUnderSelectedFolderArray;
};