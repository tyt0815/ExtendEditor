// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorActions/QuickActorActionsWidget.h"
#include "Subsystems/EditorActorSubsystem.h"
#include "DebugHeader.h"

void UQuickActorActionsWidget::SelectAllActorsWithSimilarName()
{
	if (!GetEditorActorSubsystem())
	{
		return;
	}

	TArray<AActor*> SelectedActors = EditorActorSubsystem->GetSelectedLevelActors();
	if (SelectedActors.Num() == 0)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("액터를 선택해 주세요."));
		return;
	}
	else if (SelectedActors.Num() > 1)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("하나의 액터만 선택해 주세요."));
		return;
	}

	FString NameToSearch = SelectedActors[0]->GetActorLabel();
	//NameToSearch = NameToSearch.LeftChop(4); 
	NameToSearch = CleanActorLabel(NameToSearch);

	DebugHeader::Print(NameToSearch, FColor::Emerald);

	TArray<AActor*> AllLevelActors = EditorActorSubsystem->GetAllLevelActors();
	for (AActor* LevelActor : AllLevelActors)
	{
		if (!LevelActor)
		{
			continue;
		}

		if (LevelActor->GetActorLabel().Contains(NameToSearch, SearchCase))
		{
			EditorActorSubsystem->SetActorSelectionState(LevelActor, true);
		}
	}
}

void UQuickActorActionsWidget::DuplicateActors()
{
	if (!GetEditorActorSubsystem())
	{
		return;
	}

	TArray<AActor*> SelectedActors = EditorActorSubsystem->GetSelectedLevelActors();
	if (SelectedActors.Num() == 0)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("액터를 선택해 주세요."));
		return;
	}

	for (AActor* SelectedActor : SelectedActors)
	{
		if (!SelectedActor)
		{
			return;
		}
		uint32 Counter = 0;
		for (uint32 i = 1; i <= NumberOfDuplication; ++i)
		{
			FVector Offset = FVector::ZeroVector;
			switch (DuplicationAxis)
			{
			case EDuplicationAxis::EDA_XAxis:
				Offset.X += i * OffsetDest;
				break;
			case EDuplicationAxis::EDA_YAxis:
				Offset.Y += i * OffsetDest;
				break;
			case EDuplicationAxis::EDA_ZAxis:
				Offset.Z += i * OffsetDest;
				break;
			case EDuplicationAxis::EDA_MAX:
				break;
			default:
				break;
			}

			AActor* DuplicatedActor = EditorActorSubsystem->DuplicateActor(
				SelectedActor,
				GetWorld(),
				Offset
			);
			if (DuplicatedActor)
			{
				EditorActorSubsystem->SetActorSelectionState(DuplicatedActor, true);
				++Counter;
			}
		}

		DebugHeader::Print(
			SelectedActor->GetActorLabel() +
				TEXT(" 액터를 ") +
				FString::FromInt(Counter) +
				TEXT("개 복사하였습니다."),
			FColor::Green
		);
	}
}

void UQuickActorActionsWidget::RandomizeActorTransform()
{
	if (!GetEditorActorSubsystem())
	{
		return;
	}

	TArray<AActor*> SelectedActors = EditorActorSubsystem->GetSelectedLevelActors();
	if (SelectedActors.Num() == 0)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("액터를 선택해 주세요."));
		return;
	}

	for (AActor* SelectedActor : SelectedActors)
	{
		if (!SelectedActor)
		{
			continue;
		}
		
		FRotator RandomizedRotator = FRotator::ZeroRotator;
		if (RandomRotationYaw.bRotate)
		{
			RandomizedRotator.Yaw += FMath::RandRange(RandomRotationYaw.Min, RandomRotationYaw.Max);
		}
		if (RandomRotationPitch.bRotate)
		{
			RandomizedRotator.Pitch += FMath::RandRange(RandomRotationPitch.Min, RandomRotationPitch.Max);
		}
		if (RandomRotationRoll.bRotate)
		{
			RandomizedRotator.Roll += FMath::RandRange(RandomRotationRoll.Min, RandomRotationRoll.Max);
		}
		SelectedActor->AddActorWorldRotation(RandomizedRotator);

		if (bRandomizeScale)
		{
			float RandomValue = FMath::RandRange(ScaleMin, ScaleMax);
			SelectedActor->SetActorScale3D(FVector(RandomValue));
		}

		if (bRandomizeOffset)
		{
			SelectedActor->AddActorWorldOffset(
				FVector(
					FMath::RandRange(OffsetMin, OffsetMax),
					FMath::RandRange(OffsetMin, OffsetMax),
					0.0f
				)
			);
		}
	}
}

bool UQuickActorActionsWidget::GetEditorActorSubsystem()
{
	if (!EditorActorSubsystem)
	{
		EditorActorSubsystem = GEditor->GetEditorSubsystem<UEditorActorSubsystem>();
	}
	return EditorActorSubsystem != nullptr;
}

FString UQuickActorActionsWidget::CleanActorLabel(const FString& ActorLabel)
{
	// 끝에 숫자만 붙어 있는 경우 제거
	FRegexPattern Pattern(TEXT("^(.*?)(\\d+)$"));  // non-greedy로 문자열 + 숫자
	FRegexMatcher Matcher(Pattern, ActorLabel);

	if (Matcher.FindNext())
	{
		return Matcher.GetCaptureGroup(1);  // 숫자 앞 부분만 반환
	}

	return ActorLabel;
}
