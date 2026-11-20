// Copyright Epic Games, Inc. All Rights Reserved.

// 先包含自己的 .h（里面声明了全部函数）
#include "TenkaichiDevelopmentStatics.h"
// 资产注册表过滤结构
#include "AssetRegistry/ARFilter.h"
// 资产数据
#include "AssetRegistry/AssetData.h"

#if 0 // ⚠️ 挂起：依赖 P18 Settings，P18 就位后解开（登记注释备忘录⑯）
// 开发设置类：组 A 3 个函数从这里读开关（P18 基建/工具的 Settings 类，暂注释挂起）
#include "Development/TenkaichiDeveloperSettings.h"
#endif

// 蓝图资产
#include "Engine/Blueprint.h"
// 引擎单例
#include "Engine/Engine.h"
// 资产注册表模块（扫描所有蓝图）
#include "AssetRegistry/AssetRegistryModule.h"
// 世界
#include "Engine/World.h"
// 包名工具
#include "Misc/PackageName.h"
// 模块管理器
#include "Modules/ModuleManager.h"

// UHT 生成的 .cpp 实现（对应 TenkaichiDevelopmentStatics.generated.h）
#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiDevelopmentStatics)

#if 0 // ⚠️ 挂起：依赖 P18 Settings，P18 就位后解开（登记注释备忘录⑯）

// 是否跳过开场直接进玩法：仅编辑器里且 bTestFullGameFlowInPIE 为假时返回 true
bool UTenkaichiDevelopmentStatics::ShouldSkipDirectlyToGameplay()
{
#if WITH_EDITOR
	if (GIsEditor)
	{
		return !GetDefault<UTenkaichiDeveloperSettings>()->bTestFullGameFlowInPIE;
	}
#endif
	return false;
}

// 是否加载外观背景：仅编辑器里且 bSkipLoadingCosmeticBackgroundsInPIE 为真时返回 false
bool UTenkaichiDevelopmentStatics::ShouldLoadCosmeticBackgrounds()
{
#if WITH_EDITOR
	if (GIsEditor)
	{
		return !GetDefault<UTenkaichiDeveloperSettings>()->bSkipLoadingCosmeticBackgroundsInPIE;
	}
#endif
	return true;
}

// 机器人能否攻击：仅编辑器里且 bAllowPlayerBotsToAttack 为真时返回 true
bool UTenkaichiDevelopmentStatics::CanPlayerBotsAttack()
{
#if WITH_EDITOR
	if (GIsEditor)
	{
		return GetDefault<UTenkaichiDeveloperSettings>()->bAllowPlayerBotsToAttack;
	}
#endif
	return true;
}

#endif

// 找出最适合在"服务器"上跑 cheat 的 play-in-editor 世界（单机/监听/专用服务器优先）
UWorld* UTenkaichiDevelopmentStatics::FindPlayInEditorAuthorityWorld()
{
	check(GEngine);

	// 找服务器世界（任何 PIE 世界都行，以防没有跑专用服务器；但专用服务器世界最理想）
	UWorld* ServerWorld = nullptr;
#if WITH_EDITOR
	for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
	{
		if (WorldContext.WorldType == EWorldType::PIE)
		{
			if (UWorld* TestWorld = WorldContext.World())
			{
				if (WorldContext.RunAsDedicated)
				{
					// 理想情况：专用服务器
					ServerWorld = TestWorld;
					break;
				}
				else if (ServerWorld == nullptr)
				{
					ServerWorld = TestWorld;
				}
				else
				{
					// 已有候选，看看这个是否"更好"
					if (TestWorld->GetNetMode() < ServerWorld->GetNetMode())
					{
						return ServerWorld;
					}
				}
			}
		}
	}
#endif

	return ServerWorld;
}

// 扫描资产注册表，拿到所有蓝图资产
TArray<FAssetData> UTenkaichiDevelopmentStatics::GetAllBlueprints()
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

	FName PluginAssetPath;

	TArray<FAssetData> BlueprintList;
	FARFilter Filter;
	Filter.ClassPaths.Add(UBlueprint::StaticClass()->GetClassPathName());
	Filter.bRecursivePaths = true;
	AssetRegistryModule.Get().GetAssets(Filter, BlueprintList);

	return BlueprintList;
}

// 按名字找一个蓝图生成类（去 _C 后缀，扫描所有蓝图，命中后校验基类）
UClass* UTenkaichiDevelopmentStatics::FindBlueprintClass(const FString& TargetNameRaw, UClass* DesiredBaseClass)
{
	FString TargetName = TargetNameRaw;
	TargetName.RemoveFromEnd(TEXT("_C"), ESearchCase::CaseSensitive);

	TArray<FAssetData> BlueprintList = UTenkaichiDevelopmentStatics::GetAllBlueprints();
	for (const FAssetData& AssetData : BlueprintList)
	{
		if ((AssetData.AssetName.ToString() == TargetName) || (AssetData.GetObjectPathString() == TargetName))
		{
			if (UBlueprint* BP = Cast<UBlueprint>(AssetData.GetAsset()))
			{
				if (UClass* GeneratedClass = BP->GeneratedClass)
				{
					if (GeneratedClass->IsChildOf(DesiredBaseClass))
					{
						return GeneratedClass;
					}
				}
			}
		}
	}

	return nullptr;
}

// 按短名找一个类：先验名字合法性 → 试原生类 → 试蓝图 → 校验基类
UClass* UTenkaichiDevelopmentStatics::FindClassByShortName(const FString& SearchToken, UClass* DesiredBaseClass, bool bLogFailures)
{
	check(DesiredBaseClass);

	FString TargetName = SearchToken;

	// 先检查原生类和已加载资产，再降级到资产注册表
	bool bIsValidClassName = true;
	if (TargetName.IsEmpty() || TargetName.Contains(TEXT(" ")))
	{
		bIsValidClassName = false;
	}
	else if (!FPackageName::IsShortPackageName(TargetName))
	{
		if (TargetName.Contains(TEXT(".")))
		{
			// 把 "类型'路径'" 转成 "路径"（如果它里面没有 ' 则返回完整字符串）
			TargetName = FPackageName::ExportTextPathToObjectPath(TargetName);

			FString PackageName;
			FString ObjectName;
			TargetName.Split(TEXT("."), &PackageName, &ObjectName);

			const bool bIncludeReadOnlyRoots = true;
			FText Reason;
			if (!FPackageName::IsValidLongPackageName(PackageName, bIncludeReadOnlyRoots, &Reason))
			{
				bIsValidClassName = false;
			}
		}
		else
		{
			bIsValidClassName = false;
		}
	}

	UClass* ResultClass = nullptr;
	if (bIsValidClassName)
	{
		ResultClass = UClass::TryFindTypeSlow<UClass>(TargetName);
	}

	// 还没找到的话，去资产注册表里找匹配要求的蓝图
	if (ResultClass == nullptr)
	{
		ResultClass = FindBlueprintClass(TargetName, DesiredBaseClass);
	}

	// 现在校验类（如果有的话）
	if (ResultClass != nullptr)
	{
		if (!ResultClass->IsChildOf(DesiredBaseClass))
		{
			if (bLogFailures)
			{
				UE_LOG(LogConsoleResponse, Warning, TEXT("Found an asset %s but it wasn't of type %s"), *ResultClass->GetPathName(), *DesiredBaseClass->GetName());
			}
			ResultClass = nullptr;
		}
	}
	else
	{
		if (bLogFailures)
		{
			UE_LOG(LogConsoleResponse, Warning, TEXT("Failed to find class of type %s named %s"), *DesiredBaseClass->GetName(), *SearchToken);
		}
	}

	return ResultClass;
}