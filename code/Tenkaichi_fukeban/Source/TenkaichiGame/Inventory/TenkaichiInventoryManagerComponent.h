// 版权声明（Lyra 项目自带，保留）
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "Net/Serialization/FastArraySerializer.h"

#include "TenkaichiInventoryManagerComponent.generated.h"

#define UE_API TENKAICHIGAME_API

class ULyraInventoryItemDefinition;
class ULyraInventoryItemInstance;
class ULyraInventoryManagerComponent;
class UObject;
struct FFrame;
struct FLyraInventoryList;
struct FNetDeltaSerializeInfo;
struct FReplicationFlags;

/** 一条"物品变化"消息：清单里加了/减了物品时广播，UI 或逻辑监听用 */
USTRUCT(BlueprintType)
struct FTenkaichiInventoryChangeMessage
{
	GENERATED_BODY()

	//@TODO: 未来用"标签名 + 拥有者 Actor"来定位清单，而不是直接暴露组件
	// 拥有清单的组件（变化发生在哪个清单上）
	UPROPERTY(BlueprintReadOnly, Category=Inventory)
	TObjectPtr<UActorComponent> InventoryOwner = nullptr;

	// 变化的物品实例（是哪个物品变了）
	UPROPERTY(BlueprintReadOnly, Category = Inventory)
	TObjectPtr<ULyraInventoryItemInstance> Instance = nullptr;

	// 变化后的新数量
	UPROPERTY(BlueprintReadOnly, Category=Inventory)
	int32 NewCount = 0;

	// 变化量（+1 表示多了 1 个，-2 表示少了 2 个）
	UPROPERTY(BlueprintReadOnly, Category=Inventory)
	int32 Delta = 0;
};

/** 清单里的"一格"：一个物品实例 + 它的堆叠数量 */
USTRUCT(BlueprintType)
struct FTenkaichiInventoryEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FTenkaichiInventoryEntry()
	{}

	// 调试用：打印这一格的描述文本
	FString GetDebugString() const;

private:
	// 友元：清单列表和管理器才能直接访问内部字段
	friend FLyraInventoryList;
	friend UTenkaichiInventoryManagerComponent;

	// 这个格子装的物品实例
	UPROPERTY()
	TObjectPtr<ULyraInventoryItemInstance> Instance = nullptr;

	// 堆叠数量（这个物品有几件）
	UPROPERTY()
	int32 StackCount = 0;

	// 上次看到的数量（用来算 Delta），不参与网络同步
	UPROPERTY(NotReplicated)
	int32 LastObservedCount = INDEX_NONE;
};

/** 清单：FastArraySerializer 增量同步的容器 */
USTRUCT(BlueprintType)
struct FTenkaichiInventoryList : public FFastArraySerializer
{
	GENERATED_BODY()

	// 默认构造：清单还没挂到任何组件上
	FTenkaichiInventoryList()
		: OwnerComponent(nullptr)
	{}

	// 构造：挂到持有它的组件（变化时发广播）
	FTenkaichiInventoryList(UActorComponent* InOwnerComponent)
		: OwnerComponent(InOwnerComponent)
	{}

	// 取清单里所有物品实例
	TArray<UTenkaichiInventoryItemInstance*> GetAllItems() const;

public:
	//~FFastArraySerializer 接口：网络同步时自动回调，负责广播变化
	void PreReplicateRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	void PostReplicateAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	void PostReplicateChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);
	//~结束

	// 增量序列化：只同步变化的格子
	bool NetSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FTenkaichiInventoryEntry, FTenkaichiInventoryList>(Entries, DeltaParms, *this);
	}

	// 按定义加一件物品（建实例 + 灌 Fragment 数据），返回新实例
	UTenkaichiInventoryItemInstance* AddEntry(TSubclassOf<UTenkaichiInventoryItemDefinition> ItemClass, int32 StackCount);
	// 直接加一个现成实例（当前为占位实现）
	void AddEntry(UTenkaichiInventoryItemInstance* Instance);
	// 移除一个实例
	void RemoveEntry(UTenkaichiInventoryItemInstance* Instance);

private:
	// 广播清单变化消息（给 UI / 逻辑监听）
	void BroadcastChangeMessage(FTenkaichiInventoryEntry& Entry, int32 OldCount, int32 NewCount);

private:
	// 友元：管理器才能直接访问内部清单
	friend UTenkaichiInventoryManagerComponent;

private:
	// 清单条目数组（真正存储物品的地方）
	UPROPERTY()
	TArray<FTenkaichiInventoryEntry> Entries;

	// 拥有清单的组件（变化时用来发广播），不参与网络同步
	UPROPERTY(NotReplicated)
	TObjectPtr<UActorComponent> OwnerComponent;
};

// 模板特化：告诉引擎这个结构体用自定义增量序列化（必须显式声明）
template<>
struct TStructOpsTypeTraits<FTenkaichiInventoryList> : public TStructOpsTypeTraitsBase2<FTenkaichiInventoryList>
{
	enum { WithNetDeltaSerializer = true };
};

//（此处原文件有若干空行）

/**
 * 清单管理器：挂在 Actor 上，拥有并操作整个清单
 */
UCLASS(MinimalAPI, BlueprintType)
class UTenkaichiInventoryManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// 构造函数（ObjectInitializer 有默认值，可无参构造）
	UTenkaichiInventoryManagerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// 能否加物品（目前恒 true，留扩展位）
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Inventory)
	bool CanAddItemDefinition(TSubclassOf<UTenkaichiInventoryItemDefinition> ItemDef, int32 StackCount = 1);

	// 按定义加物品，返回新实例
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Inventory)
	UTenkaichiInventoryItemInstance* AddItemDefinition(TSubclassOf<UTenkaichiInventoryItemDefinition> ItemDef, int32 StackCount = 1);

	// 直接加一个现成实例
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Inventory)
	void AddItemInstance(UTenkaichiInventoryItemInstance* ItemInstance);

	// 移除一个实例
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Inventory)
	void RemoveItemInstance(UTenkaichiInventoryItemInstance* ItemInstance);

	// 取所有物品（BlueprintCallable，可在蓝图调用）
	UFUNCTION(BlueprintCallable, Category=Inventory, BlueprintPure=false)
	TArray<UTenkaichiInventoryItemInstance*> GetAllItems() const;

	// 按定义找第一个匹配的物品实例
	UFUNCTION(BlueprintCallable, Category=Inventory, BlueprintPure)
	UTenkaichiInventoryItemInstance* FindFirstItemInstanceByDefinition(TSubclassOf<UTenkaichiInventoryItemDefinition> ItemDef) const;

	// 统计某定义物品的总数
	int32 GetTotalItemCountByDefinition(TSubclassOf<UTenkaichiInventoryItemDefinition> ItemDef) const;

	// 消耗指定 N 个物品（删除）
	bool ConsumeItemsByDefinition(TSubclassOf<UTenkaichiInventoryItemDefinition> ItemDef, int32 NumToConsume);

	//~UObject 接口：网络子对象复制（把实例复制给客户端）
	bool ReplicateSubobjects(UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

	//~UObject 接口：同步前注册所有已有实例
	void ReadyForReplication() override;

private:
	// 整个清单（网络同步字段）
	UPROPERTY(Replicated)
	FTenkaichiInventoryList InventoryList;
};

#undef UE_API