// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "Styling/SlateStyle.h"

class FSuperManagerStyle
{
public:
	static void InitializeIcons();
	static void ShutDown();

private:
	static TSharedRef<FSlateStyleSet> CreateSlateStyleSet();

	static TSharedPtr<FSlateStyleSet> CreatedSlateStyleSet;
	static FName StyleSetName;

public:
	static FName GetStyleSetName()
	{
		return StyleSetName;
	}
};