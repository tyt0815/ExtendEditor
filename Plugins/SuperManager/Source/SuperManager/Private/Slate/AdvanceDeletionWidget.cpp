// Fill out your copyright notice in the Description page of Project Settings.


#include "Slate/AdvanceDeletionWidget.h"

void SAdvanceDeletionWidget::Construct(const FArguments& InArgs)
{
	bCanSupportFocus = true;

	AssetsDataUnderSelectedFolderArray = InArgs._AssetsDataArray;
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
			.AutoHeight()
			[
				SNew(SScrollBox)
			]

			//for 3 buttons
			+SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)
			]
	];
}