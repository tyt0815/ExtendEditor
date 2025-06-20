// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
#include "QuickMaterialCreationWidget.generated.h"

class UMaterialExpressionTextureSample;

UENUM(BlueprintType)
enum class EChannelPackingType : uint8
{
	ECPT_NoChannelPacking UMETA (DisplayName = "No Channel Packing"),
	ECPT_ORM UMETA (DisplayName = "OcclusionRoughnessMetallic"),
	ECPT_MAX UMETA (DisplayName = "DefaultMAX")
};

UCLASS()
class SUPERMANAGER_API UQuickMaterialCreationWidget : public UEditorUtilityWidget
{
	GENERATED_BODY()

#pragma region QuickMaterialCreationCore

public:
	UFUNCTION(BlueprintCallable)
	void CreateMaterialFromSelectedTextures();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CreateMaterialFromSelectedTextures")
	EChannelPackingType ChannelPackingType = EChannelPackingType::ECPT_NoChannelPacking;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CreateMaterialFromSelectedTextures")
	bool bCustomMaterialName = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CreateMaterialFromSelectedTextures")
	bool bCreateMaterialInstance = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CreateMaterialFromSelectedTextures", meta = (EditCondition = "bCustomMaterialName"))
	FString MaterialName = TEXT("M_");

#pragma endregion

#pragma region QuickMaterialCreation
private:
	bool ProcessSelectedData(
		const TArray<FAssetData>& SelectedDataToProccess,
		TArray<UTexture2D*>& OutSelectedTextures,
		FString& OutSelectedTexturePackagePath
	);
	bool CheckIsNameUsed(const FString& FolderPathToCheck,const FString& MaterialNameToCheck);
	UMaterial* CreateMaterialAsset(const FString& NameOfTheMaterial, const FString PackagePath);
	UMaterialInstanceConstant* CreateMaterialInstanceConstant(const FString& NameOfTheMaterialInst, const FString PackagePath);
	void Default_CreateMaterialNode(UMaterial* Material, UTexture2D* Texture, uint32& PinsConnectedCounter);
	void ORM_CreateMaterialNode(UMaterial* Material, UTexture2D* Texture, uint32& PinsConnectedCounter);
	bool TryConnectBaseColor(UTexture2D* Texture, UMaterial* Material);
	bool TryConnectMetallic(UTexture2D* Texture, UMaterial* Material);
	bool TryConnectRoughness(UTexture2D* Texture, UMaterial* Material);
	bool TryConnectNormal(UTexture2D* Texture, UMaterial* Material);
	bool TryConnectAO(UTexture2D* Texture, UMaterial* Material);
	bool TryConnectORM(UTexture2D* Texture, UMaterial* Material);

#pragma endregion

#pragma region SupportedTextureNames
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Supported Texture Names")
	TArray<FString> BaseColorArray = {
		TEXT("_BaseColor"),
		TEXT("_Albedo"),
		TEXT("_Diffuse"),
		TEXT("_diff")
	};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Supported Texture Names")
	TArray<FString> MetallicArray = {
		TEXT("_Metallic"),
		TEXT("_metal")
	};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Supported Texture Names")
	TArray<FString> RoughnessArray = {
		TEXT("_Roughness"),
		TEXT("_RoughnessMap"),
		TEXT("_rough")
	};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Supported Texture Names")
	TArray<FString> NormalArray = {
		TEXT("_Normal"),
		TEXT("_NormalMap"),
		TEXT("_nor")
	};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Supported Texture Names")
	TArray<FString> AmbientOcclusionArray = {
		TEXT("_AmbientOcclusion"),
		TEXT("_AmbientOcclusionMap"),
		TEXT("_AO")
	};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Supported Texture Names")
	TArray<FString> ORMArray = {
		TEXT("_ORM"),
		TEXT("_ARM"),
		TEXT("_OcclusionRoughnessMetallic")
	};

#pragma endregion

};
