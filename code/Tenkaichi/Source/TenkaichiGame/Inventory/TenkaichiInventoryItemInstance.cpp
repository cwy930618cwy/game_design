// 版权声明（Lyra 项目自带，保留）
// Copyright Epic Games, Inc. All Rights Reserved.

#include "TenkaichiInventoryItemInstance.h"

#include "Inventory/TenkaichiInventoryItemDefinition.h"
#include "Net/UnrealNetwork.h"

#if UE_WITH_IRIS
#include "Iris/ReplicationSystem/ReplicationFragmentUtil.h"
#endif // UE_WITH_IRIS

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiInventoryItemInstance)

// 前置声明：FLifetimeProperty（网络复制属性登记用，引擎结构体）
class FLifetimeProperty;

// 构造函数：把 ObjectInitializer 传给父类 UObject（标准写法，无额外初始化）
UTenkaichiInventoryItemInstance::UTenkaichiInventoryItemInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

// 登记本类哪些属性要网络复制（StatTags + ItemDef 两个）
void UTenkaichiInventoryItemInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, StatTags);
	DOREPLIFETIME(ThisClass, ItemDef);
}

#if UE_WITH_IRIS
// Iris 复制框架：为本对象构建并注册属性复制片段（新复制系统用）
void UTenkaichiInventoryItemInstance::RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context, UE::Net::EFragmentRegistrationFlags RegistrationFlags)
{
	using namespace UE::Net;

	// 为本对象构建描述符并分配 PropertyReplicationFragments
	FReplicationFragmentUtil::CreateAndRegisterFragmentsForObject(this, Context, RegistrationFlags);
}
#endif // UE_WITH_IRIS

// 给指定 Tag 增加指定数量的堆叠
void UTenkaichiInventoryItemInstance::AddStatTagStack(FGameplayTag Tag, int32 StackCount)
{
	StatTags.AddStack(Tag, StackCount);
}

// 从指定 Tag 移除指定数量的堆叠
void UTenkaichiInventoryItemInstance::RemoveStatTagStack(FGameplayTag Tag, int32 StackCount)
{
	StatTags.RemoveStack(Tag, StackCount);
}

// 返回指定 Tag 的堆叠数量
int32 UTenkaichiInventoryItemInstance::GetStatTagStackCount(FGameplayTag Tag) const
{
	return StatTags.GetStackCount(Tag);
}

// 若指定 Tag 至少有 1 个堆叠则返回 true
bool UTenkaichiInventoryItemInstance::HasStatTag(FGameplayTag Tag) const
{
	return StatTags.ContainsTag(Tag);
}

// 设置本实例指向的物品定义类（菜谱）
void UTenkaichiInventoryItemInstance::SetItemDef(TSubclassOf<UTenkaichiInventoryItemDefinition> InDef)
{
	ItemDef = InDef;
}

// 按碎片类型，委托给定义（CDO）找出对应碎片；ItemDef 或 FragmentClass 为空时返回 nullptr
const UTenkaichiInventoryItemFragment* UTenkaichiInventoryItemInstance::FindFragmentByClass(TSubclassOf<UTenkaichiInventoryItemFragment> FragmentClass) const
{
	if ((ItemDef != nullptr) && (FragmentClass != nullptr))
	{
		return GetDefault<UTenkaichiInventoryItemDefinition>(ItemDef)->FindFragmentByClass(FragmentClass);
	}

	return nullptr;
}