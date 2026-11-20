// 版权声明（Lyra 项目自带，保留）
// Copyright Epic Games, Inc. All Rights Reserved.

#include "TenkaichiEquipmentManagerComponent.h"

#include "AbilitySystem/TenkaichiAbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Engine/ActorChannel.h"
#include "TenkaichiEquipmentDefinition.h"
#include "TenkaichiEquipmentInstance.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiEquipmentManagerComponent)

class FLifetimeProperty;
struct FReplicationFlags;

//////////////////////////////////////////////////////////////////////
// FTenkaichiAppliedEquipmentEntry

// 调试用：打印 "实例名 of 定义名"
FString FTenkaichiAppliedEquipmentEntry::GetDebugString() const
{
	return FString::Printf(TEXT("%s of %s"), *GetNameSafe(Instance), *GetNameSafe(EquipmentDefinition.Get()));
}

//////////////////////////////////////////////////////////////////////
// FTenkaichiEquipmentList

// 回调①：客户端收到"有装备被脱"时触发。逐个调用实例的 OnUnequipped。
void FTenkaichiEquipmentList::PreReplicateRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	for (int32 Index : RemovedIndices)
	{
		const FTenkaichiAppliedEquipmentEntry& Entry = Entries[Index];
		if (Entry.Instance != nullptr)
		{
			Entry.Instance->OnUnequipped();
		}
	}
}

// 回调②：客户端收到"有新装备穿上"时触发。逐个调用实例的 OnEquipped。
void FTenkaichiEquipmentList::PostReplicateAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	for (int32 Index : AddedIndices)
	{
		const FTenkaichiAppliedEquipmentEntry& Entry = Entries[Index];
		if (Entry.Instance != nullptr)
		{
			Entry.Instance->OnEquipped();
		}
	}
}

// 回调③：装备数量变化时触发（当前空实现，Lyra 里这段注释掉了）
void FTenkaichiEquipmentList::PostReplicateChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
// 	for (int32 Index : ChangedIndices)
// 	{
// 		const FGameplayTagStack& Stack = Stacks[Index];
// 		TagToCountMap[Stack.Tag] = Stack.StackCount;
// 	}
}

// 取角色的能力系统组件（ASC）
UTenkaichiAbilitySystemComponent* FTenkaichiEquipmentList::GetAbilitySystemComponent() const
{
	check(OwnerComponent);
	AActor* OwningActor = OwnerComponent->GetOwner();
	return Cast<UTenkaichiAbilitySystemComponent>(UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OwningActor));
}

// 穿一件装备：建实例 + 授予技能集 + 生成装备 Actor
UTenkaichiEquipmentInstance* FTenkaichiEquipmentList::AddEntry(TSubclassOf<UTenkaichiEquipmentDefinition> EquipmentDefinition)
{
	UTenkaichiEquipmentInstance* Result = nullptr;

	check(EquipmentDefinition != nullptr);
	check(OwnerComponent);
	check(OwnerComponent->GetOwner()->HasAuthority());

	// 拿装备定义的默认对象（CDO）
	const UTenkaichiEquipmentDefinition* EquipmentCDO = GetDefault<UTenkaichiEquipmentDefinition>(EquipmentDefinition);

	// 确定实例类型（定义里配的，没有就用默认）
	TSubclassOf<UTenkaichiEquipmentInstance> InstanceType = EquipmentCDO->InstanceType;
	if (InstanceType == nullptr)
	{
		InstanceType = UTenkaichiEquipmentInstance::StaticClass();
	}

	// 新建一格，塞一个装备实例进去
	FTenkaichiAppliedEquipmentEntry& NewEntry = Entries.AddDefaulted_GetRef();
	NewEntry.EquipmentDefinition = EquipmentDefinition;
	NewEntry.Instance = NewObject<UTenkaichiEquipmentInstance>(OwnerComponent->GetOwner(), InstanceType);  //TODO: 用 Actor 作 outer（UE-127172 的坑）
	Result = NewEntry.Instance;

	// 授予技能集：如果有 ASC，逐个把技能集授予给它
	if (UTenkaichiAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		for (const TObjectPtr<const UTenkaichiAbilitySet>& AbilitySet : EquipmentCDO->AbilitySetsToGrant)
		{
			AbilitySet->GiveToAbilitySystem(ASC, /*inout*/ &NewEntry.GrantedHandles, Result);
		}
	}
	else
	{
		//TODO: 加警告日志？
	}

	// 生成装备 Actor（武器模型等）
	Result->SpawnEquipmentActors(EquipmentCDO->ActorsToSpawn);

	// 通知 FastArray：这一格变了，网络增量同步时要带上它
	MarkItemDirty(NewEntry);

	return Result;
}

// 脱一件装备：回收技能 + 销毁 Actor
void FTenkaichiEquipmentList::RemoveEntry(UTenkaichiEquipmentInstance* Instance)
{
	for (auto EntryIt = Entries.CreateIterator(); EntryIt; ++EntryIt)
	{
		FTenkaichiAppliedEquipmentEntry& Entry = *EntryIt;
		if (Entry.Instance == Instance)
		{
			// 回收技能集（按句柄精确回收）
			if (UTenkaichiAbilitySystemComponent* ASC = GetAbilitySystemComponent())
			{
				Entry.GrantedHandles.TakeFromAbilitySystem(ASC);
			}

			// 销毁装备 Actor
			Instance->DestroyEquipmentActors();

			// 删除这一格，整表标脏
			EntryIt.RemoveCurrent();
			MarkArrayDirty();
		}
	}
}

//////////////////////////////////////////////////////////////////////
// UTenkaichiEquipmentManagerComponent

// 构造函数：把列表挂到本组件上，标记默认复制 + 需要初始化
UTenkaichiEquipmentManagerComponent::UTenkaichiEquipmentManagerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, EquipmentList(this)
{
	SetIsReplicatedByDefault(true);
	bWantsInitializeComponent = true;
}

// 网络复制注册：EquipmentList 要同步
void UTenkaichiEquipmentManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, EquipmentList);
}

// 穿一件装备：委托列表 AddEntry，再注册为网络子对象
UTenkaichiEquipmentInstance* UTenkaichiEquipmentManagerComponent::EquipItem(TSubclassOf<UTenkaichiEquipmentDefinition> EquipmentClass)
{
	UTenkaichiEquipmentInstance* Result = nullptr;
	if (EquipmentClass != nullptr)
	{
		Result = EquipmentList.AddEntry(EquipmentClass);
		if (Result != nullptr)
		{
			// 穿上后触发 OnEquipped 钩子
			Result->OnEquipped();

			// 如果已开启注册子对象列表且已准备复制 → 注册为子对象
			if (IsUsingRegisteredSubObjectList() && IsReadyForReplication())
			{
				AddReplicatedSubObject(Result);
			}
		}
	}
	return Result;
}

// 脱一件装备：取消子对象注册，触发卸下钩子，委托列表 RemoveEntry
void UTenkaichiEquipmentManagerComponent::UnequipItem(UTenkaichiEquipmentInstance* ItemInstance)
{
	if (ItemInstance != nullptr)
	{
		// 取消网络子对象注册
		if (IsUsingRegisteredSubObjectList())
		{
			RemoveReplicatedSubObject(ItemInstance);
		}

		// 触发卸下钩子
		ItemInstance->OnUnequipped();
		// 从列表删除（回收技能 + 销毁 Actor）
		EquipmentList.RemoveEntry(ItemInstance);
	}
}

// 复制子对象：逐个把装备实例作为子对象写进网络数据
bool UTenkaichiEquipmentManagerComponent::ReplicateSubobjects(UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	for (FTenkaichiAppliedEquipmentEntry& Entry : EquipmentList.Entries)
	{
		UTenkaichiEquipmentInstance* Instance = Entry.Instance;

		if (IsValid(Instance))
		{
			WroteSomething |= Channel->ReplicateSubobject(Instance, *Bunch, *RepFlags);
		}
	}

	return WroteSomething;
}

// 初始化组件（当前只是调用父类）
void UTenkaichiEquipmentManagerComponent::InitializeComponent()
{
	Super::InitializeComponent();
}

// 反初始化组件：先把所有装备实例收集起来，再逐个脱下
void UTenkaichiEquipmentManagerComponent::UninitializeComponent()
{
	TArray<UTenkaichiEquipmentInstance*> AllEquipmentInstances;

	// 先把所有实例收集起来再脱，避免遍历时修改列表导致副作用
	for (const FTenkaichiAppliedEquipmentEntry& Entry : EquipmentList.Entries)
	{
		AllEquipmentInstances.Add(Entry.Instance);
	}

	for (UTenkaichiEquipmentInstance* EquipInstance : AllEquipmentInstances)
	{
		UnequipItem(EquipInstance);
	}

	Super::UninitializeComponent();
}

// 准备复制：把清单里所有已存在的实例都注册为网络子对象
void UTenkaichiEquipmentManagerComponent::ReadyForReplication()
{
	Super::ReadyForReplication();

	// 注册所有已存在的装备实例
	if (IsUsingRegisteredSubObjectList())
	{
		for (const FTenkaichiAppliedEquipmentEntry& Entry : EquipmentList.Entries)
		{
			UTenkaichiEquipmentInstance* Instance = Entry.Instance;

			if (IsValid(Instance))
			{
				AddReplicatedSubObject(Instance);
			}
		}
	}
}

// 找第一件某类型的装备
UTenkaichiEquipmentInstance* UTenkaichiEquipmentManagerComponent::GetFirstInstanceOfType(TSubclassOf<UTenkaichiEquipmentInstance> InstanceType)
{
	for (FTenkaichiAppliedEquipmentEntry& Entry : EquipmentList.Entries)
	{
		if (UTenkaichiEquipmentInstance* Instance = Entry.Instance)
		{
			if (Instance->IsA(InstanceType))
			{
				return Instance;
			}
		}
	}

	return nullptr;
}

// 找所有某类型的装备
TArray<UTenkaichiEquipmentInstance*> UTenkaichiEquipmentManagerComponent::GetEquipmentInstancesOfType(TSubclassOf<UTenkaichiEquipmentInstance> InstanceType) const
{
	TArray<UTenkaichiEquipmentInstance*> Results;
	for (const FTenkaichiAppliedEquipmentEntry& Entry : EquipmentList.Entries)
	{
		if (UTenkaichiEquipmentInstance* Instance = Entry.Instance)
		{
			if (Instance->IsA(InstanceType))
			{
				Results.Add(Instance);
			}
		}
	}
	return Results;
}