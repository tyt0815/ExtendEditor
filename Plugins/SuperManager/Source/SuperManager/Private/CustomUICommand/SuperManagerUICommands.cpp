// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomUICommand/SuperManagerUICommands.h"

#define LOCTEXT_NAMESPACE "FSuperManagerModule"

FSuperManagerUICommands::FSuperManagerUICommands():
	TCommands<FSuperManagerUICommands>(
		TEXT("SuperManager"),
		FText::FromString(TEXT("Super Manager UI Commands")),
		NAME_None,
		TEXT("SuperManager")
	)
{
}

void FSuperManagerUICommands::RegisterCommands()
{
	UI_COMMAND(
		LockActorSelection,
		"Lock Actor Selection",
		"Lock actor selection in level. Once triggered, actor can no longer be selected.",
		EUserInterfaceActionType::Button,
		FInputChord(EKeys::W, EModifierKey::Alt)
	);

	UI_COMMAND(
		UnlockActorSelection,
		"Unlock All Actor Selection",
		"Remove selection lock on all actors",
		EUserInterfaceActionType::Button,
		FInputChord(EKeys::W, EModifierKey::Alt | EModifierKey::Shift)
	);
}

#undef LOCTEXT_NAMESPACE