// Copyright Epic Games, Inc. All Rights Reserved.

// 先包含自己的 .h（里面声明了全部静态函数）
#include "TenkaichiSystemStatics.h"

// FPrimaryAssetId / FPrimaryAssetTypeInfo 的类型定义
#include "Engine/AssetManagerTypes.h"

// GEngine 全局引擎单例、世界上下文相关
#include "Engine/Engine.h"

// UWorld、ServerTravel 无缝切换关卡
#include "Engine/World.h"

// UAssetManager：资产管理器（按资产 ID 拿路径、类型信息）
#include "Engine/AssetManager.h"

// 自定义日志通道 LogLyra（第 0 步已教）
#include "TenkaichiLogChannels.h"

// UMeshComponent：网格组件（批量改材质参数用）
#include "Components/MeshComponent.h"

#if 0 // ⚠️ 挂起：依赖 P13 Experience，P13 就位后解开（登记注释备忘录⑭）
// 玩家可见的"体验"定义类（P13 GameModes 才教，暂注释挂起）
#include "GameModes/TenkaichiUserFacingExperienceDefinition.h"
#endif

// UHT 生成的 .cpp 实现（对应 TenkaichiSystemStatics.generated.h）
#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiSystemStatics)

// 按资产 ID 返回软对象引用（资产未加载也能拿到引用）
TSoftObjectPtr<UObject> UTenkaichiSystemStatics::GetTypedSoftObjectReferenceFromPrimaryAssetId(FPrimaryAssetId PrimaryAssetId, TSubclassOf<UObject> ExpectedAssetType)
{
	if (UAssetManager* Manager = UAssetManager::GetIfInitialized())
	{
		FPrimaryAssetTypeInfo Info;
		if (Manager->GetPrimaryAssetTypeInfo(PrimaryAssetId.PrimaryAssetType, Info) && !Info.bHasBlueprintClasses)
		{
			if (UClass* AssetClass = Info.AssetBaseClassLoaded)
			{
				if ((ExpectedAssetType == nullptr) || !AssetClass->IsChildOf(ExpectedAssetType))
				{
					return nullptr;
				}
			}
			else
			{
				UE_LOG(LogLyra, Warning, TEXT("GetTypedSoftObjectReferenceFromPrimaryAssetId(%s, %s) - AssetBaseClassLoaded was unset so we couldn't validate it, returning null"),
					*PrimaryAssetId.ToString(),
					*GetPathNameSafe(*ExpectedAssetType));
			}

			return TSoftObjectPtr<UObject>(Manager->GetPrimaryAssetPath(PrimaryAssetId));
		}
	}
	return nullptr;
}

#if 0 // ⚠️ 挂起：依赖 P13 Experience，P13 就位后解开（登记注释备忘录⑭）
// 按"对外体验名"返回资产 ID（体验名 → 主资产 ID）
FPrimaryAssetId UTenkaichiSystemStatics::GetPrimaryAssetIdFromUserFacingExperienceName(const FString& AdvertisedExperienceID)
{
	const FPrimaryAssetType Type(ULyraUserFacingExperienceDefinition::StaticClass()->GetFName());
	return FPrimaryAssetId(Type, FName(*AdvertisedExperienceID));
}
#endif

// 服务器端：无缝切换到下一局
void UTenkaichiSystemStatics::PlayNextGame(const UObject* WorldContextObject)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (World == nullptr)
	{
		return;
	}

	const FWorldContext& WorldContext = GEngine->GetWorldContextFromWorldChecked(World);
	FURL LastURL = WorldContext.LastURL;

#if WITH_EDITOR
	// 编辑器里做 PIE 切图时要剥掉 PIE 前缀，否则会切到错误的地图
	LastURL.Map = UWorld::StripPIEPrefixFromPackageName(LastURL.Map, WorldContext.World()->StreamingLevelsPrefix);
#endif

	// 加上"无缝切换"选项，让客户端保持连接（若禁用了无缝，会退回硬切换）
	LastURL.AddOption(TEXT("SeamlessTravel"));

	FString URL = LastURL.ToString();
	// 不把 host/port 信息带上的话服务器切换会失败
	URL.RemoveFromStart(LastURL.GetHostPortString());

	const bool bAbsolute = false; // 用相对切换 TRAVEL_Relative
	const bool bShouldSkipGameNotify = false;
	World->ServerTravel(URL, bAbsolute, bShouldSkipGameNotify);
}

// 把标量材质参数批量设置到 TargetActor 的所有 Mesh 组件
void UTenkaichiSystemStatics::SetScalarParameterValueOnAllMeshComponents(AActor* TargetActor, const FName ParameterName, const float ParameterValue, bool bIncludeChildActors)
{
	if (TargetActor != nullptr)
	{
		TargetActor->ForEachComponent<UMeshComponent>(bIncludeChildActors, [=](UMeshComponent* InComponent)
		{
			InComponent->SetScalarParameterValueOnMaterials(ParameterName, ParameterValue);
		});
	}
}

// 把向量材质参数批量设置到 TargetActor 的所有 Mesh 组件
void UTenkaichiSystemStatics::SetVectorParameterValueOnAllMeshComponents(AActor* TargetActor, const FName ParameterName, const FVector ParameterValue, bool bIncludeChildActors)
{
	if (TargetActor != nullptr)
	{
		TargetActor->ForEachComponent<UMeshComponent>(bIncludeChildActors, [=](UMeshComponent* InComponent)
		{
			InComponent->SetVectorParameterValueOnMaterials(ParameterName, ParameterValue);
		});
	}
}

// 把颜色材质参数批量设置到 TargetActor 的所有 Mesh 组件（颜色转成向量后转调向量版）
void UTenkaichiSystemStatics::SetColorParameterValueOnAllMeshComponents(AActor* TargetActor, const FName ParameterName, const FLinearColor ParameterValue, bool bIncludeChildActors)
{
	SetVectorParameterValueOnAllMeshComponents(TargetActor, ParameterName, FVector(ParameterValue), bIncludeChildActors);
}

// 按类型获取 TargetActor 上（含子 Actor）的所有组件
TArray<UActorComponent*> UTenkaichiSystemStatics::FindComponentsByClass(AActor* TargetActor, TSubclassOf<UActorComponent> ComponentClass, bool bIncludeChildActors)
{
	TArray<UActorComponent*> Components;
	if (TargetActor != nullptr)
	{
		TargetActor->GetComponents(ComponentClass, /*out*/ Components, bIncludeChildActors);

	}
	return MoveTemp(Components);
}