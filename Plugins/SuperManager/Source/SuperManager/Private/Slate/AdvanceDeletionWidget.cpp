// Fill out your copyright notice in the Description page of Project Settings.


#include "Slate/AdvanceDeletionWidget.h"
#include "SuperManager.h"
#include "EditorAssetLibrary.h"
#include "DebugHeader.h"

#define LIST_ALL TEXT("List All Assets")
#define LIST_UNUSED TEXT("List Unused Assets")
#define LIST_SAME_NAME TEXT("List Same Name Assets")

void SAdvanceDeletionWidget::Construct(const FArguments& InArgs)
{
	bCanSupportFocus = true;
	StoredAssetsData = InArgs._AssetsDataToStore;
	DisplayedAssetsData = StoredAssetsData;
	FSlateFontInfo TitleTextFont = GetEmbossedTextFont();
	TitleTextFont.Size = 30;

	ComboBoxSourceItems.Add(MakeShared<FString>(LIST_ALL));
	ComboBoxSourceItems.Add(MakeShared<FString>(LIST_UNUSED));
	ComboBoxSourceItems.Add(MakeShared<FString>(LIST_SAME_NAME));

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

			+SVerticalBox::Slot()
			.AutoHeight()
			.Padding(5.0f)
			[
				SNew(SHorizontalBox)
					+SHorizontalBox::Slot()
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					.AutoWidth()
					[
						ConstructComboBox()
					]
					+ SHorizontalBox::Slot()
					.FillWidth(.6f)
					[
						ConstructHelpTextBlock(
							TEXT("Specify the listing condition in the drop. Left mouse click to go to where a asset is located."),
							ETextJustify::Center
						)
					]
					+ SHorizontalBox::Slot()
						.FillWidth(.2f)
						[
							ConstructHelpTextBlock(
								TEXT("Test\ntest\ntest\ntest\ntest"),
								ETextJustify::Right
							)
						]
			]

			// for the asset list
			+SVerticalBox::Slot()
			.VAlign(VAlign_Fill)
			[
				SNew(SScrollBox)
					+SScrollBox::Slot()
					[
						ConstructAssetListView()
					]
			]

			//for 3 buttons
			+SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)
					+SHorizontalBox::Slot()
					.FillWidth(10.f)
					.Padding(5.0f)
					[
						ConstructDeleteAllButton()
					]

					+ SHorizontalBox::Slot()
					.FillWidth(10.f)
					.Padding(5.0f)
					[
						ConstructSelectAllButton()
					]

					+ SHorizontalBox::Slot()
					.FillWidth(10.f)
					.Padding(5.0f)
					[
						ConstructDeselectAllButton()
					]
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
	const FString DisplayAssetClassName = AssetDataToDisplay->AssetClassPath.GetAssetName().ToString();
	const FString DisplayAssetName = AssetDataToDisplay->AssetName.ToString();
	FSlateFontInfo AssetClassNameFont = GetEmbossedTextFont();
	AssetClassNameFont.Size = 10.0f;
	FSlateFontInfo AssetNameFont = GetEmbossedTextFont();
	AssetNameFont.Size = 15.0f;

	TSharedRef<STableRow<TSharedPtr<FAssetData>>> TableRow = SNew(STableRow<TSharedPtr<FAssetData>>, OwnerTable)
		.Padding(5.0f)
		[
			SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
					.HAlign(HAlign_Left)
					.VAlign(VAlign_Fill)
					.FillWidth(0.05f)
					[
						ConstructCheckBox(AssetDataToDisplay)
					]

				+SHorizontalBox::Slot()
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					.FillWidth(0.55f)
					[
						ConstructTextForRowWidget(DisplayAssetClassName, AssetClassNameFont)
					]
					

				+SHorizontalBox::Slot()
					.HAlign(HAlign_Left)
					.VAlign(VAlign_Center)
					[
						ConstructTextForRowWidget(DisplayAssetName, AssetNameFont)
					]
				+SHorizontalBox::Slot()
					.HAlign(HAlign_Right)
					.VAlign(VAlign_Fill)
					[
						ConstructButtonForRowWidget(AssetDataToDisplay)
					]
		];

	return TableRow;
}

TSharedRef<SListView<TSharedPtr<FAssetData>>> SAdvanceDeletionWidget::ConstructAssetListView()
{
	ConstructedAssetListView = SNew(SListView<TSharedPtr<FAssetData>>)
		.ListItemsSource(&DisplayedAssetsData)
		.OnGenerateRow(this, &SAdvanceDeletionWidget::OnGenerateRowForList)
		.OnMouseButtonDoubleClick(this, &SAdvanceDeletionWidget::OnRowClicked);

	return ConstructedAssetListView.ToSharedRef();
}

TSharedRef<SCheckBox> SAdvanceDeletionWidget::ConstructCheckBox(const TSharedPtr<FAssetData> AssetDataToDisplay)
{
	TSharedRef<SCheckBox> CheckBoxRef = SNew(SCheckBox)
		.Type(ESlateCheckBoxType::CheckBox)
		.OnCheckStateChanged(this, &SAdvanceDeletionWidget::OnCheckBoxStateChange, AssetDataToDisplay)
		.Visibility(EVisibility::Visible);

	CheckBoxesArray.Add(CheckBoxRef);
	return CheckBoxRef;
}

TSharedRef<STextBlock> SAdvanceDeletionWidget::ConstructTextForRowWidget(
	const FString& TextContent,
	const FSlateFontInfo& FontToUse
)
{
	return SNew(STextBlock)
		.Text(FText::FromString(TextContent))
		.Font(FontToUse)
		.ColorAndOpacity(FColor::White);
}

TSharedRef<SButton> SAdvanceDeletionWidget::ConstructButtonForRowWidget(const TSharedPtr<FAssetData>& AssetDataToDisply)
{
	return SNew(SButton)
		.Text(FText::FromString("Delete"))
		.OnClicked_Lambda([this, AssetDataToDisply]()
			{
				return OnDeleteButtonClicked(AssetDataToDisply);
			}
		);
}

TSharedRef<SButton> SAdvanceDeletionWidget::ConstructTabButton()
{
	return SNew(SButton)
		.ContentPadding(FMargin(5.f));
}

TSharedRef<STextBlock> SAdvanceDeletionWidget::ConstructTextForTabButtons(const FString& TextContent)
{
	FSlateFontInfo Buttontextfont = GetEmbossedTextFont();
	Buttontextfont.Size = 15.0f;
	return SNew(STextBlock)
		.Text(FText::FromString(TextContent))
		.Font(Buttontextfont)
		.Justification(ETextJustify::Center);
}

TSharedRef<SButton> SAdvanceDeletionWidget::ConstructDeleteAllButton()
{
	TSharedRef<SButton> DeleteAllButton = ConstructTabButton();
	DeleteAllButton->SetOnClicked(FOnClicked::CreateLambda([this]() { return OnDeleteAllButtonClicked(); }));
	DeleteAllButton->SetContent(ConstructTextForTabButtons(TEXT("Delete")));
	return DeleteAllButton;
}

TSharedRef<SButton> SAdvanceDeletionWidget::ConstructSelectAllButton()
{
	TSharedRef<SButton> SelectAllButton = ConstructTabButton();
	SelectAllButton->SetOnClicked(FOnClicked::CreateLambda([this]() { return OnSelectAllButtonClicked(); }));
	SelectAllButton->SetContent(ConstructTextForTabButtons(TEXT("Select All")));
	return SelectAllButton;
}

TSharedRef<SButton> SAdvanceDeletionWidget::ConstructDeselectAllButton()
{
	TSharedRef<SButton> DeselectAllButton = ConstructTabButton();
	DeselectAllButton->SetOnClicked(FOnClicked::CreateLambda([this]() { return OnDeselectAllButtonClicked(); }));
	DeselectAllButton->SetContent(ConstructTextForTabButtons(TEXT("Deselect All")));
	return DeselectAllButton;
}

TSharedRef<SComboBox<TSharedPtr<FString>>> SAdvanceDeletionWidget::ConstructComboBox()
{
	TSharedRef<SComboBox<TSharedPtr<FString>>> ConstructedComboBox =
		SNew(SComboBox<TSharedPtr<FString>>)
		.OptionsSource(&ComboBoxSourceItems)
		.OnGenerateWidget(this, &SAdvanceDeletionWidget::OnGenerateComboContent)
		.OnSelectionChanged(this, &SAdvanceDeletionWidget::OnComboSelectionChanged)
		[
			SAssignNew(ComboDisplayTextBlock, STextBlock)
				.Text(FText::FromString(TEXT("List Assets Options")))
		];
	return ConstructedComboBox;
}

TSharedRef<STextBlock> SAdvanceDeletionWidget::ConstructHelpTextBlock(const FString& HelpText, ETextJustify::Type TextJustify)
{
	return SNew(STextBlock)
		.Text(FText::FromString(HelpText))
		.Justification(TextJustify)
		.AutoWrapText(true)
		;
}

void SAdvanceDeletionWidget::OnCheckBoxStateChange(ECheckBoxState NewState, TSharedPtr<FAssetData> AssetData)
{
	switch (NewState)
	{
	case ECheckBoxState::Unchecked:
		if (AssetsDataToDeleteArray.Contains(AssetData))
		{
			AssetsDataToDeleteArray.Remove(AssetData);
		}
		break;
	case ECheckBoxState::Checked:
		AssetsDataToDeleteArray.AddUnique(AssetData);
		break;
	case ECheckBoxState::Undetermined:
		break;
	default:
		break;
	}
}

void SAdvanceDeletionWidget::OnRowClicked(TSharedPtr<FAssetData> AssetData)
{
	TArray<FString> AssetPath;
	AssetPath.Add(AssetData->GetObjectPathString());
	UEditorAssetLibrary::SyncBrowserToObjects(AssetPath);
}

FReply SAdvanceDeletionWidget::OnDeleteButtonClicked(const TSharedPtr<FAssetData>& AssetDataToDisply)
{
	FSuperManagerModule& SuperManager = FModuleManager::LoadModuleChecked<FSuperManagerModule>(TEXT("SuperManager"));
	SuperManager.FixUpRedirectors();
	if (SuperManager.DeleteSingleAssetForAssetList(*AssetDataToDisply))
	{
		StoredAssetsData.Remove(AssetDataToDisply);
		DisplayedAssetsData.Remove(AssetDataToDisply);
		RefreshAssetListView();
	}
	return FReply::Handled();
}

FReply SAdvanceDeletionWidget::OnDeleteAllButtonClicked()
{
	if (AssetsDataToDeleteArray.Num() == 0)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("No asset currently selected"));
	}
	else
	{
		FSuperManagerModule& SuperManager = FModuleManager::LoadModuleChecked<FSuperManagerModule>(TEXT("SuperManager"));
		SuperManager.FixUpRedirectors();

		TArray<FAssetData> AssetDataToDelete;
		for (TSharedPtr<FAssetData> AssetRef : AssetsDataToDeleteArray)
		{
			AssetDataToDelete.Add(*AssetRef);
		}

		if (SuperManager.DeleteMultipleAssetsForAssetList(AssetDataToDelete))
		{
			for (const TSharedPtr<FAssetData>& DeletedAsset : AssetsDataToDeleteArray)
			{
				StoredAssetsData.Remove(DeletedAsset);
				DisplayedAssetsData.Remove(DeletedAsset);
			}
			RefreshAssetListView();
		}
	}
	return FReply::Handled();
}

FReply SAdvanceDeletionWidget::OnSelectAllButtonClicked()
{
	for (const TSharedRef<SCheckBox>& CheckBox : CheckBoxesArray)
	{
		if (!CheckBox->IsChecked())
		{
			CheckBox->ToggleCheckedState();
		}
	}
	return FReply::Handled();
}

FReply SAdvanceDeletionWidget::OnDeselectAllButtonClicked()
{
	for (const TSharedRef<SCheckBox>& CheckBox : CheckBoxesArray)
	{
		if (CheckBox->IsChecked())
		{
			CheckBox->ToggleCheckedState();
		}
	}
	return FReply::Handled();
}

TSharedRef<SWidget> SAdvanceDeletionWidget::OnGenerateComboContent(TSharedPtr<FString> SourceItem)
{
	return SNew(STextBlock)
		.Text(FText::FromString(*SourceItem.Get()));
}

void SAdvanceDeletionWidget::OnComboSelectionChanged(TSharedPtr<FString> SelectedOption, ESelectInfo::Type InSelectInfo)
{
	ComboDisplayTextBlock->SetText(FText::FromString(*SelectedOption.Get()));

	FSuperManagerModule& SuperManager = FModuleManager::LoadModuleChecked<FSuperManagerModule>(TEXT("SuperManager"));
	if (*SelectedOption == LIST_ALL)
	{
		DisplayedAssetsData = StoredAssetsData;
	}
	else if (*SelectedOption == LIST_UNUSED)
	{
		SuperManager.ListUnusedAssetsForAssetList(StoredAssetsData, DisplayedAssetsData);
	}
	else if (*SelectedOption == LIST_SAME_NAME)
	{
		SuperManager.ListSameNameAssetsForAssetList(StoredAssetsData, DisplayedAssetsData);
	}

	RefreshAssetListView();
}

void SAdvanceDeletionWidget::RefreshAssetListView()
{
	AssetsDataToDeleteArray.Empty();
	CheckBoxesArray.Empty();
	if (ConstructedAssetListView.IsValid())
	{
		ConstructedAssetListView->RebuildList();
	}
}
