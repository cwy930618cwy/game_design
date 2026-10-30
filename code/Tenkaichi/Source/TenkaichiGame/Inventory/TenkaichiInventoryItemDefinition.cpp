// 版权声明（Lyra 项目自带，保留）
// Copyright Epic Games, Inc. All Rights Reserved.

#include "TenkaichiInventoryItemDefinition.h"

#include "Templates/SubclassOf.h"
#include "UObject/ObjectPtr.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiInventoryItemDefinition)

//////////////////////////////////////////////////////////////////////
// UTenkaichiInventoryItemDefinition

// 构造函数：把 ObjectInitializer 传给父类 UObject（标准写法，无额外初始化）
UTenkaichiInventoryItemDefinition::UTenkaichiInventoryItemDefinition(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

// 按碎片类型，从 Fragments 数组里找出第一个匹配的碎片；找不到返回 nullptr
const UTenkaichiInventoryItemFragment* UTenkaichiInventoryItemDefinition::FindFragmentByClass(TSubclassOf<UTenkaichiInventoryItemFragment> FragmentClass) const
{
	if (FragmentClass != nullptr)
	{
		for (UTenkaichiInventoryItemFragment* Fragment : Fragments)
		{
			// IsA 判断：这个碎片对象是不是「FragmentClass 指定类型（或其子类）」的实例
			if (Fragment && Fragment->IsA(FragmentClass))
			{
				return Fragment;
			}
		}
	}

	return nullptr;
}

//////////////////////////////////////////////////////////////////////
// UTenkaichiInventoryFunctionLibrary

// 给「物品定义类 + 碎片类」，取该定义的 CDO，再委托它找碎片
const UTenkaichiInventoryItemFragment* UTenkaichiInventoryFunctionLibrary::FindItemDefinitionFragment(TSubclassOf<UTenkaichiInventoryItemDefinition> ItemDef, TSubclassOf<UTenkaichiInventoryItemFragment> FragmentClass)
{
	if ((ItemDef != nullptr) && (FragmentClass != nullptr))
	{
		// GetDefault<定义类>：拿到这个定义类的 CDO（类默认对象，就是那份"菜谱"）
		return GetDefault<UTenkaichiInventoryItemDefinition>(ItemDef)->FindFragmentByClass(FragmentClass);
	}
	return nullptr;
}