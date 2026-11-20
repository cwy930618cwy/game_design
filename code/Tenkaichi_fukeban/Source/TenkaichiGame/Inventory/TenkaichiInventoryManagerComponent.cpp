// 版权声明（Lyra 项目自带，保留）
// Copyright Epic Games, Inc. All Rights Reserved.

#include "TenkaichiInventoryManagerComponent.h"

#include "Engine/ActorChannel.h"
#include "Engine/World.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "TenkaichiInventoryItemDefinition.h"
#include "TenkaichiInventoryItemInstance.h"
#include "NativeGameplayTags.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(TenkaichiInventoryManagerComponent)

class FLifetimeProperty;
struct FReplicationFlags;

// 静态消息标签：清单里"格子数量变化"时，用这个标签广播
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Tenkaichi_Inventory_Message_StackChanged, "Tenkaichi.Inventory.Message.StackChanged");

//////////////////////////////////////////////////////////////////////
// FTenkaichiInventoryEntry

// 调试用：把一格打印成 "物品名 (数量 x 物品类型)"
FString FTenkaichiInventoryEntry::GetDebugString() const
{
	TSubclassOf<UTenkaichiInventoryItemDefinition> ItemDef;
	if (Instance != nullptr)
	{
		ItemDef = Instance->GetItemDef();
	}

	return FString::Printf(TEXT("%s (%d x %s)"), *GetNameSafe(Instance), StackCount, *GetNameSafe(ItemDef));
}

//////////////////////////////////////////////////////////////////////
// FTenkaichiInventoryList

// 回调①：服务端同步过来"有格子被删"时触发。把"从 N 变 0"广播出去。
void FTenkaichiInventoryList::PreReplicateRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	for (int32 Index : RemovedIndices)
	{
		FTenkaichiInventoryEntry& Stack = Entries[Index];
		BroadcastChangeMessage(Stack, /*OldCount=*/ Stack.StackCount, /*NewCount=*/ 0);
		Stack.LastObservedCount = 0;
	}
}

// 回调②：服务端同步过来"新加了一格"时触发。逐个把"从 0 变 N"广播出去。
void FTenkaichiInventoryList::PostReplicateAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	for (int32 Index : AddedIndices)
	{
		FTenkaichiInventoryEntry& Entry = Entries[Index];
		BroadcastChangeMessage(Entry, /*OldCount=*/ 0, /*NewCount=*/ Entry.StackCount);
		Entry.LastObservedCount = Entry.StackCount;
	}
}

// 回调③：服务端同步过来"某格子数量变了"时触发。把"从旧值变新值"广播出去。
void FTenkaichiInventoryList::PostReplicateChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	for (int32 Index : ChangedIndices)
	{
		FTenkaichiInventoryEntry& Entry = Entries[Index];
		// 正常情况下客户端一定记录过旧值（用 INDEX_NONE 标记"还没见过"）
		check(Entry.LastObservedCount != INDEX_NONE);
		BroadcastChangeMessage(Entry, /*OldCount=*/ Entry.LastObservedCount, /*NewCount=*/ Entry.StackCount);
		Entry.LastObservedCount = Entry.StackCount;
	}
}

// 广播核心：组一条"变化消息"（谁变的、什么变了、新旧数量、变化量），发给所有监听者
void FTenkaichiInventoryList::BroadcastChangeMessage(FTenkaichiInventoryEntry& Entry, int32 OldCount, int32 NewCount)
{
	FTenkaichiInventoryChangeMessage Message;
	Message.InventoryOwner = OwnerComponent;
	Message.Instance = Entry.Instance;
	Message.NewCount = NewCount;
	Message.Delta = NewCount - OldCount;

	// 拿到全局消息系统，用"StackChanged"标签广播出去
	UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(OwnerComponent->GetWorld());
	MessageSystem.BroadcastMessage(TAG_Tenkaichi_Inventory_Message_StackChanged, Message);
}

// 增①（最常用）：按"物品定义"造一格 —— 建 Instance + 灌 Fragment 数据 + 记 StackCount
UTenkaichiInventoryItemInstance* FTenkaichiInventoryList::AddEntry(TSubclassOf<UTenkaichiInventoryItemDefinition> ItemDef, int32 StackCount)
{
	UTenkaichiInventoryItemInstance* Result = nullptr;

	check(ItemDef != nullptr);
	check(OwnerComponent);

	AActor* OwningActor = OwnerComponent->GetOwner();
	check(OwningActor->HasAuthority());

	// 新建一格，塞一个 ItemInstance 进去
	FTenkaichiInventoryEntry& NewEntry = Entries.AddDefaulted_GetRef();
	NewEntry.Instance = NewObject<UTenkaichiInventoryItemInstance>(OwnerComponent->GetOwner());  //TODO: 用 Actor 作 outer（UE-127172 的坑）
	NewEntry.Instance->SetItemDef(ItemDef);
	// 把物品定义上挂的每个 Fragment 都"灌"进这个新实例（配料下锅）
	for (UTenkaichiInventoryItemFragment* Fragment : GetDefault<UTenkaichiInventoryItemDefinition>(ItemDef)->Fragments)
	{
		if (Fragment != nullptr)
		{
			Fragment->OnInstanceCreated(NewEntry.Instance);
		}
	}
	NewEntry.StackCount = StackCount;
	Result = NewEntry.Instance;

	// 通知 FastArray：这一格变了，网络增量同步时要带上它
	MarkItemDirty(NewEntry);

	return Result;
}

// 增②（占位）：直接塞一个现成 Instance 进来。当前没实现（Unimplemented 会在控制台报错提示）
void FTenkaichiInventoryList::AddEntry(UTenkaichiInventoryItemInstance* Instance)
{
	unimplemented();
}

// 删：按实例找格子删掉，并整表标脏（增量同步）
void FTenkaichiInventoryList::RemoveEntry(UTenkaichiInventoryItemInstance* Instance)
{
	for (auto EntryIt = Entries.CreateIterator(); EntryIt; ++EntryIt)
	{
		FTenkaichiInventoryEntry& Entry = *EntryIt;
		if (Entry.Instance == Instance)
		{
			EntryIt.RemoveCurrent();
			MarkArrayDirty();
		}
	}
}

// 查：把清单里所有"还活着的"实例收集成数组返回
TArray<UTenkaichiInventoryItemInstance*> FTenkaichiInventoryList::GetAllItems() const
{
	TArray<UTenkaichiInventoryItemInstance*> Results;
	Results.Reserve(Entries.Num());
	for (const FTenkaichiInventoryEntry& Entry : Entries)
	{
		if (Entry.Instance != nullptr) //TODO: 想把这个判断藏得更深一些
		{
			Results.Add(Entry.Instance);
		}
	}

	return Results;
}

//////////////////////////////////////////////////////////////////////
// UTenkaichiInventoryManagerComponent

// 构造函数：把清单挂到本组件上，并标记"这个组件默认参与网络复制"
UTenkaichiInventoryManagerComponent::UTenkaichiInventoryManagerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, InventoryList(this)
{
	SetIsReplicatedByDefault(true);
}

// 网络复制注册：告诉引擎"InventoryList 这个字段要同步"
void UTenkaichiInventoryManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, InventoryList);
}

// 能否加物品（当前恒 true，留扩展位：以后可加堆叠上限/唯一性检查）
bool UTenkaichiInventoryManagerComponent::CanAddItemDefinition(TSubclassOf<UTenkaichiInventoryItemDefinition> ItemDef, int32 StackCount)
{
	//TODO: 以后支持堆叠上限 / 唯一性检查等
	return true;
}

// 按"物品定义"加一件：委托给清单造格子，再把它注册为网络子对象
UTenkaichiInventoryItemInstance* UTenkaichiInventoryManagerComponent::AddItemDefinition(TSubclassOf<UTenkaichiInventoryItemDefinition> ItemDef, int32 StackCount)
{
	UTenkaichiInventoryItemInstance* Result = nullptr;
	if (ItemDef != nullptr)
	{
		Result = InventoryList.AddEntry(ItemDef, StackCount);

		// 如果已开启"注册子对象列表"且已准备复制且造出来了 → 把新实例注册为子对象
		if (IsUsingRegisteredSubObjectList() && IsReadyForReplication() && Result)
		{
			AddReplicatedSubObject(Result);
		}
	}
	return Result;
}

// 直接塞一个现成实例（同样注册为网络子对象）
void UTenkaichiInventoryManagerComponent::AddItemInstance(UTenkaichiInventoryItemInstance* ItemInstance)
{
	InventoryList.AddEntry(ItemInstance);
	if (IsUsingRegisteredSubObjectList() && IsReadyForReplication() && ItemInstance)
	{
		AddReplicatedSubObject(ItemInstance);
	}
}

// 移除一个实例：从清单删掉，并取消它的网络子对象注册
void UTenkaichiInventoryManagerComponent::RemoveItemInstance(UTenkaichiInventoryItemInstance* ItemInstance)
{
	InventoryList.RemoveEntry(ItemInstance);

	if (ItemInstance && IsUsingRegisteredSubObjectList())
	{
		RemoveReplicatedSubObject(ItemInstance);
	}
}

// 取所有物品：直接转交清单的 GetAllItems
TArray<UTenkaichiInventoryItemInstance*> UTenkaichiInventoryManagerComponent::GetAllItems() const
{
	return InventoryList.GetAllItems();
}

// 按定义找第一个匹配的实例（遍历清单，逐个比对实例的定义）
UTenkaichiInventoryItemInstance* UTenkaichiInventoryManagerComponent::FindFirstItemStackByDefinition(TSubclassOf<UTenkaichiInventoryItemDefinition> ItemDef) const
{
	for (const FTenkaichiInventoryEntry& Entry : InventoryList.Entries)
	{
		UTenkaichiInventoryItemInstance* Instance = Entry.Instance;

		if (IsValid(Instance))
		{
			if (Instance->GetItemDef() == ItemDef)
			{
				return Instance;
			}
		}
	}

	return nullptr;
}

// 统计某定义物品的总数（遍历清单，数有几个实例属于这个定义）
int32 UTenkaichiInventoryManagerComponent::GetTotalItemCountByDefinition(TSubclassOf<UTenkaichiInventoryItemDefinition> ItemDef) const
{
	int32 TotalCount = 0;
	for (const FTenkaichiInventoryEntry& Entry : InventoryList.Entries)
	{
		UTenkaichiInventoryItemInstance* Instance = Entry.Instance;

		if (IsValid(Instance))
		{
			if (Instance->GetItemDef() == ItemDef)
			{
				++TotalCount;
			}
		}
	}

	return TotalCount;
}

// 消耗 N 个指定物品（一个一个找、一个一个删，直到消耗够或找不到为止）
bool UTenkaichiInventoryManagerComponent::ConsumeItemsByDefinition(TSubclassOf<UTenkaichiInventoryItemDefinition> ItemDef, int32 NumToConsume)
{
	AActor* OwningActor = GetOwner();
	if (!OwningActor || !OwningActor->HasAuthority())
	{
		return false;
	}

	//TODO: 现在是 N 平方复杂度，因为还没有加速结构
	int32 TotalConsumed = 0;
	while (TotalConsumed < NumToConsume)
	{
		if (UTenkaichiInventoryItemInstance* Instance = UTenkaichiInventoryManagerComponent::FindFirstItemStackByDefinition(ItemDef))
		{
			InventoryList.RemoveEntry(Instance);
			++TotalConsumed;
		}
		else
		{
			return false;
		}
	}

	return TotalConsumed == NumToConsume;
}

// 准备复制：把清单里所有已存在的实例都注册为网络子对象
void UTenkaichiInventoryManagerComponent::ReadyForReplication()
{
	Super::ReadyForReplication();

	// 注册所有已存在的实例
	if (IsUsingRegisteredSubObjectList())
	{
		for (const FTenkaichiInventoryEntry& Entry : InventoryList.Entries)
		{
			UTenkaichiInventoryItemInstance* Instance = Entry.Instance;

			if (IsValid(Instance))
			{
				AddReplicatedSubObject(Instance);
			}
		}
	}
}

// 复制子对象：逐个把实例作为子对象写进网络数据（发给客户端）
bool UTenkaichiInventoryManagerComponent::ReplicateSubobjects(UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	for (FTenkaichiInventoryEntry& Entry : InventoryList.Entries)
	{
		UTenkaichiInventoryItemInstance* Instance = Entry.Instance;

		if (Instance && IsValid(Instance))
		{
			WroteSomething |= Channel->ReplicateSubobject(Instance, *Bunch, *RepFlags);
		}
	}

	return WroteSomething;
}

//////////////////////////////////////////////////////////////////////
//

//（以下是被注释掉的过滤器占位代码，Lyra 里留着未启用，照抄保留）

// UCLASS(Abstract)
// class UTenkaichiInventoryFilter : public UObject
// {
// public:
// 	virtual bool PassesFilter(UTenkaichiInventoryItemInstance* Instance) const { return true; }
// };

// UCLASS()
// class UTenkaichiInventoryFilter_HasTag : public UTenkaichiInventoryFilter
// {
// public:
// 	virtual bool PassesFilter(UTenkaichiInventoryItemInstance* Instance) const { return true; }
// };