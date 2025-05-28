// Fill out your copyright notice in the Description page of Project Settings.


#include "Slate/AdvanceDeletionWidget.h"
#include "DebugHeader.h"

void SAdvanceDeletionWidget::Construct(const FArguments& InArgs)
{
	bCanSupportFocus = true;

	StoredAssetsData = InArgs._AssetsDataToStore;
	FSlateFontInfo TitleTextFont = FCoreStyle::Get().GetFontStyle(FName("EmbossedText"));
	TitleTextFont.Size = 30;

	ChildSlot
	[
		SNew(SVerticalBox) 
			// Title
			+SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
					.Text(FText::FromString(TEXT("Advance Deletion")))
					.Font(TitleTextFont)
					.Justification(ETextJustify::Center)
					.ColorAndOpacity(FColor::White)
			]

			// for drop down to specify the listing condition and help text
			+SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)
			]

			// for the asset list
			+SVerticalBox::Slot()
			.VAlign(VAlign_Fill)
			[
				SNew(SScrollBox)
					+SScrollBox::Slot()
					[
						SNew(SListView<TSharedPtr<FAssetData>>)
							.ItemHeight(24.f)
							.ListItemsSource(&StoredAssetsData)
							.OnGenerateRow(this, &SAdvanceDeletionWidget::OnGenerateRowForList)
					]
			]

			//for 3 buttons
			+SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)
			]
	];
}

TSharedRef<ITableRow> SAdvanceDeletionWidget::OnGenerateRowForList(
	TSharedPtr<FAssetData> AssetDataToDisplay,
	const TSharedRef<STableViewBase>& OwnerTable
)
{
	if (!AssetDataToDisplay.IsValid())
	{
		return SNew(STableRow<TSharedPtr<FAssetData>>, OwnerTable);
	}
	const FString DisplayAssetName = AssetDataToDisplay->AssetName.ToString();

	TSharedRef<STableRow<TSharedPtr<FAssetData>>> TableRow = SNew(STableRow<TSharedPtr<FAssetData>>, OwnerTable)
		[
			SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Center)
				.FillWidth(0.05f)
				[
					ConstructCheckBox(AssetDataToDisplay)
				]

				+SHorizontalBox::Slot()
				[
					SNew(STextBlock)
						.Text(FText::FromString(DisplayAssetName))
				]
		];

	return TableRow;
}

TSharedRef<SCheckBox> SAdvanceDeletionWidget::ConstructCheckBox(const TSharedPtr<FAssetData> AssetDataToDisplay)
{
	return SNew(SCheckBox)
		.Type(ESlateCheckBoxType::CheckBox)
		.OnCheckStateChanged(this, &SAdvanceDeletionWidget::OnCheckBoxStateChange, AssetDataToDisplay)
		.Visibility(EVisibility::Visible);
}

void SAdvanceDeletionWidget::OnCheckBoxStateChange(ECheckBoxState NewState, TSharedPtr<FAssetData> AssetData)
{
	switch (NewState)
	{
	case ECheckBoxState::Unchecked:
		DebugHeader::Print(AssetData->AssetName.ToString() + TEXT(" is unchecked"), FColor::Green);
		break;
	case ECheckBoxState::Checked:
		DebugHeader::Print(AssetData->AssetName.ToString() + TEXT(" is checked"), FColor::Green);
		break;
	case ECheckBoxState::Undetermined:
		break;
	default:
		break;
	}
}
