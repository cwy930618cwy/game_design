// 版权声明（Lyra 项目自带，保留）
// Copyright Epic Games, Inc. All Rights Reserved.

#include "InventoryFragment_SetStats.h"

#include "Inventory/TenkaichiInventoryItemInstance.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(InventoryFragment_SetStats)

// 覆写钩子：物品实例创建时，把 InitialItemStats 表里每个 Tag 灌进实例的 StatTags
void UInventoryFragment_SetStats::OnInstanceCreated(UTenkaichiInventoryItemInstance* Instance) const
{
	Super::OnInstanceCreated(Instance);

	for (const auto& KVP : InitialItemStats)
	{
		Instance->AddStatTagStack(KVP.Key, KVP.Value);
	}
}

// 按 Tag 查初始属性表里的值（查不到返回 0）
int32 UInventoryFragment_SetStats::GetItemStatByTag(FGameplayTag Tag) const
{
	const int32* FoundStat = InitialItemStats.Find(Tag);
	return (FoundStat != nullptr) ? *FoundStat : 0;
}