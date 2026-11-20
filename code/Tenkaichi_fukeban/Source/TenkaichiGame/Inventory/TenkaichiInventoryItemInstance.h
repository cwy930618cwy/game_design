// 版权声明（Lyra 项目自带，保留）
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "System/GameplayTagStack.h"
#include "Templates/SubclassOf.h"

#include "TenkaichiInventoryItemInstance.generated.h"

// 前置声明：FLifetimeProperty（网络复制属性登记用，引擎结构体）
class FLifetimeProperty;

// 前置声明：物品定义（P08.1 已教，这里只需类名）
class UTenkaichiInventoryItemDefinition;

// 前置声明：物品碎片（P08.1 已教）
class UTenkaichiInventoryItemFragment;

// 前置声明：FFrame（引擎结构体）
struct FFrame;

// 前置声明：FGameplayTag（引擎 GameplayTags 结构体）
struct FGameplayTag;

//////////////////////////////////////////////////////////////////////

/**
 * UTenkaichiInventoryItemInstance
 *
 * 物品实例：定义（菜谱）的"活个体"，存「这件物品现在怎么样」的实时数据（弹药数、耐久等）。
 */
UCLASS(BlueprintType)
class UTenkaichiInventoryItemInstance : public UObject
{
	GENERATED_BODY()

public:
	// 构造函数：带 FObjectInitializer（UObject 子类标准写法，默认值给 Get()）
	UTenkaichiInventoryItemInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~UObject 接口开始
	// 允许这个 UObject 参与网络复制（返回 true）
	virtual bool IsSupportedForNetworking() const override { return true; }
	//~UObject 接口结束

public:
	// 给指定 Tag 增加指定数量的堆叠（StackCount 小于 1 时什么都不做）
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Inventory)
	void AddStatTagStack(FGameplayTag Tag, int32 StackCount);

	// 从指定 Tag 移除指定数量的堆叠（StackCount 小于 1 时什么都不做）
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Inventory)
	void RemoveStatTagStack(FGameplayTag Tag, int32 StackCount);

	// 返回指定 Tag 的堆叠数量（Tag 不存在时返回 0）
	UFUNCTION(BlueprintCallable, Category=Inventory)
	int32 GetStatTagStackCount(FGameplayTag Tag) const;

	// 若指定 Tag 至少有 1 个堆叠则返回 true
	UFUNCTION(BlueprintCallable, Category=Inventory)
	bool HasStatTag(FGameplayTag Tag) const;

	// 返回本实例指向的物品定义类（菜谱）
	TSubclassOf<UTenkaichiInventoryItemDefinition> GetItemDef() const
	{
		return ItemDef;
	}

	// 按碎片类型，从本实例所属定义里找出对应碎片（DeterminesOutputType 让蓝图正确推断返回类型）
	UFUNCTION(BlueprintCallable, BlueprintPure=false, meta=(DeterminesOutputType=FragmentClass))
	const UTenkaichiInventoryItemFragment* FindFragmentByClass(TSubclassOf<UTenkaichiInventoryItemFragment> FragmentClass) const;

	// 模板版 FindFragmentByClass：由返回类型自动推导碎片类，免写 StaticClass
	template <typename ResultClass>
	const ResultClass* FindFragmentByClass() const
	{
		return (ResultClass*)FindFragmentByClass(ResultClass::StaticClass());
	}

private:
#if UE_WITH_IRIS
	// Iris 复制框架：注册这个对象的所有复制片段（新复制系统用）
	virtual void RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context, UE::Net::EFragmentRegistrationFlags RegistrationFlags) override;
#endif // UE_WITH_IRIS

	// 设置本实例指向的物品定义类（仅供 FLyraInventoryList 友元调用）
	void SetItemDef(TSubclassOf<UTenkaichiInventoryItemDefinition> InDef);

	// 友元：FTenkaichiInventoryList（P08.7 才定义，负责创建/管理实例，需要直接调 SetItemDef）
	friend struct FTenkaichiInventoryList;

private:
	// 带数量的属性仓库（弹药数、耐久等），用 GameplayTag 作键，支持网络复制
	UPROPERTY(Replicated)
	FGameplayTagStackContainer StatTags;

	// 本实例指向的物品定义（菜谱），支持网络复制
	UPROPERTY(Replicated)
	TSubclassOf<UTenkaichiInventoryItemDefinition> ItemDef;
};