// Fill out your copyright notice in the Description page of Project Settings.


#include "AssetActions/QuickMaterialCreationWidget.h"
#include "EditorUtilityLibrary.h"
#include "EditorAssetLibrary.h"
#include "AssetToolsModule.h"
#include "Factories/MaterialFactoryNew.h"
#include "Factories/MaterialInstanceConstantFactoryNew.h"
#include "Materials/MaterialExpressionTextureSample.h"
#include "Materials/MaterialInstanceConstant.h"
#include "DebugHeader.h"

void UQuickMaterialCreationWidget::CreateMaterialFromSelectedTextures()
{
	if (MaterialName.IsEmpty())
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("올바른 Material 이름을 입력해 주세요."), false);
	}

	TArray<FAssetData> SelectedAssetsData = UEditorUtilityLibrary::GetSelectedAssetData();
	TArray<UTexture2D*> SelectedTextures;
	FString SelectedTexturePackagePath;
	if (!ProcessSelectedData(SelectedAssetsData, SelectedTextures, SelectedTexturePackagePath))
	{
		return;
	}
	if (CheckIsNameUsed(SelectedTexturePackagePath, MaterialName))
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, MaterialName + TEXT("은(는) 이미 존재하는 이름입니다."), true);
		return;
	}

	UMaterial* CreatedMaterial = CreateMaterialAsset(MaterialName, SelectedTexturePackagePath);
	if (CreatedMaterial == nullptr)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("생성 실패!"));
		return;
	}

	uint32 PinsConnectedCounter = 0;
	for (UTexture2D* SelectedTexture : SelectedTextures)
	{
		if (!SelectedTexture)
		{
			continue;	
		}
		switch (ChannelPackingType)
		{
		case EChannelPackingType::ECPT_NoChannelPacking:
			Default_CreateMaterialNode(CreatedMaterial, SelectedTexture, PinsConnectedCounter);
			break;
		case EChannelPackingType::ECPT_ORM:
			ORM_CreateMaterialNode(CreatedMaterial, SelectedTexture, PinsConnectedCounter);
			break;
		case EChannelPackingType::ECPT_MAX:
			break;
		default:
			break;
		}
	}

	DebugHeader::ShowNotifyInfo(FString::FromInt(PinsConnectedCounter) + TEXT(" 핀 연결 완료"));

	if (bCreateMaterialInstance)
	{
		FString MaterialInstName = MaterialName;
		MaterialInstName.RemoveFromStart(TEXT("M_"));
		MaterialInstName.InsertAt(0, TEXT("MI_"));
		UMaterialInstanceConstant* CreatedMaterialInst = CreateMaterialInstanceConstant(
			MaterialInstName,
			SelectedTexturePackagePath
		);
		CreatedMaterialInst->SetParentEditorOnly(CreatedMaterial);
		CreatedMaterialInst->PostEditChange();
	}

	MaterialName = TEXT("M_");
}

bool UQuickMaterialCreationWidget::ProcessSelectedData(
	const TArray<FAssetData>& SelectedDataToProccess,
	TArray<UTexture2D*>& OutSelectedTextures,
	FString& OutSelectedTexturePackagePath
)
{
	OutSelectedTextures.Empty();
	OutSelectedTexturePackagePath.Empty();

	for (const FAssetData& SelectedAsset : SelectedDataToProccess)
	{
		UTexture2D* SelectedTexture = Cast<UTexture2D>(SelectedAsset.GetAsset());
		if (SelectedTexture)
		{
			OutSelectedTextures.Add(SelectedTexture);
			if (OutSelectedTexturePackagePath.IsEmpty())
			{
				OutSelectedTexturePackagePath = SelectedAsset.PackagePath.ToString();
				if (!bCustomMaterialName)
				{
					MaterialName = SelectedAsset.AssetName.ToString();
					MaterialName.RemoveFromStart(TEXT("T_"));
					MaterialName.InsertAt(0, TEXT("M_"));
				}
			}
		}
		else
		{
			DebugHeader::Print(SelectedAsset.AssetName.ToString() + TEXT(" 은(는) Texture2D가 아닙니다."), FColor::Orange);
		}
	}

	if (OutSelectedTextures.IsEmpty())
	{
		return false;
	}

	return true;
}

bool UQuickMaterialCreationWidget::CheckIsNameUsed(const FString& FolderPathToCheck, const FString& MaterialNameToCheck)
{
	return UEditorAssetLibrary::DoesAssetExist(FolderPathToCheck / MaterialNameToCheck);
}

UMaterial* UQuickMaterialCreationWidget::CreateMaterialAsset(const FString& NameOfTheMaterial, const FString PackagePath)
{
	UMaterialFactoryNew* MaterialFactory = NewObject<UMaterialFactoryNew>();
	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
	UObject* CreatedObject = AssetToolsModule.Get().CreateAsset(NameOfTheMaterial, PackagePath, UMaterial::StaticClass(), MaterialFactory);
	UMaterial* CreatedMaterial = nullptr;
	CreatedMaterial = Cast<UMaterial>(CreatedObject);
	return CreatedMaterial;
}

UMaterialInstanceConstant* UQuickMaterialCreationWidget::CreateMaterialInstanceConstant(const FString& NameOfTheMaterialInst, const FString PackagePath)
{
	UMaterialInstanceConstantFactoryNew* MaterialInstFactory = NewObject< UMaterialInstanceConstantFactoryNew>();
	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
	UObject* CreatedObject = AssetToolsModule.Get().CreateAsset(NameOfTheMaterialInst, PackagePath, UMaterialInstanceConstant::StaticClass(), MaterialInstFactory);
	UMaterialInstanceConstant* CreatedMaterialInst = nullptr;
	CreatedMaterialInst = Cast<UMaterialInstanceConstant>(CreatedObject);
	return CreatedMaterialInst;
}

void UQuickMaterialCreationWidget::Default_CreateMaterialNode(UMaterial* Material, UTexture2D* Texture, uint32& PinsConnectedCounter)
{

	if (!Material->HasBaseColorConnected())
	{
		if (TryConnectBaseColor(Texture, Material))
		{
			PinsConnectedCounter++;
			return;
		}
	}
	if (!Material->HasMetallicConnected())
	{
		if (TryConnectMetallic(Texture, Material))
		{
			PinsConnectedCounter++;
			return;
		}
	}
	if (!Material->HasRoughnessConnected())
	{
		if (TryConnectRoughness(Texture, Material))
		{
			PinsConnectedCounter++;
			return;
		}
	}
	if (!Material->HasNormalConnected())
	{
		if (TryConnectNormal(Texture, Material))
		{
			PinsConnectedCounter++;
			return;
		}
	}
	if (!Material->HasAmbientOcclusionConnected())
	{
		if (TryConnectAO(Texture, Material))
		{
			PinsConnectedCounter++;
			return;
		}
	}

}

void UQuickMaterialCreationWidget::ORM_CreateMaterialNode(UMaterial* Material, UTexture2D* Texture, uint32& PinsConnectedCounter)
{
	if (!Material->HasBaseColorConnected())
	{
		if (TryConnectBaseColor(Texture, Material))
		{
			PinsConnectedCounter++;
			return;
		}
	}
	if (!Material->HasNormalConnected())
	{
		if (TryConnectNormal(Texture, Material))
		{
			PinsConnectedCounter++;
			return;
		}
	}
	if (!Material->HasRoughnessConnected())
	{
		if (TryConnectORM(Texture, Material))
		{
			PinsConnectedCounter += 3;
			return;
		}
	}
}

bool UQuickMaterialCreationWidget::TryConnectBaseColor(UTexture2D* Texture, UMaterial* Material)
{
	if (!Texture || !Material)
	{
		return false;
	}
	for (const FString& BaseColorName : BaseColorArray)
	{
		if (Texture->GetName().Contains(BaseColorName))
		{
			UMaterialExpressionTextureSample* TextureSampleNode = NewObject<UMaterialExpressionTextureSample>(Material);
			if (!TextureSampleNode)
			{
				return false;
			}
			TextureSampleNode->Texture = Texture;
			UMaterialEditorOnlyData* MaterialEditorOnlyData = Material->GetEditorOnlyData();
			MaterialEditorOnlyData->ExpressionCollection.Expressions.Add(TextureSampleNode);
			MaterialEditorOnlyData->BaseColor.Expression = TextureSampleNode;
			Material->PostEditChange();

			TextureSampleNode->MaterialExpressionEditorX -= 600;
			return true;
		}
	}
	return false;
}

bool UQuickMaterialCreationWidget::TryConnectMetallic(UTexture2D* Texture, UMaterial* Material)
{
	if (!Texture || !Material)
	{
		return false;
	}
	for (const FString& MetallicName : MetallicArray)
	{
		if (Texture->GetName().Contains(MetallicName))
		{
			UMaterialExpressionTextureSample* TextureSampleNode = NewObject<UMaterialExpressionTextureSample>(Material);
			if (!TextureSampleNode)
			{
				return false;
			}
			TextureSampleNode->Texture = Texture;
			TextureSampleNode->SamplerType = EMaterialSamplerType::SAMPLERTYPE_LinearColor;

			Texture->SRGB = false;
			Texture->CompressionSettings = TextureCompressionSettings::TC_Default;
			Texture->PostEditChange();

			UMaterialEditorOnlyData* MaterialEditorOnlyData = Material->GetEditorOnlyData();
			MaterialEditorOnlyData->ExpressionCollection.Expressions.Add(TextureSampleNode);
			MaterialEditorOnlyData->Metallic.Expression = TextureSampleNode;
			Material->PostEditChange();

			TextureSampleNode->MaterialExpressionEditorX -= 600;
			TextureSampleNode->MaterialExpressionEditorY += 240;
			return true;
		}
	}
	return false;
}

bool UQuickMaterialCreationWidget::TryConnectRoughness(UTexture2D* Texture, UMaterial* Material)
{
	if (!Texture || !Material)
	{
		return false;
	}
	for (const FString& RoughnessName : RoughnessArray)
	{
		if (Texture->GetName().Contains(RoughnessName))
		{
			UMaterialExpressionTextureSample* TextureSampleNode = NewObject<UMaterialExpressionTextureSample>(Material);
			if (!TextureSampleNode)
			{
				return false;
			}
			TextureSampleNode->Texture = Texture;
			TextureSampleNode->SamplerType = EMaterialSamplerType::SAMPLERTYPE_LinearColor;

			Texture->SRGB = false;
			Texture->CompressionSettings = TextureCompressionSettings::TC_Default;
			Texture->PostEditChange();

			UMaterialEditorOnlyData* MaterialEditorOnlyData = Material->GetEditorOnlyData();
			MaterialEditorOnlyData->ExpressionCollection.Expressions.Add(TextureSampleNode);
			MaterialEditorOnlyData->Roughness.Expression = TextureSampleNode;
			Material->PostEditChange();

			TextureSampleNode->MaterialExpressionEditorX -= 600;
			TextureSampleNode->MaterialExpressionEditorY += 240 * 2;
			return true;
		}
	}
	return false;
}

bool UQuickMaterialCreationWidget::TryConnectNormal(UTexture2D* Texture, UMaterial* Material)
{
	if (!Texture || !Material)
	{
		return false;
	}
	for (const FString& NormalName : NormalArray)
	{
		if (Texture->GetName().Contains(NormalName))
		{
			UMaterialExpressionTextureSample* TextureSampleNode = NewObject<UMaterialExpressionTextureSample>(Material);
			if (!TextureSampleNode)
			{
				return false;
			}
			TextureSampleNode->Texture = Texture;
			TextureSampleNode->SamplerType = EMaterialSamplerType::SAMPLERTYPE_Normal;

			Texture->SRGB = false;
			Texture->CompressionSettings = TextureCompressionSettings::TC_Normalmap;
			Texture->PostEditChange();

			UMaterialEditorOnlyData* MaterialEditorOnlyData = Material->GetEditorOnlyData();
			MaterialEditorOnlyData->ExpressionCollection.Expressions.Add(TextureSampleNode);
			MaterialEditorOnlyData->Normal.Expression = TextureSampleNode;
			Material->PostEditChange();

			TextureSampleNode->MaterialExpressionEditorX -= 600;
			TextureSampleNode->MaterialExpressionEditorY += 240 * 3;
			return true;
		}
	}
	return false;
}

bool UQuickMaterialCreationWidget::TryConnectAO(UTexture2D* Texture, UMaterial* Material)
{
	if (!Texture || !Material)
	{
		return false;
	}
	for (const FString& AOName : AmbientOcclusionArray)
	{
		if (Texture->GetName().Contains(AOName))
		{
			UMaterialExpressionTextureSample* TextureSampleNode = NewObject<UMaterialExpressionTextureSample>(Material);
			if (!TextureSampleNode)
			{
				return false;
			}
			TextureSampleNode->Texture = Texture;
			TextureSampleNode->SamplerType = EMaterialSamplerType::SAMPLERTYPE_LinearColor;

			Texture->SRGB = false;
			Texture->CompressionSettings = TextureCompressionSettings::TC_Default;
			Texture->PostEditChange();

			UMaterialEditorOnlyData* MaterialEditorOnlyData = Material->GetEditorOnlyData();
			MaterialEditorOnlyData->ExpressionCollection.Expressions.Add(TextureSampleNode);
			MaterialEditorOnlyData->AmbientOcclusion.Expression = TextureSampleNode;
			Material->PostEditChange();

			TextureSampleNode->MaterialExpressionEditorX -= 600;
			TextureSampleNode->MaterialExpressionEditorY += 240 * 4;
			return true;
		}
	}
	return false;
}

bool UQuickMaterialCreationWidget::TryConnectORM(UTexture2D* Texture, UMaterial* Material)
{
	if (!Texture || !Material)
	{
		return false;
	}
	for (const FString& ORMName : ORMArray)
	{
		if (Texture->GetName().Contains(ORMName))
		{
			UMaterialExpressionTextureSample* TextureSampleNode = NewObject<UMaterialExpressionTextureSample>(Material);
			if (!TextureSampleNode)
			{
				return false;
			}
			TextureSampleNode->Texture = Texture;
			TextureSampleNode->SamplerType = EMaterialSamplerType::SAMPLERTYPE_LinearColor;

			Texture->SRGB = false;
			Texture->CompressionSettings = TextureCompressionSettings::TC_Default;
			Texture->PostEditChange();

			UMaterialEditorOnlyData* MaterialEditorOnlyData = Material->GetEditorOnlyData();
			MaterialEditorOnlyData->ExpressionCollection.Expressions.Add(TextureSampleNode);
			MaterialEditorOnlyData->AmbientOcclusion.Connect(1, TextureSampleNode);
			MaterialEditorOnlyData->Roughness.Connect(2, TextureSampleNode);
			MaterialEditorOnlyData->Metallic.Connect(3, TextureSampleNode);
			Material->PostEditChange();

			TextureSampleNode->MaterialExpressionEditorX -= 600;
			TextureSampleNode->MaterialExpressionEditorY += 240 * 1;
			return true;
		}
	}
	return false;

}
