// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

// 蓝图函数库基类：让静态函数能被蓝图调用
#include "Kismet/BlueprintFunctionLibrary.h"
// TSubclassOf 模板：保存"某个类的子类"引用
#include "Templates/SubclassOf.h"

// 生成头文件（UHT 用，放最后）
#include "TenkaichiDevelopmentStatics.generated.h"

// 前置声明（.h 里只需要声明，不展开头文件）
class UClass;
class UObject;
class UWorld;
struct FAssetData;
struct FFrame;

UCLASS()
class UTenkaichiDevelopmentStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// 游戏逻辑是否应跳过开场直接进玩法（跳过热身/等待玩家等环节）。
	// 除"在编辑器里玩且 bTestFullGameFlowInPIE（见 Lyra 开发设置）为假"外，恒返回 false。
	UFUNCTION(BlueprintCallable, Category="Tenkaichi")
	static bool ShouldSkipDirectlyToGameplay();

	// 游戏逻辑是否应在编辑器里加载外观背景？
	// 除"在编辑器里玩且 bSkipLoadingCosmeticBackgroundsInPIE（见 Lyra 开发设置）为真"外，恒返回 true。
	UFUNCTION(BlueprintCallable, Category = "Tenkaichi", meta=(ExpandBoolAsExecs="ReturnValue"))
	static bool ShouldLoadCosmeticBackgrounds();

	// 机器人（Bot）是否允许攻击？
	// 除"在编辑器里玩且 bAllowPlayerBotsToAttack（见 Lyra 开发设置）为真"外，恒返回 true。
	UFUNCTION(BlueprintCallable, Category = "Tenkaichi")
	static bool CanPlayerBotsAttack();

	// 找出最适合在"服务器"上跑 cheat 的 play-in-editor 世界。
	//   单机时可能是唯一世界、监听服务器，或专用服务器。
	static UWorld* FindPlayInEditorAuthorityWorld();

	// 尝试按短名找一个类（带一些启发式规则，提升通过 cheat 控制台使用的易用性）。
	static UClass* FindClassByShortName(const FString& SearchToken, UClass* DesiredBaseClass, bool bLogFailures = true);

	// 按短名找类的模板重载：自动用 DesiredClass 的类型做基类。
	template <typename DesiredClass>
	static TSubclassOf<DesiredClass> FindClassByShortName(const FString& SearchToken, bool bLogFailures = true)
	{
		return FindClassByShortName(SearchToken, DesiredClass::StaticClass(), bLogFailures);
	}

private:
	// 扫描资产注册表，拿到所有蓝图资产。
	static TArray<FAssetData> GetAllBlueprints();
	// 按名字找一个蓝图生成类。
	static UClass* FindBlueprintClass(const FString& TargetNameRaw, UClass* DesiredBaseClass);
};