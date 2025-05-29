// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "Widgets/SCompoundWidget.h"

class SAdvanceDeletionWidget : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SAdvanceDeletionWidget) {}
		SLATE_ARGUMENT(TArray<TSharedPtr<FAssetData>>, AssetsDataToStore)
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs);

private:
	TSharedRef<ITableRow> OnGenerateRowForList(
		TSharedPtr<FAssetData> AssetDataToDisplay,
		const TSharedRef<STableViewBase>& OwnerTable
	);
	TSharedRef<SListView<TSharedPtr<FAssetData>>> ConstructAssetListView();
	TSharedRef<SCheckBox> ConstructCheckBox(const TSharedPtr<FAssetData> AssetDataToDisplay);
	TSharedRef<STextBlock> ConstructTextForRowWidget(const FString& TextContent, const FSlateFontInfo& FontToUse);
	TSharedRef<SButton> ConstructButtonForRowWidget(const TSharedPtr<FAssetData>& AssetDataToDisply);
	TSharedRef<SButton> ConstructTabButton();
	TSharedRef<STextBlock> ConstructTextForTabButtons(const FString& TextContent);
	TSharedRef<SButton> ConstructDeleteAllButton();
	TSharedRef<SButton> ConstructSelectAllButton();
	TSharedRef<SButton> ConstructDeselectAllButton();
	TSharedRef<SComboBox<TSharedPtr<FString>>> ConstructComboBox();
	void OnCheckBoxStateChange(ECheckBoxState NewState, TSharedPtr<FAssetData> AssetData);
	FReply OnDeleteButtonClicked(const TSharedPtr<FAssetData>& AssetDataToDisply);
	FReply OnDeleteAllButtonClicked();
	FReply OnSelectAllButtonClicked();
	FReply OnDeselectAllButtonClicked();
	TSharedRef<SWidget> OnGenerateComboContent(TSharedPtr<FString> SourceItem);
	void OnComboSelectionChanged(TSharedPtr<FString> SelectedOption, ESelectInfo::Type InSelectInfo);
	void RefreshAssetListView();

	TArray<TSharedPtr<FAssetData>> StoredAssetsData;
	TArray<TSharedPtr<FAssetData>> DisplayedAssetsData;
	TArray<TSharedPtr<FAssetData>> AssetsDataToDeleteArray;
	TArray<TSharedRef<SCheckBox>> CheckBoxesArray;
	TArray<TSharedPtr<FString>> ComboBoxSourceItems;
	TSharedPtr<SListView<TSharedPtr<FAssetData>>> ConstructedAssetListView;
	TSharedPtr<STextBlock> ComboDisplayTextBlock;


	FORCEINLINE FSlateFontInfo GetEmbossedTextFont() const
	{
		return FCoreStyle::Get().GetFontStyle(FName("EmbossedText"));
	}
};