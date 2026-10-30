// 版权声明（Lyra 项目自带，保留）
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "TenkaichiInventoryItemDefinition.generated.h"

// 前置声明：TSubclassOf 模板类（避免引完整定义，加快编译）
template <typename T> class TSubclassOf;

// 前置声明：物品实例（P08.2 才教，这里只需知道有这个类名）
class UTenkaichiInventoryItemInstance;

// 前置声明：FFrame 结构体（引擎）
struct FFrame;

//////////////////////////////////////////////////////////////////////

// 物品定义的「碎片」基类：所有物品特性的父类，本身是抽象类，不能直接实例化。
// 三个标记让蓝图能往数组里直接添加不同子类的对象（见总 md 第三节）。
UCLASS(MinimalAPI, DefaultToInstanced, EditInlineNew, Abstract)
class UTenkaichiInventoryItemFragment : public UObject
{
	GENERATED_BODY()

public:
	// 钩子：当这个物品定义被 new 出一个实例时调用，让碎片有机会往实例里塞初始数据
	virtual void OnInstanceCreated(UTenkaichiInventoryItemInstance* Instance) const {}
};

//////////////////////////////////////////////////////////////////////

/**
 * UTenkaichiInventoryItemDefinition
 *
 * 物品定义：菜谱本体。一个显示名 + 一堆可插拔的碎片。
 */
UCLASS(Blueprintable, Const, Abstract)
class UTenkaichiInventoryItemDefinition : public UObject
{
	GENERATED_BODY()

public:
	// 构造函数：带 FObjectInitializer（UObject 子类标准写法，默认值给 Get()）
	UTenkaichiInventoryItemDefinition(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// 物品的显示名（给玩家看的中文/英文名）
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Display)
	FText DisplayName;

	// 物品的碎片数组：靠 Instanced 标记，蓝图里能直接添加不同 Fragment 子类对象
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Display, Instanced)
	TArray<TObjectPtr<UTenkaichiInventoryItemFragment>> Fragments;

public:
	// 按碎片类型，从数组里找出对应的碎片（IsA 匹配）；找不到返回 nullptr
	const UTenkaichiInventoryItemFragment* FindFragmentByClass(TSubclassOf<UTenkaichiInventoryItemFragment> FragmentClass) const;
};

//@TODO: 将来是否改成 Subsystem？（Lyra 原注释，保留）
// 蓝图工具库：给蓝图用的静态函数，按类名找碎片
UCLASS()
class UTenkaichiInventoryFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	// 给「物品定义类 + 碎片类」，返回该定义的对应碎片（DeterminesOutputType 让蓝图正确推断返回类型）
	UFUNCTION(BlueprintCallable, meta=(DeterminesOutputType=FragmentClass))
	static const UTenkaichiInventoryItemFragment* FindItemDefinitionFragment(TSubclassOf<UTenkaichiInventoryItemDefinition> ItemDef, TSubclassOf<UTenkaichiInventoryItemFragment> FragmentClass);
};