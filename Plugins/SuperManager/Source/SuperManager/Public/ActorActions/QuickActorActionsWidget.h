// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
#include "QuickActorActionsWidget.generated.h"

class UEditorActorSubsystem;

UENUM(BlueprintType)
enum class EDuplicationAxis : uint8
{
	EDA_XAxis UMETA(DisplayName = "X Axis"),
	EDA_YAxis UMETA(DisplayName = "Y Axis"),
	EDA_ZAxis UMETA(DisplayName = "Z Axis"),
	EDA_MAX UMETA(DisplayName = "Default Max"),
};

USTRUCT()
struct FRandomRotationDetails
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	bool bRotate = false;

	UPROPERTY(EditAnywhere, meta = (EditCondition = "bRotate"))
	float Min = -180;

	UPROPERTY(EditAnywhere, meta = (EditCondition = "bRotate"))
	float Max = 180;
};

UCLASS()
class SUPERMANAGER_API UQuickActorActionsWidget : public UEditorUtilityWidget
{
	GENERATED_BODY()

public:
#pragma region ActorBatchSelection
	UFUNCTION(BlueprintCallable)
	void SelectAllActorsWithSimilarName();

	UPROPERTY(EditAnywhere, Category = "Actor Batch Selection")
	TEnumAsByte<ESearchCase::Type> SearchCase = ESearchCase::IgnoreCase;

#pragma endregion

#pragma region ActorBatchDuplication
	UFUNCTION(BlueprintCallable)
	void DuplicateActors();

	UPROPERTY(EditAnywhere, Category = "Actor Batch Duplication")
	EDuplicationAxis DuplicationAxis = EDuplicationAxis::EDA_XAxis;

	UPROPERTY(EditAnywhere, Category = "Actor Batch Duplication", meta = (ClampMin = "1"))
	uint32 NumberOfDuplication = 1;

	UPROPERTY(EditAnywhere, Category = "Actor Batch Duplication")
	float OffsetDest = 100.0f;
#pragma endregion

#pragma region ActorBatchRandomTransform
	UFUNCTION(BlueprintCallable)
	void RandomizeActorTransform();

	UPROPERTY(EditAnywhere, Category = "Actor Batch Random Transform")
	FRandomRotationDetails RandomRotationYaw;

	UPROPERTY(EditAnywhere, Category = "Actor Batch Random Transform")
	FRandomRotationDetails RandomRotationPitch;

	UPROPERTY(EditAnywhere, Category = "Actor Batch Random Transform")
	FRandomRotationDetails RandomRotationRoll;

	UPROPERTY(EditAnywhere, Category = "Actor Batch Random Transform")
	bool bRandomizeScale = false;

	UPROPERTY(EditAnywhere, Category = "Actor Batch Random Transform", meta = (EditCondition = "bRandomizeScale"), meta = (ClampMin = 0.0f))
	float ScaleMin = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Actor Batch Random Transform", meta = (EditCondition = "bRandomizeScale"), meta = (ClampMin = 0.0f))
	float ScaleMax = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Actor Batch Random Transform")
	bool bRandomizeOffset = false;

	UPROPERTY(EditAnywhere, Category = "Actor Batch Random Transform", meta = (EditCondition = "bRandomizeOffset"))
	float OffsetMin = 0.0f;
	
	UPROPERTY(EditAnywhere, Category = "Actor Batch Random Transform", meta = (EditCondition = "bRandomizeOffset"))
	float OffsetMax = 0.0f;
#pragma endregion

private:
	UEditorActorSubsystem* EditorActorSubsystem;

	bool GetEditorActorSubsystem();
	FString CleanActorLabel(const FString& ActorLabel);
};
